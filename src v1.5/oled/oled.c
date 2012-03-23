#include <stdio.h>
#include <unistd.h>
#include "oled.h"
#include "oled_cmd.c"
#include "text.c"

#define bluepx() {oled_write_d(3);oled_write_d(3);oled_write_d(3);}
//#define bluepx() {bluepx0(); bluepx0(); bluepx0();}
#define ltbluepx() {oled_write_d(63);oled_write_d(63);oled_write_d(63);}
//#define ltbluepx() {ltbluepx0();ltbluepx0();ltbluepx0();}

/* Buffer so that everything can be srawn in one 'sweep' *
 * Organization: [rows][columns]                         */
int8 frame_buffer[screenwidth][screenheight][3];


int main () {
    int i, i2, i3, pipe_fd;
    int8 arg[10];
    char buffer[100];
    char text[100];
    //pipe_fd = open("/temp/oledmgr", "r+");
    oled_init();
    fill_enable(0x01);
    /*for (i = 0; i < 100; i ++) {
        draw_circle(rand() % 130, rand() % 130, rand() % 64, rand(), rand(), rand(), rand());
        usleep(10000);
    }*/
    //fontTest1();
  // write directly to ram,  this fills up bottom 1/3 of display with color pattern
    clear_window(0, 0, 131, 131); 
    //set_row_address (30, 80);
    /*set_col_address (30, 80);
oled_write_c(0x5c);
  for (i = 0; i < 10; i++){
      oled_write_c(0x5c);
    for (i2 = 0; i2 < 25; i2 ++) {
        bluepx()
    }
    oled_write_c(0x5c);
    for (i2 = 0; i2 < 100; i2 ++) {
          ltbluepx()
    }
  }*/
    //set_display_reset_normal(); 
    //fill_enable(0x01); 
    //drawFont(getIndex('A'), 1, 0, 7, OFF);
    /*drawFont(getIndex('B'), 1, 6, 7, OFF);
    drawFont(getIndex('C'), 1, 12, 7, OFF);
    drawFont(getIndex('a'), 1, 0, 15, OFF);
    drawFont(getIndex('b'), 1, 6, 15, OFF);
    drawFont(getIndex('c'), 1, 12, 15, OFF);*/
    //oled_print("I like eggs", 0, 7, 1);
    //oled_print("Trains confuse me*", 0, 15, 1);
    //init_text();
    oled_fill(0, 0, 129, 129, oled_bg);
    oled_textbox(0, 7, "I like eggs      Trains confuse me", 1, 17, oled_orange, 3, 0, OFF);
    //oled_print("!@#$%^&*()", 0, 30, 2);
    //oled_print(":+/-{|", 0, 45, 3);
    /*oled_print("This Screen", 0, 70, 2);
    oled_print("is so cool!", 0, 86, 2);*/
    oled_textbox(0, 70, "This screen is so damn cool!     ", 2, 0, oled_blue, 1, 0, OFF);
    oled_textbox(0, 85, "This screen is so damn cool!     ", 2, 0, oled_lime, 1, 0, ON);
    oled_buffer_flush();
    char temp;
    // Main loop - scrolls text boxes at interval, etc.
    while (1) {
        
        /* Remote command processing begin */
        //printf("hmm");
        scanf("%s", buffer);
        //printf("hmm");
        printf("%u", strlen(buffer));
        //printf("hmm");
            switch (buffer[0]) {
                printf("Remote command: %u\n", buffer[0]);
                case 1:
                    for (i = 0; i < 10; i ++) {
                        arg[i] = buffer[i+1];
                        printf("->arg%u: %u\n", i, arg[i]);
                    }
                    temp = buffer[i+1];
                    //printf("len: %u\n", temp);
                    scanf("%s", buffer);
                    printf("->Text: %s\n", buffer);
                    oled_textbox(arg[0], arg[1], buffer, arg[2], arg[3], arg[4], arg[5], arg[6], arg[7], arg[8], arg[9]);
                    break;
                case 2:
                    oled_scrolltext();
                    break;
                case 3:
                    for (i = 0; i < 7; i ++) {
                        arg[i] = buffer[i+1];
                    }
                    read(pipe_fd, buffer, 100);
                    oled_fill(arg[0], arg[1], arg[2], arg[3], arg[4], arg[5], arg[6]);
                    break;
                case 4:
                    oled_buffer_flush();
                    break;
                case 5:
                    oled_init();
                    break;
                case 6:
                    // Just in case calling oled_fill with black causes a segmentation fault again
                    for (i = 0; i < 4; i ++) {
                        arg[i] = buffer[i+1];
                    }
                    oled_clear(arg[0], arg[1], arg[2], arg[3]);
                    break;
                case 7:
                    oled_textbox(0, 70, "This screen is so damn cool!     ", 2, 0, oled_blue, 1, 0, OFF);
                    break;
            }
            buffer[0] = 0;
        /* Remote command code end */
        
        sleep(1); // Scroll text boxes every 5 seconds
        //printf("Scrolling...\n");
        oled_scrolltext();
        oled_buffer_flush();
    }
}
