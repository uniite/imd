/******************************************************************************/
/*  OLED example.   8 bit mode                                                */
/*      Spark Fun Electronics   5/3/06   OSO                                  */
/*   Modified by Jon Botelho to work with gumstix (4/20/07)                   */
/******************************************************************************/
/*                                                                            */
/*                                                                            */
/******************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include "gpio.c"
#include "spi.c"

// data bus for LCD, pins on port 0
#define D0 16
#define D1 17
#define D2 18
#define D3 19
#define D4 20
#define D5 21
#define D6 22
#define D7 23

// OLED data port
//#define LCD_DATA 0x00FF0000

// other OLED pins
/*#define LCD_RW    0x00000080
#define LCD_RS    0x04000000
#define LCD_RD    0x00008000
#define LCD_RSTB  0x80000000
#define LCD_CS    0x02000000*/
#define LCD_DC  44
#define LCD_RES 45
#define LCD_CS  43


// inialize OLED
void OLED_init(void);

// reset Controller
void Reset_SSD1339(void);

// write command or data
void write_c(unsigned char out_command);
void write_d(unsigned char out_data);

// these write data to the OLED on 8 bit data bus,  depends on MCU
void LCD_out(unsigned char cmd);
unsigned int get_LCD_port(unsigned char data);

// these functions set / clear pins for OLED control lines.  they accecpt a 0 or 1 
void DC(unsigned int status);
void RES(unsigned int status);
void CS(unsigned int status);


int main (void) {
    int i = 0;
    
    gpio_init();
    
    gpio_function(LCD_DC, GPIO);
    gpio_function(LCD_RES, GPIO);
    gpio_function(LCD_CS, GPIO);
    
   int ClockDivider = 0;	
	
	
	// start the main routine
	printf("\n\n");
	printf("   ------------------------------------------\n");
	printf("   |               SPI Sample                |\n");
	printf("   ------------------------------------------\n");
	printf("   |   Direct Register Access SPI Sample     |\n");
	printf("   ------------------------------------------\n");

	printf(" -> Msg<- Initializing SPI port\n");
	int result = SPI_init(ClockDivider);	// Initialize NSSP / SPI Port
	printf(" -> Msg<- Result of SPI Init = %d\n",result);
    
  OLED_init();

  usleep(120000);


  write_c(0x8e);    // clear window command
  write_d(0);
  write_d(0);
  write_d(130);
  write_d(130);

  usleep(100000);
  
  write_c(0x92);    // fill enable command
  write_d(0x01); 
  
  usleep(10000);

  // draw 100 random circles
  for(i = 0;i < 100;i++){
      write_c(0x86);    // draw circle command
      write_d(rand() % 130);
      write_d(rand() % 130);
      write_d(rand() % 64);
      write_d(rand());
      write_d(rand());
      write_d(rand());
      write_d(rand());
      usleep(10000);
  }

  // write directly to ram,  this fills up bottom 1/3 of display with color pattern
  write_c(0x5c);
  for (i = 0; i < 2000; i++){
  write_c(0x5c);  
   write_d(i);
   write_d(i);
   write_d(i);
  }


  for(;;);
} // main()

void OLED_init(void)
{
  //LCD_out(0);
  DC(0);
  CS(0);
  Reset_SSD1339();
  write_c(0xa0); // Set Re-map / Color Depth
  write_d(0x34);//0xb4); // 262K 8bit R->G->B
  write_c(0xa1); // Set display start line
  write_d(0x00); // 00h start
  //write_c(0xa2); // Set display offset
  //write_d(0x80); // 80h start
  write_c(0xA6); // Normal display
  write_c(0xad); // Set Master Configuration
  write_d(0x8e); // DC-DC off & external VcomH voltage & external pre-charge voltage
  write_c(0xb0); // Power saving mode
  write_d(0x05);
  write_c(0xb1); // Set pre & dis_charge
  write_d(0x11); // pre=1h dis=1h
  write_c(0xb3); // clock & frequency
  write_d(0xf0); // clock=Divser+1 frequency=fh
  write_c(0xbb); // Set pre-charge voltage of color A B C
  write_d(0x1c); // color A
  write_d(0x1c); // color B
  write_d(0x1c); // color C
  write_c(0xbe); // Set VcomH
  write_d(0x1f); //
  write_c(0xc1); // Set contrast current for A B C
  write_d(0xaa); // Color A
  write_d(0xb4); // Color B
  write_d(0xc8); // Color C
  write_c(0xc7); // Set master contrast
  write_d(0x0f); // no change
  write_c(0xca); // Duty
  write_d(0x7f); // 127+1
  write_c(0xaf); // Display on
}
void Reset_SSD1339(void)
{
  RES(0);
  usleep(100000);
  RES(1);
}
void write_c(unsigned char out_command)
{
  DC(0);CS(0);
  LCD_out(out_command);
  CS(1);
  DC(1);
}

void write_d(unsigned char out_data)
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
