#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

int GetPCXRow(FILE *Infp,unsigned char far *Image, int BytesPerPixel,
		int BytesPerRow, int ImWd) {
	// Variables
	int i,b,x,p,RunLen;

	// Decrunch
	for(p=0;p<BytesPerPixel;p++) {
		x=0;
		while(x<BytesPerRow) {                  /* get whole image row  */
			if(EOF==(b=getc(Infp)))
				return(0);

			if(b>0xC0) {
				RunLen=b&0x3F;
				if(EOF==(b=getc(Infp)))   			/* get the pixel value  */
					return 0;
			} else
				RunLen=1;                		/* a single pixel value */

			if(x>=ImWd)
				break;          				/* copy image's pixel 	*/
			for(i=0;i<RunLen;i++) { 			/* copy value to image	*/
				Image[x*BytesPerPixel+p]=b;
				x++;
			}
		}
	}

	return(1);
}



char LoadPCX(char *filename,unsigned char far *dest,unsigned char pal[768]) {
	// Picture stuff
	FILE *ptr;
	if((ptr=fopen(filename,"rb"))==NULL)
		return(0);
	fseek(ptr,128,0);

	for(int p=0;p<200;p++)
		GetPCXRow(ptr,&dest[p*320],1,320,320);

	// Load palette
	fseek(ptr,-768,2);
	fread(pal,768,1,ptr);
	fclose(ptr);

	// Adjust palette to VGA
	for(p=0;p<768;p++)
		pal[p]/=4;

	return(1);
}


void LoadPalette(char *filename) {
	// Picture stuff
	FILE *ptr;
	if((ptr=fopen(filename,"rb"))==NULL) {
		puts("Error opening palette file.");
		exit(0);
	}

	// Load palette
	unsigned char pal[780];

	fseek(ptr,-768,2);
	fread(pal,768,1,ptr);
	fclose(ptr);

	for(int p=0;p<256;p++) {
		outp(0x3c8,p&0xff);
		outp(0x3c9,pal[(p*3)]/4);
		outp(0x3c9,pal[(p*3)+1]/4);
		outp(0x3c9,pal[(p*3)+2]/4);
	}
}