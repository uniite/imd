/*******************************
 IMD v2
 * Created Jan 27, 2008
 * by Jon Botelho
 ********************************/
 
# include "imd.h"
# include "input.c" 
# include "theme.c"


int main (int argc, char **argv) {	
	/* Init */
	imd_init();
	
	/* Main loop */
	imd_main();
	
	return 0;
}

void imd_debug (const char *fmt, ...) {
	if (debug_imd) {
		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
	}
}

void imd_init () {
	imd_debug("********** Initializing IMD v2 **********\n");
	
	/* Initialize SPI, GPIOs, and OLED Controller */
	oled_init();
	
	/* Open database */
	imd_debug("Opening database...");
	imd_db_open("/mnt/mmc/musicdb-test");
	imd_debug("done.\n");
	
	/* Init audio engine */
	imd_debug("Initializing audio...");
	audeng_init();
	imd_debug("done.\n");
	
	/* Init playlist */
	imd_playback_curplaylist = (imd_playlist_t *)malloc(sizeof(imd_playlist_t));
	
	imd_debug("Initializing screens...");
	/* Clear the screen */	
	oled_clear();
	oled_flush();
	
	/* Initialize screens */
	imd_scr_home_init();
	imd_scr_playback_init();
	imd_scr_options_init();
	imd_debug("done.\n");
	
	/* Show the home screen */
	imd_debug("Loading home screen...");
	imd_scr_load(imd_scr_home, 2);
	imd_debug("done.\n");
	
	# ifdef imd_debug_http
	/* Start the http debug server (serves frame buffer) */
	imd_debug("Starting http debug server...");
	pthread_create(&imd_scr_playback_thread, NULL, imd_scr_playback_progress, NULL);
	imd_debug("done.\n");
	# endif
}

/* Load a gui screen as the current screen
   - Arguments:
	screen: screen to load (must be init'd already)
	save: 0 = don't save the new screen to history
		  1 = save it to history
		  2 = clear the history and save it
***********************************************************/
void imd_scr_load (gui_screen_t *screen, int save) {
	/* Add to history if requested */
	if (save == 2) {
		imd_history_pos = 0;
	} else if (save == 1) {
		imd_history_pos++;
	}
	
	if (save) {
		imd_history_items[imd_history_pos] = screen;
	}
	
	
	/* Set current screen*/
	imd_scr_current = screen;
	
	/* Clear frame buffer */
	oled_clear();
	/* Draw the screen */
	gui_screen_draw(screen);
	/* Flush frame buffer */
	oled_flush();
}

/* Go back a screen in history */
void imd_scr_back () {
	if (imd_history_pos > 0) {
		imd_history_pos--;
		imd_scr_load(imd_history_items[imd_history_pos], 0);
	}
}

/* Triggers event on current screen */
void imd_scr_event (unsigned int type) {
	/* Make sure screen is on */
	oled_write_c(0xaf);
	/* Process event */
	imd_debug("IMD Event (%u)\n", type);
	imd_scr_current->event(type);
}

/* Sets up the home screen */
/* Currently just a music nav menu */
void imd_scr_home_init () {
	imd_scr_home = malloc(sizeof(gui_screen_t));
	
	/* Apply music nav template */
	imd_scr_musicnav_empty(imd_scr_home);
	
	/* Top status bar text */
	strcpy(imd_scr_home->textbox->text, "Home");
	
	
	/* Menu name */
	strcpy(imd_scr_home->menu->name, "home");
	
	/* Create the menu items */
	gui_menu_item_t *items;
	items = malloc(5 * sizeof(gui_menu_item_t));
	strcpy(items[0].text, "Songs");
	strcpy(items[1].text, "Artists");
	strcpy(items[2].text, "Albums");
	strcpy(items[3].text, "Genres");
	strcpy(items[4].text, "Options");
	imd_scr_home->menu->item_start = (unsigned int)items;
	imd_scr_home->menu->item_count = 5;
	
	/* Set the screen's event handler */
	imd_scr_home->event = imd_scr_home_event;
	
	/* Initialize the screen object */
	gui_screen_init(imd_scr_home);
}

/* Sets up the options screen */
void imd_scr_options_init () {
	imd_scr_options = malloc(sizeof(gui_screen_t));
	
	/* Apply music nav template */
	imd_scr_musicnav_empty(imd_scr_options);
	
	/* Top status bar text */
	strcpy(imd_scr_options->textbox->text, "Options");
	
	
	/* Menu name */
	strcpy(imd_scr_options->menu->name, "options");
	
	/* Create the menu items */
	gui_menu_item_t *items;
	items = malloc(3 * sizeof(gui_menu_item_t));
	strcpy(items[0].text, "Battery Voltage");
	strcpy(items[1].text, "OLED Off");
	strcpy(items[2].text, "Scan Files");
	imd_scr_options->menu->item_start = (unsigned int)items;
	imd_scr_options->menu->item_count = 3;
	
	/* Set the screen's event handler */
	imd_scr_options->event = imd_scr_options_event;
	
	/* Initialize the screen object */
	gui_screen_init(imd_scr_options);
}
 
