/********************************************************
 IMD ID3 Tagger v1
 * Created Dec 18, 2007
 * by Jon Botelho
 *TODO:
	write support
 * Dec 28:
	- Revised to use "low-level" file functions
	  (stdlib fopen, etc. caused seg faults)
	- Made tag fields a static size (64 bytes)
	  (to avoid segfaults, probably from alignment issues)
*********************************************************/

/* Static function prototypes */
static unsigned int audeng_id3_decint (unsigned int sint);
unsigned int audeng_id3_textdec (unsigned char *text, unsigned char textenc, 
	unsigned int size, unsigned char flags);
static int audeng_id3v2_parse (int file, unsigned int offset,
	audeng_id3v2_header_t *header, audeng_tag_t *tag);

void audeng_id3_debug (const char *fmt, ...) {
	if (debug_id3) {
		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
	}
}

/* Convert ID3 big endian to little endian (native) */
unsigned int audeng_id3_btol (unsigned int bint) {
	return 
	((bint & 0xFF) << 24) +		/* byte 1 -> byte 4 */
	((bint & 0xFF00) << 8) +	/* byte 2 -> byte 3 */
	((bint & 0xFF0000) >> 8) +	/* byte 3 -> byte 2 */
	((bint & 0xFF000000) >> 24);/* byte 4 -> byte 1 */
}

/* Set all the tag's field to null */
void audeng_tag_clear (audeng_tag_t *tag) {
	strcpy(tag->title, "");
	strcpy(tag->artist, "");
	strcpy(tag->album, "");
	strcpy(tag->genre, "");
}

int audeng_id3_get (char *filename, audeng_tag_t *tag) {
	audeng_tag_t *tag1, *tag2; /* ID3 v1 and v2 tags */
	int res1, res2; /* Results for each version */
	unsigned int temp;
	int file;
	
	/* Open file */
	file = open(filename, O_RDONLY);
	
	/* Allocate memory for tags */
	tag1 = malloc(sizeof(audeng_tag_t));
	tag2 = malloc(sizeof(audeng_tag_t));
	
	/* Check for both versions */
	res1 = audeng_id3v1_get(file, tag1);
	res2 = audeng_id3v2_get(file, tag2);
	
	/* Close file */
	close(file);
	
	/* Convert numeric genre id to text if neccesary	*
	 * Scanf is...(insert demeaning adjective here),	*
	 * so I had to do it the hard way					*/
	if (!res2 && tag2->genre && tag2->genre[0] == '(') {
		unsigned char *stemp;
		temp = (int)strstr(tag2->genre, ")");
		temp = temp - (int)tag2->genre;
		if (temp > 0 && temp < 5) {
			stemp = tag2->genre;
			stemp++;
			stemp[--temp] = 0;
			temp = atoi(stemp);
			if (temp < 140) {
				strcpy(tag2->genre, audeng_id3_genres[temp]);
			}
		}
	}
	
	if (!res1 && res2) { /* ID3 v1 only */
		/* Return ID3v1 tag data */
		audeng_id3_debug("ID3v1 tag (%s)\n", filename);
		memcpy(tag, tag1, sizeof(audeng_tag_t));
		return 0;
	} else if (!res2 && res1) { /* ID3 v2 only*/
		/* Return ID3v1 tag data */
		audeng_id3_debug("ID3v2 tag (%s)\n", filename);
		memcpy(tag, tag2, sizeof(audeng_tag_t));
		return 0;
	} else if (!res1 && !res2) { /* Both tags found. */
		/* Try to take the best of each by taking
		 * the v2 tag and filling in any blank 
		 * fields with v1 data
		 */
		audeng_id3_debug("ID3 v1+2 tag (%s)\n", filename);
		if (!tag2->title && tag1->title) {
			strcpy(tag2->title, tag1->title);
		}
		if (!tag2->artist && tag1->artist) {
			strcpy(tag2->artist, tag1->artist);
		}
		if (!tag2->album && tag1->album) {
			strcpy(tag2->album, tag1->album);
		}
		if (!tag2->genre && tag1->genre) {
			strcpy(tag2->genre, tag1->genre);
		}
		memcpy(tag, tag2, sizeof(audeng_tag_t));
		return 0;
	} else { /* No ID3 tags found */
		return 0;
	}
}

