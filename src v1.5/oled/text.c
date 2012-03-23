#include <string.h>
#include "font.h"

typedef struct {
    int8 x, y, fontsize, maxwidth, r, g, b, mode, align, curline;
    int1 inverse;
    char text[100];
} textbox;

textbox scrollingtext[50];   // Gone through at specified interval to scroll
int8 textcolor[3] = {50, 50, 50};

void drawFont(int8 fontIndex, int8 scale, int8 startX, int8 startY, int8 r, int8 g, int8 b, int1 inverse) 
{ 
    int8 bit, col; 
    int1 singleBit; 
    int8 X1, Y1, X2, Y2;
    int8 i, i2;
    
    // Trying to just put it in buffer so we can render the entire screen in one pass
    for(bit=0; bit<7; bit++) 
    {
        for (i = 0; i < scale; i ++) {
            for(col=0; col<5; col++) 
            { 
                // lets get the bit 
                singleBit=(fonts[fontIndex][col] & bitMask8[7-bit])>>bit; 

                if(inverse) 
                { 
                    singleBit=~singleBit;   // inverse 
                } 

                // draw rect? 
                if(singleBit!=0) 
                {
                    X1=col*scale; 
                    Y1=bit*scale; 
                    
                    for (i2 = 0; i2 < scale; i2 ++) {
                        frame_buffer[startY + Y1 + i][startX + X1 + i2][0] = r;
                        frame_buffer[startY + Y1 + i][startX + X1 + i2][1] = g;
                        frame_buffer[startY + Y1 + i][startX + X1 + i2][2] = b;
                    }
                }
            }  
        }
    } 
}

void eraseFont(int8 fontIndex, int8 scale, int8 startX, int8 startY, int8 r, int8 g, int8 b, int1 inverse) 
{ 
    int8 bit, col; 
    int1 singleBit; 
    int8 X1, Y1, X2, Y2;
    int8 i, i2;
    
    // Trying to just put it in buffer so we can render teh entire screen in one pass
    for(bit=0; bit<7; bit++) 
    {
        for (i = 0; i < scale; i ++) {
            for(col=0; col<5; col++) 
            { 
                // lets get the bit 
                singleBit=(fonts[fontIndex][col] & bitMask8[7-bit])>>bit; 

                if(inverse) 
                { 
                    singleBit=~singleBit;   // inverse 
                } 
                
                if(singleBit!=0) {
                    X1=col*scale; 
                    Y1=bit*scale; 
                    
                    for (i2 = 0; i2 < scale; i2 ++) {
                        frame_buffer[startY + Y1 + i][startX + X1 + i2][0] = b;
                        frame_buffer[startY + Y1 + i][startX + X1 + i2][1] = g;
                        frame_buffer[startY + Y1 + i][startX + X1 + i2][2] = r;
                    }
                }
            }  
        }
    } 
}

unsigned getIndex(char data) 
{ 
    // numbers 
    if(data>='0' && data<='9') 
    { 
        return data-48; 
    } 

    // uppercase 
    if(data>='A' && data<='Z') 
    { 
        return data-45; 
    } 

    // lowercase 
    if(data>='a' && data<='z') 
    { 
        return data-51; 
    } 
    
    // everything else...
    switch (data) {
        case '!':
            return 10;
            break;
        case '@':
            return 11;
            break;
        case '#':
            return 12;
            break;
        case '$':
            return 13;
            break;
        case '%':
            return 14;
            break;
        case '^':
            return 15;
            break;
        case '&':
            return 16;
            break;
        case '*':
            return 80;//17;
            break;
        case '(':
            return 18;
            break;
        case ')':
            return 19;
            break;
        case ' ':
        case '_':
            return 72;
            break;
        case ':':
            return 73;
            break;
        case '+':
            return 74;  // Block
            break;
        case '/':
            return 75; // Slash
            break;
        case '-':
            return 76;
            break;
        case '{':
            return 77;  // Umbrella
            break;
        case '|':
            return 78; // Key
            break;
        default:
            return BLOCK;
    }
}