/* Sets up the playback screen */
void imd_scr_playback_init () {
	imd_scr_playback = malloc(sizeof(gui_screen_t));

	/* Create graphics */
	imd_scr_playback->graphic = malloc(3 * sizeof(gui_graphic_t));
	imd_scr_playback->graphic_start = (unsigned int)imd_scr_playback->graphic;
	imd_scr_playback->graphic_count = 3;
	
	/* The top bar ("Now Playing") */
	imd_scr_playback->graphic->height = gui_font_height + (gui_line_spacing * 2);
	imd_scr_playback->graphic->y = 0;
	imd_theme_graphic(imd_scr_playback->graphic, 0);
	
	/* Progress bar background */
	imd_scr_playback->graphic++;
	imd_scr_playback->graphic->height = 8;
	imd_scr_playback->graphic->width = oled_screenwidth - 20 * 2;
	imd_scr_playback->graphic->x = 20;
	imd_scr_playback->graphic->y = 85;
	imd_scr_playback->graphic->color[0] = 0;
	imd_scr_playback->graphic->color[1] = 0;
	imd_scr_playback->graphic->color[2] = 50;
	
	/* Progress bar fill */
	imd_scr_playback->graphic++;
	imd_scr_playback->graphic->height = 6;
	imd_scr_playback->graphic->width = 0;
	imd_scr_playback->graphic->x = 21;
	imd_scr_playback->graphic->y = 86;
	imd_theme_graphic(imd_scr_playback->graphic, 2);
	
	
	/* Create text boxes */
	imd_scr_playback->textbox = malloc(6 * sizeof(gui_textbox_t));
	imd_scr_playback->textbox_start = (unsigned int)imd_scr_playback->textbox;
	imd_scr_playback->textbox_count = 6;
	
	/* "Now Playing" status */
	strcpy(imd_scr_playback->textbox->text, "Now Playing");
	imd_scr_playback->textbox->x = oled_screenwidth / 2;
	imd_scr_playback->textbox->y = 3;
	imd_scr_playback->textbox->maxwidth = 0;
	imd_scr_playback->textbox->align = 2;
	imd_theme_textbox(imd_scr_playback->textbox, 0);
	
	/* Title */
	imd_scr_playback->textbox++;
	imd_scr_playback->textbox->x = oled_screenwidth / 2;
	imd_scr_playback->textbox->y = 25;
	imd_scr_playback->textbox->maxwidth = 0;
	imd_scr_playback->textbox->align = 2;
	imd_theme_textbox(imd_scr_playback->textbox, 1);
	
	/* Artist */
	imd_scr_playback->textbox++;
	imd_scr_playback->textbox->x = oled_screenwidth / 2;
	imd_scr_playback->textbox->y = 45;
	imd_scr_playback->textbox->maxwidth = 0;
	imd_scr_playback->textbox->align = 2;
	imd_theme_textbox(imd_scr_playback->textbox, 1);
	
	/* Album */
	imd_scr_playback->textbox++;
	imd_scr_playback->textbox->x = oled_screenwidth / 2;
	imd_scr_playback->textbox->y = 65;
	imd_scr_playback->textbox->maxwidth = 0;
	imd_scr_playback->textbox->align = 2;
	imd_theme_textbox(imd_scr_playback->textbox, 1);
	
	/* Time Elapsed */
	imd_scr_playback->textbox++;
	imd_scr_playback->textbox->x = 20;
	imd_scr_playback->textbox->y = 95;
	imd_scr_playback->textbox->maxwidth = 0;
	imd_scr_playback->textbox->align = 0;
	imd_theme_textbox(imd_scr_playback->textbox, 1);
	
	/* Time Remaining */
	imd_scr_playback->textbox++;
	imd_scr_playback->textbox->x = oled_screenwidth - 20;
	imd_scr_playback->textbox->y = 95;
	imd_scr_playback->textbox->maxwidth = 0;
	imd_scr_playback->textbox->align = 1;
	imd_theme_textbox(imd_scr_playback->textbox, 1);
	
	/* Set event handler */
	imd_scr_playback->event = imd_scr_playback_event;
	
	/* Initialize the screen object */
	gui_screen_init(imd_scr_playback);
}

/* Fills in current song info on playback screen */
void imd_scr_playback_update () {
	/* Reset textbox pointer */
	imd_scr_playback->textbox = (gui_textbox_t *)imd_scr_playback->textbox_start;
	
	/* Title */
	strcpy(imd_scr_playback->textbox[1].text,
		imd_db_index_title[imd_db_rec[imd_playback_cursong].title].text);
	
	/* Artist */
	strcpy(imd_scr_playback->textbox[2].text,
		imd_db_index_artist[imd_db_rec[imd_playback_cursong].artist].text);
	
	/* Album */
	strcpy(imd_scr_playback->textbox[3].text,
		imd_db_index_album[imd_db_rec[imd_playback_cursong].album].text);
	
	/* Times */
	strcpy(imd_scr_playback->textbox[4].text, "0:00");
	strcpy(imd_scr_playback->textbox[5].text, "-0:00");
	
	gui_screen_init(imd_scr_playback);
}

/* Start playback progress thread */
void imd_scr_playback_thread_start () {
	//return;
	imd_debug("Starting playback thread...");
	imd_scr_playback_stopthread = 0;
	pthread_create(&imd_scr_playback_thread, NULL, imd_scr_playback_progress, NULL);
	imd_debug("done\n");
}

/* Stop playback progress thread */
void imd_scr_playback_thread_stop () {
	//return;
	imd_debug("Stopping playback thread...");
	imd_scr_playback_stopthread = 1;
	imd_scr_playback_pausethread = 0;
	pthread_join(imd_scr_playback_thread, 0);
	imd_debug("done\n");
}

