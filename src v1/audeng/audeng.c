# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/stat.h>
# include <sys/mman.h>
# include <mad.h>
# include <pthread.h>
# include <fcntl.h>
# include "mp3.c"

# define buffer_size 4*1024	// 4K Buffer (good for quick response times)
char *buffer;
int buffer_data_size;

FILE *song;
unsigned int song_pos = 0;	// Current position in song in bytes
unsigned int seek_pos = 0;	// Target position in song in 1/10 seconds

extern enum audio_mode audeng_mode;

typedef struct {
	void (*play) (FILE *file);
	void (*pause) ();
	void (*resume) ();
	void (*stop) ();
	void (*seek) (int posititon);
	unsigned int  (*getpos) ();
	unsigned int  (*getlen) ();
} audeng_decoder;

audeng_decoder cur_decoder;

int audeng_init();
int audeng_play (FILE *file, char type);
int audeng_pause();
int audeng_resume();
int audeng_stop();
int audeng_seek(int position);
unsigned int audeng_getpos();
unsigned int audeng_getlen();

int main (int argc, char *argv[]) {
	unsigned char command;
	unsigned char temp[2];
	unsigned short tempi;
	FILE *fifo;
	int fifo2;

	
	audeng_mode = stopped;

	buffer = malloc(buffer_size);
	//song = fopen("test.mp3", "rb");
	//audeng_play(song, 1);
	//audeng_seek(500);
	fifo = fopen("audengfifo", "rb+");
	fifo2 = open("audengfifo2", O_WRONLY);

	while (1) {
		fread(&command, 1, 1, fifo);
		//printf("command: %X\n", command);
		switch (command) {
			case 'p':
				audeng_pause();
				break;
			case 'r':
				audeng_resume();
				break;
			case 'o':
				fread(&tempi, 2, 1, fifo);
				mp3.path = (char *)malloc(tempi);
				fread(mp3.path, tempi, 1, fifo);
				printf("%s\n", filename);
				song = fopen(mp3.path, "rb");
				//free(filename);
				audeng_play(song, 1);
				break;
			case 's':
				fread(&tempi, 2, 1, fifo);
				audeng_seek(tempi);
				break;
			case 'c':
				audeng_stop();
				printf("closed");
				break;
			case 'g':
				tempi = audeng_getpos();
				//printf("Pos: %u\n", tempi);
				write(fifo2, &tempi, 2);
				tempi = audeng_getlen();
				//printf("Duration: %u\n", tempi);
				write(fifo2, &tempi, 2);
		}
	}
	free(buffer);
	return 0;
}

int audeng_init () {
	/* Nothing to be done here */
}

int audeng_play (FILE *file, char type) {
	audeng_mode = playing;
	switch (type) {
		case 0:	// WAV
			break;
		case 1:	// mp3
			audeng_mp3_init();
			mp3.play(file);
			cur_decoder.play = mp3.play;
			cur_decoder.pause = mp3.pause;
			cur_decoder.resume = mp3.resume;
			cur_decoder.stop = mp3.stop;
			cur_decoder.seek = mp3.seek;
			cur_decoder.getpos = mp3.getpos;
			cur_decoder.getlen = mp3.getlen;
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
	if (audeng_mode == playing) {
		cur_decoder.stop();
		song_pos = 0;
		fclose(song);
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
                                struct audio_dither *dither)
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
