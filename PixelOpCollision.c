#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

// Types
//---------------------------------------------------------------------------------------------------

typedef struct InitSDLValues_s
{
    SDL_Window*  Window;
    SDL_Renderer* Renderer;
    
} InitSDLValues_t;

typedef struct IntVec2
{
    int X; // X coordinate or column number
    int Y; // Y coordinate or row number
} IntVec2_t;

// Constants
//---------------------------------------------------------------------------------------------------

const char* const cBoardImagePath = "Board.png";
const char* const cPlayerImagePath = "PlayerTriangle.png";

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

// change this to change the size of the window
const IntVec2_t cScreenResolution = {1024, 768};

// this is more FYI than anything
const int cFPS = 60;

// 1/60 is 0.016666666666666666
const int cFrameDuration_ms = 16;

// Canvas render positions, see Window.xcf.
const IntVec2_t cBoardRenderTopLeft = {47, 50};
const IntVec2_t cBoardAtPlayerTopLeft = {707, 64};
const IntVec2_t cPlayerTopLeft = {794, 64};
const IntVec2_t cMultiplyTopLeft = {708, 146};
const IntVec2_t cPixelReadingTopLeft = {793, 147};

// Globals
//---------------------------------------------------------------------------------------------------

// various SDL resources required for rendering.
InitSDLValues_t SDLGlobals;

SDL_Texture* BoardImage = NULL;
IntVec2_t BoardImageSize = {0};
SDL_Texture* PlayerImage = NULL;
IntVec2_t PlayerImageSize = {0};

SDL_Texture* BoardAtPlayerTexture = NULL;
SDL_Texture* MultiplyTexture = NULL;
SDL_Texture* PixelReadingTexture = NULL;

IntVec2_t MousePosition = {0,0};


// Functions
//---------------------------------------------------------------------------------------------------

IntVec2_t InquireTextureSize(SDL_Texture* const texture)
{
    int width, height;

    SDL_QueryTexture(texture, NULL, NULL, &width, &height);

    IntVec2_t size = {width, height};

    return size;
}


SDL_Texture* LoadImage(SDL_Renderer * const renderer, const char* const path)
{
    SDL_Texture *texture = NULL;

    SDL_Surface* image = IMG_Load(path);

    if (image != NULL) 
    {
        texture = SDL_CreateTextureFromSurface(renderer, image);

        SDL_FreeSurface(image);
        image = NULL;
    } 
    else 
    {
        printf("Image '%s' could not be loaded. SDL Error: %s\n", path, SDL_GetError());
    }

    return texture;

}


void DrawWholeTexture(SDL_Texture * const texture, const IntVec2_t screenCoord)
{
    const IntVec2_t textureSize = InquireTextureSize(texture);

    SDL_Rect screenRectangle;
    screenRectangle.x = screenCoord.X;
    screenRectangle.y = screenCoord.Y;
    screenRectangle.w = textureSize.X;
    screenRectangle.h = textureSize.Y;

    SDL_RenderCopy(SDLGlobals.Renderer, texture, NULL, &screenRectangle);
}




const InitSDLValues_t InitSDL(IntVec2_t windowSize_px)
{
    InitSDLValues_t sdlInitResult = {NULL, NULL};

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) 
    {
        return sdlInitResult;
    }

    // Init the window
    SDL_Window* const window = SDL_CreateWindow("Font Demo!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowSize_px.X, windowSize_px.Y, SDL_WINDOW_SHOWN);
    if (!window) 
    {
        printf("An error occured while trying to create window : %s\n", SDL_GetError());
        return sdlInitResult;
    }

    // Init the renderer
    SDL_Renderer* const renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) 
    {
        printf("An error occured while trying to create renderer : %s\n", SDL_GetError());
        return sdlInitResult;
    }

    sdlInitResult.Window = window;
    sdlInitResult.Renderer = renderer;
    return sdlInitResult;
}

// just checks for a quit signal. You can put more keypresses and mouse button handlers here if you want.
// returns 1 on quit, returns 0 otherwise
int HandleInput()
{
    SDL_Event event = {0};

    while (SDL_PollEvent(&event)) 
    {
        // E.g., from hitting the close window button
        if (event.type == SDL_QUIT) 
        {
            return 1;
        }
        else if(event.type == SDL_MOUSEMOTION)
        {
            MousePosition.X = event.motion.x;
            MousePosition.Y = event.motion.y;
        }
    }

    return 0;
}

int PointInRect(const IntVec2_t point, const IntVec2_t rectTopLeft, const IntVec2_t rectSize)
{
    // you might be wondering what the point of IntVect2 is if SDL_Point exists,
    // well, there isn't one, I just didn't know SDL already had something.
    SDL_Rect rect;
    rect.x = rectTopLeft.X;
    rect.y = rectTopLeft.Y;
    rect.w = rectSize.X;
    rect.h = rectSize.Y;

    SDL_Point sdlPoint;
    sdlPoint.x = point.X;
    sdlPoint.y = point.Y;

    SDL_bool inRect = SDL_PointInRect(&sdlPoint, &rect);

    int bInRect = (inRect == SDL_TRUE);

    //printf("Point (%d, %d) in rect [(%d, %d), (%d, %d)]: %d (b) %d\n", point.X, point.Y, rect.x, rect.y, rect.w, rect.h, inRect, bInRect);

    return bInRect;
}

