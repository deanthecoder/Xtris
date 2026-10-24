#include <pcx.c>
#include <vga.c>
#include <sprite.c>
#include <stdio.h>

void main() {
	Sprite SnowBalls;

	// Load picture
	unsigned char far *pcx;
	char PCXPal[768];

	pcx=(char far *) farcalloc(64000,1);
	if(!pcx) {
		DestroyScreen();
		puts("Error creating PCX memory");
		exit(0);
	}

	if(!LoadPCX("balls.pcx",pcx,PCXPal)) {
		DestroyScreen();
		puts("Error opening PCX file.");
		exit(0);
	}

	// Set palette
	SetPCXPalette(PCXPal);

	// Grab sprite
	SpriteAlloc(SnowBalls,0,0,38,32);
	GrabSprite(SnowBalls,pcx,0,0,38,32);

	WriteSprite(SnowBalls,"Balls.spr");
}