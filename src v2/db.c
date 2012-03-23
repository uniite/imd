/***************************************************
 IMD DB v2
 * by Jon Botelho
 
 Changelog:
 2007
 * Dec 8: v1.5 (uses imd_dbe instead of buzhug)
 * Dec 17: v2 (ported from py to C using sqlite3)
 * Dec 27: Building sql query strings in C isn't
			fun at all...
 * Dec 27: v2.5 (imd_dbe v2 instead of sqlite3)
 * Dec 28: First usable build (v2.5)
 2008
 * ~New Years: Mourn death of imd.
 * Jan 26: Rejoice, it's working again!
 * Jan 29: Finish imd_db_emptyrec
***************************************************/

# include "db.h"
# include "audeng.c"
# include "dbe.c"



/* Multipurpose functions *****************************************************/

void imd_db_debug (const char *fmt, ...) {
	if (debug_db) {
		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
	}
}

/* Converts an integer to a ASCII decimal string */
char *imd_db_itoa (int number) {
	/* Plenty for a null terminated 32bit number w/o sign */
	char *buff;
	unsigned short i = 9;
	buff = malloc(11);
	buff[10] = 0;
	
	/* Might as well do it the "hard" way */
	/* Just keep dividing by 10 until it's 0*/
	while (number) {
		/* Convert last number to ASCII */
		buff[i] = 0x30 + number % 10;
		i--;
		/* Get rid of the last number */
		number /= 10;
	}
	
	return &buff[++i];
}

/* Alternate strlen for safety against segfaults
	Arguments:
	- string (self explanatory)
	- add	number to add to return value
	Returns:
	- Length of string + add
	- If string is invalid/null, return 0
***************************************************/
unsigned int imd_strlen (char *string, int add) {
	if (!string) {
		/* Return 0 for null string */
		return 0;
	} else {
		return strlen(string) + add;
	}
}

/* Database support functions *************************************************/

/* Returns a pointer to an empty record
	- Best way to intialize records safely
	- Unless made null, values are undtermined
	Returns:
	- Pointer to the new blank record
 **********************************************/
imd_db_srec_t *imd_db_emptyrec () {
	imd_db_srec_t *rec = malloc(sizeof(imd_db_srec_t));
	
	/* Just clear everything (0 == NULL == "") */	
	rec->flags = 0;
	strcpy(rec->title, "");
	strcpy(rec->artist, "");
	strcpy(rec->album, "");
	strcpy(rec->genre, "");
	strcpy(rec->filename, "");
	rec->mtime = 0;
	rec->atime = 0;
	rec->ctime = 0;
	rec->duration = 0;
	
	return rec;
}

/* Main database functions ****************************************************/

/* Get records from database
	Arguments:
	- filter	values to match in query
	Returns:
	- pointer to recordset struct (see declaration above)
 *****************************************************************/
imd_db_recset_t *imd_db_getrecs (imd_db_srec_t *filter) {
	imd_db_recset_t *recset;
	unsigned short *ids, idc, i;
	
	/* Perform the query */
	ids = imd_db_select(filter, &idc);
	
	/* Allocate memeory for the record set */
	recset = (imd_db_recset_t *)malloc(sizeof(imd_db_recset_t));
	recset->rec = (imd_db_rec_t2 *)malloc(idc * sizeof(imd_db_rec_t));
	recset->count = idc;
	/* Get the values for the records */
	for (i = 0; i < idc; i++) {
		memcpy(&recset->rec[i], &imd_db_rec[ids[i]], sizeof(imd_db_rec_t));
		recset->rec[i].id = ids[i];
	}
	return recset;
}

/* Reads file times and tags from audio file and 
   adds it to the database. (currently just mp3 files)
   - Called by scanfiles
   
	*Note: for decent WAV support we may need a "smart"
		   filename parser to extract title and artist,
		   *but it's kind of impossible to get it right
	*Note: I gotta stop commenting on my own comments
	*Serious Note: add support for ogg when we can do
				   vorbis comments?
	*Note: Need to better handle absence of tags?
*******************************************************/
void imd_db_addfile (char *filename, char *dir, struct stat *info, unsigned char ftype) {
	audeng_tag_t tag;
	char path[strlen(dir) + strlen(filename) + 2]; /* Full filename */
	
	/* Open the file for the next step */
	sprintf(path, "%s/%s", dir, filename);
	
	/* Get the appropriate tag for the file type */
	if (ftype == imd_ftype_mp3) {
			/* If we can't find an ID3 tag,  *
			 * use the filename as the title */
			if (audeng_id3_get(path, &tag) || !tag.title) {
				strncpy(tag.title, filename, strlen(filename) < 64 ? strlen(filename) : 63);
			}
	}
	
	/* Fill in the record */
	imd_db_srec_t record;
	strcpy(record.title, tag.title);
	strcpy(record.artist, tag.artist ? tag.artist : "");
	strcpy(record.album, tag.album ? tag.album : "");
	strcpy(record.genre, tag.genre ? tag.genre : "");
	strcpy(record.filename, path);
	record.mtime = info->st_mtime;
	record.atime = info->st_atime;
	record.ctime = info->st_ctime;
	
	/* Add the record to the database */
	imd_db_debug("\tAdded file as %u\n", imd_db_insert(&record));
}

