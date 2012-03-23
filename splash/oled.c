#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "oled.h"
#include "oled_cmd.c"
#include "text.c"

/* Buffer so that everything can be srawn in one 'sweep' *
 * Organization: [rows][columns]                         */
unsigned char frame_buffer[oled_screenheight][oled_screenwidth][3];

typedef unsigned char byte;

int main (int argc, char **argv) {
	int i, i2, row = 127;
	char bmp_hdr[54];
  unsigned char buff[3];
	FILE *file;
	
  oled_init();
  oled_write_c(0xae); /* Display off */
	oled_clear();
	
	file = fopen("/root/splash.bmp", "rb");
	fread(&bmp_hdr, 1, 54, file);	
	
	for (i = 0; i < 128; i++) {
		for (i2 = 0; i2 < 128; i2++) {
			fread(&buff, 1, 3, file);
			frame_buffer[row][i2][0] = buff[2] / 4;
			frame_buffer[row][i2][1] = buff[1] / 4;
			frame_buffer[row][i2][2] = buff[0] / 4;
		}
		row--;
	}
	
	gui_textbox_t txtLoading;
	strcpy(txtLoading.text, "Loading...");
	txtLoading.x = oled_screenwidth / 2;
	txtLoading.y = 100;
	txtLoading.maxwidth = 0;
	txtLoading.align = 2;
	txtLoading.mode = 0;
	txtLoading.color[0] = 17;
	txtLoading.color[1] = 27;
	txtLoading.color[2] = 45;
	gui_textbox_init(&txtLoading);
	//gui_textbox_draw(&txtLoading);
	
	oled_flush();
	
	oled_write_c(0xaf); /* Display on */
	
	return 0;
}