/* Playback progress thread (displays elapsed time, etc.) */
void *imd_scr_playback_progress () {
	unsigned int i, elapsed, duration, time_rem, min, sec, min_rem, sec_rem;
	while (1) {
		//imd_debug("Playback thread: %u\n", audeng_getlen());
		
		/* Small delay to prevent overload */
		for (i = 0; i < 5; i++) {
			/* See if we need to exit */
			if (imd_scr_playback_stopthread) {
				imd_scr_playback_stopthread = 0;
				pthread_exit(0);
			}
			usleep(100000);
		}
		
		/* Make sure we're in playback mode and a song is playing */
		if (imd_scr_current != imd_scr_playback || !audeng_getlen()) {
			continue;
		}
		
		/* Check if thread is paused */
		if (imd_scr_playback_pausethread) {
			imd_scr_playback_pausethread--;
			continue;
		}
		
		/* Times are in tenths of seconds */
		elapsed = audeng_getpos();
		duration = audeng_getlen();
		
		/* If we've reached the end of the song... */
		if (elapsed >= duration) {
			/* Play the next track */
			imd_playback_next();
			pthread_exit(0);
		}
		
		/* Fromatted times */
		sec = (elapsed % 600) / 10;
		min = (elapsed - sec * 10) / 600;
		time_rem = duration - elapsed;
		sec_rem = (time_rem % 600) / 10;
		min_rem = (time_rem - (sec_rem * 10)) / 600;
		
		/* Times, with some neat tricks from python */
		imd_scr_playback->textbox = 
			(gui_textbox_t *)imd_scr_playback->textbox_start;
		sprintf(imd_scr_playback->textbox[4].text, 
			"%u:%s%u", min, (sec > 9) ? "" : "0", sec);
		imd_debug("%u:%s%u/", min, (sec > 9) ? "" : "0", sec);
		sprintf(imd_scr_playback->textbox[5].text, 
			"-%u:%s%u", min_rem, (sec_rem > 9) ?  "" : "0", sec_rem);
		imd_debug("-%u:%s%u\n", min_rem, (sec_rem > 9) ?  "" : "0", sec_rem);
		
		/* Progress bar */
		imd_scr_playback->graphic = 
			(gui_graphic_t *)imd_scr_playback->graphic_start;
		imd_scr_playback->graphic[2].width = 
			(elapsed * 100 / duration) * (oled_screenwidth - 20 * 2) / 100;
		
		/* Redraw the screen */
		gui_screen_init(imd_scr_playback);
		oled_partialclear(0, 80, 127, 120);
		gui_screen_draw(imd_scr_playback);
		oled_partialflush(0, 80, 127, 120);
	}
}

/* Playback screen event handler */
void imd_scr_playback_event (unsigned int type) {
	imd_scr_playback_thread_stop();
	switch (type) {
		case 0: /* Tilt Left */
			/* Go back */
			if (imd_history_pos == 0) {
				imd_scr_load(imd_scr_home, 0);
			} else {
				imd_scr_playback_thread_stop();
				imd_scr_back();
			}
			break;
		case 1: /* Tilt Right */
			/* Play/Pause */
			if (audeng_mode == paused) {
				audeng_resume();
				imd_scr_playback_thread_start();
			} else {
				audeng_pause();
			}
			break;
		case 2: /* Tilt up */
			/* Previous song */
			imd_playback_prev();
			break;
		case 3: /* Tilt down */
			/* Next song */
			imd_playback_next();
			break;
		case 8: /* 3-way switch up */
			/* Volume up */
			audeng_vol += 5;
			imd_playback_vol();
			break;
		case 10: /* 3-way switch down */
			/* Volume down */
			audeng_vol -= 5;
			imd_playback_vol();
			break;
	}
}

/* Show playback screen */
void imd_scr_playback_show() {
	/* Set screen contents to current song */
	imd_scr_playback_update();
	/* Draw the screen */
	imd_scr_load(imd_scr_playback, 0);
	/* Start playback thread */
	imd_scr_playback_thread_start();
}

/* Start playback of cuurrent playlist */
void imd_playback_play () {
	/* Set current song id */
	imd_playback_cursong = imd_playback_curplaylist->ids[imd_playback_playlistpos];
	imd_debug("Playing %s\n", imd_db_rec[imd_playback_cursong].filename);
	
	oled_clear();
	
	/* Show playback screen */
	imd_scr_playback_show();
	
	/* Play it */
	audeng_play(imd_db_rec[imd_playback_cursong].filename, 1);
}

/* Play previous track in current playlist */
void imd_playback_prev () {
	imd_debug("Prev track\n");
	/* Stop playback thread */
	//imd_scr_playback_thread_stop();
	/* Make sure we don't overshoot the playlist */
	if (imd_playback_playlistpos > 0) {
		/* Just go to the previous song */
		imd_playback_playlistpos--;
	} else {
		if (1) { // Add code to check for repeat mode
			/* Repeat mode on: reset to last playlist item */
			imd_playback_playlistpos = imd_playback_curplaylist->len;
		} else {
			/* Repeat mode off: stop playback */
			//imd_playback_stop();
		}
	}
	
	imd_playback_play();
}

/* Play next track in current playlist */
void imd_playback_next () {
	imd_debug("Next track\n");
	/* Stop playback thread */
	//imd_scr_playback_thread_stop();
	/* Make sure we don't overshoot the playlist */
	if (imd_playback_playlistpos < imd_playback_curplaylist->len - 1) {
		/* Just go to the next song */
		imd_playback_playlistpos++;
	} else {
		if (1) { // Add code to check for repeat mode
			/* Repeat modeon: reset to first playlist item */
			imd_playback_playlistpos = 0;
		} else {
			/* Repeat mode off: stop playback */
			audeng_stop();
		}
	}
	
	imd_playback_play();
}

