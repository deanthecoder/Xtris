#include <io.h>

struct WAV {
	unsigned char rID[4];			// Should be 'RIFF'
	unsigned long rLen;				// Length of data chunk
	unsigned char wID[4];			// Should be 'WAVE'
	unsigned char fID[4];			// Should be 'fmt'
	unsigned long fLen;				// Length of data in format chunk
	unsigned int wFormatTag;		// Wave format (1==8 bit)
	unsigned int nChannels;			// 1=mono 2=stereo
	unsigned int nSamplesPerSec;    // Playback frequency
	unsigned long dLen;				// Length of audio
	unsigned char *Data;
};

unsigned RESET,READ,WRITE,DATA,STATUS;

char ResetDSP(void) {
	outportb(RESET,1);
	delay(10);
	outportb(RESET,0);
	delay(10);

	if(!(inportb(DATA)&0x80==0x80) || !(inportb(READ)==0xAA))
		return(0);
	return(1);
}



char LoadWAV(WAV &Sound,char *name) {
	// Open WAV file
	FILE *ptr;

	ptr=fopen(name,"rb");

	if(!ptr)
		return(0);

	// Load WAV info
	char dID[4];

	fread(&Sound,sizeof(Sound)-5,1,ptr);
	fseek(ptr,0x32,0);
	fread(dID,4,1,ptr);
	fread(&Sound.dLen,4,1,ptr);

	// Check WAV format
	char WAVError=0;
	if(memcmp(Sound.rID,"RIFF",4))
		if(!WAVError) WAVError=1;
	if(memcmp(Sound.wID,"WAVE",4))
		if(!WAVError) WAVError=2;
	if(memcmp(Sound.fID,"fmt",3))
		if(!WAVError) WAVError=3;
	if(memcmp(dID,"data",4))
		if(!WAVError) WAVError=4;

	if(WAVError)
		return(0);

	// Check length
	if(Sound.dLen>62000)
		Sound.dLen=62000;

	// Allocate WAV audio data
	Sound.Data=(unsigned char *) calloc(Sound.dLen+10,1);

	if(!Sound.Data)
		return(0);

	// Load WAV audio data
	fread(Sound.Data,Sound.dLen,1,ptr);

	// Return
	fclose(ptr);
	return(1);
}



void SetPort(unsigned port) {
	RESET=port+0x6;
	READ=port+0xA;
	STATUS=WRITE=port+0xC;
	DATA=port+0xE;
}



int SBInit(void) {
	unsigned int address,ret;
	char *str;

	// Get address
	str=getenv("BLASTER");
	if(!str)
		return(0);
	else {
		ret=sscanf(str,"A%x",&address);
		if(ret!=1)
			ret=sscanf(str,"a%x",&address);
		if(ret!=1 || address<0x210 || address>0x260)
			return(0);
	}

	// Set address
	SetPort(address);

	// Initialise SoundBlaster
	return(ResetDSP());
}



void WriteDSP(unsigned char ch) {
	asm mov dx,STATUS;
loop:
	asm in al,dx;
	asm test al,0x80;
	asm jnz loop

	_AL=ch;
	asm mov dx,WRITE;
	asm out dx,al;
}


char Lo(unsigned num) {
	asm mov dx,num
	return(_DL);
}



char Hi(unsigned num) {
	asm mov dx,num
	return(_DH);
}



void SetRate(unsigned long rate)
{
	unsigned long val;

	if(rate<4000) return;
	val=256-1000000/rate;
	WriteDSP(0x40);
	WriteDSP((unsigned char)val);
}



void WAVPlay(WAV Sample) {
	unsigned size,page,offset;

	// Halt DMA
	WriteDSP(0xd0);

	size=Sample.dLen-1;

	// Set the playback frequency
	SetRate(Sample.nSamplesPerSec);

	// Set up the DMA chip
	offset=(FP_SEG(Sample.Data)<<4)+FP_OFF(Sample.Data);
	page=(FP_SEG(Sample.Data)+(FP_OFF(Sample.Data)>>4))>>12;

	outportb(0x0a,5);			// Mask channel
	outportb(0x0c,0);			// Clear flip/flip
	outportb(0x0b,0x49);		// Set playback mode
	outportb(2,Lo(offset));		// Set address
	outportb(2,Hi(offset));
	outportb(0x83,page);
	outportb(3,Lo(size));		// Set size
	outportb(3,Hi(size));
	outportb(0x0a,1);			// Enable channel

	WriteDSP(0x14);             			// Set 8 bit mode
	WriteDSP((unsigned char)size);   		// Write length
	WriteDSP((unsigned char)(size>>8));
}