/* Update a music file already in the database
	- Called by scanfiles
***********************************************/
void imd_db_updatefile (char *filename, char *dir, struct stat *info, unsigned char ftype) {
	audeng_tag_t tag;
	char path[strlen(dir) + strlen(filename) + 2]; /* Full filename */
	unsigned short *ids, idc;
	
	/* Get the file's full path */
	sprintf(path, "%s/%s", dir, filename);
	
	printf("Getting tag\n");
	/* Get the appropriate tag for the file type */
	if (ftype == imd_ftype_mp3) {
			/* If we can't find an ID3 tag,  *
			 * use the filename as the title */
			if (audeng_id3_get(path, &tag) || !tag.title) {
				strncpy(tag.title, filename,
					strlen(filename) < 64 ? strlen(filename) : 63);
			}
	}
	
	/* Fill in the record */
	imd_db_srec_t record;
	strcpy(record.title, tag.title);
	strcpy(record.artist, tag.artist ? tag.artist : "");
	strcpy(record.album, tag.album ? tag.album : "");
	strcpy(record.genre, tag.genre ? tag.genre : "");
	strcpy(record.filename, path);
	record.mtime = info->st_mtime;
	record.atime = info->st_atime;
	record.ctime = info->st_ctime;
	
	record.flags = 16; /* Filename is query condition */
	
	/* Get file's database record id */
	ids = imd_db_select(&record, &idc);
	if (!idc) {
		return; /* File not found in database */
	}
	
	/* Update all fields except filename and duration */
	record.flags = 0xEF;
	
	/* Update the database */
	imd_db_update(&record, ids[0]);
}

/* Clean out deleted files */
void imd_db_cleanfiles () {
	struct stat info; /* File info */
	unsigned int i;
	
	/* Go through every record */
	for (i = 0; i < 4000; i++) {
		if (!stat(imd_db_rec[i].filename, &info)) {
			/* If file doesn't exist, mark the record as invalid */
			imd_db_rec[i].flags = 0;
		}
	}
	
	/* Clean out the indexes */
	//imd_db_cleanindex();
}

/* Scans through dir recursively for music.
	- All new music files are added to the database.
	*Note: it is a recursive function
***************************************************/
void imd_db_scanfiles (char *dir) {
	DIR *dp; /* Dir pointer */
	struct dirent *entry; /* Dir entry */
	struct stat info; /* File info */
	imd_db_srec_t query; /* Filter for querying */
	char ftype = -1; /* File type */
	unsigned short *ids, idc; /* Results of db query */
	
	dp = opendir(dir);
	if (!dp) {
		return;
	}
	
	chdir(dir);
	while ((entry = readdir(dp))) {
		lstat(entry->d_name, &info);
		if (S_ISDIR(info.st_mode)) { /* It's a directory */
			if (!strcmp(".", entry->d_name) || !strcmp("..", entry->d_name)) {
				/* . and .. aren't real directories, so skip them */
				continue;
			}
			/* Scan the directory */
			imd_db_scanfiles(entry->d_name);
		} else { /* It's a file */
			char *ext;
			ext = strrchr(entry->d_name, '.');
			if (ext) { /* Make sure it has the a file extension */
				/* Initialize the neccesary variables */
				sprintf(query.filename, "%s/%s", dir, entry->d_name);
				printf("\tAdding file: %s\n", query.filename);
				
				/* TODO: Check if the file is already in the database? */
				/* Check modification time */
				
				if (!strcmp(ext, ".mp3")) {
					ftype = imd_ftype_mp3;
				} else if (!strcmp(ext, ".wav")) {
					/* Do something with another file type */
				}
				query.flags = 16; /* Filename is query condition */
				ids = imd_db_select(&query, &idc);
				if (idc && imd_db_rec[ids[0]].mtime != info.st_mtime) {
					/* If it has changed since the last scan, update it */
					/* Be careful: the gumstix doesn't like switches (I've gotten random segfaults) */
					imd_db_updatefile(entry->d_name, dir, &info, ftype);
				} else if (!idc) {
					/* If it's not already in the database, just add it */
					imd_db_addfile(entry->d_name, dir, &info, ftype);
				}
				//free(ids);
			}
		}
	}
}
