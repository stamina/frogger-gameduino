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
#include <avr/pgmspace.h>
#include <SPI.h>
#include <GD.h>
#include "font8x8.h" // fixed width ascii font

// stretching for font
static byte stretch[16] = {
  0x00, 0x03, 0x0c, 0x0f,
  0x30, 0x33, 0x3c, 0x3f,
  0xc0, 0xc3, 0xcc, 0xcf,
  0xf0, 0xf3, 0xfc, 0xff
};

void vis_gd_begin()
{
  vis_spi_begin(); // start SPI init
  vis_spi_setclockdivider(SPI_CLOCK_DIV2_2X); // fastest clock speed
  vis_spi_setbitorder(SPI_MSBFIRST); // bit shift out from right side first
  vis_spi_setdatamode(SPI_MODE0); // Sample (Rising) Setup (Falling)
  vis_gd_wr(J1_RESET, 1); // HALT coprocessor
  vis_gd_wstart(RAM_SPR); // Hide all sprites
  for (uint16_t i = 0; i < 512; i++)
    vis_gd_xhide();
  vis_gd_end(); // end spi
  vis_gd_fill(RAM_PIC, 0, 1024 * 10); // Zero all character RAM from 0x0000 till 0x27ff, 10k
  vis_gd_fill(RAM_SPRPAL, 0, 2048); // Sprite palletes black, from 0x3800 till 0x3fff, 2k
  vis_gd_fill(RAM_SPRIMG, 0, 64 * 256); // Clear all sprite data from 0x4000 till 0x7ff, 16k
  vis_gd_fill(VOICES, 0, 256); // Silence, from 0x2a00 till 0x2aff, 256 bytes
  vis_gd_fill(PALETTE16A, 0, 128); // Black 16-, 4-palletes and COMM, from 0x2840 till 0x28FF, 128 bytes
  vis_gd_wr16(SCROLL_X, 0); // set orientation to x=0
  vis_gd_wr16(SCROLL_Y, 0); // set orientation to y=0
  vis_gd_wr(JK_MODE, 0); // sprite collision mode
  vis_gd_wr(SPR_DISABLE, 0); // enable sprite display
  vis_gd_wr(SPR_PAGE, 0); // use sprite page 0
  vis_gd_wr(IOMODE, 0); // pin 2 I/O not used
  vis_gd_wr16(BG_COLOR, 0); // background to black
  vis_gd_wr16(SAMPLE_L, 0); // audio left sample
  vis_gd_wr16(SAMPLE_R, 0); // audio right sample
  vis_gd_wr(MODULATOR, 64); // disable ring modulator
  vis_gd_wr16(SCREENSHOT_Y, 0); // screenshot line select to 0
}

// write 1 byte to a certain address
void vis_gd_wr(unsigned int addr, byte v)
{
  vis_gd_wstart(addr);
  vis_spi_transfer(v);
  vis_gd_end();
}

// start an spi WRITE transaction to addr
void vis_gd_wstart(unsigned int addr)
{
  vis_gd_start(0x8000|addr); // note the 16th bit indicates the write flag
}

// start an spi transaction to addr
void vis_gd_start(unsigned int addr)
{
  SPI_PORT &= ~(1 << SPI_SS_PIN); // start communicating with SPI slave
  vis_spi_transfer(highByte(addr));
  vis_spi_transfer(lowByte(addr));
}

// end the SPI transaction
void vis_gd_end()
{
  SPI_PORT |= (1 << SPI_SS_PIN); // stop communicating with SPI slave
}

// write 4 bytes to Sprite Control dwords to in RAM_SPR to set x/y pos to 400: offscreen
void vis_gd_xhide()
{
  vis_spi_transfer(lowByte(400));
  vis_spi_transfer(highByte(400));
  vis_spi_transfer(lowByte(400));
  vis_spi_transfer(highByte(400));
  current_spr++;
}