void DoMultiply(const IntVec2_t playerPosition)
{
    const int mouseInBoard = PointInRect(MousePosition, cBoardRenderTopLeft, BoardImageSize);

    if(mouseInBoard)
    {
        SDL_SetRenderTarget(SDLGlobals.Renderer, MultiplyTexture);
        SDL_SetRenderDrawColor(SDLGlobals.Renderer, 0, 0, 0, 255);
        //SDL_RenderClear(SDLGlobals.Renderer);

        SDL_SetRenderTarget(SDLGlobals.Renderer, BoardAtPlayerTexture);
        SDL_SetRenderDrawColor(SDLGlobals.Renderer, 0, 0, 0, 255);
        SDL_RenderClear(SDLGlobals.Renderer);

        // read what's in the board in the player's sprite area, store it in BoardAtPlayerTexture
        {
            const IntVec2_t boardRelCoordinate = {MousePosition.X - cBoardRenderTopLeft.X, MousePosition.Y - cBoardRenderTopLeft.Y};
            SDL_SetRenderTarget(SDLGlobals.Renderer, BoardAtPlayerTexture);

            SDL_Rect boardSrcRect = {0};
            boardSrcRect.w = PlayerImageSize.X;
            boardSrcRect.h = PlayerImageSize.Y;
            boardSrcRect.x = boardRelCoordinate.X;
            boardSrcRect.y = boardRelCoordinate.Y;

            SDL_RenderCopy(SDLGlobals.Renderer, BoardImage, &boardSrcRect, NULL);
        }

        SDL_SetRenderTarget(SDLGlobals.Renderer, MultiplyTexture);
        SDL_RenderCopy(SDLGlobals.Renderer, PlayerImage, NULL, NULL);
        SDL_RenderCopy(SDLGlobals.Renderer, BoardAtPlayerTexture, NULL, NULL);



    }
    else
    {
        SDL_SetRenderTarget(SDLGlobals.Renderer, MultiplyTexture);
        SDL_SetRenderDrawColor(SDLGlobals.Renderer, 255, 0, 0, 255);
        SDL_RenderClear(SDLGlobals.Renderer);

        SDL_SetRenderTarget(SDLGlobals.Renderer, BoardAtPlayerTexture);
        SDL_SetRenderDrawColor(SDLGlobals.Renderer, 255, 0, 0, 255);
        SDL_RenderClear(SDLGlobals.Renderer);
    }


    // set the target back to the screen.
    SDL_SetRenderTarget(SDLGlobals.Renderer, NULL);
}

static int Old_HasCollided(const IntVec2_t playerPosition)
{
    DoMultiply(playerPosition);

    void* pixelReadingTexturePixels = NULL;
    int pitch = 0;

    // old method: unfortunately this isn't too good because I can't think of a way
    // to get the pixels out of a non-streaming texture.
    // any white pixel in MultiplyTexture will be considered a collision

    //SDL_SetRenderTarget(SDLGlobals.Renderer, PixelReadingTexture);
    //SDL_RenderCopy(SDLGlobals.Renderer, MultiplyTexture, NULL, NULL);

    const int lockErrorCode = SDL_LockTexture(PixelReadingTexture, NULL, &pixelReadingTexturePixels, &pitch);


    if(lockErrorCode != 0)
    {
        printf("Unable to lock multiply texture: %s\n", SDL_GetError());
        return 0;
    }

    // for some reason, SDL_RenderReadPixels requires a non-null value for the destination rectangle.
    SDL_Rect bugWorkaround_Dest = {0};
    bugWorkaround_Dest.w = PlayerImageSize.X;
    bugWorkaround_Dest.h = PlayerImageSize.Y;
    bugWorkaround_Dest.x = 0;
    bugWorkaround_Dest.y = 0;

    SDL_SetRenderTarget(SDLGlobals.Renderer, MultiplyTexture);
    // how slow is this?
    SDL_RenderReadPixels(SDLGlobals.Renderer, &bugWorkaround_Dest, SDL_PIXELFORMAT_RGBA8888, pixelReadingTexturePixels, pitch);


    const uint32_t* const pixels = (uint32_t*) pixelReadingTexturePixels;

    const int pixelCount = PlayerImageSize.X * PlayerImageSize.Y;

    int hasCollided = 0;

    int pixelIndex = 0;
    for(pixelIndex = 0; pixelIndex < pixelCount; pixelIndex++)
    {
        const uint32_t pixelValue = pixels[pixelIndex];

        if(pixelValue == 0xFFFFFFFF)
        {
            hasCollided = 1;
            break;
        }
    }

    SDL_UnlockTexture(PixelReadingTexture);

    SDL_SetRenderTarget(SDLGlobals.Renderer, NULL);

    return hasCollided;
}

