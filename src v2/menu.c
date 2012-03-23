/*******************************
 OLED Menu v2
 * Created Dec 11, 2007
 * by Jon Botelho
 ********************************/
 
// Initialize menu
void gui_menu_init (gui_menu_t *menu) {
	unsigned int i, i2;
	
	/* Make sure we never render too many items */
	if (menu->item_vcount > menu->item_count) {
		menu->item_vcount = menu->item_count;
	}
	
	if (!menu->direction) { // Vertical menu
		if (menu->item_htype) { // If the item_h is in lines, convert it to pixels
			menu->item_htype = 0;
			menu->item_h *= gui_font_height;
			// Adds even padding around the menu item
			menu->item_h += gui_line_spacing * 2;
		}
		if (!menu->height) { // Automatic height and item_h based on item_vcount
			menu->height = menu->item_h * menu->item_vcount;
		} else { // Automatic item_vcount based on height and item_h
			menu->item_vcount = menu->height / menu->item_h;
		}
		
		// Setup layout textboxes (these will stay in the same place, but the text will change as you scroll)
		menu->textbox_start = (unsigned int)malloc(menu->item_vcount * sizeof(gui_textbox_t));
		menu->textbox = (gui_textbox_t *)menu->textbox_start;
		for (i = 0; i < menu->item_vcount; i++) {
			menu->textbox[i].x = menu->x + 2;
			menu->textbox[i].y = menu->y + menu->item_h * i + 2;
			menu->textbox[i].z = menu->z;
			for (i2 = 0; i2 < 3; i2++)
				menu->textbox[i].color[i2] = menu->item_fg[i2];
			menu->textbox[i].maxwidth = menu->item_w - 1;
			menu->textbox[i].align = 0;
			menu->textbox[i].mode = 2;
			//gui_textbox_init(&menu->textbox[i]);
		}
		/* Set up menu graphics */
		menu->item_bg.x = menu->x;
		menu->item_bg.z = menu->z;
		menu->item_bg.width = menu->item_w;
		menu->item_bg.height = menu->item_h;
		menu->item_sel_bg.x = menu->x;
		menu->item_sel_bg.z = menu->z;
		menu->item_sel_bg.width = menu->item_w;
		menu->item_sel_bg.height = menu->item_h;
	} else { // Horizontal menu
		if (menu->item_wtype) { // Not really functional...don't rely on item_wtype
			menu->item_wtype = 0;
			menu->item_w *= gui_font_width;
			// Adds even padding around the menu item
			menu->item_w += gui_char_spacing * 2;
		}
		if (menu->item_htype) { // If the item_h is in lines, convert it to pixels
			menu->item_htype = 0;
			menu->item_h *= gui_font_height;
			// Adds even padding around the menu item
			menu->item_h += gui_line_spacing * 2;
		}
		if (!menu->width) { // Automatic width and item_w based on item_vcount
			menu->width = menu->item_w * menu->item_vcount;
		} else { // Automatic item_vcount based on height and item_w
			menu->item_vcount = menu->width / menu->item_w;
		}
		if (!menu->height) { // Automatic height and item_h based on item_vcount
			menu->height = menu->item_h;
		}
		
		// Setup layout textboxes (these will stay in the same place, but the text will change as you scroll)
		menu->textbox_start = (unsigned int)malloc(menu->item_vcount * sizeof(gui_textbox_t));
		menu->textbox = (gui_textbox_t *)menu->textbox_start;
		for (i = 0; i < menu->item_vcount; i++) {
			menu->textbox[i].x = menu->x + menu->item_w * i + 2;
			menu->textbox[i].y = menu->y + 2;
			menu->textbox[i].z = menu->z;
			for (i2 = 0; i2 < 3; i2++)
				menu->textbox[i].color[i2] = menu->item_fg[i2];
			menu->textbox[i].maxwidth = menu->item_w - 2;
			menu->textbox[i].align = 0;
			menu->textbox[i].mode = 2;
		}
		/* Set up menu graphics */
		menu->item_bg.y = menu->y;
		menu->item_bg.z = menu->z;
		menu->item_bg.width = menu->item_w;
		menu->item_bg.height = menu->item_h;
		menu->item_sel_bg.y = menu->y;
		menu->item_sel_bg.z = menu->z;
		menu->item_sel_bg.width = menu->item_w;
		menu->item_sel_bg.height = menu->item_h;
	}

}

// Draw a menu (can be used to refresh/redraw menu)
void gui_menu_draw (gui_menu_t *menu) {
	unsigned int i, i2;
	
	menu->item = (gui_menu_item_t *)menu->item_start;
	menu->textbox = (gui_textbox_t *)menu->textbox_start;
	
	gui_debug("\n\tDrawing menu (%u items)\n", menu->item_vcount);
	
	for (i = 0; i < menu->item_vcount; i++) {
		gui_debug("\t* Drawing menu item %u\n", i);
		if (i == menu->cursor) {
			/* Draw background graphic for the menu item */
			if (!menu->direction) { /* Vertical */
				menu->item_sel_bg.y = menu->textbox[i].y - 2;
			} else { /* Horizontal */
				menu->item_sel_bg.x = menu->textbox[i].x - 2;
			}
			gui_graphic_draw(&menu->item_sel_bg);
			strcpy(menu->textbox[i].text, menu->item[menu->top + i].text);
			
			/* Change fg color for selected item */
			for (i2 = 0; i2 < 3; i2++)
				menu->textbox[i].color[i2] = menu->item_sel_fg[i2];
			gui_textbox_init(&menu->textbox[i]);
			gui_textbox_draw(&menu->textbox[i]);
			
			/* Change the fg color back */
			for (i2 = 0; i2 < 3; i2++)
				menu->textbox[i].color[i2] = menu->item_fg[i2];
		} else {
			if (!menu->direction) { /* Vertical */
				menu->item_bg.y = menu->textbox[i].y - 2;
			} else { /* Horizontal */
				menu->item_bg.x = menu->textbox[i].x - 2;
			}
			gui_graphic_draw(&menu->item_bg);
			strcpy(menu->textbox[i].text, menu->item[menu->top + i].text);
			gui_textbox_init(&menu->textbox[i]);
			gui_textbox_draw(&menu->textbox[i]);
		}
	}
}

