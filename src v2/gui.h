#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>
#include <dirent.h>
#include <time.h>

# include "oled.c"

# ifndef debug_gui
# 	define debug_gui 1
# endif

# define gui_font_height 13
# define gui_font_width 16
# define gui_line_spacing 1
# define gui_char_spacing 1



/* Plenty of types */

/** Textbox instance *************************************************************************
   text: a string containg the actual content of the text box
    x, y: coordinates of the text box
    color: 18-bit foreground color (color = {r, g, b})
    mode: 0=single line, 1=single line auto scroll at interval, 
		  2=single line scroll on highlight, 3=multi line
    align(horizontal): 0=left, 1=right (text ends at x,y), 2=center (text is centered on x,y)
    maxwidth: maximum width of text box (0 = automatic)
********************************************************************************************/
typedef struct {
    unsigned char x, y, z, fontsize, maxwidth, color[3], mode, align, offset, endoffset, line, len;
    char text[255];
} gui_textbox_t;

typedef struct {
	// 0 = SVG (built in shapes)
	unsigned char type;
	// SVG shape: 0 = rectangle
	unsigned char shape;
	// 0 = solid color, 1 = gradient
	unsigned char fill_type;
	// Color (for solid color fill or gradient base)
	unsigned char color[3];
	// Second color (for gradients)
	unsigned char color2[3];
	/*********************
	Gradient types:
	0: from edges (vertical)
	1: from center (vertical)
	2: from edges (horizontal)
	3: from center (horizontal)
	4: from top edge
	**********************/
	unsigned char gradient_type;
	// Coordinates (top left corner)
	unsigned char x, y, z;
	// Dimensions
	unsigned char width, height;
} gui_graphic_t;

typedef struct {
	char text[255];
	unsigned char type; /* (type > 0) == static position */
	void *data;
} gui_menu_item_t;
 
typedef struct {
	// Name of menu, should it be kept from gui.py?
	char name[255];
	// Position and dimensions of menu
	// Note: it is best to leave width = 0 for horizontal menus and height = 0 for vertical ones
	//            to ensure the dimensions come out right (item_h/item_w divide in evenly).
	unsigned char x, y, z, width, height;
	// Menu item characteristcs
	unsigned char item_w, item_h;
	// Number of items that are visible on screen at once. Used if menu->height = 0
	unsigned char item_vcount;
	// Determines the units of item_w and item_h: 0 = by pixels, 1 = by characters(w)/lines(h)
	unsigned char item_wtype, item_htype;
	/******************************************************
	  item_wtype not functional - not worth the added complexity,
	  especially with the modifications t hat would be needed for
	  gui_menu_scroll - all the widths would have to be calculated
	  to ensure that the copy command could still be used, which
	  contributes substantially to the responsiveness of the menu
	 *******************************************************/
	
	// Text (foreground) color: item_fg is normal, item_sel_fg is selected (18-bit rgb)
	unsigned char item_fg[3], item_sel_fg[3];
	// Graphic used for each menu item's background: item_sel_bg is for selected items
	gui_graphic_t item_bg, item_sel_bg;
	// Direction of menu (0 = vertical, 1= horizontal)
	unsigned char direction;
	// Menu items array (pointer to single item)
	gui_menu_item_t *item;
	// Menu item array start (in memory) and count
	unsigned int item_start, item_count;
	// Textboxes (statically positioned) for layout/template
	gui_textbox_t *textbox;
	// Start of textbox array (count is item_vcount)
	unsigned int textbox_start;
	// Current selected item number/index, top menu item's id, cursor position
	unsigned int index, top, cursor;
} gui_menu_t;

typedef struct {
	// Pointers to gui widget array items
	gui_textbox_t *textbox;
	gui_menu_t *menu;
	gui_graphic_t *graphic;
	// Starting address of arrays
	unsigned int textbox_start, menu_start, graphic_start;
	// Number of items in each array
	unsigned int textbox_count, menu_count, graphic_count;
	// Input bindings: link inputs such as accelerometer and buttons to functions
	/***************************
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
	//int (*bind[11])();
	void (*event) ();
	//unsigned int bind_start, bind_count;
} gui_screen_t;


/* Global Vars */
gui_textbox_t gui_text_scrolling[50];   // Gone through at specified interval to scroll



/* Prototypes */

/* text.c */
void gui_text_render (byte charcode, byte x, byte y, byte color[3]);
short gui_text_getwidth (gui_textbox_t *textbox, byte *charcount);
byte gui_text_renderline (gui_textbox_t *textbox);
void gui_textbox_init (gui_textbox_t *textbox);
void gui_textbox_draw (gui_textbox_t *textbox);
void gui_textbox_scroll (gui_textbox_t *textbox);

/* gfx.c */
void gui_graphic_draw (gui_graphic_t *graphic);
void gui_graphic_svg (gui_graphic_t *graphic);
void gui_graphic_gradient (gui_graphic_t *graphic, 
		unsigned char filt[oled_screenheight][oled_screenwidth]);

/* menu.c */
void gui_menu_init (gui_menu_t *menu);
void gui_menu_draw (gui_menu_t *menu);
void gui_menu_scroll (gui_menu_t *menu, unsigned char direction, 
	unsigned char animation, unsigned int start_index, unsigned int sel_index);

/* gui.c */
void gui_debug (char *fmt, ...);
void gui_screen_init (gui_screen_t *screen);
void gui_screen_draw (gui_screen_t *screen);
void gui_color_copy (unsigned char *dest[3], unsigned char *src[3]);