/* Changes volume */
void imd_playback_vol () {
	char vol[15];
	
	/* Make sure volume isn't too high or too low */
	if (audeng_vol < 0)
		audeng_vol = 0;
	if (audeng_vol > 100)
		audeng_vol = 100;
	
	/* Textboxes */
	imd_scr_playback->textbox = 
	(gui_textbox_t *)imd_scr_playback->textbox_start;
	sprintf(imd_scr_playback->textbox[4].text, "Volume");
	sprintf(imd_scr_playback->textbox[5].text, "%u", audeng_vol);
	
	/* Volume bar */
	imd_scr_playback->graphic = 
		(gui_graphic_t *)imd_scr_playback->graphic_start;
	imd_scr_playback->graphic[2].width = 
		(audeng_vol * 100 / 100) * (oled_screenwidth - 20 * 2) / 100;
	
	/* Redraw screen */
	gui_screen_init(imd_scr_playback);
	oled_partialclear(0, 80, 127, 120);
	gui_screen_draw(imd_scr_playback);
	oled_partialflush(0, 80, 127, 120);
	
	/* Set the volume */
	sprintf(vol, "aumix -v %u", audeng_vol);
	system(vol);
	
	/* Restart playback thread in paused mode */
	/* After a short delay, the volume bar will hide */
	imd_scr_playback_pausethread = 3;
	imd_scr_playback_thread_start();
}



/* Handles events for the home screen */
/***************************
Types
	0	Accelerometer X- (tilt left)
	1	Accelerometer X+ (tilt right)
	2	Accelerometer Y- (tilt up)
	3	Accelerometer Y+ (tilt down)
	4	Accelerometer X Alt (tilt left and right quickly)
	5	Accelerometer Y Alt (tilt up and down quickly)
	6	Accelerometer Z- (put down)
	7	Accelerometer Z+ (pick up)
	8	3-Way Switch Up
	9	3-Way Switch Middle (push in)
	10	3-Way Switch Down
****************************/
void imd_scr_home_event (unsigned int type) {
	imd_db_srec_t *query = imd_db_emptyrec();
	
	switch (type) {
		case 0: /* Tilt Left */
			/* Go to playback screen */
			imd_scr_load(imd_scr_playback, 0);
			imd_scr_playback_thread_start();
			break;
		case 1: /* Tilt Right */
			/* Load the screen selected on the menu */
			switch (imd_scr_home->menu->index) {
				case 0: /* (All) Songs */
					imd_scr_load(imd_scr_musicnav(0, query), 1);
					break;
				case 1: /* Artist */
					imd_scr_load(imd_scr_musicnav(1, query), 1);
					break;
				case 2: /* Albums */
					imd_scr_load(imd_scr_musicnav(2, query), 1);
					break;
				case 3: /* Genres */
					imd_scr_load(imd_scr_musicnav(3, query), 1);
					break;
				case 4: /* Options */
					imd_scr_load(imd_scr_options, 1);
					break;
			}
			break;
		case 2: /* Tilt up */
			gui_menu_scroll(imd_scr_home->menu, 0, 0, 2, 3); /* Scroll up */
			break;
		case 3: /* Tilt down */
			gui_menu_scroll(imd_scr_home->menu, 1, 0, 2, 3); /* Scroll down */
			break;
	}
}

void imd_scr_options_event (unsigned int type) {
	gui_screen_t *screen = imd_scr_current;
	FILE *AD3;
	
	switch (type) {
		case 0: /* Tilt Left */
			imd_scr_back(); /* Go back */
			break;
		case 1: /* Tilt Right */
			/* Load the screen selected on the menu */
			switch (screen->menu->index) {
				case 0: /* Battery Voltage */
					AD3 = fopen("/sys/class/ucb1x00/ucb1x00/AD3", "r");
					int val, volt0, volt1;
					fscanf(AD3, "%d\n", &val);
					fclose(AD3);
					volt1 = val * 1000 / 1024 * 75 / 100;
					volt0 = volt1 / 100;
					volt1 -= volt0 * 100;
					screen->menu->item = (gui_menu_item_t *)screen->menu->item_start;
					sprintf(
						screen->menu->item[0].text,
						"Battery Voltage: %d.%dv",
						volt0, volt1
					);
					imd_scr_load(imd_scr_options, 0); 
					break;
				case 1:
					/* Turn screen off */
					oled_write_c(0xae);
					break;
				case 2:
					/* Scan files */
					sprintf(
						screen->menu->item[2].text,
						"Scanning files, please wait..."
					);
					imd_scr_load(imd_scr_options, 0);
					imd_db_scanfiles("/mnt/mmc/music");
					imd_db_cleanfiles();
					sprintf(
						screen->menu->item[2].text,
						"Scan done."
					);
					imd_scr_load(imd_scr_options, 0);
			}
			break;
		case 2: /* Menu up */
			gui_menu_scroll(screen->menu, 0, 0, 2, 3); /* Scroll up */
			break;
		case 3: /* Menu down */
			gui_menu_scroll(screen->menu, 1, 0, 2, 3); /* Scroll down */
			break;
	}
}