/* Get ID3v1 Tag */
int audeng_id3v1_get (int file, audeng_tag_t *tag) {
	unsigned char buff[31];
	unsigned int temp;
	/* Very simple; just seek to -128, and read fields in order */
	lseek(file, -128, SEEK_END);
	read(file, &buff, 3);
	if (!strncmp(buff, "TAG", 3)) { /* Make sure a tag is there */
		/* Process for reading Title, Artist, and Album:
		 *  1a. Read 30 bytes from file to get raw data
		 *  1b. Get the length of the text data
		 *  2. Allocate memory for tag text
		 *  3. Null terminate the field
		 *  4. Copy over the text
		 */
		
		/* Title */
		read(file, &buff, 30);
		temp = strlen(buff);
		buff[30] = 0;
		strcpy(tag->title, buff);
		
		/* Artist */
		read(file, &buff, 30);
		temp = strlen(buff);
		buff[30] = 0;
		strcpy(tag->artist, buff);
		
		/* Album */
		read(file, &buff, 30);
		temp = strlen(buff);
		buff[30] = 0;
		strcpy(tag->album, buff);
		
		/* Genre */
		lseek(file, -1, SEEK_END); /* Skip over year and comments */
		read(file, &buff, 1); /* Get genre byte */
		/* Allocate memory for genre text, and copy it to the tag */
		strcpy(tag->genre, audeng_id3_genres[buff[0]]);
		
		return 0; /* Success */
	} else {
		/* Otherwise, return an error */
		return 1;
	}
}

/* Get ID3v2 Tag 
 * Offset is start of tag in file
 */
int audeng_id3v2_get (int file, audeng_tag_t *tag) {
	int i;
	unsigned char buffer[512];
	unsigned char *temp;
	int offset;
	audeng_id3v2_header_t header;
	unsigned char partial; /* Number of chars partilaly matchgn ID3 */
	/* Look through first 10KB of file */
	for (i = 0; i < 10240; i += 512) {
		lseek(file, i, SEEK_SET);
		if (read(file, &buffer, 512)) {
			temp = strstr(buffer, "ID3");
			audeng_id3_debug("\tTag found @ %u\n", i + temp - (int)(&buffer));
			if (temp) {
				partial = 0;
				/* header @ i + temp - buffer */
				offset = i + (int)temp - (int)(&buffer);
				break;
			} else if (!strcmp(&buffer[510], "ID")) {
				partial = 2;
			} else if (buffer[511] == 'I') {
				partial = 1;
			} else if (!strncmp(buffer, "D3", 2) && partial == 1) {
				/* header @ i - 1 */
				offset = i - 1;
				break;
			} else if (buffer[0] == '3' && partial == 2) {
				/* header @ i - 2 */
				offset = i - 2;
				break;
			} else {
				partial = 0;
			}
		}
	}
	/* If tag found (loop exited early), parse it */
	if (i < 10240) {
		/* Get ID3 header @ offset and parse tag data */
		lseek(file, offset, SEEK_SET);
		if (!read(file, &header, 10))
			return 1;
		header.size = audeng_id3_btol(header.size);
		audeng_id3v2_parse(file, (unsigned int)offset, &header, tag);
		return 0;
	}
	/* Look through last 10KB of file */
	for (i = 0; i > -10240; i -= 512) {
		lseek(file, i, SEEK_END);
		if (read(file, &buffer, 512)) {
			temp = strstr(buffer, "3DI");
			if (temp) {
				partial = 0;
				/* footer @ i + (buffer - temp) (SEEK_END) */
				offset = i + (int)temp - (int)(&buffer);
				break;
			} else if (!strcmp(&buffer[510], "3D")) {
				partial = 2;
			} else if (buffer[511] == '3') {
				partial = 1;
			} else if (!strncmp(buffer, "DI", 2) && partial == 1) {
				/* footer @ i - 1 (SEEK_END) */
				offset = i - 1;
				break;
			} else if (buffer[0] == 'I' && partial == 2) {
				/* footer @ i - 2 (SEEK_END) */
				offset = i - 2;
				break;
			} else {
				partial = 0;
			}
		}
	}
	/* If tag found (loop exited early), parse it */
	if (i > -10240) {
		/* Read the size from the footer to find the header */
		struct stat info;
		fstat(file, &info);
		offset = info.st_size + offset;
		lseek(file, offset, SEEK_SET);
		if (!read(file, &header, 10))
			return 1;
		header.size = audeng_id3_btol(header.size);
		/* Header isn't included in size, so -10 */
		offset = offset - header.size - 10;
		/* Get ID3 header @ offset and parse tag data */
		lseek(file, offset, SEEK_SET);
		if (!read(file, &header, 10))
			return 1;
		header.size = audeng_id3_btol(header.size);
		audeng_id3v2_parse(file, (unsigned int)offset, &header, tag);
		return 0;
	}
	return 1;
}

