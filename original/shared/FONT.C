#include "fonts.dat"
#include <string.h>
#include <ctype.h>


int zat(char *string,char ch)
{
	int loop=0;

	while(loop<strlen(string) && string[loop]!=ch)
		loop++;

	if(string[loop]!=ch)
		return(0);

	return(loop+1);
}

void VGAText(int x,int y,char *s,char col) {
	// Variables
	int ptr,i,c,loop,pos,tmp;
	char ch,*check="ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890-=! œ$%^&*()_+,.?'";

	check[39]='"';

	// Temp sprite store
	static Sprite Letter;

	if(!Letter.data)
		SpriteAlloc(Letter,0,0,5,5);

	// Display string
	s=strupr(s);
	for(pos=0;pos<strlen(s);pos++) {
		// Get character
		ch=s[pos];

		if(ch==' ')
			x+=5;
		else {
			tmp=zat(check,ch);
			if(!tmp)
				tmp=52;

			// Copy character information to letter sprite
			ptr=tmp-1;
			c=0;
			for(loop=0;loop<ptr;loop++)
				c+=(6*Textw[loop]);

			for(i=0;i<6*Textw[ptr];i++)
				Letter.data[i]=(VGATextInfo[c+i])?col:0;

			// Define sprite
			Letter.width=Textw[ptr];
			Letter.height=6;
			Letter.trans=0;

			// Display sprite
			PutSprite(Letter,x,y);

			x+=(Textw[ptr]+1);
		}
	}
}
