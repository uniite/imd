# include "mp3.h"

# define buffer_size 4*1024	// 4K Buffer (good for quick response times)

extern char *buffer;
extern int buffer_data_size;

struct mad_decoder audeng_mad_decoder;

extern FILE *song;
extern unsigned int song_pos;	// Current position in song in bytes
extern unsigned int seek_pos;	// Target position in song in 1/10 seconds

unsigned int sample_pos;		// Position in file in samples

struct audio_dither {
  mad_fixed_t error[3];
  mad_fixed_t random;
};

enum audio_mode {
	opened,
	stopped,
	playing,
	paused,
	seeking
};

enum audio_mode audeng_mode;

typedef struct {
	unsigned int	start;			// Start of frame in file (in bytes)
	unsigned short	size;			// Size of frame in bytes
	unsigned short 	bitrate;		// Bitrate in kbps
	unsigned short 	samplerate;		// Samplerate in hz
	unsigned short 	channelmode;	// Channel mode (2 bits)
	unsigned char 	padding;		// Padding used (1 bit)
	unsigned char 	crc;			// CRC used (1 bit)
} mp3_frame;

void audeng_mp3_init ();
static void audeng_mp3_open (FILE *file);
static void audeng_mp3_play ();
void audeng_mp3_pause ();
void audeng_mp3_resume ();
void audeng_mp3_stop ();
void audeng_mp3_close ();
void audeng_mp3_seek (int position);
unsigned int audeng_mp3_getpos ();
unsigned int audeng_mp3_getlen ();
static void *audeng_mad_play (void *arg);
static enum mad_flow audeng_mad_input (void *data, struct mad_stream *stream);
static enum mad_flow audeng_mad_output (void *data, struct mad_header const *header, struct mad_pcm *pcm);
static int audeng_mp3_bufferframes ();
static void audeng_mp3_findframe (FILE *file, int offset, mp3_frame *frame);
static int getxing ();
static void audeng_mp3_scan ();
extern inline signed long audio_linear_dither(unsigned int bits, mad_fixed_t sample, struct audio_dither *dither);

typedef struct {
	unsigned int	size;			// Size of file in bytes
	unsigned int 	start;			// Start of first valid audio frame
	unsigned int	duration;		// Duration of song in 1/10 seconds
	unsigned short 	bitrate;		// Average bitrate in kbps
	unsigned char	vbr;			// 2 = VBR with Xing, 1 = VBR, 0 = CBR
	unsigned char	xing[100];		// Xing TOC for VBR files
	unsigned short 	samplerate;		// Samplerate in hz
	enum mpeg_channelmode channelmode;	// Channel mode (2 bits)
	void (*play) (FILE *file);
	void (*pause) ();
	void (*resume) ();
	void (*stop) ();
	void (*seek) (int position);
	unsigned int (*getpos) ();
	unsigned int (*getlen) ();
	unsigned char	*path;			// Full path of file ex. /root/test.mp3
} audeng_mp3;

audeng_mp3 mp3;
pthread_t audeng_mp3_thread;

void audeng_mp3_init () {
	mp3.play = audeng_mp3_play;
	mp3.pause = audeng_mp3_pause;
	mp3.resume = audeng_mp3_resume;
	mp3.stop = audeng_mp3_stop;
	mp3.seek = audeng_mp3_seek;
	mp3.getpos = audeng_mp3_getpos;
	mp3.getlen = audeng_mp3_getlen;
}

void audeng_mp3_play (FILE *file) {
	audeng_mp3_scan();
	audeng_mode = playing;
	// Initiliaze decoder (struct)
	mad_decoder_init(&audeng_mad_decoder, &buffer, audeng_mad_input, 0, 0, audeng_mad_output, 0, 0);
	// Launch decoder in separate thread
	pthread_create(&audeng_mp3_thread, NULL, audeng_mad_play, (void *)file);
}

void audeng_mp3_pause () {
	audeng_mode = paused;
}

void audeng_mp3_resume () {
	audeng_mode = playing;
}