// start a write at a sprite position
void vis_gd_wstartspr(unsigned int sprnum)
{
  vis_gd_start((0x8000 | RAM_SPR) + (sprnum << 2));
  current_spr = 0;
}

// read 1 byte starting from addr
byte vis_gd_rd(unsigned int addr)
{
  vis_gd_start(addr);
  byte r = vis_spi_transfer(0);
  vis_gd_end();
  return r;
}

// read 2 bytes starting from addr
unsigned int vis_gd_rd16(unsigned int addr)
{
  unsigned int r;
  vis_gd_start(addr);
  r = vis_spi_transfer(0);
  r |= (vis_spi_transfer(0) << 8);
  vis_gd_end();
  return r;
}

// reads the x-pos part of the sprite control line
unsigned int vis_gd_rd_spr_xpos(unsigned int address)
{
  int posx = vis_gd_rd16(RAM_SPR + (address << 2));
  return posx &= 0x1ff;
}

// reads the y-pos part of the sprite control line
unsigned int vis_gd_rd_spr_ypos(unsigned int address)
{
  int posy = vis_gd_rd16(RAM_SPR + (address << 2) + 2);
  return posy &= 0x1ff;
}

// reads the image part of the sprite control line
byte vis_gd_rd_spr_image(unsigned int address)
{
  byte spr_image = vis_gd_rd(RAM_SPR + (address << 2) + 3);
  return (spr_image >> 1) & 0x3f;
}

// reads the palette part of the sprite control line
byte vis_gd_rd_spr_pal(unsigned int address)
{
  byte spr_pal = vis_gd_rd(RAM_SPR + (address << 2) + 1);
  return (spr_pal >> 4);
}

// write 2 bytes
void vis_gd_wr16(unsigned int addr, unsigned int v)
{
  vis_gd_wstart(addr);
  vis_spi_transfer(lowByte(v));
  vis_spi_transfer(highByte(v));
  vis_gd_end();
}

// write count*bytes starting from addr
void vis_gd_fill(int addr, byte v, unsigned int count)
{
  vis_gd_wstart(addr);
  while (count--)
    vis_spi_transfer(v);
  vis_gd_end();
}

#if defined(__AVR_ATmega1284P__) || defined(__AVR_AT90USB1286__)
// copies arrays of bytes (GD background pic, char, pal) to the GD via spi
void vis_gd_copy(unsigned int addr, uint_farptr_t src, int count)
{
  vis_gd_wstart(addr);
  while (count--) {
    vis_spi_transfer(pgm_read_byte_far(src)); // Read a byte from the program space with a 32-bit (far) address and transfer it to the GD via spi
    src++;
  }
  vis_gd_end();
}
#else
void vis_gd_copy(unsigned int addr, PROGMEM prog_uchar *src, int count)
{
  vis_gd_wstart(addr);
  while (count--) {
    SPI.transfer(pgm_read_byte_near(src));// Read a byte from the program space with a 16-bit (near) address and transfer it to the GD via spi
    src++;
  }
  vis_gd_end();
}
#endif

// set color to palette address
void vis_gd_setpal(int pal, unsigned int rgb)
{
  vis_gd_wr16(RAM_PAL + (pal << 1), rgb);
}

// write sprite control lines, 32 bits, mostly used for single line, fixed address writing
void vis_gd_sprite(int spr, int x, int y, byte image, byte palette, byte rot, byte jk)
{
  vis_gd_wstart(RAM_SPR + (spr << 2));
  vis_spi_transfer(lowByte(x));
  vis_spi_transfer((palette << 4) | (rot << 1) | (highByte(x) & 1));
  vis_spi_transfer(lowByte(y));
  vis_spi_transfer((jk << 7) | (image << 1) | (highByte(y) & 1));
  vis_gd_end();
}

