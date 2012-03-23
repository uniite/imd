/********************************************
 IMD Database Engine v2
 * Created Dec 7, 2007
 *  by Jon Botelho
 * 12/8: First working version
 * 12/9: added getrecs (works better for menu)
 * 12/15: v2 created, based on v1 (imd_dbe.py)
	- Involved porting to C
	- Revised data model, since dicts are
		more of Python's forte
 * 12/16: Gave up, decided to use sqlite
 	- Fixed to 4000 records to avoid
		problems with realloc, still
		didn't work
 * 12/27: Got aggrivated with sqlite...
 * (a few hours later):
	First working build of v2
	- Revised everything...
	- Added imd_db_clean function
 * 12/28: First usable build
 
 *Memory/disk usage per record: 512bytes
	ex. 4,000 records = 2MB
	Note: additional RAM neeeded for vars
	other than the recordset and index
*********************************************/


// Create new database, overwriting any existing files
void imd_db_create (char *filename) {
	imd_db_fn = malloc(strlen(filename));
	strcpy(imd_db_fn, filename);
	imd_db_info.string_sz = imd_db_string_sz;
	imd_db_info.rec_sz = sizeof(imd_db_rec_t);
	
	imd_db_debug("Creating database.\n");
	// Opening in write mode will create a new file
	imd_db_fd = fopen(imd_db_fn, "w");
	// Write info header to file
	imd_db_debug("Writing header...\n");
	
	// Set item 0 of indexes to "" (empty string)
	char *empty = "";
	imd_db_getindex(imd_db_index_title,
		&imd_db_info.title_count, empty, 1);
	imd_db_getindex(imd_db_index_artist,
		&imd_db_info.artist_count, empty, 1);
	imd_db_getindex(imd_db_index_album,
		&imd_db_info.album_count, empty, 1);
	imd_db_getindex(imd_db_index_genre,
		&imd_db_info.genre_count, empty, 1);
	// Save changes and close database
	imd_db_commit();
	//fclose(imd_db_fd);
	imd_db_close();
}

// Open database
unsigned char imd_db_open (char *filename) {
	if (imd_db_status) {
		// If db is already open, don't do anything
		return 1;
	}
	
	// Open data file
	imd_db_fn = malloc(strlen(filename));
	strcpy(imd_db_fn, filename);
	imd_db_fd = fopen(imd_db_fn, "r+");
	
	// Load info header
	fread(&imd_db_info, 1, sizeof(imd_db_info), imd_db_fd);
	// Load all records from file into memory
	fseek(imd_db_fd, 512, SEEK_SET);
	fread(&imd_db_rec, imd_db_info.rec_sz, imd_db_info.rec_count, imd_db_fd);
	
	// Load indexes into memory
	fread(&imd_db_index_title, sizeof(imd_db_index_t), imd_db_info.title_count, imd_db_fd);
	fread(&imd_db_index_artist, sizeof(imd_db_index_t), imd_db_info.artist_count, imd_db_fd);
	fread(&imd_db_index_album, sizeof(imd_db_index_t), imd_db_info.album_count, imd_db_fd);
	fread(&imd_db_index_genre, sizeof(imd_db_index_t), imd_db_info.genre_count, imd_db_fd);
	
	fclose(imd_db_fd);
	
	imd_db_status = 1;
	return 0;
}

// Close database
void imd_db_close () {
	if (imd_db_status) {
		// Just close database file and free memory
		fclose(imd_db_fd);
	}
}

// Commit database changes to file
void imd_db_commit () {
	unsigned short i;
	
	// Open database file for read/write access
	printf("Saving database to %s\n", imd_db_fn);
	imd_db_fd = fopen(imd_db_fn, "w");
	
	// Save the database header
	fwrite(&imd_db_info, 1, sizeof(imd_db_info), imd_db_fd);
	fseek(imd_db_fd, 512, SEEK_SET);
	
	// Write all data to file
	for (i = 0; i < imd_db_info.rec_count; i++) {
		fwrite(&imd_db_rec[i], 1, imd_db_info.rec_sz, imd_db_fd);
	}
	for (i = 0; i < imd_db_info.title_count; i++) {
		fwrite(&imd_db_index_title[i], 1, sizeof(imd_db_index_t), imd_db_fd);
	}
	for (i = 0; i < imd_db_info.artist_count; i++) {
		fwrite(&imd_db_index_artist[i], 1, sizeof(imd_db_index_t), imd_db_fd);
	}
	for (i = 0; i < imd_db_info.album_count; i++) {
		fwrite(&imd_db_index_album[i], 1, sizeof(imd_db_index_t), imd_db_fd);
	}
	for (i = 0; i < imd_db_info.genre_count; i++) {
		fwrite(&imd_db_index_genre[i], 1, sizeof(imd_db_index_t), imd_db_fd);
	}
	fclose(imd_db_fd);
}

