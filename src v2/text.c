/*******************************
OLED Text v2
*Rewritten Dec 8, 2007
*Originally created in April
*by Jon Botelho
 ********************************/

#include "font_arial.h"

byte textcolor[3] = {50, 50, 50};

/* Renders single glyph (charcode),  
 *	to frame_buffer at location (x, y), 
 *	with 18-bit color (color[r, g, b]) 
 ****************************************/
void gui_text_render (byte charcode, byte x, byte y, byte color[3]) {
	int row, col, i;
	
	/* Convert charcode from ASCII (we don't want nonprinting characters) */
	charcode = charcode - 32;
	
	for (row = 0; row < gui_font_height; row++) {
		for (col = 0; col < gui_font_width; col++) {
			/* If pixel needs to be set for glyph... */
			if (oled_font_arial[charcode][row] & 1 << col) {
				/* Fill the pixels in the frame buffer */
				for (i = 0; i < 3; i++) {
					frame_buffer[y + row][x + col][i] = color[i];
				}
			}
		}
	}
}

/* Get exact width of a string of text as rendered on screen
 * Arguments: 
 * 	- text: the actual text
 * 	- maxwidth: widest it can be (in pixels)
 * 	- charcount (pointer): value doesn't matter
 * Returns: 
 * 	width of text in pixels
 *  - charcount (pointer): number of characters that fit
 ***************************************************************************/
short gui_text_getwidth (gui_textbox_t *textbox, byte *charcount) {
	byte width = 0;
	
	*charcount = 0;
			
	/* Keep adding on characters until we have enoguh to satisfy the maxwidth */ 
	while (width <= textbox->maxwidth 
		   && textbox->offset + *charcount < textbox->len)
	{
		width += oled_font_width_arial[textbox->text[*charcount + textbox->offset] - 32];
		/* 1 pixel character spacing */
		width++;
		(*charcount)++;
	}
		
	/* If width is too a bit too large, just take off the last character(s) */
	while (width > textbox->maxwidth) {
		width -= oled_font_width_arial[textbox->text[*charcount + textbox->offset] - 32];
		/* 1 pixel character spacing */
		width++;
		(*charcount)--;
	}
	
	return width;
}

/* Render a line of text
 * Arguments:
 * 	- textbox: textbox to render text for 
 * Returns:
 *  number of characters rendered (useful for multiline text)
 ****************************************************************/
byte gui_text_renderline (gui_textbox_t *textbox) {
	byte charcount; /* Number of characters rendered */
	byte curx, i;
	
	/* Width of line of text */
	byte textwidth = gui_text_getwidth(textbox, &charcount);
		
	/* Determine starting point for text rendering based on alignment */
	switch (textbox->align) {
		case 0: // Left aligned: starting point stays the same
			curx = textbox->x;
			break;
		case 1: // Right aligned: starting point is to the left
			curx = textbox->x - textwidth;
			break;
		case 2: // Center aligned: starting point is centered on x
			curx = textbox->x - textwidth / 2;
	}
		
	/* Render each character of textbox->text, starting at the current offset, up to the charcount */
	for (i = textbox->offset; i < charcount + textbox->offset; i++) {
		gui_text_render(
			textbox->text[i],
			curx,
			textbox->y + textbox->line * (gui_font_height + 1),
			textbox->color
		);
		/* Increase curx as we go, by the character's width (for proper spacing) */
		curx += oled_font_width_arial[textbox->text[i] - 32] + 1;
	}
	
	return charcount;
}

/* Initialzies a textbox (must be called before gui_textbox_draw)
 *  Arguments:
 *   - textbox: textbox to initialize
 *****************************************************************/  
void gui_textbox_init (gui_textbox_t *textbox) {
	textbox->len = strlen(textbox->text); /* Length of text in characters */
	textbox->offset = 0;
	/* If maxwidth is set to auto, set the maxwidth as large as possible */
	if (textbox->maxwidth == 0) {
		switch (textbox->align) {
			case 0: /* Left aligned */
				textbox->maxwidth = oled_screenwidth - textbox->x;
				break;
			case 1: /* Right aligned */
				textbox->maxwidth = textbox->x;
				break;
			case 2: /* Center aligned */
				if (textbox->x == oled_screenwidth / 2) {
					textbox->maxwidth = oled_screenwidth;
				} else if (textbox->x < oled_screenwidth / 2) {
					textbox->maxwidth = textbox->x * 2;
				} else if (textbox->x > oled_screenwidth / 2) {
					textbox->maxwidth = (oled_screenwidth - textbox-> x) * 2;
				}
		}
	}
}

/* Draw a textbox to the fram buffer
 *  Arguments:
 *   - textbox: textbox to draw
 *****************************************************************/  
void gui_textbox_draw (gui_textbox_t *textbox) {
	gui_debug("\tDrawing textbox @ %u, %u ('%s')\n", textbox->x, textbox->y, textbox->text);
	
	/* If single line text box... */
	if (textbox->mode < 3) {
		/* Just render whatever text can fit on the line, and save how many characters fit */
		textbox->endoffset = gui_text_renderline(textbox);
	/* Otherwise, it must be a multi line textbox */
	} else {
		/* Keep rendering lines of text until we have rendered all the text */
		textbox->endoffset = 0;
		while (textbox->endoffset < textbox->len) {
			textbox->offset = textbox->endoffset;
			textbox->endoffset += gui_text_renderline(textbox);
			textbox->line++;
		}
	}
}

/* Scroll a textbox
 *  Arguments:
 *   - textbox: textbox to scroll
 *****************************************************************/ 
void gui_textbox_scroll (gui_textbox_t *textbox) {
	if (textbox->fontsize != 0) {
		if (textbox->endoffset == textbox->len) {
			textbox->offset = 0;
		} else {
			textbox->offset = textbox->endoffset;
		}
		textbox->endoffset = gui_text_renderline(textbox);
	}
}