/* Creates a new text box and draws it on screen
    x, y: coordinates of the text box
    text: a string containg the actual content of the text box
    color: 18-bit color like in oled spec (color[0] = BBBBBGGG, color[1] = GGGRRRRR)
    mode: 0=single line, 1=single line auto scroll at interval, 2=single line scroll on highlight, 3=multi line
    align(horizontal): 0=left, 1=right (text ends at x,y), 2=center (text is ceneterd on x,y)
    maxwidth: maximum width of text box (in characters)
*/
int8 oled_textbox( int8 x, int8 y, char text[1000], int8 fontsize, int8 maxwidth, int8 r, int8 g, int8 b, int8 mode, int8 align, int1 inverse) {
    int8 charwidth = fontsize * 5 + 1;
    int8 charheight = fontsize * 7 + 1;
    int16 tlen = strlen(text);
    int8 i, i2, temp;
    int8 width = (screenwidth - x) / charwidth;
    
    // Can't have a textbox without text :(
    if (tlen == 0)
        return -1;
    
    // Make sure maxwidth doesn't make the text go off the screen
    if (maxwidth == 0 || maxwidth > width) {
        maxwidth = width;
    }
    
    if (maxwidth > tlen) {
        maxwidth = tlen;
    }
    
    printf("Drawing text box...\n->Width: %u chars\n", maxwidth);
    
    if (mode != 3) {
        if (inverse) {
            oled_fill(x, y, x + width * charwidth, y + fontsize * 7 + 1, r, g, b);
            for (i = 0; i < maxwidth; i ++) {
                drawFont(getIndex(text[i]), fontsize, x + charwidth * i, y, oled_bg, OFF);
            }
        } else {
            oled_fill(x, y, x + width * charwidth, y + fontsize * 7 + 1, oled_bg);
            for (i = 0; i < maxwidth; i ++) {
                drawFont(getIndex(text[i]), fontsize, x + charwidth * i, y, r, g, b, OFF);
            }
        }
        if (mode > 0) {
            /* We need to store info on this text box (to be accesable later on from scroll animation),
               but first an empty slot has to be found in scrollingtext (array of text boxes)          */
            for (i = 0; i < 50; i ++) {
                if (scrollingtext[i].fontsize == 0) {
                    scrollingtext[i].x = x;
                    scrollingtext[i].y = y;
                    scrollingtext[i].fontsize = fontsize;
                    scrollingtext[i].r = r;
                    scrollingtext[i].g = g;
                    scrollingtext[i].b = b;
                    scrollingtext[i].mode = mode;
                    scrollingtext[i].align = align;
                    scrollingtext[i].maxwidth = maxwidth;
                    scrollingtext[i].curline = 0;
                    scrollingtext[i].inverse = inverse;

                    temp = (strlen(text) / maxwidth) *  maxwidth;
                    // Note to self: START CODING FOR THE OLED IN PYTHON
                    // Apparently theres no strcopy in my string.h so I'll do it myself
                    for (i2 = 0; i2 < strlen(text); i2 ++) { // Gotta fix text fields to have no set length (text[] vs. current text[100])
                        scrollingtext[i].text[i2] = text[i2];
                    }
                    break;
                }
            }
        }
    } else {
        int8 lines = tlen / maxwidth;
        printf("->Lines: %u\n", lines);
        // Can't have text going offscreen vertically either
        if ( y + (lines * charheight) > screenheight) {
            lines = (screenheight - y) / charheight;
        }
        
        if (inverse) {
            oled_fill(x, y, x + width * charwidth, y + (lines * charheight), r, g, b);

        } else {
            oled_fill(x, y, x + width * charwidth, y + (lines * charheight), oled_bg);
        }
        
        // For each line of text (i), draw each character (i2) in that line (i)
        if (inverse) {
            oled_fill(x, y, x + width * charwidth, y + (i * charheight), r, g, b);
        }
        for (i = 0; i < lines; i ++) {            
            if (inverse) {
                for (i2 = 0; i2 < maxwidth; i2 ++) {
                    drawFont(getIndex(text[(i * maxwidth) + i2]), fontsize, x + charwidth * i2, y + (i * charheight), oled_bg, OFF);
                }
            } else {
                for (i2 = 0; i2 < maxwidth; i2 ++) {
                    drawFont(getIndex(text[(i * maxwidth) + i2]), fontsize, x + charwidth * i2, y + (i * charheight), r, g, b, OFF);
                }
            }
        }
    }
}

// Scrolls all the text boxes that were told to do so (and have text that overflows)
void oled_scrolltext(void) {
    int8 i, i2, width, offset, oldoffset, charwidth, x, y, fontsize, r, g, b;
    int1 inverse;
    
    for (i = 0; i < 50; i ++) {
        if (scrollingtext[i].fontsize != 0) {
            printf("->Textbox %u: %s\n", i, scrollingtext[i].text);
            
            x = scrollingtext[i].x;
            y = scrollingtext[i].y;
            fontsize = scrollingtext[i].fontsize;
            r = scrollingtext[i].r;
            g = scrollingtext[i].g;
            b = scrollingtext[i].b;
            inverse = scrollingtext[i].inverse;
            
            charwidth = scrollingtext[i].fontsize * 5 + 1;
            width = scrollingtext[i].maxwidth;
            
            if (scrollingtext[i].curline == 0) {
                oldoffset = (strlen(scrollingtext[i].text) / width - 1) * width;
            } else {
                oldoffset = width * (scrollingtext[i].curline - 1);
            }
            
            printf("->Old Offset: %u\n", oldoffset, strlen(scrollingtext[i].text));
            
            offset = width * scrollingtext[i].curline;
            
            printf("->Offset: %u\n->Current Line: %u\n", offset, scrollingtext[i].curline);
            
            if (inverse) {
                oled_fill(x, y, x + width * charwidth, y + fontsize * 7 + 1, r, g, b);
                for (i2 = 0; i2 < width; i2 ++) {
                    drawFont(getIndex(scrollingtext[i].text[i2 + offset]), fontsize, x + charwidth * i2, y, oled_bg, OFF);
                }
            } else {
                oled_fill(x, y, x + width * charwidth, y + fontsize * 7 + 1, oled_bg);
                for (i2 = 0; i2 < width; i2 ++) {
                    drawFont(getIndex(scrollingtext[i].text[i2 + offset]), fontsize, x + charwidth * i2, y, r, g, b, OFF);
                }
            }

            /*for (i2 = 0; i2 < width; i2 ++) {
                eraseFont(getIndex(scrollingtext[i].text[i2 + oldoffset]), fontsize, x + charwidth * i2, y, oled_bg, inverse);
            }
            for (i2 = 0; i2 < width; i2 ++) {
                drawFont(getIndex(scrollingtext[i].text[i2 + offset]), fontsize, x + charwidth * i2, y, r, g, b, inverse);
            }*/
            
            if ((offset + width) >= strlen(scrollingtext[i].text) && offset != 0) {
                scrollingtext[i].curline = 0;
                printf("->Reset scroll...\n");
            } else {
                scrollingtext[i].curline ++;
            }
        }
    }
}