// Get ids of records that match the query
// Returns array of result ids, and sets count to number of records in result
unsigned short *imd_db_select (imd_db_srec_t *record, unsigned short *count) {
	unsigned short title, artist, album, genre, i;
	unsigned short *result;
	
	// Get indexes for query (flags tell which fields to query)
	if (record->flags & imd_db_flag_title) {
		title = imd_db_getindex(imd_db_index_title,
			&imd_db_info.title_count, record->title, 0);
	}
	if (record->flags & imd_db_flag_artist) {
		artist = imd_db_getindex(imd_db_index_artist,
			&imd_db_info.artist_count, record->artist, 0);
	}
	if (record->flags & imd_db_flag_album) {
		album = imd_db_getindex(imd_db_index_album,
			&imd_db_info.album_count, record->album, 0);
	}
	if (record->flags & imd_db_flag_genre) {
		genre = imd_db_getindex(imd_db_index_genre,
			&imd_db_info.genre_count, record->genre, 0);
	}
	
	result = malloc(imd_db_info.rec_count * 2);
	
	// Make an inverted copy of flags, useful in next step
	unsigned short flags = ~record->flags;
	
	// Go through each record, and if the flag for the field
	// wasn't set, or the field equals the query value
	// for all fields, add it to the result set
	// Note: (flags & #) = !(record->flags & #)
	//	which is why the inverted flags var works
	*count = 0;
	for (i = 0; i < imd_db_info.rec_count; i++) {
		if ((flags & 1 || imd_db_rec[i].title == title)
			&& (flags & 2 || imd_db_rec[i].artist == artist)
			&& (flags & 4 || imd_db_rec[i].album == album)
			&& (flags & 8 || imd_db_rec[i].genre == genre )
			&& (flags & 16 || !strcmp(imd_db_rec[i].filename, record->filename))
			&& (flags & 32 || imd_db_rec[i].mtime == record->mtime)
			&& (flags & 64 || imd_db_rec[i].atime == record->atime)
			&& (flags & 128 || imd_db_rec[i].ctime == record->ctime)
			&& (flags & 256 || imd_db_rec[i].duration == record->duration))
		{
			result[*count] = i;
			(*count)++;
		}
	}
	
	return result;
}

/** Looks up or adds string in index, and returns the id *****
 -Arguments:
	index	index to look up string in
	count	number of items in index
	value	string to look up
	create	0 = read only, 1 = write (add value to index)
*************************************************************/
unsigned short imd_db_getindex (imd_db_index_t *index, unsigned short *count, char *value, unsigned char create) {
	unsigned short i, id = 0, idok = 0;
	
	for (i = 0; i < *count; i++) {
		
		if (!strcmp(index[i].text, value)) {
			id = i;
			idok = 1;
			break;
		}
	}
	
	/* If title not already indexed, add it */
	if ((idok == 0) && create && (*count)) {
		/* Look for an empty index entry */
		for (i = 0; i < *count; i++) {
			if (index[i].flags == 0) {
				id = i;
				idok = 1;
				break;
			}
		}
		
		/* If no blank index entries, just make a new id */
		if (!idok) {
			id = i;
			/* Increase the index count */
			(*count)++;
		}
		if (index == (imd_db_index_t *)&imd_db_index_artist)
			printf("id (%s) = %u\n", value, id);
		
		/* Fill in the index entry */
		strcpy(index[id].text, value);
		/* Set flag to in use */
		index[id].flags = 1;
	} else if (*count == 0) {
		/* For special case where the index is empty */
		id = 0;
		(*count)++;
		strcpy(index[id].text, value);
		/* Set flag to in use */
		index[id].flags = 1;
	}
	return id;
}

// Add a new record, returns array of matching record ids
unsigned short imd_db_insert (imd_db_srec_t *record) {
	unsigned short i, id = 0, idok = 0;
	
	// Make sure record with filename doesn't already exist
	for (i = 0; i < imd_db_info.rec_count; i++) {
		if (!strcmp(imd_db_rec[i].filename, record->filename)) {
			return 0;
		}
	}
	
	// Check to see if theres an empty record than can be filled
	for (i = 0; i < imd_db_info.rec_count; i++) {
		if (imd_db_rec[id].flags == 0) {
			id = i;
			idok = 1;
			break;
		}
	}
	
	// If there are no empty records, add a new one
	if (!idok) {
		id = i;
		imd_db_info.rec_count++;
	}
	
	// Set record's indexless fields
	strncpy(imd_db_rec[id].filename, record->filename, 200);
	imd_db_rec[id].mtime = record->mtime;
	imd_db_rec[id].atime = record->atime;
	imd_db_rec[id].ctime = record->ctime;
	imd_db_rec[id].duration = record->duration;
	
	/** Indexing **/
	
	// Index title
	imd_db_rec[id].title = imd_db_getindex(imd_db_index_title,
		&imd_db_info.title_count, record->title, 1);
	
	// Index artist
	imd_db_rec[id].artist = imd_db_getindex(imd_db_index_artist,
		&imd_db_info.artist_count, record->artist, 1);
	
	// Index album
	imd_db_rec[id].album = imd_db_getindex(imd_db_index_album,
		&imd_db_info.album_count, record->album, 1);
	
	// Index genre
	imd_db_rec[id].genre = imd_db_getindex(imd_db_index_genre,
		&imd_db_info.genre_count, record->genre, 1);
	
	// Indicate that the record needs to be saved
	imd_db_rec[id].flags = 2;
	
	return id;
}

