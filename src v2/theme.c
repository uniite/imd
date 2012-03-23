/*******************************
 IMD Themes v1
 * Created Jan 27, 2008
 * by Jon Botelho
 ********************************/

/* Applies theme to textbox
 style
	0	status bar text
	1	regular text
	2	selected text
*******************************/
void imd_theme_textbox (gui_textbox_t *textbox, unsigned int style) {
	switch (style) {
		case 0:
			/* Layer 1 (above background) */
			textbox->z = 1;
			/* Light gray text, single line */
			textbox->color[0] = 50;
			textbox->color[1] = 50;
			textbox->color[2] = 50;
			textbox->mode = 0;
			break;
		case 1:
			/* Blue */
			textbox->z = 1;
			textbox->color[0] = 0;
			textbox->color[1] = 0;
			textbox->color[2] = 50;
			textbox->mode = 0;
			break;
		case 2:
			/* Black */
			textbox->z = 1;
			textbox->color[0] = 0;
			textbox->color[1] = 0;
			textbox->color[2] = 0;
			textbox->mode = 0;
			break;
	}
}

/* Applies theme to graphic
 style
	0	status bar
	1	regular item bg (menu)
	2	selected item bg (menu)
*******************************/
void imd_theme_graphic (gui_graphic_t *graphic, unsigned int style) {
	switch (style) {
		case 0:
			graphic->type = 0; /* SVG */
			graphic->shape = 0; /* Rectangle */
			graphic->fill_type = 1; /* Gradient */
			graphic->gradient_type = 4; /* color -> color2 (vertical) */
			/* Orange */
			graphic->color[0] = 0;
			graphic->color[1] = 10;
			graphic->color[2] = 20;
			/* Lighter shade of blue */
			graphic->color2[0] = 0;
			graphic->color2[1] = 10;
			graphic->color2[2] = 50;
			graphic->z = 0; /* Background layer */
			/* Span screen width */
			graphic->x = 0; 
			graphic->width = oled_screenwidth - 1;
			break;
		case 1:
			graphic->type = 0; /* SVG */
			graphic->shape = 0; /* Rectangle */
			graphic->fill_type = 0; /* Solid Fill*/
			/* Black to keep power consumption low */
			graphic->color[0] = 0;
			graphic->color[1] = 0;
			graphic->color[2] = 0;
			break;
		case 2:
			graphic->type = 0; /* SVG */
			graphic->shape = 0; /* Rectangle */
			graphic->fill_type = 1; /* Gradient */
			graphic->gradient_type = 1; /* color -> color2 (vertical) */
			/* Orange */
			graphic->color[0] = 60;
			graphic->color[1] = 50;
			graphic->color[2] = 10;
			/* Lighter shade of orange */
			graphic->color2[0] = 60;
			graphic->color2[1] = 20;
			graphic->color2[2] = 10;
			break;
	}
}

/* Applies theme to menu
 style
	0	generic
	1	vertical (default style)
	2	horizontal (default style)
*******************************/
void imd_theme_menu (gui_menu_t *menu, unsigned int style) {

	/* Set generic theme to all menus */

	/* Colors */
	menu->item_fg[0] = 0;
	menu->item_fg[1] = 50;
	menu->item_fg[2] = 0;
	menu->item_sel_fg[0] = 50;
	menu->item_sel_fg[1] = 50;
	menu->item_sel_fg[2] = 50;
	
	/* Set menu to first item */
	menu->top = 0;
	menu->cursor = 0;
	menu->index = 0;
	
	/* Apply theme to menu item backgrounds */
	imd_theme_graphic(&menu->item_bg, 1);
	imd_theme_graphic(&menu->item_sel_bg, 2);
	
	/* Add any sepcific settings for the style */
	switch (style) {
		case 0:
			break;
			
		case 1:
			menu->z = 0; /* Background */
			menu->width = oled_screenwidth - 1;
			/* Auto dimensions (based on item_h, vcount) */
			menu->height = 0;
			menu->item_h = 1;
			menu->item_htype = 1;
			
			menu->direction = 0; /* Vertical menu */
			
			break;
			
		case 2:
			menu->z = 0; /* Background */
			/* Auto dimensions (based on item_w, vcount) */
			menu->width = 0;
			menu->height = 0;
			menu->item_h = 1;
			menu->item_htype = 1;
			
			menu->direction = 1; /* Horizontal menu */
			
			break;
	}
}
