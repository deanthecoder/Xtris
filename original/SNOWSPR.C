#include <pcx.c>
#include <vga.c>
#include <sprite.c>
#include <stdio.h>

void main() {
	Sprite Snowman;

	// Load picture
	unsigned char far *pcx;
	char PCXPal[768];

	pcx=(char far *) farcalloc(64000,1);
	if(!pcx) {
		DestroyScreen();
		puts("Error creating PCX memory");
		exit(0);
	}

	if(!LoadPCX("snowy.pcx",pcx,PCXPal)) {
		DestroyScreen();
		puts("Error opening PCX file.");
		exit(0);
	}

	// Set palette
	SetPCXPalette(PCXPal);

	// Grab sprite
	SpriteAlloc(Snowman,0,0,58,62);
	GrabSprite(Snowman,pcx,0,0,57,62);

	WriteSprite(Snowman,"Snowman.spr");
}