/*******************************
OLED Text v2
*Rewritten Dec 8, 2007
*Originally created in April
*by Jon Botelho
 ********************************/

#include "font_arial.h"

#define gui_font_height 13
#define gui_font_width 16
#define gui_line_spacing 1
#define gui_char_spacing 1

/** Textbox instance *************************************************************************
   text: a string containg the actual content of the text box
    x, y: coordinates of the text box
    color: 18-bit foreground color (color = {r, g, b})
    mode: 0=single line, 1=single line auto scroll at interval, 2=single line scroll on highlight, 3=multi line
    align(horizontal): 0=left, 1=right (text ends at x,y), 2=center (text is ceneterd on x,y)
    maxwidth: maximum width of text box (0 = automatic)
********************************************************************************************/
typedef struct {
    unsigned char x, y, z, fontsize, maxwidth, color[3], mode, align, offset, endoffset, line, len;
    char text[255];
} gui_textbox_t;

gui_textbox_t gui_text_scrolling[50];   // Gone through at specified interval to scroll
int8 textcolor[3] = {50, 50, 50};

// Renders single glyph (charcode),  to frame_buffer at location (x, y), with 18-bit color (color[r, g, b])
void gui_text_render (unsigned char charcode, unsigned char x, unsigned char y, unsigned char color[3]) {
	int row, col, i;
	charcode = charcode - 32;	// Convert charcode from ascii (we don't do nonprinting characters)
	for (row = 0; row < gui_font_height; row++) {
		for (col = 0; col < gui_font_width; col++) {
			if (oled_font_arial[charcode][row] & 1 << col) {	// If pixel needs to be set for glyph...
				for (i = 0; i < 3; i++) {				// Fill the pixels in the frame buffer
					frame_buffer[y + row][x + col][i] = color[i];
				}
			}
		}
	}
}

/* Get exact width of a string of text as rendered on screen							*
* Arguments: text (the actual text), maxwidth (largest it can be), charcount (value doesn't matter)	*
* Returns: width of text in pixels, charcount (number of characters that fit)				*/
short gui_text_getwidth (gui_textbox_t *textbox, unsigned char *charcount) {
	unsigned char width = 0;
	*charcount = 0;
	unsigned char temp;
	printf("\t\tWidth: %u\n", width);
	printf("\t\tMaxidth: %u\n", textbox->maxwidth);
	printf("\t\tOffset: %u\n", textbox->offset);
	printf("\t\tCharcount: %u\n", *charcount);
	printf("\t\tLen: %u\n", textbox->len);
	// Keep adding on characters until we have enoguh to satisfy the maxwidth
	while (width <= textbox->maxwidth 
		   && textbox->offset + *charcount < textbox->len)
	{
		width += oled_font_width_arial[textbox->text[*charcount + textbox->offset] - 32];
		// 1 pixel character spacing
		width++;
		(*charcount)++;
	}
		
	// If width is too a bit too large, just take off the last character(s)
	while (width > textbox->maxwidth) {
		width -= oled_font_width_arial[textbox->text[*charcount + textbox->offset] - 32];
		// 1 pixel character spacing
		width++;
		(*charcount)--;
	}
	
	return width;
}

/* Render a line of text, according to the specs in the textbox provided	*
*   Returns: number of characters rendered (useful for multiline text)	*/
unsigned char gui_text_renderline (gui_textbox_t *textbox) {
	unsigned char charcount; // Number of characters rendered
	unsigned char curx, i;
	
	// Width of line of text
	unsigned char textwidth = gui_text_getwidth(textbox, &charcount);
	/*printf("\t\tMaxwidth ('%s') = %u\n", textbox->text, textbox->maxwidth);
	printf("\t\tTextwidth ('%s') = %u\n", textbox->text, textwidth);*/
		
	// Determine starting point for text rendering based on alignment
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
		
	// Render each character of textbox->text, starting at the current offset, up to the charcount
	for (i = textbox->offset; i < charcount + textbox->offset; i++) {
		gui_text_render(textbox->text[i], curx, textbox->y + textbox->line * (gui_font_height + 1), textbox->color);
		// Increase curx as we go, by the character's width (for proper spacing)
		//printf("->char_width '%c' = %u\n", textbox->text[i], oled_font_width_arial[textbox->text[i] - 32]);
		curx += oled_font_width_arial[textbox->text[i] - 32] + 1;
	}
	
	return charcount;
}

/* Initialzies textbox										*
*   See gui_textbox_t type declaration at the begining of this file for more info 	*/
void gui_textbox_init (gui_textbox_t *textbox) {
	unsigned char i;					// General purpose counter
	unsigned char charcount = 0;		// Number of characters that can fit in line on screen
	unsigned char textwidth = 0;		// Width of rendered line of text in pixels
	unsigned char curx;				// Current x position (for rendering textbox)
	textbox->len = strlen(textbox->text); // Length of text in characters
	//printf("--->Len: %u\n", textbox->len);
	textbox->offset = 0;
	// If maxwidth is set to auto, set the maxwidth as large as possible (without going off-screen)
	if (textbox->maxwidth == 0) {
		switch (textbox->align) {
			case 0: // Left aligned
				textbox->maxwidth = oled_screenwidth - textbox->x;
				break;
			case 1: // Right aligned
				textbox->maxwidth = textbox->x;
				break;
			case 2: // Center aligned
				if (textbox->x == oled_screenwidth / 2) {
					textbox->maxwidth = oled_screenwidth;
				} else if (textbox->x < oled_screenwidth / 2) {
					textbox->maxwidth = textbox->x * 2;
				} else if (textbox->x > oled_screenwidth / 2) {
					textbox->maxwidth = (oled_screenwidth - textbox-> x) * 2;
				}
		}
	}
	//gui_textbox_draw(textbox);
}

// Draw textbox
void gui_textbox_draw (gui_textbox_t *textbox) {
	printf("\tDrawing textbox @ %u, %u ('%s')\n", textbox->x, textbox->y, textbox->text);
	// If single line text box...
	if (textbox->mode < 3) {
		// Just render whatever text can fit on the line, and save how many characters fit
		textbox->endoffset = gui_text_renderline(textbox);
	// Otherwise, it must be a multi line textbox
	} else {
		// Keep rendering lines of text until we have rendered all the text
		textbox->endoffset = 0;
		while (textbox->endoffset < textbox->len) {
			textbox->offset = textbox->endoffset;
			textbox->endoffset += gui_text_renderline(textbox);
			textbox->line++;
		}
	}
}

// Scrolls textbox with index "id"
void gui_textbox_scroll (gui_textbox_t *textbox) {
	// Go through all 50 scrolling textbox slots, updating each one that is in use
	if (textbox->fontsize != 0) {
		if (textbox->endoffset == textbox->len) {
			textbox->offset = 0;
		} else {
			textbox->offset = textbox->endoffset;
		}
		textbox->endoffset = gui_text_renderline(textbox);
	}
}
