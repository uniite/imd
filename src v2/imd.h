# ifdef debug_all
# 	define debug_imd 1
# 	define debug_oled 1
# 	define debug_gui 1
# 	define debug_db 1
# 	define debug_audeng 1
# 	define debug_id3 1
# else
# 	define debug_imd 1
# 	define debug_oled 0
# 	define debug_gui 0
# 	define debug_db 1
# 	define debug_audeng 0
# 	define debug_id3 0
# endif


# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <pthread.h>
# include <string.h>
# include <dirent.h>
# include <time.h>

# include "gui.c"
# include "db.c"

# ifdef imd_debug_http
# 	include "http.c"
# endif


/* Types *********************************************************************/

/* Data for musicnav menu item */
typedef struct {
	/* Relates to field arg in imd_scr_musicnav */
	unsigned int field;
	/* Query the menu item is based on */
	imd_db_srec_t *query;
} imd_musicnav_itemdata_t; 

/* Playlist */
typedef struct {
	/* Name of the playlist */
	unsigned char name[255];
	/* Number of ids in playlist */
	unsigned int len;
	/* Array of pointers to song ids */
	unsigned short *ids;
} imd_playlist_t;


/* Global vars ***************************************************************/
 
/* Home screem */
gui_screen_t *imd_scr_home;
/* Options screen */
gui_screen_t *imd_scr_options;
/* Playback ("Now Playing") screen */
gui_screen_t *imd_scr_playback;

/* History */
unsigned int imd_history_pos; /* Current position in history */
gui_screen_t *imd_history_items[50]; /* Array of pointers to screens */

/* Current screen */
gui_screen_t *imd_scr_current;

/* Current song id */
unsigned short imd_playback_cursong;

/* Current playlist */
imd_playlist_t *imd_playback_curplaylist;
/* Current position in current playlist */
unsigned int imd_playback_playlistpos;

/* Playback thread */
pthread_t imd_scr_playback_thread;
/* Set to 1 to stop the thread */
unsigned int imd_scr_playback_stopthread = 0;
/* Set to n > 0 to pause the thread for n cycles */ 
unsigned int imd_scr_playback_pausethread = 0;

# ifdef imd_debug_http
/* HTTP debug server thread */
pthread_t imd_http_thread;
# endif

/* Function prototypes *******************************************************/

/* input.c */
void imd_main ();

/* theme.c */
void imd_theme_textbox (gui_textbox_t *textbox, unsigned int style);
void imd_theme_graphic (gui_graphic_t *graphic, unsigned int style);
void imd_theme_menu (gui_menu_t *menu, unsigned int style);

/* imd.c */
void imd_debug (const char *fmt, ...);
void imd_init ();
void imd_scr_load (gui_screen_t *screen, int save);
void imd_scr_back ();
void imd_scr_event (unsigned int type);
void imd_scr_home_init ();
void imd_scr_options_init ();
void imd_scr_playback_init ();
void imd_scr_playback_update ();
void imd_scr_playback_thread_start ();
void imd_scr_playback_thread_stop ();
void *imd_scr_playback_progress ();
void imd_scr_playback_event (unsigned int type);
void imd_scr_playback_show ();
void imd_playback_play ();
void imd_playback_prev ();
void imd_playback_next ();
void imd_playback_vol ();
void imd_scr_home_event (unsigned int type);
void imd_scr_options_event (unsigned int type);
void imd_scr_musicnav_event (unsigned int type);
void imd_scr_musicnav_search_event (unsigned int type);
void imd_scr_musicnav_empty (gui_screen_t *screen);
gui_screen_t *imd_scr_musicnav (unsigned int field, imd_db_srec_t *query);
void imd_scr_musicnav_search (gui_screen_t *screen);
int imd_strifind (char *a, char *b);
int imd_stricmp (const void *a, const void *b);
int imd_icmp (int a, int b);
int imd_menu_itemcmp (const void *a, const void *b);
int imd_rec_titlecmp (const void *a, const void *b);
int imd_rec_artistcmp (const void *a, const void *b);
int imd_rec_albumcmp (const void *a, const void *b);
int imd_rec_genrecmp (const void *a, const void *b);
int imd_rec_filenamecmp (const void *a, const void *b);
int imd_rec_mtimecmp (const void *a, const void *b);
int imd_rec_atimecmp (const void *a, const void *b);
int imd_rec_ctimecmp (const void *a, const void *b);
