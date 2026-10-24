void Intro(void) {
	// Variables
	int i;
	unsigned char far *pcx;
	char PCXPal[768],BlackPal[768];

	// Load logo
	pcx=(char far *) farcalloc(64000,1);
	if(!pcx) {
		DestroyScreen();
		puts("Error creating picture memory");
		exit(0);
	}

	if(!LoadPCX("intro.ils",pcx,PCXPal)) {
		DestroyScreen();
		farfree(pcx);
		puts("Error opening picture file.");
		exit(0);
	}

	// Set palette to black
	for(i=0;i<768;BlackPal[i++]=0);
	SetPalette(BlackPal);

	// Dump picci
	ClsPCX(pcx);
	Dump();

	// Fade intro on
	FadeIn(0.04,PCXPal);
	delay(3500);

	// Fade to black (name that group...)
	FadeOut(0.06);

	delay(300);
	Cls(0);

	// Free memory
	farfree(pcx);
}



void Shareware(void) {
	// Variables
	int i;
	unsigned char far *pcx;
	char PCXPal[768],BlackPal[768];

	// Load logo
	pcx=(char far *) farcalloc(64000,1);
	if(!pcx) {
		DestroyScreen();
		puts("Error creating picture memory");
		exit(0);
	}

	if(!LoadPCX("exit.ils",pcx,PCXPal)) {
		DestroyScreen();
		farfree(pcx);
		exit(0);
	}

	// Set palette to black
	for(i=0;i<768;BlackPal[i++]=0);
	SetPalette(BlackPal);

	// Dump picci
	ClsPCX(pcx);
	Dump();

	// Fade intro on
	FadeIn(0.04,PCXPal);
	delay(5000);

	// Fade to black (name that group...)
	FadeOut(0.06);

	delay(300);
	Cls(0);

	// Free memory
	farfree(pcx);

	// Quit
	DestroyScreen();
	puts("Thankyou for playing Xtris.");
}



char Overlap(int x,int y,char grid[16]) {
	// Variables
	char Rested=0;
	int r,c;

	// Check
	for(c=0;c<4;c++)
		for(r=0;r<4;r++)
			if(grid[(r*4)+c] && Filled[(y+r)*20+c+x+4])
				Rested=1;
	return(Rested);
}



void Spin(int x,int y,char *grid) {
	// Variables
	char ngrid[16];
	int r,c,ptr;

	// Rotate
	ptr=0;
	for(c=3;c>=0;c--)
		for(r=0;r<4;r++)
			ngrid[ptr++]=grid[(r*4)+c];

	// Shift up
	while(!(ngrid[0]+ngrid[1]+ngrid[2]+ngrid[3])) {
		for(c=4;c<16;c++)
			ngrid[c-4]=ngrid[c];
		for(c=12;c<16;c++)
			ngrid[c]=0;
	}

	// Return if a collision
	if(Overlap(x,y,ngrid))
		return;

	// Sound
	if(SBDink)
		WAVPlay(Dink);

	// Update grid
	for(c=0;c<16;c++)
		grid[c]=ngrid[c];
}



void ErasePos(int bx,int by) {
	int x,y;

	for(y=0;y<4;y++)
		for(x=0;x<4;x++)
			if(Grid[(y*4)+x])
				FillBox(left+((bx+x)*10),top+((by-1+y)*10),
						left+((bx+x)*10)+9,top+((by-1+y)*10)+9,0);
}



void DisplayBlock(int bx,int by) {
	int x,y;

	for(y=0;y<4;y++)
		for(x=0;x<4;x++)
			if(Grid[(y*4)+x])
				PutSprite(Blk,left+((bx+x)*10),top+((by+y)*10));
}



void ShowScore(void) {
	// Variables
	static int wave=0;
	char string[15],s[2];
	int y,ch;

	// Display score
	sprintf(string,"Score - %06u",score);

	wave++;
	wave%=2880;

	for(ch=13;ch>=0;ch--) {
		y=5*sin((wave>>3)+(ch*12.8));

		sprintf(s,"%c",string[ch]);

		VGAText(10+(ch*5),7+y,s,Mint+10+y);
	}
}



char AnotherGo(void) {
	// Variables
	char ch;
	int pulse;

	// Create electric effect
	VGAText(137,180,"Another Go?",SkyBlue+5);
	VGAText(139,180,"Another Go?",SkyBlue+5);
	VGAText(138,179,"Another Go?",SkyBlue+5);
	VGAText(138,181,"Another Go?",SkyBlue+5);
	VGAText(137,179,"Another Go?",SkyBlue+5);
	VGAText(137,181,"Another Go?",SkyBlue+5);
	VGAText(139,179,"Another Go?",SkyBlue+5);
	VGAText(139,181,"Another Go?",SkyBlue+5);

	// Get answer
	pulse=0;
	do {
		// Find color
		pulse++;
		pulse%=2880;

		// Display question
		VGAText(138,180,"Another Go?",SkyBlue+11+(4*sin(pulse/8)));

		// Move Snow
		DrawSnow();
		Dump();

		ch=0;
		if(bioskey(1))
			ch=toupper(bioskey(0));
	} while (ch!='Y' && ch!='N');

	return((ch=='Y')?1:0);
}



