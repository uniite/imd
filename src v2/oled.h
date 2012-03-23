# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
# include <unistd.h>
# include <sys/mman.h>

# ifndef debug_oled
# 	define debug_oled 1
# endif

# define oled_dc		42
# define oled_res	45
# define oled_cs		43

# define oled_ramwidth 130
# define oled_ramheight 130
# define oled_screenwidth 128
# define oled_screenheight 128

/* Basic 18-bit colors */
# define oled_red        63, 0, 0
# define oled_green      0, 26, 0
# define oled_lime       0, 63, 0
# define oled_blue       0, 0, 63
# define oled_black      0, 0, 0
# define oled_grey       24, 24, 24 
# define oled_white      63, 63, 63
# define oled_yellow     63, 63, 0
# define oled_ltblue     0, 0, 63
# define oled_pink       63, 0, 63
# define oled_ltblue     0, 0, 63
# define oled_orange     63, 19, 0
# define oled_purple     26, 0, 26

/* Background color */
# define oled_bg oled_black

typedef unsigned char byte;

/* Buffer so that everything can be srawn in one 'sweep' *
 * Organization: [rows][columns]                         */
unsigned char frame_buffer[oled_screenheight][oled_screenwidth][3];


/* Frame buffer for SPEED, organized as [row][col][buffer] */
extern unsigned char frame_buffer[oled_screenheight][oled_screenwidth][3];

void oled_debug (char *fmt, ...);
void oled_rgb (byte r, byte g, byte b, byte color[3]);
void oled_fill (byte x, byte y, byte x2, byte y2, byte r, byte g, byte b);
void oled_partialclear (byte x, byte y, byte x2, byte y2);
void oled_clear ();
void oled_buffer_flush(byte row, byte row2, byte col, byte col2);
void oled_flush ();
void oled_partialflush (byte col, byte row, byte col2, byte row2);
void oled_init (void);
void oled_write_c (unsigned char out_data);
void oled_write_d (unsigned char out_data);