/****************************************************************************
* Scroll a Menu																*
* -Arguments:																*
* 	menu: pointer to menu													*
* 	direction: 0 = up, 1 = down, 2 = left, 2 = right						*
* 	animation: 0 = none, >1 = number of items to scroll per redraw 			*
* 	(the slow SPI will provide the timing delay...)							*
*	*animation option  not yet supported (all scrolls do one item)			*
* 	start_index: top most menu item to show									*
* 	sel_index: selected item (relative to items shwon on screen)			*
*****************************************************************************/
void gui_menu_scroll (gui_menu_t *menu, unsigned char direction, 
	unsigned char animation, unsigned int start_index, unsigned int sel_index) {
	// Use of a combination of copying and redrawing to make the scrolling responsive (remember to redraw the menu header and footer!)
	switch (direction) {
		unsigned char x, x2, x3, y, y2, y3;
		case 0: // Scroll up
			// Equaly as simple as scrolling down, just have to decrease the index/cursor instead
			// First, we simply have to check to make sure the index isn't already at 0 (anything lower is invalid)
			if (menu->index > 0) {
				menu->index--;
				x = x3 = menu->x;
				x2 = menu->x + menu->width;
				// Move the cursor up if possible
				if (menu->cursor > 0) {
					menu->cursor--;
					gui_menu_draw(menu);
					y = menu->y + menu->cursor * menu->item_h;
					y2 = menu->y + (menu->cursor + 2) * menu->item_h;
					oled_partialflush(x, y, x2, y2);
				} else {
					// If the cursor can't move up, move up the menu
					menu->top--;
					y = menu->y + menu->item_h;
					y2 = menu->y + menu->height - menu->item_h;
					y3 = menu->y + menu->item_h * 2;
					oled_copy(x, y, x2, y2, x3, y3);
					gui_menu_draw(menu);
					y = menu->y;
					y2 = menu->y + menu->item_h * 2;
					oled_partialflush(x, y, x2, y2);
				}
			}
			break;
		case 1: // Scroll down
			// Very simple...just increase the menu index to go down the list...
			// ...as long as it isn't already at the limit
			if (menu->index < menu->item_count - 1) {
				menu->index++;
				x = x3 = menu->x;
				x2 = menu->x + menu->width;
				// Move down the cursor if possible
				if (menu->cursor < menu->item_vcount - 1) {
					menu->cursor++;
					gui_menu_draw(menu);
					y = menu->y + (menu->cursor - 1) * menu->item_h;
					y2 = menu->y + (menu->cursor + 1) * menu->item_h;
					oled_partialflush(x, y, x2, y2);
				} else {
					// Move the menu down if the cursor can't
					menu->top++;
					y = menu->y + menu->item_h;
					y2 = menu->y + menu->height - menu->item_h;
					y3 = menu->y;
					oled_copy(x, y, x2, y2 - 1, x3, y3);
					gui_menu_draw(menu);
					y = menu->y + (menu->item_vcount - 2) * menu->item_h;
					y2 = menu->y + menu->item_vcount * menu->item_h;
					oled_partialflush(x, y, x2, y2);
				}
			}
			break;
		case 2: // Scroll left
			// Similar to scrolling up
			if (menu->index > 0) {
				menu->index--;
				y = y3 = menu->y;
				y2 = menu->y + menu->height;
				// Move the cursor left if possible
				if (menu->cursor > 0) {
					menu->cursor--;
					gui_menu_draw(menu);
					x = menu->x + menu->cursor * menu->item_w;
					x2 = x + 2 * menu->item_w;
					oled_partialflush(x, y, x2, y2);
				} else {
					// If the cursor can't move left, move the menu left
					menu->top--;
					x = menu->x + menu->item_w;
					x2 = menu->x + menu->width - menu->item_w;
					x3 = menu->x + menu->item_w * 2;
					oled_copy(x, y, x2, y2, x3, y3);
					gui_menu_draw(menu);
					x = menu->x;
					x2 = menu->x + menu->item_w * 2;
					oled_partialflush(x, y, x2, y2);
				}
			}
			break;
		case 3: // Scroll right
			// Similar to scrolling down
			if (menu->index < menu->item_count - 1) {
				menu->index++;
				y = y3 = menu->y;
				y2 = menu->y + menu->height;
				// Move right the cursor if possible
				if (menu->cursor < menu->item_vcount - 1) {
					menu->cursor++;
					gui_menu_draw(menu);
					x = menu->x + (menu->cursor - 1) * menu->item_w;
					x2 = x + 2 * menu->item_w;
					oled_partialflush(x, y, x2, y2);
				} else {
					// Move the menu right if the cursor can't
					menu->top++;
					x = menu->x + menu->item_w;
					x2 = menu->x + menu->width - menu->item_w - 1;
					x3 = menu->x;
					oled_copy(x, y, x2, y2, x3, y3);
					gui_menu_draw(menu);
					x = menu->x + (menu->item_vcount - 2) * menu->item_w;
					x2 = menu->x + menu->item_vcount * menu->item_w;
					oled_partialflush(x, y, x2, y2);
				}
			}
			break;
	}
}
