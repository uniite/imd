#include "audeng.h"
#include "mp3.c"
#include "id3.c"


void audeng_debug (const char *fmt, ...) {
	if (debug_audeng) {
		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
	}
}

void audeng_dummy () {
}

unsigned int audeng_dummy2 () {
	return 0;
}

int audeng_init () {
	audeng_mode = stopped;
	audeng_buffer = malloc(audeng_buff_sz);
	
	/* Set volume */
	audeng_vol = 80;
	char cmd[15];
	sprintf(cmd, "aumix -v %u", audeng_vol);
	system(cmd);
	
	/* Assign dummy functions that return 0 */
	cur_decoder.play = audeng_dummy;
	cur_decoder.pause = audeng_dummy;
	cur_decoder.resume = audeng_dummy;
	cur_decoder.stop = audeng_dummy;
	cur_decoder.seek = audeng_dummy;
	cur_decoder.getpos = audeng_dummy2;
	cur_decoder.getlen = audeng_dummy2;
	
	/* Pointer to output file (sound card or fifo) */
	audeng_output_fd = fopen("audeng_output", "w+");
	
	return 0;
}

int audeng_play (char *path, char type) {
	/* Make sure to stop any files already playing */
	if (audeng_mode == playing) {
		cur_decoder.stop();
		audeng_song_pos = 0;
		fclose(audeng_song);
	}
	audeng_mode = playing;
	audeng_song = fopen(path, "rb");
	switch (type) {
		case 0:	// WAV
			break;
		case 1:	// mp3
			audeng_mp3.path = malloc(strlen(path));
			strcpy(audeng_mp3.path, path);
			audeng_mp3_init();
			audeng_mp3_play();
			cur_decoder.play = audeng_mp3.play;
			cur_decoder.pause = audeng_mp3.pause;
			cur_decoder.resume = audeng_mp3.resume;
			cur_decoder.stop = audeng_mp3.stop;
			cur_decoder.seek = audeng_mp3.seek;
			cur_decoder.getpos = audeng_mp3.getpos;
			cur_decoder.getlen = audeng_mp3.getlen;
			break;
	}
	return 0;
}

int audeng_pause () {
	if (audeng_mode == playing) {
		cur_decoder.pause();
		return 0;
	} else {
		return 1;
	}
}

int audeng_resume () {
	if (audeng_mode == paused) {
		cur_decoder.resume();
		return 0;
	} else {
		return 1;
	}
}

int audeng_stop () {
	if (audeng_mode != stopped) {
		cur_decoder.stop();
		audeng_song_pos = 0;
		fclose(audeng_song);
		return 0;
	} else {
		return 1;
	}
}

int audeng_seek (int position) {
	switch (audeng_mode) {
		case playing:
		case paused:
		case opened:
			cur_decoder.seek(position);
			return 0;
		default:
			return 1;
	}
}

unsigned int audeng_getpos () {
	return cur_decoder.getpos();
}

unsigned int audeng_getlen () {
	return cur_decoder.getlen();
}

/* The following two routines and data audio_dither structure are from the ever-brilliant
   Rob Leslie. (from mad.c in MPG321)
*/

static inline
unsigned long prng(unsigned long state)
{
  return (state * 0x0019660dL + 0x3c6ef35fL) & 0xffffffffL;
}

inline
signed long audio_linear_dither(unsigned int bits, mad_fixed_t sample,
                                struct audeng_dither_t *dither)
{
  unsigned int scalebits;
  mad_fixed_t output, mask, random;

  enum {
    MIN = -MAD_F_ONE,
    MAX =  MAD_F_ONE - 1
  };

  /* noise shape */
  sample += dither->error[0] - dither->error[1] + dither->error[2];

  dither->error[2] = dither->error[1];
  dither->error[1] = dither->error[0] / 2;

  /* bias */
  output = sample + (1L << (MAD_F_FRACBITS + 1 - bits - 1));

  scalebits = MAD_F_FRACBITS + 1 - bits;
  mask = (1L << scalebits) - 1;

  /* dither */
  random  = prng(dither->random);
  output += (random & mask) - (dither->random & mask);

  dither->random = random;

  /* clip */
  if (output > MAX) {
    output = MAX;

    if (sample > MAX)
      sample = MAX;
  }
  else if (output < MIN) {
    output = MIN;

    if (sample < MIN)
      sample = MIN;
  }

  /* quantize */
  output &= ~mask;

  /* error feedback */
  dither->error[0] = sample - output;

  /* scale */
  return output >> scalebits;
}