void audeng_mp3_stop () {
	song_pos = seek_pos = sample_pos = 0;
	audeng_mode = stopped;
	// *Does anything else needs to be cleared/reset?
	//	such as...threads, structs
}

// Seek to position in 1/10 seconds (ex. position of 1000 is 1:40 or 100 seconds)
void audeng_mp3_seek (int position) {
	if (mp3.vbr < 2) {	// No Xing TOC, so just seek CBR style
		// Just get position as percentage of file duration and multiply by size of file in bytes
		seek_pos = (position * 100 / mp3.duration) * mp3.size / 100;
	} else {
		// Make a better educated guess with Xing TOC
		// Similar to CBR but we look up the percentage in xing which is per 255, and apply that to the file size
		seek_pos = (mp3.size * 100 / 255) * mp3.xing[position * 100  / mp3.duration] / 100;
	}
	sample_pos = position * mp3.samplerate / 10;
	while (audeng_mode == paused) {}
	audeng_mode = seeking;
}

unsigned int audeng_mp3_getpos () {
	return sample_pos * 10 / mp3.samplerate;
}

unsigned int audeng_mp3_getlen () {
	return mp3.duration;
}

void *audeng_mad_play (void *arg) {
	FILE *file = arg;
	int result;
	
	// Run the decoder
	result = mad_decoder_run(&audeng_mad_decoder, MAD_DECODER_MODE_SYNC);
	// Clean up
	mad_decoder_finish(&audeng_mad_decoder);
}

static enum mad_flow audeng_mad_input (void *data, struct mad_stream *stream) {
	switch (audeng_mode) {
		case seeking:
			song_pos = seek_pos;
			//printf("New pos: %u\n", song_pos);
			audeng_mode = playing;
		case playing:
			audeng_mp3_bufferframes();
			mad_stream_buffer(stream, buffer, buffer_data_size);
			return MAD_FLOW_CONTINUE;
		case stopped:
			return MAD_FLOW_STOP;
	}
}

static enum mad_flow audeng_mad_output (void *data, struct mad_header const *header, struct mad_pcm *pcm) {
	unsigned int nsamples = pcm->length;
	mad_fixed_t const *left_ch, *right_ch;
	static struct audio_dither dither;

	left_ch = pcm->samples[0];
	right_ch = pcm->samples[1];
	sample_pos += 1152;
	
	//return MAD_FLOW_CONTINUE;	
	
	while (nsamples--) {
		signed int sample;
		sample = audio_linear_dither(16, *left_ch++, &dither);
		putchar(sample);
		putchar(sample >> 8);
		//putchar(sample);
		if (pcm->channels == 2) {
			sample = audio_linear_dither(16, *right_ch++, &dither);
			putchar(sample);
			putchar(sample >> 8);
			//putchar(sample);
		} else {
			sample = audio_linear_dither(16, *left_ch++, &dither);
			putchar(sample);
			putchar(sample >> 8);
			//putchar(sample);
		}
	}

	return MAD_FLOW_CONTINUE;
}

static int audeng_mp3_bufferframes () {
	mp3_frame cur_frame;
	int x;
	
	audeng_mp3_findframe(song, song_pos, &cur_frame);
	song_pos = cur_frame.start;
	x = cur_frame.size;
	while (x < buffer_size) {
		audeng_mp3_findframe(song, song_pos + x, &cur_frame);
		x += cur_frame.size;
	}
	x -= cur_frame.size;
	fseek(song, song_pos, SEEK_SET);
	fread(buffer, buffer_size, 1, song);
	song_pos += x;
	buffer_data_size = buffer_size;
	
	//printf("Pos: %u\n", song_pos);
	
	return 0;
}

