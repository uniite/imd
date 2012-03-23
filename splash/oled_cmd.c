#include "gpio.c"
#include "spi.c"

// Frame buffer for SPEED, organized as [row][col][buffer]
extern unsigned char frame_buffer[oled_screenheight][oled_screenwidth][3];

void oled_rgb (int8 r, int8 g, int8 b, int8 color[3]);
void oled_fill (int8 x, int8 y, int8 x2, int8 y2, int8 r, int8 g, int8 b);
void oled_partialclear (int8 x, int8 y, int8 x2, int8 y2);
void oled_clear ();
void oled_buffer_flush(int8 row, int8 row2, int8 col, int8 col2);
void oled_flush ();
void oled_partialflush (int8 col, int8 row, int8 col2, int8 row2);
void oled_init (void);
void write_oled_data(void);
void SSD1339_init(void);
void SSD1339_reset(void);
void oled_write_c(unsigned char out_data);
void oled_write_d(unsigned char out_data);
void DC(unsigned int status);
void RES(unsigned int status);
void CS(unsigned int status);
void LCD_out(unsigned char cmd);

/*void gpio_set (unsigned int gpio) {
	char cmd[30];
	sprintf(cmd, "pxaregs GPSR1_%u 1", gpio);
	system(cmd);
}

void gpio_clear (unsigned int gpio) {
	char cmd[30];
	sprintf(cmd, "pxaregs GPCR1_%u 1", gpio);
	system(cmd);
}

void gpio_direction (unsigned int gpio, unsigned int dir) {
	char cmd[30];
	sprintf(cmd, "pxaregs GPDR1_%u %u", gpio, dir);
	system(cmd);
}

void gpio_function (unsigned int gpio, unsigned int function) {
	char cmd[30];
	sprintf(cmd, "pxaregs GAFR1L_%u %s", gpio, function);
	system(cmd);
}*/

/* Convert the more common 24-bit RGB to the 18-bit format the OLED takes *
 * Nothing too fancy, just divides the color straight out.                */
void oled_rgb (int8 r, int8 g, int8 b, int8 color[3]) {
    color[0] = b / 4;
    color[1] = g / 4;
    color[2] = r / 4;
}

/* Fill a rectangular area of the frame buffer, similar to built in "graphics acceleration" *
  * command, but when you're running a 400Mhz gumstix this is plenty fast :)                             */
void oled_fill (int8 x, int8 y, int8 x2, int8 y2, int8 r, int8 g, int8 b) {
    int row, col, i;
    for (col = x; col <= x2; col ++) {
        for (row = y; row <= y2; row ++) {
            frame_buffer[row][col][0] = r;
            frame_buffer[row][col][1] = g;
            frame_buffer[row][col][2] = b;
        }
    }
}


/* Same as oled_fill but it fills with the background color (black) */
void oled_partialclear (int8 x, int8 y, int8 x2, int8 y2) {
    int row, col, i;
    for (col = x; col <= x2; col ++) {
        for (row = y; row <= y2; row ++) {
            frame_buffer[row][col][0] = 0;
            frame_buffer[row][col][1] = 0;
            frame_buffer[row][col][2] = 0;
        }
    }
}

/* Clear the whole frame buffer */
void oled_clear () {
    oled_partialclear (0, 0, oled_screenwidth - 1, oled_screenheight - 1);
}

/* Partial flush of area covered by arguments (x, y, x2, y2) */
void oled_partialflush (int8 col, int8 row, int8 col2, int8 row2) {
	printf("Flush (%u, %u, %u, %u)\n", col, row, col2, row2);
    int i, i2, i3;
    oled_write_c(0x75);		// Set Row Address
    oled_write_d(row);		// Start Row
    oled_write_d(row2);		// End Row
    oled_write_c(0x15);		// Set Column Address
    oled_write_d(col);		// Start Column
    oled_write_d(col2);		// End Column
    oled_write_c(0x5c);		// Write to RAM
	/* Add some filler data (to align screen) */
	/*for (i = 0; i <= 1; i ++) {
        for (i2 = 0; i2 <= oled_screenwidth; i2 ++) {
			oled_write_d(0);
			oled_write_d(0);
			oled_write_d(0);
		}
	}*/
    for (i = row; i <= row2; i ++) {
        for (i2 = col; i2 <= col2; i2 ++) {
            oled_write_d(frame_buffer[i][i2][0]);
            oled_write_d(frame_buffer[i][i2][1]);
            oled_write_d(frame_buffer[i][i2][2]);
		}
    }
    // we need to add an additional NOP as per specs 
    oled_write_c(0xe3);

    // lets pause a sec 
    usleep(300);
}

