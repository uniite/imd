# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
# include <unistd.h>
# include <string.h>
# include <sys/stat.h>
# include <sys/mman.h>
# include <mad.h>
# include <pthread.h>
# include <fcntl.h>


/* Audio buffer size (smaller = less delay for pausing, etc.) */
# define audeng_buff_sz 8*1024


/* Types **********************************************************************/

/* ID3v2 Header/Footer			*
 * only difference with footer  *
 * is id="3DI" instead of "ID3" */
typedef struct {
	unsigned char id[3];
	unsigned char version[2];
	unsigned char flags;
	unsigned int size;
} audeng_id3v2_header_t;

/* ID3v2.2- Frame Header */
typedef struct {
	unsigned char type[3];
	unsigned char size[3];
} audeng_id3v22_frame_t;

/* ID3v2.3+ Frame Header */
typedef struct {
	unsigned char type[4];
	unsigned int size;
	unsigned char flags[2];
} audeng_id3v23_frame_t;

/* ID3 tag data (simple) */
typedef struct {
	char title[64];
	char artist[64];
	char album[64];
	char genre[64];
} audeng_tag_t;

typedef struct {
	void (*play) (FILE *file);
	void (*pause) ();
	void (*resume) ();
	void (*stop) ();
	void (*seek) (int posititon);
	unsigned int  (*getpos) ();
	unsigned int  (*getlen) ();
} audeng_decoder;

enum mpeg_channelmode {
	Stereo			= 0,
	Joint_Stereo	= 1,
	Dual_Channel	= 2,
	Mono			= 3
};

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
} audeng_mp3_t;

struct audeng_dither_t {
  mad_fixed_t error[3];
  mad_fixed_t random;
};

enum audeng_mode_t audeng_mode;

typedef struct {
	unsigned int	start;			// Start of frame in file (in bytes)
	unsigned short	size;			// Size of frame in bytes
	unsigned short 	bitrate;		// Bitrate in kbps
	unsigned short 	samplerate;		// Samplerate in hz
	unsigned short 	channelmode;	// Channel mode (2 bits)
	unsigned char 	padding;		// Padding used (1 bit)
	unsigned char 	crc;			// CRC used (1 bit)
} audeng_mp3_frame;

enum audeng_mode_t {
	opened,
	stopped,
	playing,
	paused,
	seeking
};

/* Global Vars ***************************************************************/

/* ID3 genres */
const char audeng_id3_genres[149][30] = {
     "Blues", "Classic Rock", "Country", "Dance", "Disco", "Funk", "Grunge",
     "Hip-Hop", "Jazz", "Metal", "New Age", "Oldies", "Other", "Pop", "R&B",
     "Rap", "Reggae", "Rock", "Techno", "Industrial", "Alternative", "Ska",
     "Death Metal", "Pranks", "Soundtrack", "Euro-Techno", "Ambient", "Trip-Hop",
     "Vocal", "Jazz+Funk", "Fusion", "Trance", "Classical", "Instrumental",
     "Acid", "House", "Game", "Sound Clip", "Gospel", "Noise", "Alt. Rock",
     "Bass", "Soul", "Punk", "Space", "Meditative", "Instrumental Pop",
     "Instrumental Rock", "Ethnic", "Gothic", "Darkwave", "Techno-industrial",
     "Electronic", "Pop-Folk", "Eurodance", "Dream", "Southern Rock", "Comedy",
     "Cult", "Gangsta", "Top 40", "Christian Rap", "Pop/Funk", "Jungle",
     "Native American", "Cabaret", "New Wave", "Psychadelic", "Rave",
     "Showtunes", "Trailer", "Lo-Fi", "Tribal", "Acid Punk", "Acid Jazz",
     "Polka", "Retro", "Musical", "Rock & Roll", "Hard Rock", "Folk",
     "Folk/Rock", "National Folk", "Swing", "Fast-Fusion", "Bebob", "Latin",
     "Revival", "Celtic", "Bluegrass", "Avantegarde", "Gothic Rock",
     "Progressive Rock", "Psychedelic Rock", "Symphonic Rock", "Slow Rock",
     "Big Band", "Chorus", "Easy Listening", "Acoustic", "Humour", "Speech",
     "Chanson", "Opera", "Chamber Music", "Sonata", "Symphony", "Booty Bass",
     "Primus", "Porn Groove", "Satire", "Slow Jam", "Club", "Tango", "Samba",
     "Folklore", "Ballad", "Power Ballad", "Rythmic Soul", "Freestyle", "Duet",
     "Punk Rock", "Drum Solo", "A capella", "Euro-House", "Dance Hall", "Goa",
     "Drum & Bass", "Club House", "Hardcore", "Terror", "Indie", "BritPop",
     "NegerPunk", "Polsk Punk", "Beat", "Christian Gangsta", "Heavy Metal",
     "Black Metal", "Crossover", "Contemporary Christian", "Christian Rock",
	 "Merengue", "Salsa", "Thrash Metal", "Anime", "JPop", "SynthPop", "Unknown"
};

