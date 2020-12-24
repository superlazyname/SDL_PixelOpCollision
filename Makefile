all: PixelOpCollision

PixelOpCollision: PixelOpCollision.c
	gcc -o PixelOpCollision PixelOpCollision.c -lSDL2 -lSDL2_image -g

clean:
	rm PixelOpCollision