void imd_scr_musicnav_event (unsigned int type) {
	unsigned int i;
	/* Current screen */
	gui_screen_t *screen = imd_scr_current;
	gui_screen_t *subscreen;
	/* Current screen's selected menu item data */
	imd_musicnav_itemdata_t *data = screen->menu->item[screen->menu->index].data;
	
	switch (type) {
		case 0: /* Tilt Left */
			imd_scr_back(); /* Go back */
			break;
		case 1: /* Tilt Right */
			if (data->field < 4) { /* Load another musicnav screen */
				imd_scr_load(imd_scr_musicnav(data->field, data->query), 1);
			} else if (data->field == 10) {
				/* Get song ids */
				unsigned short *ids, idc;
				ids = imd_db_select(data->query, &idc);
				imd_playback_cursong = ids[0];
				imd_db_srec_t *query = malloc(sizeof(imd_db_srec_t));
				memcpy(query, data->query, sizeof(imd_db_srec_t));
				query->flags -= 1;
				imd_db_recset_t *recset = imd_db_getrecs(query);
				/* Sort by title */
				imd_debug("Sort %u items\n", recset->count);
				qsort(recset->rec, recset->count, sizeof(imd_db_rec_t), 
					imd_rec_titlecmp);
				/* Get sorted ids */
				idc = recset->count;
				ids = (unsigned short *)malloc(idc * 2);
				for (i = 0; i < idc; i++) {
					ids[i] = recset->rec[i].id;
				}
				/* Put together playlist */
				if (idc > 0) {
					
					/* Get list of all files in selected playlist */
					if (1) { // Change to check for playback mode (play all vs. play just album, etc.)
						data->query->flags = 0;
					} else if (0) {
						data->query->flags -= imd_db_flag_title;
					}
					//ids = imd_db_select(data->query, &idc);
					
					/* Set up the playlist */
					strcpy(imd_playback_curplaylist->name, ""); /* Doesn't matter */
					imd_playback_curplaylist->len = idc;
					imd_playback_curplaylist->ids = ids;
					
					/* Get song's index in playlist */
					imd_playback_playlistpos = 0; /* Just in case */
					/* Loop through playback until we find the current song */
					for (i = 0; i < idc; i++) {
						if (ids[i] == imd_playback_cursong) {
							imd_playback_playlistpos = i;
							break;
						}
					}
					
					/* Start playback */
					imd_playback_play();
					/* Set proper place in history */
					imd_history_pos++;
				} else {
					/* This would be a weird error */
				}
			}
			break;
		case 2: /* Tilt up */
			gui_menu_scroll(screen->menu, 0, 0, 2, 3); /* Scroll up */
			break;
		case 3: /* Tilt down */
			gui_menu_scroll(screen->menu, 1, 0, 2, 3); /* Scroll down */
			break;
		case 5: /* Tilt left+right */
			subscreen = (gui_screen_t *)malloc(sizeof(gui_screen_t));
			/* Show sub menu */
			imd_scr_musicnav_search(subscreen);
			/* Base it on existing menu */
			screen->textbox = (gui_textbox_t *)screen->textbox_start;
			strcpy(subscreen->textbox->text, screen->textbox->text);
			subscreen->menu = (gui_menu_t *)subscreen->menu_start;
			i = screen->menu->item_count;
			subscreen->menu->item = malloc(i * sizeof(gui_menu_item_t));
			subscreen->menu->item_start = (unsigned int)subscreen->menu->item;
			subscreen->menu->item_count = i;
			for (i = 0; i < screen->menu->item_count; i++) {
				strcpy(
					subscreen->menu->item[i].text, 
					screen->menu->item[i].text
				);
			}
			/* Show the screen */
			gui_screen_init(subscreen);
			strcpy(subscreen->textbox->text, screen->textbox->text);
			imd_scr_load(subscreen, 0);
			break;
	}
}

void imd_scr_musicnav_search_event (unsigned int type) {
	unsigned int i;
	/* Current screen */
	gui_screen_t *screen = imd_scr_current;
	gui_screen_t *subscreen;
	
	switch (type) {
		case 0: /* Tilt Left */
			gui_menu_scroll(&screen->menu[1], 2, 0, 2, 3); /* Scroll left */
			break;
		case 1: /* Tilt Right */
			gui_menu_scroll(&screen->menu[1], 3, 0, 2, 3); /* Scroll right */
			break;
		case 2: /* Tilt up */
			/* Exit Search */
			imd_history_pos++;
			imd_scr_back();
			break;
		case 3: /* Tilt down */
			/* If it's backspace, remove a letter */
			if (screen->menu[1].index == 0) {
				i = strlen(screen->textbox[1].text);
				if (i > 8)
					screen->textbox[1].text[i - 1] = 0;
			} else {
				/* Otherwise, add selected letter to search */
				strcat(
					screen->textbox[1].text, 
					screen->menu[1].item[screen->menu[1].index].text
				);
			}
			char *search = screen->textbox[1].text + 8;
			if (!strlen(search)) {
				screen->menu->index = 
				screen->menu->top = 
				screen->menu->cursor = 0;
			} else {
				for (i = 0; i < screen->menu->item_count; i++) {
					if (imd_strifind(screen->menu->item[i].text, search) == 0) {
						screen->menu->index = screen->menu->top = i;
						screen->menu->cursor = 0;
						break;
					}
				}
			}
			gui_screen_init(screen);
			imd_scr_load(screen, 0);
			break;
		case 5: /* Tilt left+right */
			/* Close search (done) */
			subscreen = screen;
			screen = imd_history_items[imd_history_pos];
			/* Load original menu with new position */
			screen->menu->index = subscreen->menu->index;
			screen->menu->top = subscreen->menu->top;
			screen->menu->cursor = subscreen->menu->cursor;
			imd_history_pos++;
			imd_scr_back();
			break;
	}
}

