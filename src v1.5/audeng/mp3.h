/*  Lookup table for bitrates in kbps							*
 * -First dimension goes V1 L1, V1 L2, V1 L3, V2, L1, V2 L2/3	*
 * -Second dimension is just botrate code from the MPEG header	*/
const short mp3_bitrates[] = {
	0, 32, 40, 48, 56, 64, 80, 96, 112,
	128, 160, 192, 224, 256, 320, -1
};

/*  Lookup table for samplerates in hz		*
 * -First dimension is the version bits		*
 * -Second dimension is the header bits		*/
 const unsigned short mp3_samplerates[] = {
	44100,	48000,	32000,	0
};

enum mpeg_channelmode {
	Stereo			= 0,
	Joint_Stereo	= 1,
	Dual_Channel	= 2,
	Mono			= 3
};