/* Lookup table for bitrates in Kbps
 *  - First dimension goes V1 L1, V1 L2, V1 L3, V2, L1, V2 L2/3
 *  - Second dimension is the bitrate code from the MPEG header
 *****************************************************************/
const short mp3_bitrates[] = {
	0, 32, 40, 48, 56, 64, 80, 96, 112,
	128, 160, 192, 224, 256, 320, -1
};

/* Lookup table for samplerates in Hz
 *  - First dimension is the version bits
 *  - Second dimension is the header bits
 *****************************************/
const unsigned short mp3_samplerates[] = {
	44100,	48000,	32000,	0
};

/* Audio buffer */
char *audeng_buffer;
int audeng_buffer_sz;

/* Current song info */
FILE *audeng_song;
unsigned int audeng_song_pos = 0; /* Current position in song in bytes */
unsigned int audeng_seek_pos = 0; /* Target position in song in 1/10 seconds */
unsigned int audeng_sample_pos;   /* Position in file in samples */

/* Current volume level */
int audeng_vol = 0;

/* Where to output decoded audio */
FILE *audeng_output_fd;

/* Current mode */
extern enum audeng_mode_t audeng_mode;

/* Current decoder */
audeng_decoder cur_decoder;

/* MP3 Decoder instance */
audeng_mp3_t audeng_mp3;

/* MP3 decoding thread */
pthread_t audeng_mp3_thread;

/* Prototypes ****************************************************************/

/* id3.c */
void audeng_id3_debug (const char *fmt, ...);
unsigned int audeng_id3_btol (unsigned int bint);
void audeng_tag_clear (audeng_tag_t *tag);
int audeng_id3_get (char *filename, audeng_tag_t *tag);
int audeng_id3v1_get (int file, audeng_tag_t *tag);
int audeng_id3v2_get (int file, audeng_tag_t *tag);
unsigned int audeng_id3_textdec (unsigned char *text, unsigned char textenc, 
	unsigned int size, unsigned char flags);

/* mp3.c */
void audeng_mp3_init ();
void audeng_mp3_play ();
void audeng_mp3_pause ();
void audeng_mp3_resume ();
void audeng_mp3_stop ();
void audeng_mp3_close ();
void audeng_mp3_seek (int position);
unsigned int audeng_mp3_getpos ();
unsigned int audeng_mp3_getlen ();


/* audeng.c */
void audeng_debug (const char *fmt, ...);
void audeng_dummy ();
unsigned int audeng_dummy2 ();
int audeng_init ();
int audeng_play (char *path, char type);
int audeng_pause ();
int audeng_resume ();
int audeng_stop ();
int audeng_seek (int position);
unsigned int audeng_getpos ();
unsigned int audeng_getlen ();
inline signed long audio_linear_dither(
	unsigned int bits, mad_fixed_t sample, struct audeng_dither_t *dither);
