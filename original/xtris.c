#include <time.h>
#include <bios.h>
#include <math.h>

#include <globals.c>
#include <vga.c>
#include <sprite.c>
#include <font.c>
#include <wave.c>
#include "pcx.c"
#include "tetglobs.c"
#include "snowfunc.c"
#include "tetfuncs.c"


void main() {
	// Variables
	int bx,by,num,p,q,x,y,ch,c,a,b,r,sx,sy,oby,bonus;
	char done,Color,finished,first;
	float dif;

	// Load Hall of fame
	LoadFame();

	// Setup screen, etc
	SetupScreen();

	// Load sprites
	ReadSprite(PlayBox,"Area.spr");
	ReadSprite(Snowman,"Snowman.spr");
	ReadSprite(SnowBalls,"Balls.spr");
	SetPalette(PlayBox.pal);

	// Create shape block
	SpriteAlloc(Blk,0,0,9,9);
	GrabSprite(Blk,vscreen,0,0,9,9);

	// Create snowfield
	randomize();
	for(p=0;p<FLAKES;p++) {
		flake[p].vx=flake[p].x=random(320);
		flake[p].y=random(190);
		flake[p].off=random(10);
		flake[p].amplitude=random(1)+2;
		flake[p].speed=random(2);
	}

	// Load sounds
	if(SBInit()) {
		SBFall=LoadWAV(Fall,"Fall.snd");
		SBScore=LoadWAV(Score,"line.snd");
		SBDink=LoadWAV(Dink,"Dink.snd");
		SBBaa=LoadWAV(Baa,"Sheep.snd");
	}

	// Intro
	Intro();

	first=1;

	// Main Loop
	do {
		// Display playing screen
		Cls(0);
		top=(200-PlayBox.height)/2;
		left=(320-PlayBox.width)/2;
		PutSprite(PlayBox,left,top);
		left+=6;
		top+=6;
		CheatPos=0;

		// Build snowman and snowballs
		PutSprite(SnowBalls,20,200-SnowBalls.height);
		PutSprite(Snowman,235,200-Snowman.height);

		// Create snow layer
		Line(0,199,319,199,White);

		if(!first) {
			Dump();
			SetPalette(PlayBox.pal);
		} else {
			first=0;
			FadeIn2(0.04,PlayBox.pal);
		}

		// Clear game
		c=0;
		for(p=0;p<18;p++) {
			for(q=0;q<4;q++)
				Filled[c++]=1;
			for(q=4;q<16;q++)
				Filled[c++]=0;
			for(q=16;q<20;q++)
				Filled[c++]=1;
		}
		for(p=0;p<20;p++)
			Filled[c++]=1;
		for(p=0;p<216;p++)
			Filled2[p]=0;

		score=0;
		dif=1;

		// Loop
		do {
			// Select a new block and set initial position
			num=random(BLOCKS);

			for(p=0;p<16;p++)
				Grid[p]=Block[num][p];

			Color=BlkCol[num];

			for(p=0;p<100;p++)
				Blk.data[p]=Color+mask[p];

			bx=4;
			done=finished=Overlap(bx,0,Grid);

			// Increase difficultly a tad
			dif+=0.004;

			// Drop block
			oby=by=0;
			start=clock();

			while (!done) {
				// Apply 'gravity'
				oby=dif*((clock()-start)/CLK_TCK);

				if(oby!=by) {
					done=Overlap(bx,oby,Grid);
					if(!done) {
						by=oby;

						// Erase old position
						ErasePos(bx,by);
					} else
						score+=10;
				}

				// Allow for key control
				if(bioskey(1) && !done) {
					ch=bioskey(0);
					switch(ch) {
						// Up
						case 0x4800 :   ErasePos(bx,by+1);
										Spin(bx,by,Grid);
										break;
						// Left
						case 0x4b00 :	if(!Overlap(bx-1,by,Grid))
											ErasePos(bx--,by+1);
										break;
						// Right
						case 0x4d00 :	if(!Overlap(bx+1,by,Grid))
											ErasePos(bx++,by+1);
										break;
						// Down
						case 0x5000 :   // Play sample
										if(SBFall)
											WAVPlay(Fall);

										while(!Overlap(bx,by+1,Grid)) {
											// Update block and apply gravity
											ErasePos(bx,++by);
											DisplayBlock(bx,by);

											// Move Snow
											DrawSnow();

											// Update score display
											score+=2;
											ShowScore();
											Dump();
											waitvlb();
										}

										done=1;
										break;
						// Escape
						case 0x011b :   if(Sure()) {
											finished=1;
											done=1;
										}
										break;
						// Cheat?
						default 	:	if(isalpha(ch) && CheatPos>-1) {
											if((ch&0xff)==Cheat[11-CheatPos])
												CheatPos++;
											else
												CheatPos=0;

											if(CheatPos==12) {
												score+=20000;
												CheatPos=-1;

												if(SBBaa)
													WAVPlay(Baa);
											}
										}
										break;
					}
				}

				// Move Snow
				DrawSnow();

				// Display block
				if(!done)
					DisplayBlock(bx,by);

				// Update score display
				ShowScore();
				Dump();
				waitvlb();
			}

			// Position shape permanently
			if(!finished) {
				for(y=0;y<4;y++)
					for(x=0;x<4;x++)
						if(Grid[(y*4)+x]) {
							PutSprite(Blk,left+((bx+x)*10),top+((by+y)*10));
							Filled[((y+by)*20)+x+bx+4]=1;
							Filled2[((y+by)*12)+x+bx]=Color;
						}
			}

			// Check for solid lines
			r=18;
			bonus=250;
			while (--r>0 && !finished) {
				// Get score for line
				num=0;
				for(c=0;c<12;c++)
					num+=(Filled2[(r*12)+c])?1:0;

				// If full, erase it
				if(num==12) {
					// Play sample
					if(SBScore)
						WAVPlay(Score);

					// Erase in memory
					for(a=r;a>0;a--)
						for(b=0;b<12;b++) {
							Filled[(a*20)+b+4]=Filled[(a*20)-20+b+4];
							Filled2[(a*12)+b]=Filled2[(a*12)+b-12];
						}
					for(b=0;b<12;b++) {
						Filled[b+4]=0;
						Filled2[b]=0;
					}

					// Erease line from screen
					for(a=top+(r*10)-1;a>=top;a--)
						for(b=left;b<left+121;b++)
							PutPixel(b,a+10,GetPixel(b,a));

					FillBox(left,top,left+120,top+10,Black);
					r=18;

					// Increase score
					score+=bonus;
					bonus*=2;
					dif+=0.1;
				}
			}
		} while (!finished);

		// Display hall of fame
		ShowHallOfFame();

	} while(AnotherGo());

	// Write hall of fame
	WriteFame();

	// ShutDown
	if(Fall.Data) free(Fall.Data);
	if(PlayBox.data) free(PlayBox.data);
	if(Blk.data) free(Blk.data);
	if(Snowman.data) free(Snowman.data);
	if(SnowBalls.data) free(SnowBalls.data);

	// Quit
	Shareware();
}