/*
 * Original Gameduino library for Arduino by
 * James Bowman <jamesb@excamera.com>
 * 
 * Created by Bas Brugman
 * http:// www.visionnaire.nl 2011
 *
 * Copyright (c) 2011, Bas Brugman
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by the <organization>.
 * 4. Neither the name of the <organization> nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef _GD_H_INCLUDED
#define _GD_H_INCLUDED

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <util/delay.h>
#include "usart.h"

// proto's
void vis_gd_begin(void);
void vis_gd_wr(unsigned int addr, byte v);
void vis_gd_wstart(unsigned int addr);
void vis_gd_wstartspr(unsigned int sprnum);
void vis_gd_start(unsigned int addr);
void vis_gd_end(void);
void vis_gd_xhide(void);
byte vis_gd_rd(unsigned int addr);
unsigned int vis_gd_rd16(unsigned int addr);
unsigned int vis_gd_rd_spr_xpos(unsigned int address);
unsigned int vis_gd_rd_spr_ypos(unsigned int address);
byte vis_gd_rd_spr_image(unsigned int address);
byte vis_gd_rd_spr_pal(unsigned int address);
void vis_gd_fill(int addr, byte v, unsigned int count);
void vis_gd_wr16(unsigned int addr, unsigned int v);
void vis_gd_ascii(void);
void vis_gd_setpal(int pal, unsigned int rgb);
void vis_gd_putstr(int x, int y, const char *s);
void vis_gd_sprite(int spr, int x, int y, byte image, byte palette, byte rot, byte jk);
void vis_gd_xsprite(int ox, int oy, int x, int y, byte image, byte palette, byte rot, byte jk);
void vis_gd_waitvblank(void);
#if defined(__AVR_ATmega1284P__) || defined(__AVR_AT90USB1286__)
void vis_gd_copy(unsigned int addr, uint_farptr_t src, int count);
uint_farptr_t g_flashbits_src;
byte g_flashbits_mask;
void vis_gd_fb_begin(uint_farptr_t s);
byte vis_gd_fb_get1(void);
unsigned short vis_gd_fb_getn(byte n);
void vis_gd_uncompress(unsigned int addr, uint_farptr_t src);
#else
void vis_gd_copy(unsigned int addr, prog_uchar *src, int count);
#endif
void vis_gd_voice(int v, byte wave, unsigned int freq, byte lamp, byte ramp);

// GD's globals
byte current_spr; // Current sprite nr, incremented by xsprite/xhide above
byte current_frame; // active frame buffer 0/1

// GD's constants
#define RGB(r,g,b)    ((((r) >> 3) << 10) | (((g) >> 3) << 5) | ((b) >> 3)) // RGB color conversion according ARGB1555 format
#define TRANSPARENT   (1 << 15) // transparent bit for char color palette
#define RAM_PIC       0x0000 // Screen Picture, 64 x 64 chars of 8x8 pixels = 4096 bytes (512x512, 400x300 visible)
#define RAM_CHR       0x1000 // Screen Characters, 256 x 16 bytes = 4096 bytes
#define RAM_PAL       0x2000 // Screen Character Palette, 256 x 8 bytes (4 colors x 2 bytes) = 2048 bytes
#define IDENT         0x2800 // Gameduino identification—always reads as 0x6D 
#define REV           0x2801 // Gameduino revision, reads 0x10 atm
#define FRAME         0x2802 // Frame counter, increments at the end of each displayed frame 
#define VBLANK        0x2803 // Set to 1 during the video blanking period 
#define SCROLL_X      0x2804 // Horizontal background scrolling register, 0–511 
#define SCROLL_Y      0x2806 // Vertical background scrolling register, 0–511 
#define JK_MODE       0x2808 // Sprite collision class mode enable 0–1 
#define J1_RESET      0x2809 // reset Co-proc
#define SPR_DISABLE   0x280a // En-/Disable sprite display
#define SPR_PAGE      0x280b // Sprite page 0 or 1, used for double buffering
#define IOMODE        0x280c // pin 2 control mode
#define BG_COLOR      0x280e // background color
#define SAMPLE_L      0x2810 // audio left sample
#define SAMPLE_R      0x2812 // audio right sample
#define MODULATOR     0x2814 // ring modulator, voice used for modulation, 1 byte
#define SCREENSHOT_Y  0x281e // screenshot line select to 0-299
#define PALETTE16A    0x2840 // 16-color palette RAM A, 32 bytes
#define PALETTE16B    0x2860 // 16-color palette RAM B, 32 bytes
#define PALETTE4A     0x2880 // 4-color palette RAM A, 8 bytes
#define PALETTE4B     0x2888 // 4-color palette RAM B, 8 bytes
#define COMM          0x2890 // Communication buffer, 6 bytes, Co-processor communication block
#define COLLISION     0x2900 // Collision detection RAM, 256 bytes
#define VOICES        0x2a00 // Voice controls
#define J1_CODE       0x2b00 // J1 coprocessor microcode RAM
#define SCREENSHOT    0x2c00 // screenshot line RAM, 800 bytes
#define RAM_SPR       0x3000 // Sprite Control, 2 pages of 256 x 4 = 2048 bytes, 0x3000 - 0x33ff, 0x3400 - 0x37ff
#define RAM_SPRPAL    0x3800 // Sprite Palettes, 4 x 256 = 2048 bytes
#define RAM_SPRIMG    0x4000 // Sprite Image, 64 x 256 = 16384 bytes

#ifndef GET_FAR_ADDRESS // at some point this will become official... https://savannah.nongnu.org/patch/?6352
#if defined(__AVR_ATmega1284P__) || defined(__AVR_AT90USB1286__)
#define GET_FAR_ADDRESS(var)                          \
({                                                    \
    uint_farptr_t tmp;                                \
                                                      \
    __asm__ __volatile__(                             \
                                                      \
            "ldi    %A0, lo8(%1)"           "\n\t"    \
            "ldi    %B0, hi8(%1)"           "\n\t"    \
            "ldi    %C0, hh8(%1)"           "\n\t"    \
            "clr    %D0"                    "\n\t"    \
        :                                             \
            "=d" (tmp)                                \
        :                                             \
            "p"  (&(var))                             \
    );                                                \
    tmp;                                              \
}) 
#else
#define GET_FAR_ADDRESS(var) (var)
#endif
#endif


#endif