/* Applys music nav template to screen */
void imd_scr_musicnav_empty (gui_screen_t *screen) {
	/* Create top and bottom status bars */
	screen->graphic = malloc(2 * sizeof(gui_graphic_t));
	screen->graphic_start = (unsigned int)screen->graphic;
	screen->graphic_count = 2;
	
	/* Top bar (first 8 rows of screen) */
	screen->graphic->height = gui_font_height + (gui_line_spacing * 2);
	screen->graphic->y = 0;
	imd_theme_graphic(screen->graphic, 0);
	
	/* Bottom bar (last 8 rows of screen) */
	screen->graphic++;
	screen->graphic->height = gui_font_height + (gui_line_spacing * 2);
	screen->graphic->y = oled_screenwidth - screen->graphic->height - 1;
	imd_theme_graphic(screen->graphic, 0);
	
	/* Reset graphic pointer */
	screen->graphic = (gui_graphic_t *)screen->graphic_start;
	
	
	/* Create text boxes for the status bars */
	screen->textbox = malloc(2 * sizeof(gui_textbox_t));
	screen->textbox_start = (unsigned int)screen->textbox;
	screen->textbox_count = 2;
	
	/* Top, center of screen */
	strcpy(screen->textbox->text, "");
	screen->textbox->x = oled_screenwidth / 2;
	screen->textbox->y = 3;
	screen->textbox->maxwidth = 0;
	screen->textbox->align = 2;
	imd_theme_textbox(screen->textbox, 0);
	
	/* Bottom, center of screen (based on top one) */
	screen->textbox++;
	strcpy(screen->textbox->text, "");
	screen->textbox->x = oled_screenwidth / 2;
	screen->textbox->maxwidth = 0;
	screen->textbox->align = 2;
	screen->textbox->y = screen->graphic[1].y + 3;
	imd_theme_textbox(screen->textbox, 0);
	
	/* Reset textbox pointer */
	screen->textbox = (gui_textbox_t *)screen->textbox_start;
	
	
	/* Create the menu */
	screen->menu = malloc(sizeof(gui_menu_t));
	screen->menu_start = (unsigned int)screen->menu;
	screen->menu_count = 1;
	/* Fullscreen menu (minus status bars) */
	screen->menu->x = 0;
	screen->menu->y = gui_font_height + (gui_line_spacing * 2) + 1;
	screen->menu->z = 0;
	screen->menu->item_w = oled_screenwidth - 1; /* Full width item */
	screen->menu->item_vcount = 6; /* Items visible at once */
	screen->menu->item_wtype = 0; /* Width is in pixels */
	/* Apply default vertical menu theme */
	imd_theme_menu(screen->menu, 1);
}

/* Retruns music navigation screen
   - Arguments:
		field: 0 = Songs, 1 = Artists, 2 = Albums, 3 = Genres
		query: used directly with imd_db_select to filter results
********************************************************************/
gui_screen_t *imd_scr_musicnav (unsigned int field, imd_db_srec_t *query) {
	unsigned int i;
	unsigned short *ids;
	unsigned short idc;
	gui_screen_t *screen = malloc(sizeof(gui_screen_t));
	imd_musicnav_itemdata_t *data;
	
	/* Apply music nav template */
	imd_scr_musicnav_empty(screen);
	
	
	/* Get items from the database that match query */
	ids = imd_db_select(query, &idc);
	
	/* Get a list of all the different values for field from the result set */
	/* Just like the SQL unique command */
	imd_db_index_t *index; /* Index of field */
	unsigned char values[4000];
	unsigned int count = 0;
	for (i = 0; i < 4000; i++) {
		values[i] = 0; /* Probably not the best way but oh well */
	}
	
	/* Top status bar text and menu name */
	switch (field) {
		case 0:
			strcpy(screen->textbox->text, "Songs");
			strcpy(imd_scr_home->menu->name, "songs");
			index = imd_db_index_title;
			/* If the value is used, it gets a 1! */
			/* count = number of unique values */
			for (i = 0; i < idc; i++) {
				if (!values[imd_db_rec[ids[i]].title]) {
					count++;
				}
				values[imd_db_rec[ids[i]].title]++;
			}
			break;
		case 1:
			strcpy(screen->textbox->text, "Artists");
			strcpy(imd_scr_home->menu->name, "artists");
			index = imd_db_index_artist;
			for (i = 0; i < idc; i++) {
				if (!values[imd_db_rec[ids[i]].artist]) {
					count++;
				}
				values[imd_db_rec[ids[i]].artist] = 1;
			}
			break;
		case 2:
			strcpy(screen->textbox->text, "Albums");
			strcpy(imd_scr_home->menu->name, "albums");
			index = imd_db_index_album;
			for (i = 0; i < idc; i++) {
				if (!values[imd_db_rec[ids[i]].album]) {
					count++;
				}
				values[imd_db_rec[ids[i]].album] = 1;
			}
			break;
		case 3:
			strcpy(screen->textbox->text, "Genres");
			strcpy(imd_scr_home->menu->name, "genres");
			index = imd_db_index_genre;
			for (i = 0; i < idc; i++) {
				if (!values[imd_db_rec[ids[i]].genre]) {
					count++;
				}
				values[imd_db_rec[ids[i]].genre] = 1;
			}
			break;
		default:
			/* Invalid field */
			imd_scr_musicnav_empty(screen);
			return screen;
	}
	
	
	/* Create the menu items */
	if (field != 0) {
		count++; /* Allow room for all songs option if needed */
	}
	screen->menu->item = malloc(count * sizeof(gui_menu_item_t));
	screen->menu->item_start = (unsigned int)screen->menu->item;
	screen->menu->item_count = count;
	
	/* Fill the menu items */
	if (field != 0) {
		screen->menu->item->data = malloc(sizeof(imd_musicnav_itemdata_t));
		data = screen->menu->item->data;
		strcpy(screen->menu->item->text, "All Songs");
		screen->menu->item->type = 1; /* Static position */
		data->query = malloc(sizeof(imd_db_srec_t));
		memcpy(data->query, query, sizeof(imd_db_srec_t));
		data->field = 0;
		screen->menu->item++;
	}
	for (i = 0; i < 4000; i++) {
		if (values[i]) {
			/* Set item text */
			if (i == 0) {
				/* If its blank, set it to "Unknown..." at top of menu */
				screen->menu->item->type = 1;
				switch (field) {
					case 0:
						/* Is this even possible? */
						strcpy(screen->menu->item->text, "Unknown Title");
						break;
					case 1:
						strcpy(screen->menu->item->text, "Unknown Artist");
						break;
					case 2:
						strcpy(screen->menu->item->text, "Unknown Album");
						break;
					case 3:
						strcpy(screen->menu->item->text, "Unknown Genre");
						break;
				}
			} else {
				screen->menu->item->type = 0;
				strcpy(screen->menu->item->text, index[i].text);
			}
			
			/* Set up item data, inheriting the query */
			screen->menu->item->data = malloc(sizeof(imd_musicnav_itemdata_t));
			data = screen->menu->item->data;
			data->query = malloc(sizeof(imd_db_srec_t));
			memcpy(data->query, query, sizeof(imd_db_srec_t));
			
			switch (field) {
				case 0:
					/* Event is playback */
					data->field = 10; /* Display playback */
					data->query->flags += imd_db_flag_title;
					strcpy(data->query->title, index[i].text);
					break;
				case 1:
					/* Event is musicnav filtered by Artist */
					data->field = 2; /* Display Albums */
					data->query->flags += imd_db_flag_artist;
					strcpy(data->query->artist, index[i].text);
					break;
				case 2:
					/* Event is musicnav filtered by Album */
					data->field = 0; /* Display Songs */
					data->query->flags += imd_db_flag_album;
					strcpy(data->query->album, index[i].text);
					break;
				case 3:
					/* Event is musicnav filtered by Genre */
					data->field = 1; /* Display Artists */
					data->query->flags += imd_db_flag_genre;
					strcpy(data->query->genre, index[i].text);
					break;
			}
			screen->menu->item++;
		}
	}
	
	/* Sort the menu items alphabetically */
	qsort((gui_menu_item_t *)screen->menu->item_start, count, sizeof(gui_menu_item_t), imd_menu_itemcmp);
	
	/* Set the screen's event handler */
	screen->event = imd_scr_musicnav_event;
	
	/* Init the screen */
	gui_screen_init(screen);
	
	return screen;
}

