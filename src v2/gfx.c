/********************************
 OLED Graphics v1
 * Created Dec 14, 2007
 * by Jon Botelho
 ********************************/
 
/********************************************
Ideas for future additions 
 * bmp/jpg/png/gif types
 * image fills, patterns
 * more shapes (ex. rounded rectangle, circle)
 ********************************************/
 
void gui_graphic_draw (gui_graphic_t *graphic) {
	switch (graphic->type) {
		case 0:
			gui_graphic_svg(graphic);
			break;
	}
}

void gui_graphic_svg (gui_graphic_t *graphic) {
	unsigned char row, col;
	unsigned char image_filt[oled_screenheight][oled_screenwidth];
	
	switch (graphic->shape) {
		case 0:
			gui_debug("\tDrawing graphic (svg) @ %u, %u (%u x %u) %u\n",
					graphic->x, graphic->y, graphic->width, 
					graphic->height, graphic->fill_type
			);
			
			for (row = graphic->y; row <= graphic->y + graphic->height; row++) {
				for (col = graphic->x; col <= graphic->x + graphic->width; col++) {
					oled_fill (col, row, col, row, graphic->color[0], graphic->color[1], graphic->color[2]);
					image_filt[row][col] = 1;
				}
			}
			break;
	}
	
	if (graphic->fill_type == 1) {
		gui_graphic_gradient (graphic, image_filt);
	}
}

/* Just apply gradient to whichever pixels aren't blank in image, 
 * changing color by grad_step for each step (pixel row/column, depending on direction of gradient) */
void gui_graphic_gradient (gui_graphic_t *graphic,  
		unsigned char filt[oled_screenheight][oled_screenwidth])
{
	unsigned char row, row2, col, i, grad_step[3], stop, r, g, b;
	switch (graphic->gradient_type) {
		case 1:
			row2 = graphic->height / 2;
			for (i = 0; i < 3; i++) {
				grad_step[i] = (graphic->color[i] - graphic->color2[i]) / row2;
			}
			
			for (row = graphic->y; row < graphic->y + row2; row++) {
				for (col = graphic->x; col <= graphic->x + graphic->width; col++) {
					if (filt[row][col]) {
						r = graphic->color[0] - (grad_step[0] * (row - graphic->y));
						g = graphic->color[1] - (grad_step[1] * (row - graphic->y));
						b = graphic->color[2] - (grad_step[2] * (row - graphic->y));
						oled_fill (col, row, col, row, r, g, b);
					}
				}
			}
			for (i = 0; i < 3; i++) {
				graphic->color2[i] = graphic->color[i] - (grad_step[i] * (row - graphic->y));
			}
			row2 = row;
			stop = graphic->y + graphic->height;
			for (; row < graphic->y + graphic->height; row++) {
				for (col = graphic->x; col <= graphic->x + graphic->width; col++) {
					if (filt[row][col]) {
						r = graphic->color2[0] + (grad_step[0] * (row - row2));
						g = graphic->color2[1] + (grad_step[1] * (row - row2));
						b = graphic->color2[2] + (grad_step[2] * (row - row2));
						oled_fill (col, row, col, row, r, g, b);
					}
				}
			}
			break;
		case 4:
			for (i = 0; i < 3; i++) {
				grad_step[i] = (graphic->color[i] - graphic->color2[i]) / graphic->height;
			}
			for (row = graphic->y; row <= graphic->y + graphic->height; row++) {
				for (col = graphic->x; col <= graphic->x + graphic->width; col++) {
					if (filt[row][col]) {
						r = graphic->color[0] - (grad_step[0] * (row - graphic->y));
						g = graphic->color[1] - (grad_step[1] * (row - graphic->y));
						b = graphic->color[2] - (grad_step[2] * (row - graphic->y));
						oled_fill (col, row, col, row, r, g, b);
					}
				}
			}
			break;
	}
}