static void audeng_mp3_findframe (FILE *file, int offset, mp3_frame *frame) {
	unsigned char temp[2];
	unsigned char result = 0;
	fseek(file, offset, SEEK_SET);
	while (fread(&temp, 2, 1, song)) {
		offset += 2;
		if (temp[0] == 0xFF && (temp[1] == 0xFA || temp[1] == 0xFB)) {	// Find Frame Sync
			frame->crc = (temp[1] & 1) ? 0 : 1;
			fread(&temp, 2, 1, song);
			offset += 2;
			frame->bitrate = temp[0] >> 4;
			frame->samplerate = mp3_samplerates[(temp[0] & 15) >> 2];
			frame->padding = (temp[0] & 3) >> 1;
			frame->channelmode = temp[1] >> 6;
			frame->bitrate = mp3_bitrates[frame->bitrate];
			if (frame->samplerate > 0 && frame->bitrate > 0) {	// Check for problems
				frame->size = 144 * frame->bitrate * 1000 / frame->samplerate + frame->padding;
				frame->start = offset - 4;
				fseek(song, frame->start + frame->size, SEEK_SET);
				fread(&temp, 2, 1, song);	// BUG: will not work on last frame
				if (temp[0] == 0xFF && (temp[1] == 0xFA || temp[1] == 0xFB)) {
					result = 1;	// Success
				}
			}
		} else if (temp[1] == 0xFF) {
			offset--;
		}
		fseek(song, offset, SEEK_SET);
		if (result) {	// Valid frame found so return it...
			return;
		}
	}
}

static int getxing () {
	int xing_start;
	char temp[4];
	unsigned int tempi;
	mp3_frame first_frame;
	audeng_mp3_findframe(song, 0, &first_frame);
	switch (first_frame.channelmode) {
		case Mono:
			xing_start = 17;
			break;
		default:	// All other modes (stereo/joint stereo/dual channel)
			xing_start = 32;
	}
	xing_start += first_frame.start + 4;
	fseek(song, xing_start, SEEK_SET);
	fread(&tempi, 4, 1, song);
	if (tempi == 0x676E6958) {	// Check for Xing Header
		fread(&temp, 4, 1, song);
		// Finds start of Xing TOC by adding 4 bytes for each optional field, indicated by bit 0 (Frames) and bit 1 (Bytes)
		xing_start += (temp[3] & 2) * 2 + (temp[3] & 1) * 4;
		if (temp[3] & 2) {	// Get frame count to calculate song duration
			fread(&temp, 4, 1, song);
			tempi = (temp[0] << 24) + (temp[1] << 16) + (temp[2] << 8) + temp[3];
			mp3.duration = tempi / (mp3.samplerate / 1152) * 10;
		}
		fseek(song, xing_start, SEEK_SET);
		fread(&(mp3.xing), 100, 1, song);		// Get Xing TOC
		return 1;	// Xing found...definately vbr file
	}
	return 0;	// No Xing found...treat it as a CBR file
}

// Basically retrieve file info (easy for CBR), and get Xing TOC (for VBR)
static void audeng_mp3_scan () {
	struct stat file_info;
	unsigned int avg_bitrate = 0;
	mp3_frame cur_frame;
	unsigned char i;
	unsigned char sec;
	
	stat(mp3.path, &file_info);
	mp3.size = file_info.st_size;
	audeng_mp3_findframe(song, 0, &cur_frame);
	mp3.samplerate = cur_frame.samplerate;
	mp3.channelmode = cur_frame.channelmode;
	if (getxing()) {	// Check if it's VBR
		// Skim through the file to get an average bitrate
		for (i = 1; i < 100; i ++) {
			audeng_mp3_findframe(song, mp3.size * i / 100, &cur_frame);
			avg_bitrate += cur_frame.bitrate;
		}
		mp3.bitrate = avg_bitrate / 99;
		mp3.vbr = 2;
	} else {
		mp3.bitrate = cur_frame.bitrate;
		mp3.duration = mp3.size / (mp3.bitrate * 125) * 10;
	}
	/*printf("File name: %s\n", mp3.path);
	printf("Bitrate: %dkbps %s\n", mp3.bitrate, mp3.vbr ? "VBR":"");
	printf("Filesize: %u bytes\n", file_info.st_size);*/
	sec = mp3.duration / 10 % 60;
	//printf("Duration: %u:%s%u\n", mp3.duration / 600, sec < 10 ? "0": "", sec);
}