/* Adds a pop-up "keyboard" to the screen to search */
void imd_scr_musicnav_search (gui_screen_t *screen) {
	unsigned int i, count;
	
	/* Create top and bottom status bars */
	screen->graphic = malloc(2 * sizeof(gui_graphic_t));
	screen->graphic_start = (unsigned int)screen->graphic;
	screen->graphic_count = 2;
	
	/* Top bar (first 8 rows of screen) */
	screen->graphic->height = gui_font_height + (gui_line_spacing * 2);
	screen->graphic->y = 0;
	imd_theme_graphic(screen->graphic, 0);
	
	/* Bottom bar (last 8 rows of screen) */
	screen->graphic++;
	screen->graphic->height = gui_font_height + (gui_line_spacing * 2);
	screen->graphic->y = oled_screenwidth - screen->graphic->height - 1;
	imd_theme_graphic(screen->graphic, 0);
	
	/* Reset graphic pointer */
	screen->graphic = (gui_graphic_t *)screen->graphic_start;
	
	
	/* Create text boxes for the status bars */
	screen->textbox = malloc(2 * sizeof(gui_textbox_t));
	screen->textbox_start = (unsigned int)screen->textbox;
	screen->textbox_count = 2;
	
	/* Top, center of screen */
	strcpy(screen->textbox->text, "Stuff");
	screen->textbox->x = oled_screenwidth / 2;
	screen->textbox->y = 3;
	screen->textbox->maxwidth = 0;
	screen->textbox->align = 2;
	imd_theme_textbox(screen->textbox, 0);
	
	/* Bottom, center of screen (based on top one) */
	//screen->textbox++;
	strcpy(screen->textbox[1].text, "Search: ");
	screen->textbox[1].x = oled_screenwidth / 2;
	screen->textbox[1].maxwidth = 0;
	screen->textbox[1].align = 2;
	screen->textbox[1].y = screen->graphic[1].y - 10;
	imd_theme_textbox(&screen->textbox[1], 0);
	
	/* Reset textbox pointer */
	screen->textbox = (gui_textbox_t *)screen->textbox_start;
	
	
	/* Create the menu */
	screen->menu = malloc(2 * sizeof(gui_menu_t));
	screen->menu_start = (unsigned int)screen->menu;
	screen->menu_count = 2;
	/* Fullscreen menu (minus status bars) */
	screen->menu->x = 0;
	screen->menu->y = gui_font_height + (gui_line_spacing * 2) + 1;
	//screen->menu->z = 0;
	screen->menu->item_w = oled_screenwidth - 1; /* Full width item */
	screen->menu->item_vcount = 6; /* Items visible at once */
	screen->menu->item_wtype = 0; /* Width is in pixels */
	/* Apply default vertical menu theme */
	imd_theme_menu(screen->menu, 1);
	
	/* Add on menu to act as keyboard */
	imd_debug("h menu\n");
	screen->menu++;
	screen->menu->x = 2;
	/* Just above bottom bar */
	screen->menu->y = oled_screenheight - gui_font_height;
	//screen->menu->z = 0;
	screen->menu->item_w = 16; /* Fits one character */
	screen->menu->item_vcount = 8; /* Items visible at once */
	screen->menu->item_wtype = 0; /* Width is in pixels */
	/* Apply default horizontal menu theme */
	imd_theme_menu(screen->menu, 2);
	screen->menu->item_htype = 0;
	screen->menu->item_h = gui_font_height - 2;
	imd_debug("memcpy\n");
	/* Allocate memory for 27 items (#, A-Z) */
	count = 27;
	screen->menu->item = malloc(count * sizeof(gui_menu_item_t));
	screen->menu->item_start = (unsigned int)screen->menu->item;
	screen->menu->item_count = count;
	imd_debug("memcpy!\n");
	/* Fill in the items */
	strcpy(screen->menu[0].item[0].text, "-");
	imd_debug("sprintf %u\n", 0);
	for (i = 1; i < count; i++) {
		imd_debug("sprintf %u\n", i);
		sprintf(screen->menu->item[i].text, "%c", 'A' + i - 1);
	}
	
	screen->event = imd_scr_musicnav_search_event;
}