// write sprite control lines, 32 bits, mostly used for bulk writing
void vis_gd_xsprite(int ox, int oy, int x, int y, byte image, byte palette, byte rot, byte jk)
{
  if (rot & 2)
    x = -16-x;
  if (rot & 4)
    y = -16-y;
  if (rot & 1) {
    int s;
    s = x; x = y; y = s;
  }
  ox += x;
  oy += y;
  vis_spi_transfer(lowByte(ox));
  vis_spi_transfer((palette << 4) | (rot << 1) | (highByte(ox) & 1));
  vis_spi_transfer(lowByte(oy));
  vis_spi_transfer((jk << 7) | (image << 1) | (highByte(oy) & 1));
  current_spr++;
}

// Wait for the VBLANK to go from 0 to 1: this is the start
// of the vertical blanking interval.
void vis_gd_waitvblank()
{
  while (vis_gd_rd(VBLANK) == 1)
    ;
  while (vis_gd_rd(VBLANK) == 0)
    ;
}

void vis_gd_ascii()
{
  long i;
  for (i = 0; i < 768; i++) {
#if defined(__AVR_ATmega1284P__) || defined(__AVR_AT90USB1286__)
    byte b = pgm_read_byte_far(GET_FAR_ADDRESS(font8x8) + i);
#else
    byte b = pgm_read_byte(font8x8 + i);
#endif
    byte h = stretch[b >> 4];
    byte l = stretch[b & 0xf];
    vis_gd_wr(0x1000 + (16 * ' ') + (2 * i), h); // RAM_CHR offset
    vis_gd_wr(0x1000 + (16 * ' ') + (2 * i) + 1, l);
  }
  for (i = 0x20; i < 0x80; i++) {
    vis_gd_setpal(4 * i + 0, TRANSPARENT);
    vis_gd_setpal(4 * i + 3, RGB(255,255,255)); // white color
  }
  vis_gd_fill(RAM_PIC, ' ', 4096); // write empty spaces
}

// put string into SCREEN RAM 64x64 bytes
void vis_gd_putstr(int x, int y, const char *s)
{
  vis_gd_wstart((y << 6) + x); // offset is RAM_PIC, i.e. 0x0000 
  while (*s)
    vis_spi_transfer(*s++);
  vis_gd_end();
}

void vis_gd_voice(int v, byte wave, unsigned int freq, byte lamp, byte ramp)
{
  vis_gd_wstart(VOICES + (v << 2));
  vis_spi_transfer(lowByte(freq));
  vis_spi_transfer(highByte(freq) | (wave << 7));
  vis_spi_transfer(lamp);
  vis_spi_transfer(ramp);
  vis_gd_end();
}

#if defined(__AVR_ATmega1284P__) || defined(__AVR_AT90USB1286__) // far ptr version
void vis_gd_fb_begin(uint_farptr_t s) {
  g_flashbits_src = s;
  g_flashbits_mask= 0x01;
}

byte vis_gd_fb_get1() {
  byte r = (pgm_read_byte_far(g_flashbits_src) & g_flashbits_mask) != 0;
  g_flashbits_mask <<= 1;
  if (!g_flashbits_mask) {
    g_flashbits_mask = 1;
    g_flashbits_src++;
  }
  return r;
}

unsigned short vis_gd_fb_getn(byte n) {
  unsigned short r = 0;
  while (n--) {
    r <<= 1;
    r |= vis_gd_fb_get1();
  }
  return r;
}

void vis_gd_uncompress(unsigned int addr, uint_farptr_t src)
{
  vis_gd_fb_begin(src);
  byte b_off = vis_gd_fb_getn(4);
  byte b_len = vis_gd_fb_getn(4);
  byte minlen = vis_gd_fb_getn(2);
  unsigned short items = vis_gd_fb_getn(16);
  while (items--) {
    if (vis_gd_fb_get1() == 0) {
      vis_gd_wr(addr++, vis_gd_fb_getn(8));
    } else {
      int offset = -vis_gd_fb_getn(b_off) - 1;
      int l = vis_gd_fb_getn(b_len) + minlen;
      while (l--) {
        vis_gd_wr(addr, vis_gd_rd(addr + offset));
        addr++;
      }
    }
  }
}

#endif