// Update an existing record
int imd_db_update (imd_db_srec_t *record, short id) {
	// Make sure the id is for a valid record
	if (id - 1 > imd_db_info.rec_count || !imd_db_rec[id].flags) {
		return 1;
	}
	
	/** Update the fields accoridng to record->flags **/
	
	// Bit 0: Title
	if (record->flags & 1) {
		// Update record's title
		imd_db_rec[id].title = imd_db_getindex(imd_db_index_title,
			&imd_db_info.title_count, record->title, 1);
	}
	
	// Bit 1: Artist
	if (record->flags & 2) {
		// Update record's artist
		imd_db_rec[id].artist = imd_db_getindex(imd_db_index_artist,
			&imd_db_info.artist_count, record->artist, 1);
	}
	
	// Bit 2: Album
	if (record->flags & 4) {
		// Update record's album
		imd_db_rec[id].album = imd_db_getindex(imd_db_index_album,
			&imd_db_info.album_count, record->album, 1);
	}
	
	// Bit 3: Genre
	if (record->flags & 8) {
		// Update record's genre
		imd_db_rec[id].genre = imd_db_getindex(imd_db_index_genre,
			&imd_db_info.genre_count, record->genre, 1);
	}
	
	// Bit 4: Filename
	if (record->flags & 16) {
		// Update record's filename
		strcpy(imd_db_rec[id].filename, record->filename);
	}
	
	// Bit 5: Modified Date
	if (record->flags & 32) {
		// Update record's mtime
		imd_db_rec[id].mtime = record->mtime;
	}
	
	// Bit 6: Accessed Date
	if (record->flags & 64) {
		// Update record's mtime
		imd_db_rec[id].atime = record->atime;
	}
	
	// Bit 7: Created Date
	if (record->flags & 128) {
		// Update record's mtime
		imd_db_rec[id].ctime = record->ctime;
	}
	
	// Bit 8: Duration
	if (record->flags & 256) {
		// Update record's mtime
		imd_db_rec[id].duration = record->duration;
	}
	
	// Indicate the record needs to be saved
	imd_db_rec[id].flags = 2;
	
	return 0;
}

// Delete a record
int imd_db_delete (unsigned short id) {
	// Make sure the id is for a valid record
	if (id - 1 > imd_db_info.rec_count || !imd_db_rec[id].flags) {
		return 1;
	}
	
	// Pretty simple: just set deletion flag on record and null the index
	imd_db_rec[id].flags = 0;
	
	// Can't delete indexes in case of other dependencies
	// Implement later in imd_db_clean, and adjust imd_db_getindex
	
	return 0;
}

/* Cleans out unused index entries */
void imd_db_cleanindex () {
	unsigned short i;
	/* Set every index entry's flags to 0 */
	for (i = 0; i < imd_db_info.title_count; i++) {
		imd_db_index_title[i].flags = 0;
	}
	for (i = 0; i < imd_db_info.artist_count; i++) {
		imd_db_index_artist[i].flags = 0;
	}
	for (i = 0; i < imd_db_info.album_count; i++) {
		imd_db_index_album[i].flags = 0;
	}
	for (i = 0; i < imd_db_info.genre_count; i++) {
		imd_db_index_genre[i].flags = 0;
	}
	
	/* Now go through all the records in the database
		and set the index's flags to 1 if it is used
		by that record. The result is all the unused
		index entries will have flags = 0, hence 
		they are marked for deletion (which is taken
		care of by getindex)
	***************************************************/
	for (i = 0; i < imd_db_info.rec_count; i++) {
		imd_db_index_title[imd_db_rec[i].title].flags = 1;
		imd_db_index_artist[imd_db_rec[i].artist].flags = 1;
		imd_db_index_album[imd_db_rec[i].album].flags = 1;
		imd_db_index_genre[imd_db_rec[i].genre].flags = 1;
	}
}
