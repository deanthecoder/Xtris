#include <mem.h>

struct {
	char sID[3];
	int width,height;
	char trans;
	char dID[4];
} SPR;



struct Sprite {
	unsigned char *data;
	int width,height;
	unsigned char trans;

	unsigned char pal[768];
};



void SpriteAlloc(Sprite &Dest,int x1,int y1,int x2,int y2) {
	// Allocate sprite memory
//	Dest.data=new char[(x2-x1+1)*(y2-y1+1)];
	Dest.data=(unsigned char *) malloc((x2-x1+1)*(y2-y1+1));

	if(!Dest.data) {
		DestroyScreen();
		puts("Error allocating sprite memory");
		exit(0);
	}
}



char GrabSprite(Sprite &Dest,unsigned char far *Src,
				int x1,int y1,int x2,int y2) {
	// Check regions
	if(x1<0 || x1>=320 || y1<0 || y1>=200)
		return(0);
	if(x2<x1 || y2<y1)
		return(0);

	if(!Dest.data)
		return(0);

	// Get sprite info
	Dest.width=(x2-x1+1);
	Dest.height=(y2-y1+1);
	Dest.trans=0;

	// Get sprite data
	unsigned int c=0;
	char ch;
	for(int y=y1;y<=y2;y++)
		for(int x=x1;x<=x2;x++) {
			ch=Src[(y*320)+x];
			Dest.data[c++]=ch;
		}

	// Get palette info
	c=0;
	for(int p=0;p<256;p++) {
		outp(0x3c7,p&0xff);
		Dest.pal[c++]=inportb(0x3c9);
		Dest.pal[c++]=inportb(0x3c9);
		Dest.pal[c++]=inportb(0x3c9);
	}

	return(1);
}



void PutSprite(Sprite Image,int l,int t) {
	// Check regions
	if(l<0 || t<0 || l>319 || t>199)
		return;

	// Get pointers
	unsigned int ssg=FP_SEG(Image.data),sof=FP_OFF(Image.data),
				 dsg=FP_SEG(vscreen),dof=FP_OFF(vscreen)+(t*320)+l,
				 wdth,h,w;
	unsigned char c1=Image.trans;

	// Get regions
	h=min(Image.height,200-t);
	w=min(Image.width,320-l);
	wdth=Image.width;

	// Main bit
	asm push ds
	for(int y=0;y<h;y++) {
		asm {
			// Set Dest
			mov ax,dsg
			mov es,ax
			mov di,dof

			// Set source
			mov ax,ssg
			mov ds,ax
			mov si,sof

			// Loop
			mov cx,w
		}

		loop:
		asm {
			// Read byte into al
			lodsb

			// Compare with transparant
			cmp al,c1

			je Missed

			// Copy byte
			stosb
			jmp Endbit
		}

		// Miss byte
		Missed:
		asm inc di

		Endbit:
		asm loop loop

		dof+=320;
		sof+=wdth;
	}
	asm pop ds
}



void WriteSprite(Sprite Image,char *filename) {
	// Variables
	unsigned length;
	FILE *ptr;

	// Open file
	ptr=fopen(filename,"wb");

	if(!ptr) {
		DestroyScreen();
		puts("Error creating sprite file.");
		exit(0);
	}

	// Load sprite info into header structure
	SPR.sID[0]='S';
	SPR.sID[1]='P';
	SPR.sID[2]='R';
	SPR.width=Image.width;
	SPR.height=Image.height;
	SPR.trans=Image.trans;
	SPR.dID[0]='D';
	SPR.dID[1]='A';
	SPR.dID[2]='T';
	SPR.dID[3]='A';

	// Write header
	fwrite(&SPR,sizeof(SPR),1,ptr);

	// Write palette
	fwrite(&Image.pal,768,1,ptr);

	// Write sprite info
	length=Image.height*Image.width;
	fwrite(Image.data,length,1,ptr);

	// Close file
	fclose(ptr);
}



void ReadSprite(Sprite &Image,char *filename) {
	// Variables
	unsigned length;
	FILE *ptr;

	// Open file
	ptr=fopen(filename,"rb");

	if(!ptr) {
		DestroyScreen();
		puts("Error opening sprite file.");
		exit(0);
	}

	// Load header
	fread(&SPR,sizeof(SPR),1,ptr);

	if(memcmp(SPR.sID,"SPR",3) || memcmp(SPR.dID,"DATA",4)) {
		DestroyScreen();
		puts("Error - Not a sprite file.");
		exit(0);
	}

	// Read palette
	fread(Image.pal,768,1,ptr);

	// Allocate memory
	length=SPR.width*SPR.height;
	SpriteAlloc(Image,1,1,SPR.width,SPR.height);

	// Read sprite data
	fread(Image.data,length,1,ptr);

	// Copy info to sprite structure
	Image.width=SPR.width;
	Image.height=SPR.height;
	Image.trans=SPR.trans;


	// Close file
	fclose(ptr);
}