int HasCollided(const IntVec2_t playerPosition)
{
    return Old_HasCollided(playerPosition);

    #if 0

    DoMultiply(playerPosition);

    void* multiplyPixels = NULL;
    int pitch = 0;

    // the docs say this method is "very slow", what is "very slow"? Too slow for a 32x32 image?
    SDL_RenderReadPixels(SDLGlobals.Renderer, NULL, 0, &multiplyPixels, &pitch);

    // old method: unfortunately this isn't too good because I can't think of a way
    // to get the pixels out of a non-streaming texture.


    // any white pixel in MultiplyTexture will be considered a collision

    SDL_SetRenderTarget(SDLGlobals.Renderer, PixelReadingTexture);
    SDL_RenderCopy(SDLGlobals.Renderer, MultiplyTexture, NULL, NULL);

    const int lockErrorCode = SDL_LockTexture(PixelReadingTexture, NULL, &multiplyPixels, &pitch);

    if(lockErrorCode != 0)
    {
        printf("Unable to lock multiply texture: %s\n", SDL_GetError());
        return 0;
    }

    const uint32_t* const pixels = (uint32_t*) multiplyPixels;

    const int pixelCount = pitch * PlayerImageSize.Y;

    int hasCollided = 0;

    int pixelIndex = 0;
    for(pixelIndex = 0; pixelIndex < pixelCount; pixelIndex++)
    {
        const uint32_t pixelValue = pixels[pixelIndex];

        if(pixelValue == 0xFFFFFFFF)
        {
            hasCollided = 1;
            break;
        }
    }



    return hasCollided;

    #endif
    
}




void Render(const int didCollide)
{
    if(didCollide)
    {
        SDL_SetRenderDrawColor(SDLGlobals.Renderer, 100, 0, 0, 255);
    }
    else
    {
        SDL_SetRenderDrawColor(SDLGlobals.Renderer, 0, 0, 100, 255);
    }

    SDL_RenderClear(SDLGlobals.Renderer);

    DrawWholeTexture(BoardImage, cBoardRenderTopLeft);

    DrawWholeTexture(BoardAtPlayerTexture, cBoardAtPlayerTopLeft);

    DrawWholeTexture(MultiplyTexture, cMultiplyTopLeft);

    DrawWholeTexture(PlayerImage, cPlayerTopLeft);

    DrawWholeTexture(PlayerImage, MousePosition);
    
    DrawWholeTexture(PixelReadingTexture, cPixelReadingTopLeft);

    SDL_RenderPresent(SDLGlobals.Renderer);


}

void FrameDelay(unsigned int targetTicks)
{
    // Block at 60 fps

    // ticks is in ms
    const unsigned int ticks = SDL_GetTicks();

    if (targetTicks < ticks) 
    {
        return;
    }

    if (targetTicks > ticks + cFrameDuration_ms) 
    {
        SDL_Delay(cFrameDuration_ms);
    } 
    else 
    {
        SDL_Delay(targetTicks - ticks);
    }
}

// convenience functions to reduce typing and typos
static SDL_Texture* AllocateTexture(const IntVec2_t size)
{
    return SDL_CreateTexture(SDLGlobals.Renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, size.X, size.Y);
}

int main()
{
    // initialization
    SDLGlobals = InitSDL(cScreenResolution);

    BoardImage = LoadImage(SDLGlobals.Renderer, cBoardImagePath);
    BoardImageSize = InquireTextureSize(BoardImage);
    PlayerImage = LoadImage(SDLGlobals.Renderer, cPlayerImagePath);
    PlayerImageSize = InquireTextureSize(PlayerImage);

    MultiplyTexture = AllocateTexture(PlayerImageSize);

    // must be STREAMING access so that we can access its pixels and check for white (collisions)
    // I think this just has to be a separate texture because there's no way to blend with a streaming texture
    PixelReadingTexture = SDL_CreateTexture(SDLGlobals.Renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, PlayerImageSize.X, PlayerImageSize.Y);

    // NOTE:
    //  This texture is used as an alpha channel for the blending, 
    //  The alpha of this texture is set to 255 iff there's something on the board to collide with in the bounds of the player sprite
    //  at the player's current position.
    BoardAtPlayerTexture = AllocateTexture(PlayerImageSize);
    SDL_SetTextureBlendMode(BoardAtPlayerTexture, SDL_BLENDMODE_MOD);

    // main loop
    unsigned int targetTicks = SDL_GetTicks() + cFrameDuration_ms;
    while(1)
    {
        IntVec2_t playerPosition = {0,0};

        int quitSignal = HandleInput();

        if(quitSignal)
        {
            break;
        }

        const int didCollide = HasCollided(playerPosition);

        Render(didCollide);
        FrameDelay(targetTicks);
        targetTicks = SDL_GetTicks() + cFrameDuration_ms;
    }

    SDL_DestroyTexture(BoardImage);
    SDL_DestroyTexture(PlayerImage);
    SDL_DestroyTexture(MultiplyTexture);
    SDL_DestroyTexture(BoardAtPlayerTexture);
    SDL_DestroyTexture(PixelReadingTexture);

    return 0;
}