char Sure(void) {
	// Variables
	char ch;
	int pulse;
	clock_t AddTime;

	// Get time
	AddTime=clock();

	// Save old screen section bit thing
	Sprite OldScreen;

	SpriteAlloc(OldScreen,131,89,209,110);
	GrabSprite(OldScreen,vscreen,131,89,209,100);

	// Create electric effect
	VGAText(131,89,"Are you sure?",SkyBlue+5);
	VGAText(131,90,"Are you sure?",SkyBlue+5);
	VGAText(131,91,"Are you sure?",SkyBlue+5);
	VGAText(132,89,"Are you sure?",SkyBlue+5);
	VGAText(132,91,"Are you sure?",SkyBlue+5);
	VGAText(133,89,"Are you sure?",SkyBlue+5);
	VGAText(133,90,"Are you sure?",SkyBlue+5);
	VGAText(133,91,"Are you sure?",SkyBlue+5);

	// Get answer
	pulse=0;
	do {
		// Find color
		pulse++;
		pulse%=2880;

		// Display question
		VGAText(132,90,"Are you sure?",SkyBlue+11+(4*sin(pulse/8)));

		// Move Snow
		DrawSnow();
		ShowScore();
		Dump();

		ch=0;
		if(bioskey(1))
			ch=toupper(bioskey(0));
	} while (ch!='Y' && ch!='N');

	// Restore screen
	for(unsigned c=0;c<OldScreen.width*OldScreen.height;c++)
		if(OldScreen.data[c]==0)
			OldScreen.data[c]=Black;

	PutSprite(OldScreen,131,89);
	free(OldScreen.data);

	// Adjust time
	start+=(clock()-AddTime);

	return((ch=='Y')?1:0);
}



void FadeIn2(float speed,unsigned char pal[768]) {
	// Variables
	unsigned char blank[768];
	float llevel;
	int p;

	// Fade to user palette
	for(llevel=0;llevel<=1.0;llevel+=speed) {
		for(p=0;p<768;p++)
			blank[p]=pal[p]*llevel;
		SetPalette(blank);

		// Draw Snow
		DrawSnow();

		// Display Score
		ShowScore();

		Dump();
		waitvlb();
	}
}


char Allowed(char ch) {
	ch=toupper(ch);
	return(zat("ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890-=! œ$%^&*()_+,.?'",ch));
}



void LoadFame(void) {
	// Variables
	FILE *ptr;
	int i;

	// Open file
	ptr=fopen("fame.scr","rb");

	if(!ptr) {
		// Create hall of fame
		for(i=0;i<10;i++) {
			strcpy(fame[i].name,"Dean Edis");
			fame[i].scr=(10-i)*1000;
		}
	} else
		// Load hall of fame
		fread(&fame,sizeof(fame),1,ptr);

	fclose(ptr);
}



void WriteFame(void) {
	// Variables
	FILE *ptr;
	int i;

	// Open file
	ptr=fopen("fame.scr","wb");

	// Write names
	if(ptr)
		fwrite(&fame,sizeof(fame),1,ptr);

	fclose(ptr);
}



void ShowHallOfFame(void) {
	// Variables
	int i,pos,tpos,ypos,ch;
	char string[30],nname[15];

	// Clear play area
	FillBox(left,top,left+PlayBox.width-12,top+PlayBox.height-12,Black);

	// Find new score position
	tpos=0;
	while(score<fame[tpos].scr && tpos<10)
		tpos++;

	// Insert new element (if necessary)
	if(tpos<10) {
		for(pos=9;pos>tpos;pos--) {
			strcpy(fame[pos].name,fame[pos-1].name);
			fame[pos].scr=fame[pos-1].scr;
		}

		fame[tpos].scr=score;
	}

	// Copy hall onto screen
	VGAText(134,30,"Hall Of Fame",Teal+15);

	for(i=0;i<10;i++) {
		FillBox(left,50+(i*6),left+PlayBox.width-12,54+(i*6),
				(i==tpos)?Blue+5:Black);

		sprintf(string,"%06u",fame[i].scr);
		VGAText(left+PlayBox.width-38,50+(i*6),string,(i==tpos)?White:Orange+14-i);

		if(i!=tpos)
			VGAText(left+3,50+(i*6),fame[i].name,Orange+14-i);
	}

	// If not on scoreboard, quit
	if(tpos==10) {
		sprintf(string,"You scored %06u points.",score);
		VGAText(left+3,125,string,White);

		// Sound event
		if(SBBaa)
			WAVPlay(Baa);

		return;
	}

	// Sound event
	SBWoo=LoadWAV(Woo,"Woo.snd");
	if(SBWoo)
		WAVPlay(Woo);

	// Get name
	ypos=50+(tpos*6);
	sprintf(nname,"             ");
	pos=0;

	while(pos>=0) {
		// Update screen
		FillBox(left,ypos,left+PlayBox.width-39,ypos+5,Black);
		FillBox(left,ypos,left+PlayBox.width-39,ypos+4,Blue+5);

		VGAText(left+3,ypos,nname,White);

		// Get keypress
		while(!bioskey(1)) {
			DrawSnow();
			Dump();
		}

		ch=bioskey(0);

		// Add letter if necessary
		if(ch&0xff)
			switch(ch) {
				case 0x0e08 : 	if(pos>0)
									nname[--pos]=' ';
								break;
				case 0x1c0d :   if(!pos)
									strcpy(nname,"NONAME");
								pos=-1;
								break;
				default 	:	if(Allowed(ch))
									if(pos<12)
										nname[pos++]=toupper(ch);
								break;
			}
	}

	// Copy name to fame table
	strcpy(fame[tpos].name,nname);

	// Erase sound
	if(SBWoo) {
		free(Woo.Data);
		SBWoo=0;
	}
}
