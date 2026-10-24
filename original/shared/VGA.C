#include <conio.h>
#include <stdio.h>
#include <alloc.h>
#include <stdlib.h>
#include <dos.h>

#define SGN(x)		(x==0)?0:(x>0)?1:-1
#define VGA256		256

typedef unsigned char far * VGA;

VGA vscreen;
unsigned sscreen,oscreen;


unsigned max(unsigned a,unsigned b) {
	return((a>b)?a:b);
}



unsigned min(unsigned a,unsigned b) {
	return((a>b)?b:a);
}



void waitvlb(void) {
	asm mov dx,3DAh
	l1:
	asm	in al,dx
	asm	and al,08h
	asm	jnz l1
	l2:
	asm	in al,dx
	asm	and al,08h
	asm	jz  l2
}



void Cls(char color) {
	// Fill 'vs' with color
	asm {
		mov es,sscreen
		mov di,oscreen

		mov cx,32000

		mov al,color
		mov ah,color

		rep stosw
	}
}



void SetupScreen(void) {
	while(kbhit()) getch();

	/* initialize graphics mode */
	vscreen=(char far *) farcalloc(64000,1);

	if(!vscreen) {
		puts("Error allocating screen memory.");
		exit(0);
	}
	sscreen=FP_SEG(vscreen);
	oscreen=FP_OFF(vscreen);

	asm mov ax,0x0013
	asm int 0x10

	Cls(0);
}



void DestroyScreen(void) {
	farfree(vscreen);
	while(kbhit()) getch();

	asm mov ax,0x0003
	asm int 0x10
}



void Dump(void) {
	unsigned a=sscreen,b=oscreen;

	asm {
		push ds
		mov ax,0xa000
		mov es,ax
		xor di,di

		mov ax,a
		mov ds,ax
		mov si,b

		mov cx,32000
		rep movsw
		pop ds
	}
}



void ClsPCX(char far *src) {
	unsigned a=sscreen,b=oscreen;
	unsigned c=FP_SEG(src),d=FP_OFF(src);

	asm {
		push ds
		mov ax,a
		mov es,ax
		mov di,b

		mov ax,c
		mov ds,ax
		mov si,d

		mov cx,32000
		rep movsw
		pop ds
	}
}



void PutPixel(int x,int y,char color) {
	if(x<0 || x>319 || y<0 || y>199)
		return;

	asm {
		mov ax,sscreen
		mov es,ax
		mov bx,x
		mov dx,y
		mov di,bx
		mov bx,dx
		shl dx,8
		shl bx,6
		add dx,bx
		add di,dx
		add di,oscreen

		mov al,color
		stosb
		}
}



int GetPixel(int x,int y) {
	if(x<0 || x>319 || y<0 || y>199)
		return(0);

	return(vscreen[(y*320)+x]);
}



void fPutPixel(int x,int y,char color) {
	if(x<0 || x>319 || y<0 || y>199)
		return;

	y=y*320+x;

	asm {
		mov ax,0xa000
		mov es,ax
		mov al,color

		mov di,y
		mov [es:di],al
		}
}



void Line(int a,int b,int c,int d,char color) {
	int i,s,d1x,d1y,d2x,d2y,u,v,m,n;

	// Check boundries
	if((a<0 || a>320) && (b<0 || b>200)
	   && (c<0 || c>320) && (d<0 || d>200))
		return;

	u=c-a;
	v=d-b;
	d1x=SGN(u);
	d1y=SGN(v);
	d2x=SGN(u);
	d2y=0;
	m=abs(u);
	n=abs(v);
	if(!(m>n)) {
		d2x=0;
		d2y=SGN(v);
		m=abs(v);
		n=abs(u);
	}
	s=m>>1;
	for(i=0;i<=m/2;i++) {
		// Inline putpixel 1
		if(a>-1 && a<320 && b>-1 && b<200)
			asm {
				mov ax,sscreen
				mov es,ax
				mov bx,a
				mov dx,b
				mov di,bx
				mov bx,dx
				shl dx,8
				shl bx,6
				add dx,bx
				add di,dx
				add di,oscreen

				mov al,color
				stosb
			}

		// Inline putpixel 2
		if(c>-1 && c<320 && d>-1 && d<200)
			asm {
				mov ax,sscreen
				mov es,ax
				mov bx,c
				mov dx,d
				mov di,bx
				mov bx,dx
				shl dx,8
				shl bx,6
				add dx,bx
				add di,dx
				add di,oscreen

				mov al,color
				stosb
			}

		s+=n;
		if(!(s<m)) {
			s-=m;
			a+=d1x;
			b+=d1y;

			c-=d1x;
			d-=d1y;
		} else {
			a+=d2x;
			b+=d2y;

			c-=d2x;
			d-=d2y;
		}
	}
}



void FillBox(int l,int t,int r,int b,char col) {
	// Dimensions
	int width=(r-l)+1,
		height=(b-t)+1;

	// Check regions
	if((l<0 || l>319) && (t<0 || t>199))
		return;

	// Clip
	if(l<0)
		l=0;
	if(r>319)
		r=319;
	if(t<0)
		t=0;
	if(b>199)
		b=199;

	// Get addresses
	unsigned a=sscreen,o=oscreen;

	// Get initial position
	o+=(t*320)+l;

	asm {
		// Set dest address
		mov ax,a
		mov es,ax
		mov di,o
	}

	for(int y=0;y<height;y++)
		asm {
			// Write line
			mov al,col
			mov cx,width
			rep stosb

			// Add 320 to dest address
			add o,320
			mov di,o
		}
}



void VGASquarePixel(void) {
	outp(0x3c2,0xe3);
}



void GetPalette(unsigned char *pal) {
	// Get palette info
	int c=0,p;
	for(p=0;p<256;p++) {
		outp(0x3c7,p&0xff);
		pal[c++]=inportb(0x3c9);
		pal[c++]=inportb(0x3c9);
		pal[c++]=inportb(0x3c9);
	}
}



void SetPalette(unsigned char pal[768]) {
	for(int p=0;p<256;p++) {
		outp(0x3c8,p&0xff);
		outp(0x3c9,pal[(p*3)]);
		outp(0x3c9,pal[(p*3)+1]);
		outp(0x3c9,pal[(p*3)+2]);
	}
}




void FadeOut(float speed) {
	// Variables
	unsigned char pal[768],blank[768];
	float llevel;
	int p;

	// Read palette
	GetPalette(pal);

	// Fade to black
	for(llevel=1.0;llevel>=0;llevel-=speed) {
		for(p=0;p<768;p++)
			blank[p]=pal[p]*llevel;
		SetPalette(blank);
		waitvlb();
	}
}



void FadeIn(float speed,unsigned char pal[768]) {
	// Variables
	unsigned char blank[768];
	float llevel;
	int p;

	// Fade to user palette
	for(llevel=0;llevel<=1.0;llevel+=speed) {
		for(p=0;p<768;p++)
			blank[p]=pal[p]*llevel;
		SetPalette(blank);
		waitvlb();
	}
}