/* Converts ID3 synchsafe integer to a regular one
 * Arguments: sint (synchsafe integer)
 * Returns: standard integer
 */
static unsigned int audeng_id3_decint (unsigned int sint) {
	return ((sint & 0x7F000000) >> 1)
			+ ((sint & 0x7F0000) >> 1)
			+ ((sint & 0x7F00) >> 1)
			+ (sint & 0x7F);
}

/* Decodes ID3 text fields.
 * Useful for decoding Unicode (IMD can't yet handle it)
 * Arguments:
 *	text - text to decode
 * 	textenc - 0=ISO-8859-1, 1=UTF-16 w/ BOM, 2=UTF-16BE w/o BOM, 3=UTF-8
 * Returns: length of string (and text becomes ASCII)
 * NOTE: when decoding UTF-16, watch out for FF 
 *       if the tag uses unsynchronization, esp for BOM
 */
unsigned int audeng_id3_textdec (
	unsigned char *text, unsigned char textenc, 
	unsigned int size, unsigned char flags
){
	int i;
	/* For ASCII encoded as UTF16, otherwise, size will stay same */
	unsigned char res[size / 2];
	unsigned int bom;
	
	switch (textenc) {
		case 0:
			return size; /* Nothing to be done */
		case 1:
			bom = (text[0] << 8) + text[1];
			if (flags & 128) { /* Undo Unsychonronization if neccesary */
				if (bom == 0xFF00) {
					bom = 0xFFFE;
					i = 3;
				} else if (bom == 0xFEFF && text[2] == 0) {
					/* This extra if is just precauntionary */
					i = 3;
				}
			} else {
				i = 2;
			}
			/* Only use it if it's ASCII compatible */
			/* Not really sure about UTF-16, but here it goes... */
			if (bom == 0xFFFE) {
				for (; i < size; i+=2) {
					if (text[i + 1] == 0) {
						res[(i- 1) / 2] = text[i];
					} else { /* Definitely not ASCII */
						return 0;
					}
				}
			} else if (bom == 0xFEFF) {
				for (; i < size; i+=2) {
					if (text[i] == 0) {
						res[(i- 1) / 2] = text[i + 1];
					} else { /* Definitely not ASCII */
						return 0;
					}
				}
			} else {
				/* Invalid BOM */
				return 0;
			}
			/* Subtract BOM */
			size = size / 2 - 1;
			/* Copy it to original string and add null */
			strncpy(text, res, size);
			text[size] = 0;
			return size;
		case 2:
			/* Only use it if it's ASCII compatible */
			/* Try UTF-16LE, since x86 is the norm... */
			for (i = 0; i < size; i+=2) {
				if (text[i + 1] == 0) {
					res[i / 2] = text[i];
				} else { /* Definitely not ASCII */
					return 0;
				}
			}
			size = size / 2;
			/* Copy it to original string and add null */
			strncpy(text, res, size);
			text[size] = 0;
			return size;
		case 3:
			/* Make sure all the UTF-8 is ASCII */
			for (i = 0; i < size; i++) {
				if (!(text[i] & 0x80)) { /* Not ASCII */
					return 0;
				}
			}
			return size;
	}
	
	/* If all else fails */
	return 0;
}