/* Finds first occurence of b in a, returns -1 for no match */
int imd_strifind (char *a, char *b) {
	unsigned int i, i2, len, len2, res;
	i2 = res = 0;
	
	len = strlen(a);
	len2 = strlen(b);
	
	/* Can't find larger string in smaller one */
	if (len2 > len)
		return -1;
	
	/* Go through each character to find match */
	for (i = 0; i < len; i++) {
		if (i2 && tolower(a[i]) == tolower(b[i2])) {
			/* Try to match next character */
			i2++;
			if (i2 == len2)
				return res;
		} else if (tolower(a[i]) == tolower(b[0])) {
			/* First character matched */
			i2 = 1;
			res = i;
			if (len2 == 1)
				return res;
		} else { /* a[i] != b[i2] */
			i2 = 0;
			res = i;
		}
		
		/* If there is no chance of a match, exit */
		if (len - res < len2)
			return -1;
	}
	
	/* No match found */
	return -1;
}

/* Case-insensitive strcmp */
int imd_stricmp (const void *a, const void *b) {
	const char *str1 = a, *str2 = b;
	char x, y;
	int i;
	for (i = 0; i < strlen(a); i++) {
		/* Convert to lower case */
		x = tolower(str1[i]);
		y = tolower(str2[i]);
		/* If not equal, see if its greater or less than */
		if (x != y) {
			if (x > y) {
				return 1; /* a > b */
			} else {
				return -1; /* a < b */
			}
		}
	}
	return 0; /* a == b */
}

/* Compares two integers. Used by imd_rec_itemcmp_*time */
int imd_icmp (int a, int b) {
	if (a == b)
		return 0; /* a == b */
	else if (a > b)
		return 1; /* a > b */
	else
		return -1; /* a < b */
}

/* Compares two menu items. Designed for use with qsort */
/* Good example of why my naming system needs a rework... */
int imd_menu_itemcmp (const void *a, const void *b) {
	const gui_menu_item_t *item1 = a, *item2 = b;
	
	/* Statically placed items (ex. "All Songs") rule all! */
	if (item1->type != item2->type) {
		if (item1->type < item2->type) {
			return 1;
		} else {
			return -1;
		}
	} else {
		/* Case insensitive version of strcmp (gcc only!) */
		return imd_stricmp(item1->text, item2->text);
	}
}

/* Functions to compare two db records.
	- Sort by title, artist, etc. based on function 
	- Designed for use with qsort 
******************************************************/
int imd_rec_titlecmp (const void *a, const void *b) {
	const imd_db_rec_t *rec1 = a, *rec2 = b;
	int res;
	
	/* Compare the two values (case insensitive) */
	res = imd_stricmp(
		imd_db_index_title[rec1->title].text,
		imd_db_index_title[rec2->title].text
	);
	return res;
}

int imd_rec_artistcmp (const void *a, const void *b) {
	const imd_db_rec_t *rec1 = a, *rec2 = b;
	
	return imd_stricmp(
		imd_db_index_artist[rec1->artist].text,
		imd_db_index_artist[rec2->artist].text
	);
}

int imd_rec_albumcmp (const void *a, const void *b) {
	const imd_db_rec_t *rec1 = a, *rec2 = b;
	
	return imd_stricmp(
		imd_db_index_album[rec1->album].text,
		imd_db_index_album[rec2->album].text
	);
}

int imd_rec_genrecmp (const void *a, const void *b) {
	const imd_db_rec_t *rec1 = a, *rec2 = b;
	
	return imd_stricmp(
		imd_db_index_genre[rec1->genre].text,
		imd_db_index_genre[rec2->genre].text
	);
}

int imd_rec_filenamecmp (const void *a, const void *b) {
	const imd_db_rec_t *rec1 = a, *rec2 = b;
	
	return imd_stricmp(
		rec1->filename,
		rec2->filename
	);
}

int imd_rec_mtimecmp (const void *a, const void *b) {
	const imd_db_rec_t *rec1 = a, *rec2 = b;
	
	return imd_icmp(rec1->mtime, rec2->mtime);
}

int imd_rec_atimecmp (const void *a, const void *b) {
	const imd_db_rec_t *rec1 = a, *rec2 = b;
	
	return imd_icmp(rec1->atime, rec2->atime);
}

int imd_rec_ctimecmp (const void *a, const void *b) {
	const imd_db_rec_t *rec1 = a, *rec2 = b;
	
	return imd_icmp(rec1->ctime, rec2->ctime);
}
