/****************************************************************************/
/*                                                                          */
/*   Copyright (c) 2006 Tim Crawford     <timcrawford@comcast.net>          */
/*                                                                          */
/*   This program is free software; you can redistribute it and/or modify   */
/*   it under the terms of the GNU General Public License version 2 as      */
/*   published by the Free Software Foundation.                             */
/*                                                                          */
/*   Alternatively, this software may be distributed under the terms of BSD */
/*   license.                                                               */
/*                                                                          */
/*   See README and COPYING for more details.                               */
/*                                                                          */
/****************************************************************************/
/*   Use this Makefile as an example on how to build:                       */
/*   http://svn.gumstix.org/wikifiles/hello-world/Makefile                  */
/*   This program demonstrates how to program the GPIO                      */
/*                                                                          */
/*   gpio(u32 direction, u32 set_clear, u32 gpio_bit)                       */
/*   example:                                                               */
/*     gpio(OUT, SET, 59); //This will program GPIO(59) as a GPIO function  */
/*                         // set the direction as output, and set the bit  */
/*                                                                          */
/*   gpio_function(u32 gpio_bit, u32 function);                             */
/*   example:                                                               */
/*     gpio_function(59, GPIO); //This will program GPIO(59) as a GPIO funct*/
/*                                                                          */
/*   gpio_direction(u32 gpio_bit, u32 direction)                            */
/*   example:                                                               */
/*     gpio_direction(59, OUT); //This will program GPIO(59) as an output   */
/*                                                                          */
/*   gpio_set(u32 gpio_bit)                                                 */
/*   example:                                                               */
/*     gpio_set(59); //This will program GPIO(59) to a logic 1              */
/*                                                                          */
/*   gpio_clear(u32 gpio_bit)                                               */
/*   example:                                                               */
/*     gpio_clear(59); //This will program GPIO(59) to a logic 0            */
/*                                                                          */
/*   gpio_status(u32 gpio_bit)                                              */
/*   example:                                                               */
/*     i = gpio_status(59); //i is set to the logic level of GPIO(59)       */
/*                                                                          */
/****************************************************************************/
#include <stdio.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#define MAP_SIZE 4096
#define MAP_MASK ( MAP_SIZE - 1 )
#define GPLR0   0x40E00000
#define GPLR1   0x40E00004
#define GPLR2   0x40E00008
#define GPDR0   0x40E0000C
#define GPDR1   0x40E00010
#define GPDR2   0x40E00014
#define GPSR0   0x40E00018
#define GPSR1   0x40E0001C
#define GPSR2   0x40E00020
#define GPCR0   0x40E00024
#define GPCR1   0x40E00028
#define GPCR2   0x40E0002C
#define GAFR0_L 0x40E00054
#define GAFR0_U 0x40E00058
#define GAFR1_L 0x40E0005C
#define GAFR1_U 0x40E00060
#define GAFR2_L 0x40E00064
#define GAFR2_U 0x40E00068

#define IN 250
#define OUT 251
#define GPIO 0
#define AF0 0
#define AF1 1
#define AF2 2
#define AF3 3
#define SET 252
#define CLEAR 253

typedef unsigned int u32;

void *map, *regaddr;
static void putmem(u32 addr, u32 val)
{
   regaddr = map + (addr & MAP_MASK);
   *(u32*) regaddr = val;
}
static int getmem(u32 addr)
{
    u32 val;

    regaddr = map + (addr & MAP_MASK);
    val = *(u32*) regaddr;
    return val;
}
void gpio_set(u32 gpio)
{
    u32 pos;
    u32 bit = 1;

    pos = gpio / 32;
    bit <<= gpio % 32;
    putmem(GPSR0 + (pos * 4), bit);
}
void gpio_clear(u32 gpio)
{
    u32 pos;
    u32 bit = 1;

    pos = gpio / 32;
    bit <<= gpio % 32;
    putmem(GPCR0 + (pos * 4), bit);
}
u32 gpio_status(u32 gpio)
{
    u32 pos;
    u32 bit = 1;
    u32 data;

    pos = gpio / 32;
    bit <<= gpio % 32;
    data = getmem(GPLR0 + (pos * 4));
    data &= bit;
    if (data == 0)
      return(0);
    else
      return(1);
}
void gpio_direction(u32 gpio, u32 dir)
{
    u32 pos;
    u32 bit = 1;
    u32 data;

    pos = gpio / 32;
    bit <<= gpio % 32;
    data = getmem(GPDR0 + (pos * 4));
    data &= ~bit;
    if (dir == OUT)
      data |= bit;
    putmem(GPDR0 + (pos * 4), data);
}
void gpio_function(u32 gpio, u32 fun)
{
    u32 pos;
    u32 bit = 3;
    u32 data;

    pos = gpio / 16;
    bit <<= (gpio % 16) * 2;
    fun <<= (gpio % 16) * 2;
    data = getmem(GAFR0_L + (pos * 4));
    data &= ~bit;
    data |= fun;
    putmem(GAFR0_L + (pos * 4), data);
}
u32 gpio(u32 dir, u32 set, u32 reg)
{
    if ((dir != IN) & (dir != OUT)){
      printf("ERROR: must specify a valid direction\n");
      return(1);
    }
    if ((set != SET) & (set != CLEAR)){
      printf("ERROR: must specify a valid level\n");
      return(1);
    }
    if (reg > 84){
      printf("ERROR: not a valid register -->%d\n", reg);
      return(1);
    }
    gpio_function(reg, GPIO);
    gpio_direction(reg, dir);
    if (dir == OUT){
      if (set == SET)
        gpio_set(reg);
      else
        gpio_clear(reg);
    }
    return(0);
}

int gpio_init() {
  // Initialize GPIOs, then SPI, and finally the OLED
    unsigned int i, ii, rval, speed, count, tmp;
    int fd;
    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd<0) {
        perror("open(\"/dev/mem\")");
        return 2;
    }
    map = mmap(0,
              MAP_SIZE,
              PROT_READ | PROT_WRITE,
              MAP_SHARED,
              fd,
#ifdef __ARM_EABI__
              0x40E00000 / MAP_SIZE
#else
              0x40E00000 & ~MAP_MASK
#endif
             );
    if (map == (void*)-1 ) {
       perror("mmap()");
       return 1;
    }
    return 0;
}
