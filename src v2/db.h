# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
# include <unistd.h>
# include <string.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/mman.h>
# include <string.h>
# include <dirent.h>
# include <fcntl.h>

/* Bits for each field to signal it's used */
# define imd_db_field_id			1
# define imd_db_field_title		2
# define imd_db_field_artist		4
# define imd_db_field_album		8
# define imd_db_field_genre		16
# define imd_db_field_path		32
# define imd_db_field_mtime		64
# define imd_db_field_atime		128
# define imd_db_field_ctime		256

/* Size of string in database */
# define imd_db_string_sz 64

/* Record flags */
# define imd_db_flag_title		1
# define imd_db_flag_artist		2
# define imd_db_flag_album		4
# define imd_db_flag_genre		8
# define imd_db_flag_filename	16
# define imd_db_flag_mtime		32
# define imd_db_flag_atime		64
# define imd_db_flag_ctime		128
# define imd_db_flag_duration	256

/* File types */
# define imd_ftype_mp3	1


/* Types**********************************************************************/

typedef struct {
	// Number of records in db
	unsigned short rec_count;
	// Size of record (imd_db_rec_t)
	unsigned short rec_sz;
	// Size of string in database
	unsigned short string_sz;
	// Lengths of indexes
	unsigned short title_count;
	unsigned short artist_count;
	unsigned short album_count;
	unsigned short genre_count;
} imd_db_info_t;

/**************************
IMD DB record
 * flags:
	0 = empty/deleted
	1 = valid data
	2 = new data (unsaved)
 * title...genre:
	pointers to strings in
	db index arrays.
 * filename: string, b/c
	filename is unique,
	hence less efficient
	with indexing.
 * mtime..duration:
	similar reasoning to
	filename.
 * Size: 256bytes
**************************/
typedef struct {
	unsigned short flags;
	unsigned short title;
	unsigned short artist;
	unsigned short album;
	unsigned short genre;
	// Should take up remainder of 256bytes:
	// 256 - (5 * short + 4 * int) = 230
	char filename[230];
	unsigned int mtime;
	unsigned int atime;
	unsigned int ctime;
	unsigned int duration;
} imd_db_rec_t;

// Independent (indexless) record (all fields int or string)
typedef struct {
	// Indicates which fields are valid
	// Bit 0 = title, 1 = artist, etc.
	unsigned short flags;
	// The actual fields
	char title[imd_db_string_sz];
	char artist[imd_db_string_sz];
	char album[imd_db_string_sz];
	char genre[imd_db_string_sz];
	char filename[230];
	unsigned int mtime;
	unsigned int atime;
	unsigned int ctime;
	unsigned int duration;
} imd_db_srec_t;

typedef struct {
	unsigned char flags; /* 0 = Empty, 1 = Valid */
	char text[imd_db_string_sz];
} imd_db_index_t;

/* Special record type for getrecs */
/* Only change is it has id instead of flags */
typedef struct {
	unsigned short id;
	unsigned short title;
	unsigned short artist;
	unsigned short album;
	unsigned short genre;
	// Should take up remainder of 256bytes:
	// 256 - (5 * short + 4 * int) = 230
	char filename[230];
	unsigned int mtime;
	unsigned int atime;
	unsigned int ctime;
	unsigned int duration;
} imd_db_rec_t2;

/* Record set */
typedef struct {
	/* Array of records */
	imd_db_rec_t2 *rec;
	/* Number of records in set */
	unsigned int count;
	/* Current record's index (useful for callback) */
	unsigned int index;
} imd_db_recset_t;

/* Global vars ***************************************************************/

/* Info about current database */
imd_db_info_t imd_db_info;

/* The actual records */
imd_db_rec_t imd_db_rec[4000];

/* Database indexes
 *  - Note: item 0 is reserved for empty fields.
************************************************/
imd_db_index_t imd_db_index_title[4000];
imd_db_index_t imd_db_index_artist[4000];
imd_db_index_t imd_db_index_album[4000];
imd_db_index_t imd_db_index_genre[4000];

/* Current database's filename */
char *imd_db_fn;
/* Current databse's file descriptor */
FILE *imd_db_fd;

/* Indicates status of current database (0 = closed, 1 = open) */
unsigned char imd_db_status = 0;

/* Records matching sql query */
imd_db_recset_t *imd_db_sqlrecs;


/* Function prototypes *******************************************************/

/* dbe.c */
void imd_db_create (char *filename);
unsigned char imd_db_open (char *filename);
void imd_db_close ();
void imd_db_commit ();
unsigned short *imd_db_select (imd_db_srec_t *record, unsigned short *count);
unsigned short imd_db_getindex (imd_db_index_t *index, unsigned short *count, 
	char *value, unsigned char create);
unsigned short imd_db_insert (imd_db_srec_t *record);
int imd_db_update (imd_db_srec_t *record, short id);
int imd_db_delete (unsigned short id);
void imd_db_cleanindex ();

/* db.c */
void imd_db_debug (const char *fmt, ...);
char *imd_db_itoa (int number);
unsigned int imd_strlen (char *string, int add);
imd_db_srec_t *imd_db_emptyrec ();
imd_db_recset_t *imd_db_getrecs (imd_db_srec_t *filter);
void imd_db_addfile (char *filename, char *dir, struct stat *info, unsigned char ftype);
void imd_db_updatefile (char *filename, char *dir, struct stat *info, unsigned char ftype);
void imd_db_cleanfiles ();
void imd_db_scanfiles (char *dir);
