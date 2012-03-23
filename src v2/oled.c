# include "oled.h"
# include "gpio.c"
# include "spi.c"

static void SSD1339_init (void);
static void SSD1339_reset (void);
static void DC (unsigned int val);
static void RES (unsigned int val);
static void CS (unsigned int val);
static void oled_write (byte data);

/* Same as printf, but for reporting debugging info */
void oled_debug (char *fmt, ...) {
	if (debug_oled) {
		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
	}
}

/* Convert the more common 24-bit RGB to the 18-bit format the OLED takes	*
 * Nothing too fancy, just divides the color straight out.					*/
void oled_rgb (byte r, byte g, byte b, byte color[3]) {
	color[0] = b / 4;
	color[1] = g / 4;
	color[2] = r / 4;
}

/* Fill a rectangular area of the frame buffer, basically software		*
 * "graphics acceleration" but with a 400Mhz gumstix :)					*/
void oled_fill (byte x, byte y, byte x2, byte y2, byte r, byte g, byte b) {
	int row, col;
	for (col = x; col <= x2; col ++) {
		for (row = y; row <= y2; row ++) {
			frame_buffer[row][col][0] = r;
			frame_buffer[row][col][1] = g;
			frame_buffer[row][col][2] = b;
		}
	}
}


/* Same as oled_fill but it fills with the background color (black) */
void oled_partialclear (byte x, byte y, byte x2, byte y2) {
	int row, col;
	
	oled_debug("Clear (%u, %u, %u, %u)\n", x, y, x2, y2);
	
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
void oled_partialflush (byte x, byte y, byte x2, byte y2) {
	int row, col;
	
	oled_debug("Flush (%u, %u, %u, %u)\n", x, y, x2, y2);
	
	/* Make sure coords are within screen */
	if (y2 > oled_screenheight)
		y2 = oled_screenheight;
	if (x2 > oled_screenwidth)
		x2 = oled_screenwidth;
	
	/* Set up controller for data transfer */
	oled_write_c(0x75);		// Set Row Address
	oled_write_d(y);		// Start Row
	oled_write_d(y2);		// End Row
	oled_write_c(0x15);		// Set Column Address
	oled_write_d(x);		// Start Column
	oled_write_d(x2);		// End Column
	oled_write_c(0x5c);		// Write to RAM
	
	/* Send the frame buffer data */
	for (row = y; row <= y2; row ++) {
		for (col = x; col <= x2; col ++) {
			oled_write_d(frame_buffer[row][col][0]);
			oled_write_d(frame_buffer[row][col][1]);
			oled_write_d(frame_buffer[row][col][2]);
		}
	}
	
	/* Datasheet calls for a NOP */
	oled_write_c(0xe3);

	/* Small pause */
	usleep(300);
}

/* Alias to keep compatibilty with old function declaration */
void oled_buffer_flush (byte row, byte row2, byte col, byte col2) {
	oled_partialflush (row, col, row2, col2);
}

/* Flush the whole frame buffer */
void oled_flush () {
	oled_partialflush(0, 0, oled_screenwidth - 1, oled_screenheight - 1);
}

/* Directly copy a portion of the screen. Doesn't affect frame buffer. */
void oled_copy (byte x, byte y, byte x2, byte y2, byte x3, byte y3) {
	oled_debug("Copy (%u, %u, %u, %u) -> (%u, %u) \n", x, y, x2, y2, x3, y3);
	
	/* Copy command */
	oled_write_c(0x8a);
	/* Area to copy */
	oled_write_d(x);
	oled_write_d(y);
	oled_write_d(x2);
	oled_write_d(y2);
	/* Coords to paste */
	oled_write_d(x3);
	oled_write_d(y3);
	
	/* Datasheet calls for a NOP */
	oled_write_c(0xe3);

	/* Small pause */
	usleep(300);	
}

/* Init the OLED display and supporting I/O */
void oled_init (void) {
	oled_debug("OLED Init\n");
	gpio_init();
	gpio_function(oled_dc, GPIO);
	gpio_function(oled_res, GPIO);
	gpio_function(oled_cs, GPIO);
	oled_debug("\tGPIOs OK\n");
	SPI_init(0);  // 0 Clock divider = ~1.6Mbps
	oled_debug("\tSPI OK\n");
	SSD1339_init();
	oled_debug("\tSSD1339 initiliazed\n\tOLED Ready\n\n");
}

static void SSD1339_init(void) {
  //oled_write(0);
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

static void SSD1339_reset (void) {
  RES(0);
  usleep(100000);
  RES(1);
}

void oled_write_c(unsigned char out_command) {
  DC(0);
  CS(0);
  oled_write(out_command);
  CS(1);
  DC(1);
}

void oled_write_d(unsigned char out_data) {
  DC(1);
  CS(0);
  oled_write(out_data);
  CS(1);
  DC(1);
}


/* These functions handle the OLED control lines (val = 0 or 1) */
 
static void DC (unsigned int val) {
	gpio_direction(oled_dc, OUT);
	if (val)
		gpio_set(oled_dc);
	else
		gpio_clear(oled_dc);
}

static void RES (unsigned int val) {
	gpio_direction(oled_res, OUT);
	if (val)
		gpio_set(oled_res);
	else
		gpio_clear(oled_res);
}

static void CS (unsigned int val) {
	gpio_direction(oled_cs, OUT);
	if (val)
		gpio_set(oled_cs);
	else
		gpio_clear(oled_cs);
}


/* Send data to the OLED controller */
static void oled_write(byte data) {
	SPI_TxRx(data);
}
