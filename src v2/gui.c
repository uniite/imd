/********************************
 OLED GUI v2
 * Created Dec 11, 2007
 * by Jon Botelho
 ********************************/

# include "gui.h"
# include "text.c"
# include "gfx.c"
# include "menu.c"

/* Same as printf, but for reporting debugging info */
void gui_debug (char *fmt, ...) {
	if (debug_gui) {
		va_list args;
		
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
	}
}

void gui_screen_init (gui_screen_t *screen) {
	unsigned int i;
	
	screen->textbox = (gui_textbox_t *)screen->textbox_start;
	for (i = 0; i < screen->textbox_count; i++) {
		gui_textbox_init(screen->textbox);
		screen->textbox++;
	}
	
	screen->menu = (gui_menu_t *)screen->menu_start;
	for (i = 0; i < screen->menu_count; i++) {
		gui_menu_init(screen->menu);
		screen->menu++;
	}
	
	/* Graphics don't need to initialize */
}

void gui_screen_draw (gui_screen_t *screen) {
	unsigned int i;
	unsigned char z;
	
	gui_debug("\nDrawing screen\n");
	
	for (z = 0; z < 5; z++) {
		gui_debug("* Z Index %u\n", z);
		
		screen->graphic = (gui_graphic_t *)screen->graphic_start;
		for (i = 0; i < screen->graphic_count; i++) {
			if (screen->graphic->z == z) {
				gui_graphic_draw(screen->graphic);
				screen->graphic++;
			}
		}
		
		screen->textbox = (gui_textbox_t *)screen->textbox_start;
		for (i = 0; i < screen->textbox_count; i++) {
			if (screen->textbox->z == z) {
				gui_textbox_draw(screen->textbox);
				screen->textbox++;
			}
		}

		screen->menu = (gui_menu_t *)screen->menu_start;
		for (i = 0; i < screen->menu_count; i++) {
			if (screen->menu->z == z) {
				gui_menu_draw(screen->menu);
				screen->menu++;
			}
		}

	}
}

void gui_color_copy (unsigned char *dest[3], unsigned char *src[3]) {
	unsigned char i;
	for (i = 0; i < 3; i++) {
		dest[i] = src[i];
	}
}