/* Parse ID3v2 tag */
static int audeng_id3v2_parse (
	int file, unsigned int offset,
	audeng_id3v2_header_t *header, audeng_tag_t *tag
) {
	unsigned int temp;
	unsigned int tsize; /* Size of text field (with null) */
	unsigned char textenc; /* 0=ISO-8859-1, 1/2=UTF-16, 3=UTF-8 */
	/* End of tag in file. Extra 10 bytes for header (footer not included) */
	unsigned int end = offset + header->size + 10;
	/* Skip over header */
	offset += 10;
	/* Note: the only major differences between versions are the frame headers */
	if (header->version[0] < 3) { /* ID3 v2.2 and earlier */
		audeng_id3v22_frame_t frame;
		while (offset < end) {
			lseek(file, offset, SEEK_SET);
			/* Read ID3 frame */
			if (!read(file, (void *)(&frame), 6))
				return 1;
			unsigned int frame_sz;
			/* Convert frame size to int */
			/* TODO add unsycnh support */
			frame_sz = frame.size[0] << 16;
			frame_sz += frame.size[1] << 8;
			frame_sz += frame.size[2];
			audeng_id3_debug("\tFrame type: %s\n", frame.type);
			if (frame.type[0] == 'T') {
					/* Get the text encoding */
					if (!read(file, &textenc, 1))
						return 1;
					/* Text field */
					unsigned char tfield[frame_sz - 1];
					tsize = frame_sz - 1;
					/* Ignore the field if it's empty */
					if (!tsize) {
						offset += 6 + frame_sz; /* Skip to next frame */
						continue;
					}
					/* Read the actual text field */
					if (!read(file, &tfield, tsize))
						return 1;
					/* Decode the text (it may be in unicode) */
					tsize = audeng_id3_textdec(tfield, textenc, tsize, header->flags);
					audeng_id3_debug("\tField value: %s\n", tfield);
					/* Need strncmp because type isn't null terminated */
					if (!strncmp(frame.type, "TT2", 3)) { /* Title */
						strncpy(tag->title, tfield, tsize < 64 ? tsize : 63);
					} else if (!strncmp(frame.type, "TP1", 3)) { /* Artist */
						strncpy(tag->artist, tfield, tsize < 64 ? tsize : 63);
					} else if (!strncmp(frame.type, "TAL", 3)) { /* Album */
						strncpy(tag->album, tfield, tsize < 64 ? tsize : 63);
					} else if (!strncmp(frame.type, "TCO", 3)) { /* Content Type (Genre) */
						strncpy(tag->genre, tfield, tsize < 64 ? tsize : 63);
					}
			}
			offset += 6 + frame_sz; /* Skip to next frame */
		}
	} else if (header->version[0] < 5) { /* ID3 v2.3 and 2.4 */
		if (header->flags & 64) { /* If bit 6 set (Extended Header) */
			if (!read(file, &temp, 4)) /* Get size of ext. header */
				return 1;
			//temp = audeng_id3_decint(temp); /* Skip over it */
			temp = audeng_id3_btol(temp);
			audeng_id3_debug("\tExtended header size: %u\n", temp);
			offset += temp + 4;
		}
		audeng_id3v23_frame_t frame;
		while (offset < end) {
			lseek(file, offset, SEEK_SET);
			/* Read ID3 frame */
			if (!read(file, &frame, 10))
				return 1;
			frame.size = audeng_id3_btol(frame.size);
			audeng_id3_debug("\tReading frame @ offset %u/%u of size %u\n", offset, end, frame.size);
			if (frame.size > header->size) {
				/* Invalid frame */
				return 1;
			}
			audeng_id3_debug("\tFrame type: %s\n", frame.type);
			if (frame.type[0] == 'T') {
					/* Get the text encoding */
					if (!read(file, &textenc, 1))
						return 1;
					/* Text field */
					tsize = frame.size - 1;
					/* Ignore the field if it's empty */
					if (!tsize) {
						offset += 10 + frame.size; /* Skip to next frame */
						continue;
					}
					unsigned char tfield[tsize];
					/* Read the actual text field */
					if (!read(file, &tfield, tsize))
						return 1;
					/* Decode the text (it may be in unicode) */
					tsize = audeng_id3_textdec(tfield, textenc, tsize, header->flags);
					audeng_id3_debug("\tField value: %s\n", tfield);
					/* Need strncmp because type isn't null terminated */
					if (!strncmp(frame.type, "TIT2", 4)) { /* Title */
						strncpy(tag->title, tfield, tsize < 64 ? tsize : 63);
					} else if (!strncmp(frame.type, "TPE1", 4)) { /* Artist */
						strncpy(tag->artist, tfield, tsize < 64 ? tsize : 63);
					} else if (!strncmp(frame.type, "TALB", 4)) { /* Album */
						strncpy(tag->album, tfield, tsize < 64 ? tsize : 63);
					} else if (!strncmp(frame.type, "TCON", 4)) { /* Content Type (Genre) */
						strncpy(tag->genre, tfield, tsize < 64 ? tsize : 63);
					}
			}
			offset += 10 + frame.size; /* Skip to next frame */
		}
	}
	return 0;
}