/* Alias to keep compatibilty with old function declaration */
void oled_buffer_flush (int8 row, int8 row2, int8 col, int8 col2) {
	oled_partialflush (row, col, row2, col2);
}

/* Flush the whole frame buffer */
void oled_flush () {
	oled_partialflush(0, 0, oled_screenwidth - 1, oled_screenheight - 1);
}

void oled_init (void) {
	printf("OLED Init\n");
    gpio_init();
    gpio_function(LCD_DC, GPIO);
    gpio_function(LCD_RES, GPIO);
    gpio_function(LCD_CS, GPIO);
    printf("\tGPIOs OK\n");
    SPI_init(0);  // 0 Clock divider = ~1.6Mbps
    printf("\tSPI OK\n");
    SSD1339_init();
    printf("\tSSD1339 initiliazed\n\tOLED Ready\n\n");
}

// this method processes the complete buffer 
void write_oled_data(void) 
{ 
    int8 x; 

    // lets send the buffer. First part is a command 
    oled_write_c(oled_buffer[0]); 

    // now lets process the reset (args) 
    for(x=1; x<command_index; x++) 
    { 
        oled_write_d(oled_buffer[x]); 
    } 

    // we need to add an additional NOP as per specs 
    oled_write_c(0xe3);

    // lets pause a sec 
    usleep(300);
}

void SSD1339_init(void)
{
  //LCD_out(0);
  DC(0);
  CS(0);
  SSD1339_reset();
  oled_write_c(0xa0); // Set Re-map / Color Depth
  oled_write_d(0xb4); //0x34); // 262K 8bit R->G->B
  oled_write_c(0xa1); // Set display start line
  oled_write_d(0x00); // 00h start
  /* Makes some extra screen area visible */
  oled_write_c(0xa2); // Set display offset
  oled_write_d(0x80); // 80h start
  oled_write_c(0xA6); // Normal display
  oled_write_c(0xad); // Set Master Configuration
  oled_write_d(0x8e); // DC-DC off & external VcomH voltage & external pre-charge voltage
  oled_write_c(0xb0); // Power saving mode
  oled_write_d(0x05); //
  oled_write_c(0xb1); // Set pre & dis_charge
  oled_write_d(0x11); // pre=1h dis=1h
  oled_write_c(0xb3); // clock & frequency
  oled_write_d(0xf0); // clock=Divser+1 frequency=fh
  oled_write_c(0xbb); // Set pre-charge voltage of color A B C
  oled_write_d(0x1c); // color A
  oled_write_d(0x1c); // color B
  oled_write_d(0x1c); // color C
  oled_write_c(0xbe); // Set VcomH
  oled_write_d(0x1f); //
  oled_write_c(0xc1); // Set contrast current for A B C
  oled_write_d(0xaa); // Color A
  oled_write_d(0xb4); // Color B
  oled_write_d(0xc8); // Color C
  oled_write_c(0xc7); // Set master contrast
  oled_write_d(0x0f); // no change
  oled_write_c(0xca); // Mux Ratio
  oled_write_d(0x80); // 128
  oled_write_c(0xaf); // Display on
}
void SSD1339_reset(void)
{
  RES(0);
  usleep(100000);
  RES(1);
}
void oled_write_c(unsigned char out_command)
{
  DC(0);CS(0);
  LCD_out(out_command);
  CS(1);
  DC(1);
}

void oled_write_d(unsigned char out_data)
{
  DC(1);CS(0);
  LCD_out(out_data);
  CS(1);
  DC(1);
}

// these functions set/clear pins for LCD control lines.  they accecpt a 0 or 1 
void DC(unsigned int status)
{
	gpio_direction(LCD_DC, OUT);
	if (status)
		gpio_set(LCD_DC);
	else
		gpio_clear(LCD_DC);
}

void RES(unsigned int status)
{
	gpio_direction(LCD_RES, OUT);
	if (status)
		gpio_set(LCD_RES);
	else
		gpio_clear(LCD_RES);
}

void CS(unsigned int status)
{
	gpio_direction(LCD_CS, OUT);
	if (status)
		gpio_set(LCD_CS);
	else
		gpio_clear(LCD_CS);
}

// send to the LCD
void LCD_out(unsigned char cmd)
{
    SPI_TxRx(cmd);
}
