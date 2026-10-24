void WAVPlay(WAV Sample) {
	unsigned size,page,offset,channel=1;

	// Halt DMA
	WriteDSP(0xd0);

	size=Sample.dLen-1;

	// Set the playback frequency
	SetRate(Sample.nSamplesPerSec);

	// Set up the DMA chip
	offset=(FP_SEG(Sample.Data)<<4)+FP_OFF(Sample.Data);
	page=(FP_SEG(Sample.Data)+(FP_OFF(Sample.Data)>>4))>>12;

	outportb(0x0A,5);
	outportb(0x0C,0);
	outportb(0x0B,0x49);
	outportb(0x02,Lo(offset));
	outportb(0x02,Hi(offset));
	outportb(0x83,page);
	outportb(0x03,Lo(size));
	outportb(0x03,Hi(size));
	outportb(0x0A,1);

	WriteDSP(0x14);
	WriteDSP((unsigned char)size);   /* write length to DSP */
	WriteDSP((unsigned char)(size>>8));
}
