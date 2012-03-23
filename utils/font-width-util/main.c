#include <stdio.h>
#include <stdlib.h>
#include "font_arial.h"

int main(int argc, char *argv[]) {
	unsigned char width, empty_count, i, row, col, lbound, rbound;
	printf("unsigned char oled_font_width_arial[224] = {\n");
	for (i = 0; i < 224; i++) {
		for (row = 0; row < 13; row++) {
			for(col = 0; col < 16; col++) {
                if (oled_font_arial[i][row] & 1 << col) {
					if (col < lbound) { // Get smallest left boundary
						lbound = col;
					}
					if (col > rbound) {	// Get largest rigth boundary
						rbound = col;
					}
                }
            }
        }
        width = rbound - lbound + 1;
        rbound = lbound = 0;
        printf("\t%u,\t// 0x%X %c\n", width, i + 32, i + 32);
    }
    printf("};\n");
	return 0;
}
