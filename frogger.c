/*
 * Frogger "Brugmania" Arcade version for Gameduino
 *
 * Based on example code of http://excamera.com/sphinx/gameduino/
 * by James Bowman
 *
 * Sprite Sheet credits go to GaryCXJk
 *
 * Totally recreated by by Bas Brugman
 * http:// www.visionnaire.nl 2011
 *
 * Original Frogger by Konami (c) 1981 / Distributed by SEGA
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
//#define VIS_DEBUG // use for debugging over UART
#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n)) // definition of the mcu's prescaler, use 0 to go full speed

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SPI.h" // Serial Peripheral Interface, serial communication used for talking to the Gameduino shield
#include "GD.h" // GameDuino library with SPI wrapper functions and definitions of all functions to memory addresses
#include "scores.h" // Highscores & Achievement functionally
#include "frogger.h" // header file with prototypes and declarations of the main functionality
#include "font8x8.h" // Old Skool 8x8 pixel font, useful for debugging and standard text in games
#include "froggerbg.h" // Frogger's background characters: 8x8 px
#include "sprite.h" // Frogger's sprite data: 16x16 px tiles 
#include "control.h" // Joysticks and buttons control definitions
#include <util/delay.h> // avr's delay functions

#ifdef VIS_DEBUG
#include "usart.h" // UART Serial library, used for debugging over RX/TX pins
#endif

// write voices
static void squarewave(uint16_t freq, byte amp)
{
  vis_gd_voice(0, 0, freq,     amp,    amp);
  vis_gd_voice(1, 0, 3 * freq, amp/3,  amp/3);
  vis_gd_voice(2, 0, 5 * freq, amp/5,  amp/5);
  vis_gd_voice(3, 0, 7 * freq, amp/7,  amp/7);
  vis_gd_voice(4, 0, 9 * freq, amp/9,  amp/9);
  vis_gd_voice(5, 0, 11 * freq, amp/11,  amp/11);
}

// generated jumping and dying sounds
static void sound()
{
  byte note;
  if (p1_dying > 0 && p1_dying < 65 && !godmode) { 
    note = 84 - (p1_dying / 2);
    squarewave(MIDI(note), 100);
  } else if (p2_dying > 0 && p2_dying < 65) { 
    note = 84 - (p2_dying / 2);
    squarewave(MIDI(note), 100);
  } else if (p1_leaping) {
    if (p1_leaping & 1)
      note = 60 + p1_leaping;
    else
      note = 72 + p1_leaping;
    squarewave(MIDI(note), 100);
  } else if (p2_leaping) {
    if (p2_leaping & 1)
      note = 72 + p2_leaping;
    else
      note = 84 + p2_leaping;
    squarewave(MIDI(note), 100);
  } else if (game && level_timer > 2592 && level_timer < 3240) {
    note = (level_timer / 10);
    squarewave(MIDI(note), 100);
  } else {
    squarewave(0, 0);  // silence
  }
}

// draws general sprites, like moving cars
static void draw_general_sprite(uint16_t x, uint16_t y, byte anim, byte rot, byte jk)
{
  draw_sprite(x + 80, y + 24, anim, rot, jk);
}

// draws general sprites for the intro, other offsets needed
static void draw_general_intro_sprite(uint16_t x, uint16_t y, byte anim, byte rot, byte jk)
{
  draw_sprite(x + 8, y + 8, anim, rot, jk);
}

// draws, aligns and colors textual content
static void draw_text(const char *text, uint16_t x, uint16_t y, uint16_t color, const byte align)
{
  if (align == ALIGN_CENTER) {
    vis_gd_putstr((byte)((50 - strlen(text)) / 2), y, text);
  } else {
    vis_gd_putstr(x, y, text);
  }
}

// inits the pointers to the message queue
static void msg_init(uint8_t *head, uint8_t *tail)
{
  *head = *tail = 0;
}

// adds a new text message to the queue 
static void msg_enqueue(MsgT *q, uint8_t *tail, MsgT msg)
{	
  if (!msg_full(*tail, MSG_SIZE - 1))
    q[(*tail)++] = msg;
}

// return the next text message
static MsgT msg_dequeue(MsgT *q, uint8_t *head)
{	
  return q[(*head)++];
}

// checks if message queue is full 
static uint8_t msg_full(uint8_t tail, const uint8_t size)
{
    return tail == size ? 1 : 0;
}

// check if message queue is empty
static uint8_t msg_empty(uint8_t head, uint8_t tail)
{	
   return head == tail ? 1 : 0;
}

// display messages from the queue
static void display_msg()
{
  if (displaymsg) {
    if ((!strcmp(current_msg.display_text, GAME_INSECTBONUS)) || (!strcmp(current_msg.display_text, GAME_FEMALEBONUS))) { // show a sprite
      if (current_msg.display_time == GAME_TEXTTIMESHORT) {
        vis_gd_wstartspr(bonus200spr);
        draw_general_sprite(bonusx, 24, 61, 0, 0);
        vis_gd_end();
        vis_gd_wstartspr(256 + bonus200spr);
        draw_general_sprite(bonusx, 24, 61, 0, 0);
        vis_gd_end();
        insecthome = 0; // hide insect during bonus msg
      } else if (current_msg.display_time == 0) {
        vis_gd_wstartspr(bonus200spr);
        draw_general_sprite(400, 340, 61, 0, 0); // offscreen
        vis_gd_end();
        vis_gd_wstartspr(256 + bonus200spr);
        draw_general_sprite(400, 340, 61, 0, 0); // offscreen
        vis_gd_end();
        displaymsg = 0;
      }
    } else if (current_msg.display_time == GAME_TEXTTIME) {
      draw_text(current_msg.display_text, 20, 20, 0, ALIGN_CENTER);
    } else if (current_msg.display_time == 0) {
      bank_clear();
      displaymsg = 0;
    }
    current_msg.display_time--; // keep showing until time == 0
  }
  if (!displaymsg && (!msg_empty(msg_head, msg_tail))) { // get the next msg
    current_msg = msg_dequeue(msgs, &msg_head);
    displaymsg = 1;
  }
}

// gives the ascii chars a different color
static void color_chars(unsigned int rgb, byte type)
{
  uint16_t start = 0x20, end = 0x80;
  if (type == COLOR_UPCASE) {
    start = 0x41;
    end = 0x5B;
  } else if (type == COLOR_LOWERCASE) {
    start = 0x61;
    end = 0x7B;
  }
  for (uint16_t i = start; i < end; i++) {
    vis_gd_setpal(4 * i + 0, TRANSPARENT);
    vis_gd_setpal(4 * i + 3, rgb);
  }
}
 
// shrinks the green timer, 3 secs per shrink
// last 9 secs are shown as a red bar
static void draw_level_timer() 
{
  if (!level_timer) { // reset to full green bar
    for (byte i = 0; i < 15; i++)
      vis_gd_wr(atxy(i + 9, 31), BG_GREEN);
  } else if (level_timer >= 2592) { // 9 secs left, red bar
    for (byte i = 12; i < 15; i++)
      vis_gd_wr(atxy(i + 9, 31), BG_RED);
  }
  byte passed_secs =  (level_timer / 72);
  for (byte i = 0; i < (passed_secs / 3); i++) {
    // when the timer bar is empty (15 x 3secs) and msgs are still shown, we dont want to keep adding
    // black char blocks 
    if (i < 15)
      vis_gd_wr(atxy(i + 9, 31), BG_BLACK);
  }
}

// checks for an extra life every 20k points
static void check_score(long *score, byte *prev_score_divisor, byte *lives) {
  if (*score / 20000 != *prev_score_divisor) {
    (*lives)++;
    *prev_score_divisor = *score / 20000;
    MsgT extra = { GAME_TEXTTIME, GAME_EXTRA };
    msg_enqueue(msgs, &msg_tail, extra);
  }
}

// draws the "lives left" sprites exluding the current active frog
static void draw_lives() 
{
  // first clear complete life row
  for (byte i = 0; i < 28; i++)
    vis_gd_wr(atxy(i, 30), BG_BLACK);
  // player 1
  for (byte i = 0; i < p1_lives - 1; i++)
    vis_gd_wr(atxy(i, 30), BG_LIFE);
  if (gamemode) {
    // player 2
    for (byte i = 27; i > 28 - p2_lives; i--)
      vis_gd_wr(atxy(i, 30), BG_LIFE);
  }
}

// returns the frog's current location
static byte location(uint16_t x, uint16_t y)
{
  if (x < 8 || x > 232 || (y > 232 && y <= 248)) { // touching screen edges: left, right, bottom
    return LOC_EDGE;
  } else if (y <= 40) {
    return LOC_HOME;
  } else if (y > 40 && y < 136) {
    return LOC_RIVER;
  } else if (y == 136) {
    return LOC_MIDDLE_BANK;
  } else if (y > 136 && y < 232) {
    return LOC_ROAD;
  } else if (y == 232) {
    return LOC_BOTTOM_BANK;
  }
  return LOC_UNKNOWN; // used for waiting on screen text msgs in between levels
}

// drawing black sprites to hide objects that outrun the center playing field
// on the left and the right
static void draw_curtains()
{
  byte i = 255; // spr page 0 
  uint16_t j = 255 + 256; // spr page 1 
  // bottoms
  for (int z = 56; z <= 312; z += 256) {
    for (int y = 152; y < 256; y += 16) {
      for (int x = 0; x < 32; x += 16) {
        vis_gd_sprite(i--, x + z, y, 63, 0, 0, 0);
        vis_gd_sprite(j--, x + z, y, 63, 0, 0, 0);
      }
    }
  }
  // tops
  for (int z = -8; z <= 320; z += 320) {
    for (int y = 72; y < 152; y += 16) {
      for (int x = 0; x < 96; x += 16) {
        vis_gd_sprite(i--, x + z, y, 63, 0, 0, 0);
        vis_gd_sprite(j--, x + z, y, 63, 0, 0, 0);
      }
    }
  }
}

// returns the background chars video address with taking centering the screen into account
// x-offset of 11 (400-224px=176/2/8px char_width)
// y-offset of 3 (300-256px=44/2/8px char_height)
static uint16_t atxy(byte x, byte y)
{
  return RAM_PIC + 64 * (y + 3) + (x + 11);
}

// displays the current score on screen
static void draw_score(uint16_t dst, long n)
{
  vis_gd_wr(dst + 0, BG_ZERO + (n / 100000) % 10);  // hundred-thousands
  vis_gd_wr(dst + 1, BG_ZERO + (n / 10000) % 10);   // ten-thousands
  vis_gd_wr(dst + 2, BG_ZERO + (n / 1000) % 10);    // thousands
  vis_gd_wr(dst + 3, BG_ZERO + (n / 100) % 10);     // hundreds
  vis_gd_wr(dst + 4, BG_ZERO + (n / 10) % 10);      // tens
  vis_gd_wr(dst + 5, BG_ZERO + n % 10);             // ones
}

// calculates the positive or negative speed (x-pos) according the y-pos of the road object
// this changes per 4 levels: speed increase with t/2
static int roadat(byte y, uint16_t tt)
{
  switch (y) {
    case 152: return ((-tt / 2) * speed_factor); // trucks
    case 168: return (tt * speed_factor); // race cars
    case 184: return ((-tt / 2)* (speed_factor)); // normal purple
    case 200: return ((tt / 2) * speed_factor); // bulldozers
    case 216: return (-tt * speed_factor); // normal yellow
  }
  return tt;
}

// calculates the positive or negative speed (x-pos) according the y-pos of the river object
// this can change per level
static int riverat(byte y, uint16_t tt)
{
  switch (y) {
    case 56: return (tt / 2); // top logs, don't change this speed, else it's impossible to get far
    case 72: return ((-tt / 2) * speed_factor); // top turtles
    case 88: return (tt * speed_factor); // middle logs
    case 104: return ((tt / 2) * speed_factor); // bottom logs
    case 120: return (-tt / 2); // bottom turtles, don't change this speed, else it's impossible to get far
  }
  return tt;
}

// draws the cars
// amount is number of cars
// anim is the sprite number
// spacing is pixels between each car
// x is horizontal positive or negative travel speed, based on t
// y is y position
// make sure the interval is always 224 + car and space, so it travels correctly in and out off the screen
static void set_cars(byte amount, byte anim, byte spacing, uint16_t x, byte y)
{
  x = roadat(y, x); // calculate speed
  uint16_t space = spacing + 16;
  uint16_t interval = amount * space;
  uint16_t l_x = x;
  for (byte i = 0; i < amount; i++) {
    if (i > 0) {
      if (l_x >= (i * space)) { // check if it's time to start a new object/group
        x = l_x - (i * space);
        draw_cars(x, y, anim, interval);
      }
    } else {
      draw_cars(x, y, anim, interval);
    }
  }
}

// draws a car with a certain interval
static void draw_cars(uint16_t x, byte y, byte anim, uint16_t interval)
{
  x %= interval;
  if ( (x >= 0) && (x < 240)) {
    draw_general_sprite(x, y, anim, 0, 0); // draw actual car
  } else {
    draw_general_sprite(x, y, 9, 0, 0); // ran out of middle part, show black sprite
  }
}

// draws the turtle groups
// amount is number of groups
// length is group length
// spacing is pixels between each group
// x is horizontal positive or negative travel speed, based on t
// y is y position
// make sure the interval is always 224 + group length, so it travels correctly in and out off the screen
static void set_turtles(byte amount, byte length, byte spacing, uint16_t x, byte y)
{
  x = riverat(y, x); // calculate speed
  uint16_t space = (spacing + (length * 16));
  uint16_t interval = amount * space;
  uint16_t l_x = x;
  byte l_length = length;
  for (byte i = 0; i < amount; i++) {
    if (i > 0) {
      if (l_x >= (i * space)) { // check if it's time to start a new object/group
        x = l_x - (i * space);
        draw_turtles(x, y, length, interval, i);
      }
    } else {
      draw_turtles(x, y, length, interval, i);
    }
    length = l_length; // reset length for next possible log
  }
}

// draws an animated turtle group, with a certain interval
static void draw_turtles(uint16_t x, byte y, byte length, uint16_t interval, byte diving)
{
  byte anim;
  if ((diving == turtledive1 && y == 120) || (diving == turtledive2 && y == 72 )) {
    anim = diving_anim[(t / 32) % 8];
  } else {
    anim = 50 + ((t / 32) % 3); // swap sprite 50, 51, 52 every 32 t's
  }
  x %= interval;
  for (byte i = 0; i < length; i++) {
    draw_general_sprite(x, y, anim, 0, 0);
    x -= 16;
  }
}

// draws the log groups
// amount is number of groups
// length is group length
// spacing is pixels between each group
// x is horizontal positive or negative travel speed, based on t
// y is y position
// make sure the interval is always 224 + group length, so it travels correctly in and out off the screen
static void set_logs(byte amount, byte length, byte spacing, uint16_t x, byte y)
{
  x = riverat(y, x); // calculate speed 
  uint16_t space = (spacing + (32 + (length * 16)));
  uint16_t interval = amount * space;
  uint16_t l_x = x;
  byte l_length = length;
  for (byte i = 0; i < amount; i++) {
    if (i > 0) {
      if (l_x >= (i * space)) { // check if it's time to start a new log
        x = l_x - (i * space);
        // randomly show a croc or normal log in top river lane
        (l_length == 2 && level > 1 && croclog == i) ? draw_croc(x, y, interval) : draw_logs(x, y, length, interval);
        // give snake an x pos
        if ((l_length == 4) && (level > 2) && (snakelog == i))
          snakelogx = x % interval;
        // give female an x pos
        if ((l_length == 1) && (femalelog == i))
          femalelogx = x % interval;
      }
    } else {
      // randomly show a croc or normal log in top river lane
      (l_length == 2 && level > 1 && croclog == i) ? draw_croc(x, y, interval) : draw_logs(x, y, length, interval);
      // give snake an x pos
      if ((l_length == 4) && (level > 2) && (snakelog == i))
        snakelogx = x % interval;
      // give female an x pos
      if ((l_length == 1) && (femalelog == i))
        femalelogx = x % interval;
    }
    length = l_length; // reset length for next possible log
  }
}

// draws a log, with a certain interval
static void draw_logs(uint16_t x, byte y, byte length, uint16_t interval)
{
  x %= interval;
  draw_general_sprite(x, y, 88, 0, 0); // front
  // otter pops up on the left half, goes to the right, from the front log sprite,
  // after a collision, otter = 4 to swap direction for next otter
  // the y-pos check makes sure the otter switches lanes frequently
  if (x >= 0 && x <= 112 && otter == 3 && (t % 512 == 0) && ottery != y) { 
    otter = 1;
    otterx = x + 22;
    ottery = y;
  }
  // otter pops up on the right half, turns left, from a tail log sprite,
  // after a collision, otter = 3 to swap direction for next otter
  // the y-pos check makes sure the otter switches lanes frequently
  if (x > 112 && x <= 224 && otter == 4 && (t % 512 == 0) && ottery != y) {
    otter = 2;
    otterx = x - (length * 16) - 16 - 22;
    ottery = y;
  }
  while (length--) { // middle pieces
    x -= 16;
    draw_general_sprite(x, y, 87, 0, 0);
  }
  draw_general_sprite(x - 16, y, 86, 0, 0); // tail
}

// draws a croc, with a certain interval
static void draw_croc(uint16_t x, byte y, uint16_t interval)
{
  x %= interval;
  byte anim = 72 + ((t / 100) % 2); // swap open or closed mouth
  draw_general_sprite(x, y, anim, 0, 0); // head
  draw_general_sprite(x - 16, y, 71, 0, 0);
  draw_general_sprite(x - 32, y, 70, 0, 0); // tail
}

// draws a snake, can appear on the longest logs
// touching their heads is deadly
static void draw_log_snake()
{
  if (snakelogx) {
    uint16_t x = snakelogx;
    uint16_t y = 88;
    byte anim[6] = {80, 82, 84, 81, 83 ,85};
    vis_gd_wstartspr((current_frame ? 0 + snaketailspr : 256 + snaketailspr)); // start at tail slot
    byte head = anim[((t / 10) % 3)];
    byte tail = anim[((t / 10) % 3) + 3];
    uint16_t xoffset = ((t / 3) % 161); // 2 * 80px max
    if (xoffset < 81) {
      x -= xoffset;  // move as far as 80 pixels (log length) every 3 t's to the left
      draw_general_sprite(x, y, head, 0, 0); // head
      draw_general_sprite(x + 16, y, tail, 0, 0); // tail
    } else {
      x = x - 160 + xoffset; // move as far as 80 pixels (log length) every 3 t's to the right again
      draw_general_sprite(x, y, head, 2, 0); // head
      draw_general_sprite(x - 16, y, tail, 2, 0); // tail
    }
    vis_gd_end();
  }
}

// draws a snake, can appear on the river bank
// touching their heads is deadly
static void draw_bank_snake()
{
  uint16_t x = 0;
  uint16_t y = 136;
  byte anim[6] = {80, 82, 84, 81, 83 ,85};
  vis_gd_wstartspr((current_frame ? 0 + snaketailspr2 : 256 + snaketailspr2)); // start at tail slot
  byte head = anim[(t / 10) % 3];
  byte tail = anim[((t / 10) % 3) + 3];
  uint16_t xoffset = ((t / 3) % 480); // 3 times as slow
  if (xoffset < 241) {
    draw_general_sprite(x + xoffset, y, head, 2, 0); // head
    draw_general_sprite(x + xoffset - 16, y, tail, 2, 0); // tail
  } else {
    draw_general_sprite(x + 480 - xoffset, y, head, 0, 0); // head
    draw_general_sprite(x + 480 + 16 - xoffset, y, tail, 0, 0); // tail
  }
  vis_gd_end();
}

// draws a female frog, leaping on a log to be picked up
static void draw_female()
{
  if (femalelogx) {
    uint16_t x = femalelogx;
    uint16_t y = 104;
    if (p1_female) {
      vis_gd_wstartspr((current_frame ? 0 + female_spr : 256 + female_spr)); // hide female below
      vis_gd_xhide();
      vis_gd_end();
      vis_gd_wstartspr((current_frame ? 0 + femalecarry_spr : 256 + femalecarry_spr)); // show female on top
      x = p1_frogx - 1;
      y = p1_frogy + 1;
      draw_general_sprite(x, y, frog_anim_pink[0], p1_frogface, 0);
    } else if (p2_female) {
      vis_gd_wstartspr((current_frame ? 0 + female_spr : 256 + female_spr)); // hide female below
      vis_gd_xhide();
      vis_gd_end();
      vis_gd_wstartspr((current_frame ? 0 + femalecarry_spr : 256 + femalecarry_spr)); // show female on top
      x = p2_frogx - 1;
      y = p2_frogy + 1;
      draw_general_sprite(x, y, frog_anim_pink[0], p2_frogface, 0);
    } else {
      vis_gd_wstartspr((current_frame ? 0 + femalecarry_spr : 256 + femalecarry_spr)); // hide female on top
      vis_gd_xhide();
      vis_gd_end();
      vis_gd_wstartspr((current_frame ? 0 + female_spr : 256 + female_spr));
      uint16_t xoffset = ((t % 72) * 2) + 2;
      byte interval = (t / 72) % 8;
      static byte female_frogface[] = {0, 3, 3, 3, 0, 5, 5, 5, 0}; // face directions
      if ((interval == 2) || (interval == 3)) {
        if (xoffset <= 16) { // at second 2 and 3 she leaps to the left
          female_leaping++;
          x -= xoffset;
          x -= (interval == 2) ? 0 : 16;
        } else {
          x -= 16;
          x -= (interval == 2) ? 0 : 16;
        }
      } else if ((interval == 6) || (interval == 7)) {
        if (xoffset <= 16) { // at second 6 and 7 she leaps to the right
          female_leaping++;
          x -= 32;
          x += xoffset;
          x += (interval == 6) ? 0 : 16;
        } else {
          x -= 16;
          x += (interval == 6) ? 0 : 16;
        }
      } else if ((interval == 4) || (interval == 5)) { // stay on the left side of the log
          x -= 32;
          female_leaping = 0;
      } else {
        female_leaping = 0;
      }
      if (female_leaping >= 8) {
        female_leaping = 0;
      }
      draw_general_sprite(x, y, frog_anim_pink[female_leaping / 2], female_frogface[interval], 0);
    }
    vis_gd_end();
  }
}

// draws an otter
// touching them or getting flanked by them is deadly
static void draw_otter()
{
  vis_gd_wstartspr((current_frame ? 0 + otterspr : 256 + otterspr));
  byte anim = 74 + ((t / 72) % 2); // swap sprite every second
  if (otter == 1 || otter == 2) {
    if (otter == 2) {
      draw_general_sprite(otterx, ottery, anim, 2, 0);
      if (otter_touched != 0xff) {
        otter = 3;
      }
    } else {
      draw_general_sprite(otterx + riverat(ottery, t) * 2, ottery, anim, 0, 0); // otter swims twice the speed of river object
      if (otter_touched != 0xff) {
        otter = 4;
      }
    }
  } else {
    draw_general_sprite(350, 310, anim, 0, 0); // draw offscreen
  }
  vis_gd_end();
}

// changes game vars and raises difficulty per level
static void change_environment() 
{
  srand(t + 1); // kinda random seed based on time taken per level
  turtledive1 = (byte)(rand() / (RAND_MAX / 8));
  turtledive2 = (byte)(rand() / (RAND_MAX / 7));
  for (byte i = 0; i < 5; i++) { 
    // this sets the car lane amount per level to 1, 2, 1, 2, 1, 2.. and sometimes 1 extra
    caramountrand[i] = ((level - 1) % 2) + 1 + (byte)(rand() / (RAND_MAX / 2));
    // random bytes calculated each level to change space between cars per lane
    carspacerand[i] = 224 + (byte)(rand() / (RAND_MAX / 20));
    riverspacerand[i] = 172 + (byte)(rand() / (RAND_MAX / 16));
  }
  speed_factor = ((level - 1) / 10) + 1; // speed increases after every 10 levels
  // this sets the river object lane amount and will be limited per waterlane
  // (depending the length of the object), minimum is 2
  riveramountrand[0] = 2 + (byte)(rand() / (RAND_MAX / 4)); // bottom lane
  riveramountrand[1] = 2 + (byte)(rand() / (RAND_MAX / 4));
  riveramountrand[2] = 2 + (byte)(rand() / (RAND_MAX / 2));
  riveramountrand[3] = 3 + (byte)(rand() / (RAND_MAX / 4));
  riveramountrand[4] = 2 + (byte)(rand() / (RAND_MAX / 3)); // top lane
  // set female frog log number
  femalelog = (byte)(rand() / (RAND_MAX / riveramountrand[1] + 1));
  if (level > 1) // set croc
    croclog = (byte)(rand() / (RAND_MAX / riveramountrand[4] + 1));
  if (level > 2) {
    // set snake
    snakelog = (byte)(rand() / (RAND_MAX / riveramountrand[2] + 1));
    // init otter
    otter = 3;
  }
}

// show croc randomly in home slot, starting from level 3 when no insect is shown
static void show_croc() 
{
  vis_gd_wstartspr((current_frame ? 0 + crocheadspr : 256 + crocheadspr));
  if ((t > showtime_croc) && !insecthome && (level > 2) && (rand() / (RAND_MAX / 128)) == 0) {
    showcrochead ^= 1; // toggle
    crocheadhome = (rand() / (RAND_MAX / 5)); // 0 - 4
    // check for free homes
    if (showcrochead && done[crocheadhome] == 0) { // there is room
      crocheadhome++;
      showtime_croc  = t + GAME_TEXTTIME;
    } else {
      crocheadhome = 0;
    }
  }
  if (showcrochead) {
    if (crocheadhome && (t < showtime_croc)) {
      byte anim = ((showtime_croc - t) > 192) ? 65 : 66; // 2 step animation
      draw_general_sprite(homes[crocheadhome - 1], 40, anim, 0, 0);
    } else {
      crocheadhome = 0; // reset and park offscreen
      draw_general_sprite(400, 350, 65, 0, 0);
    }
  } else {
    crocheadhome = 0; // reset and park offscreen
    draw_general_sprite(400, 350, 65, 0, 0);
  }
  vis_gd_end();
}

// show insect randomly in home slot, for extra bonus
static void show_insect() 
{
  vis_gd_wstartspr((current_frame ? 0 + insectspr : 256 + insectspr));
  if ((t > showtime_insect) && !crocheadhome && (rand() / (RAND_MAX / 128)) == 0) {
    showinsect ^= 1; // toggle
    insecthome = (rand() / (RAND_MAX / 5)); // 0 - 4
    // check for free homes
    if (showinsect && done[insecthome] == 0) { // there is room
      insecthome++;
      showtime_insect = t + 360 + (rand() / (RAND_MAX / 96));
    } else {
      insecthome = 0;
    }
  }
  if (showinsect) {
    if (insecthome && (t < showtime_insect)) {
      draw_general_sprite(homes[insecthome - 1], 40, 62, 0, 0);
    } else { 
      insecthome = 0; // reset and park offscreen
      draw_general_sprite(400, 350, 62, 0, 0);
    }
  } else {
    insecthome = 0; // reset and park offscreen
    draw_general_sprite(400, 350, 62, 0, 0);
  }
  vis_gd_end();
}

// shows the current hiscore
static void draw_highscore()
{
  long score = hiscore;
  if (p1_score > hiscore && p1_score > p2_score)
    score = p1_score;
  if (p2_score > hiscore && p2_score > p1_score)
    score = p2_score;
  draw_score(atxy(11, 1), score);
}

// checks if the right control combination has been entered during the intro, to enable
// the god mode. konami_code needs to contain 0b11111111 and then the final middle btn
// needs to be pressed.
static void check_konami_code()
{
  static uint16_t timespan;
  byte p1_joy = vis_control_read_joystick_p1();
  byte p1_btn = vis_control_read_buttons_p1();
  switch (konami_code)
  {
  case 0x00:
    if (p1_joy == CONTROL_UP) { 
      timespan = t; // start counting time, since this combo needs to be entered within 5 secs
      konami_code |= 0x80;
    }
    break;
  case 0x80:
    if (p1_joy == CONTROL_UP) {
      konami_code |= 0x40;
    }
    break;
  case 0xc0:
    if (p1_joy == CONTROL_DOWN) { 
      konami_code |= 0x20;
    }
    break;
  case 0xe0:
    if (p1_joy == CONTROL_DOWN) { 
      konami_code |= 0x10;
    }
    break;
  case 0xf0:
    if (p1_joy == CONTROL_LEFT) { 
      konami_code |= 0x08;
    }
    break;
  case 0xf8:
    if (p1_joy == CONTROL_LEFT) { 
      konami_code |= 0x04;
    }
    break;
  case 0xfc:
    if (p1_joy == CONTROL_RIGHT) {
      konami_code |= 0x02;
    }
    break;
  case 0xfe:
    if (p1_joy == CONTROL_RIGHT) {
      konami_code |= 0x01;
    }
    break;
  case 0xff: // joystick sequence completed, now the final btn 2
    if (p1_btn == CONTROL_BTN2) {
      godmode ^= 1; // toggle
    }
    break;
  }
  if (t > (timespan + 360)) { // time expired, reset
    konami_code = 0x00;
  }
  if (p1_btn == CONTROL_BTN2 || p1_btn == CONTROL_BTN3) { // start game
    konami_code = 0x00; // reset
    frog_introx = 440;
    frog_introy = 144;
    intro = 0; // disable intro
    game = 1; // enable game
    // start 1/2-player game
    gamemode = (p1_btn == CONTROL_BTN2) ? 0 : 1;
  }
}

// initialize p1 frog, starts at bottom
// resetting game vars
static void p1_init()
{
  if (p1_lives) { // only reset when lives left
    level_timer = 0;
    p1_frogx = 120;
    p1_frogy = 232;
    p1_leaping = 0;
    p1_frogface = 0;
    p1_dying = 0;
    p1_female = 0;
  }
}

// initialize p2 frog, starts at bottom
// resetting game vars
static void p2_init()
{
  if (p2_lives) { // only reset when lives left
    level_timer = 0;
    p2_frogx = 104;
    p2_frogy = 232;
    p2_leaping = 0;
    p2_frogface = 0;
    p2_dying = 0;
    p2_female = 0;
  }
}

// screen shows the little frogger intro,
// some game info and the top 10 highscores/ latest achievements lists
static void intro_start()
{
  // 7 frogs and logo
  byte amount = 7;
  static byte frog_letters[] = {90, 91, 92, 93, 93, 94, 91}; // sprite slots, frogger logo letters
  byte leap = ((t / 36) % 2);
  if (t < 360) { // first leap in from the right
    vis_gd_wstartspr((current_frame ? 0 + p1_frogspr : 256 + p1_frogspr));
    for (byte i = 0; i < amount; i++) {
      if (t >= (i * 36)) {
        if (leap) { // leap
          if (!i) { // only change for first frog, the rest just follows
            frog_leaping++;
            frog_introx -= 2;
          }
          draw_general_intro_sprite(frog_introx + (i * 36), frog_introy, frog_anim_green[frog_leaping / 9], 3, 0);
        } else { // stay still
          draw_general_intro_sprite(frog_introx + (i * 36), frog_introy, frog_anim_green[0], 3, 0);
        }
        if (frog_leaping == 36)
          frog_leaping = 0;
      }
    }
  } else if ((t >= 360) && (t < 504)) { // draw heads up
    vis_gd_wstartspr((current_frame ? 0 + p1_frogspr : 256 + p1_frogspr));
    for (byte i = 0; i < amount; i++) {
      draw_general_intro_sprite(84 + (i * 36), frog_introy, frog_anim_green[0], 0, 0);
    }
  } else if ((t >= 504) && (t < 576)) { // move up
    vis_gd_wstartspr((current_frame ? 0 + p1_frogspr : 256 + p1_frogspr));
    for (byte i = 0; i < amount; i++) {
        if (leap) { // leap
          if (!i) { // only change for first frog, the rest just follows
            frog_leaping++;
            frog_introy -= 3;
          }
          draw_general_intro_sprite(84 + (i * 36), frog_introy, frog_anim_green[frog_leaping / 9], 0, 0);
        } else { // stay still
          draw_general_intro_sprite(84 + (i * 36), frog_introy, frog_anim_green[0], 0, 0);
        }
        if (frog_leaping == 36)
          frog_leaping = 0;
    }
  } else if ((t >= 576) && (t < 927)) { // show letters
    vis_gd_wstartspr((current_frame ? 0 + p1_frogspr : 256 + p1_frogspr));
    for (byte i = 0; i < amount; i++) {
      byte csprite = frog_anim_green[0];
      if (t > (648 + (i * 36)))
        csprite = frog_letters[i];
      draw_general_intro_sprite(84 + (i * 36), frog_introy, csprite, 0, 0);
    }
  } else if ((t >= 927) && (t < 4527)) {
    if ((t - 927) % 720 == 0)
      screen_clear();
    if ((t / 72) % 2) {
      draw_text(GAME_BTNINFO, 0, 8, 0, ALIGN_CENTER);
    } else {
      for (uint8_t i = 0; i < 50; i++) {
        vis_gd_wr(RAM_PIC + 64 * 8 + i, BG_BLACK);
      }
    }
    draw_text(GAME_CREDIT1, 0, 35, 0, ALIGN_CENTER);
    draw_text(GAME_CREDIT2, 0, 36, 0, ALIGN_CENTER);
    // show current top of highscores
    if ( (t >= 927) && (t < 1647)) {
      draw_text(GAME_HISCORES, 0, 10, 0, ALIGN_CENTER);
      char text[7];
      byte text_offsetx = 12;
      for (uint8_t i = 0; i < 10; i++) {
        sprintf(text, "%02d", i+1); 
        draw_text(text, text_offsetx, 13 + (i * 2), 0, ALIGN_X);
        sprintf(text, "%06lu", scoreinfo.scores[i].points); 
        draw_text(text, text_offsetx + 3, 13 + (i * 2), 0, ALIGN_X);
        draw_text(scoreinfo.scores[i].player, text_offsetx + 10, 13 + (i * 2), 0, ALIGN_X);
        draw_text(GAME_LVLTXTLOW, text_offsetx + 19, 13 + (i * 2), 0, ALIGN_X);
        sprintf(text, "%02d", scoreinfo.scores[i].level); 
        draw_text(text, text_offsetx + 25, 13 + (i * 2), 0, ALIGN_X);
      }
    } else if ( (t >= 1647) && (t < 2367)) { // show latest achievements
        draw_text(GAME_ACHIEV, 0, 10, 0, ALIGN_CENTER);
        byte text_offsetx = 4;
        for (uint8_t i = 0; i < 6; i++) {
          draw_text(scoreinfo.achievements[i].name, text_offsetx, 16 + (i * 2), 0, ALIGN_X);
          draw_text(scoreinfo.achievements[i].player, text_offsetx + 30, 16 + (i * 2), 0, ALIGN_X);
        }
    } else if ( (t >= 2367) && (t < 3087)) { // show point table
        draw_text(GAME_POINTTABLE, 0, 10, 0, ALIGN_CENTER);
        byte text_offsetx = 10;
        for (uint8_t i = 0; i < 7; i++) {
          draw_text(game_points[i], text_offsetx, 14 + (i * 2), 0, ALIGN_X);
        }
        draw_text(game_points[7], text_offsetx, 29, 0, ALIGN_X);
    } else if ( (t >= 3087) && (t < 3807)) { // show achievements info
        draw_text(GAME_ACHIEVINFO, 0, 10, 0, ALIGN_CENTER);
        byte text_offsetx = 1;
        for (uint8_t i = 0; i < 12; i+=2) {
          draw_text(game_achievs[i], text_offsetx, 12 + (i * 2), 0, ALIGN_X);
          draw_text(game_achievs[i+1], text_offsetx + 2, 13 + (i * 2), 0, ALIGN_X);
        }
    } else if ( (t >= 3807) && (t < 4527)) { // show ways to die
        draw_text(GAME_DIEINFO, 0, 10, 0, ALIGN_CENTER);
        byte text_offsetx = 2;
        for (uint8_t i = 0; i < 9; i++) {
          draw_text(game_deaths[i], text_offsetx, 14 + (i * 2), 0, ALIGN_CENTER);
        }
    }
  } else {
    screen_clear();
    vis_gd_wstart(RAM_SPR); // Hide all sprites
    for (uint16_t i = 0; i < 512; i++)
      vis_gd_xhide();
    frog_introx = 440;
    frog_introy = 144;
  }
  vis_gd_end(); // end spi communication with GD
  t = (t == 4527) ? 0 : t + 1; // inc/reset general timer, basically everything that moves at a certain speed is based on this
  current_frame = (t & 1); // swap frame buffer, modulus 2
  vis_gd_waitvblank(); // wait for complete screen draw
  vis_gd_wr(SPR_PAGE, current_frame); // swap frame buffer
  // keep checking for cheat mode
  check_konami_code();
}

// screen where you can enter your name, if you've managed to get 
// in the top 10 highscore/latest achievement list
static void save_scores_start()
{
  byte p1_frogx = 76; 
  byte p1_frogy = 128; 
  byte p2_frogx = 76; 
  byte p2_frogy = 208; 
  current_frame = (t & 1); // swap frame buffer, modulus 2
  draw_text(GAME_GRATS, 0, 2, 0, ALIGN_CENTER);
  draw_text(GAME_TOPLIST, 0, 5, 0, ALIGN_CENTER);
  if (p1_score_pos != -1 || p1_achievs > 0) { // made the list
    draw_text(GAME_NAMEP1, 0, 12, 0, ALIGN_CENTER);
    change_letters(GAME_PLAYER1, &p1_done, &p1_current_letter, &p1_name[0]);
    vis_gd_wstartspr((current_frame ? 0 + p1_frogspr : 256 + p1_frogspr));
    draw_general_intro_sprite(p1_frogx + (p1_current_letter * 16) , p1_frogy, 2, 0, 0);
    vis_gd_end(); // end spi communication with GD
  }
  if (p2_score_pos != -1 || p2_achievs > 0) { // 2-player mode
    draw_text(GAME_NAMEP2, 0, 22, 0, ALIGN_CENTER);
    change_letters(GAME_PLAYER2, &p2_done, &p2_current_letter, &p2_name[0]);
    vis_gd_wstartspr((current_frame ? 0 + p2_frogspr : 256 + p2_frogspr));
    draw_general_intro_sprite(p2_frogx + (p2_current_letter * 16) , p2_frogy, 22, 0, 0);
    vis_gd_end(); // end spi communication with GD
  }
  t++; // general timer, basically everything that moves at a certain speed is based on this
  vis_gd_waitvblank(); // wait for complete screen draw
  vis_gd_wr(SPR_PAGE, current_frame); // swap frame buffer
  if ((p1_done && !gamemode) || (p1_done && p2_done && gamemode)) {
    if (p1_done) { // done with name creation 
      p1_finalname[0] = 0; // clear 
      for (uint8_t i = 0; i < 16; i = i + 2) { // skip the \0's
        if (p1_name[i] == 0x2e) // replace dots with spaces
          p1_name[i] = 0x20;
        strcat(p1_finalname, &p1_name[i]);
      }
      strcpy(scores_updated[p1_score_pos].player, p1_finalname);
    }
    if (p2_done) { // made the list
      p2_finalname[0] = 0; // clear 
      for (uint8_t i = 0; i < 16; i = i + 2) { // skip the \0's
        if (p2_name[i] == 0x2e) // replace dots with spaces
          p2_name[i] = 0x20;
        strcat(p2_finalname, &p2_name[i]);
      }
      strcpy(scores_updated[p2_score_pos].player, p2_finalname);
    }
    // save top 10
    for (uint8_t i = 0; i < 10; i++) {
      scoreinfo.scores[i] = scores_updated[i];
    }
    // save latest players who reached 1 to 6 achievements
    for (uint8_t i = 0; i < 12; i += 2) {
      strcpy(scoreinfo.achievements[i / 2].name, game_achievs[i]);
      if (p1_achievs & (1 << (i / 2))) {
        strcpy(scoreinfo.achievements[i / 2].player, p1_finalname);
      } else if (p2_achievs & (1 << (i / 2))) {
        strcpy(scoreinfo.achievements[i / 2].player, p2_finalname);
      }
    }
    vis_score_write(scoreinfo, gameidx); // write back to EEPROM
    saving = 0;
    intro = 1; // start intro again
  }
}

// reads the controls and changes the letters of the player's names accordingly
static void change_letters(byte player, byte *p_done, byte *current_letter, char *p_name)
{
  byte offsetx = 10; 
  byte offsety;
  byte joy, btn;
  if (player == GAME_PLAYER1) {
    offsety = 14;
    joy = vis_control_read_joystick_p1();
    btn = vis_control_read_buttons_p1();
  } else {
    offsety = 24;
    joy = vis_control_read_joystick_p2();
    btn = vis_control_read_buttons_p2();
  }
  if (btn == CONTROL_BTN2) { // toggle done or undo to correct mistakes
    *p_done ^= 1;
  } else if (!*p_done) {
    if (joy == CONTROL_UP) {
      if (p_name[*current_letter] == 0x2e && !*current_letter) { // starting dot and first letter 
        p_name[*current_letter] = 0x41;
      } else if (p_name[*current_letter] == 0x2e) { // starting dot
        p_name[*current_letter] = 0x61;
      } else if (p_name[*current_letter] == 0x7e) { // reset 
        p_name[*current_letter] = 0x20;
      } else {
        p_name[*current_letter]++;
      }
    } else if (joy == CONTROL_DOWN) {
      if (p_name[*current_letter] == 0x20) { // reset 
        p_name[*current_letter] = 0x7e;
      } else {
        p_name[*current_letter]--;
      }
    } else if (joy == CONTROL_RIGHT) {
      *current_letter += 2;
      if (*current_letter > 14) // reset
        *current_letter = 0;
    } else if (joy == CONTROL_LEFT) {
      if (!*current_letter) // reset
        *current_letter = 16;
      *current_letter -= 2;
    }
  }
  if (*p_done) { // hide/show "done" text
    draw_text(GAME_DONE, 42, offsety, 0, ALIGN_X);
  } else {
    for (uint8_t i = 0; i < 4; i++) {
      vis_gd_wr(RAM_PIC + 64 * offsety + (42 + i), BG_BLACK);
    }
  }
  for (uint8_t i = 0; i < 16; i += 2) { // draw the 8 dots
    draw_text(&p_name[i], offsetx + (2 * i), offsety, 0, ALIGN_X);
  }
}

// resets the player names to default dots
static void reset_names()
{
  for (uint8_t i = 0; i < 16; i+=2) {
    p1_name[i] = 0x2e;
    p2_name[i] = 0x2e;
  }
}

// starting game, inits some game vars
static void game_start()
{
  level = 0;
  p1_lives = 3;
  p2_lives = 3;
  p1_score = 0;
  p2_score = 0;
  p1_prev_score_divisor = 0;
  p2_prev_score_divisor = 0;
  level_start();
  hiscore = scoreinfo.scores[0].points; // show top of saved ranking initially
}

// redraws the central purple bank, to reset messages
static void bank_clear()
{
  for (byte y = 1; y < 3; y++)
    vis_gd_copy(atxy(0, y + 15), (uint16_t)(froggerbg_pic + (y * 28) + 420), 28);
}

// clears the background
static void screen_clear()
{
  vis_gd_fill(RAM_PIC, BG_BLACK, 4096); // fill complete screen with black
}

// initializes the screen: clears and loads gfx into the Gameduino
static void init_start() {
  vis_gd_begin(); // init the Gameduino
  vis_gd_ascii(); // load the ascii text font
  color_chars(COLOR_GREEN, COLOR_UPCASE); // uppercase letters in green
  vis_gd_copy(RAM_CHR + 2048, (uint16_t)froggerbg_chr, sizeof(froggerbg_chr)); // background chars, shifted 128 chars (times 16 bytes) to give ascii font (32-127) char space
  vis_gd_copy(RAM_PAL + 1024, (uint16_t)froggerbg_pal, sizeof(froggerbg_pal)); // char palette, shifted 128 (times 8 bytes)
  vis_gd_copy(PALETTE16A, (uint16_t)sprite_sprpal, sizeof(sprite_sprpal)); // sprite palette
  vis_gd_copy(PALETTE16B, (uint16_t)sprite_sprpalb, sizeof(sprite_sprpalb)); // sprite palette
  vis_gd_uncompress(RAM_SPRIMG, (uint16_t)sprite_sprimg); // put sprites into ram
  vis_gd_fill(RAM_PIC, BG_BLACK, 4096); // fill complete screen with black first
  vis_control_init(); // set ports correctly for reading the joysticks/buttons
}

// initializes intro
static void init_intro() {
  screen_clear(); // black screen
  vis_gd_wstart(RAM_SPR); // Hide all sprites
  for (uint16_t i = 0; i < 512; i++)
    vis_gd_xhide();
  vis_gd_end();
  vis_control_enable_p1(); // enable reads player 1
  vis_control_enable_p2(); // enable reads player 2
  current_frame = 0; // reset current frame for sprite page
  scoreinfo = vis_score_read(GAME_NAME); // read score and achievement info
  t = 0; // reset global timer
}

// initializes game
static void init_game() {
  current_frame = 0; // reset current frame for sprite page
  p1_achievs = 0; // reset latest achievements
  p2_achievs = 0;
  t = 0; // reset global timer
  game_start();
}

// initializes saving screen
static void init_saving() {
  screen_clear(); // black screen
  vis_gd_wstart(RAM_SPR); // Hide all sprites
  for (uint16_t i = 0; i < 512; i++)
    vis_gd_xhide();
  vis_gd_end();
  vis_control_enable_p1(); // enable reads player 1
  vis_control_enable_p2(); // enable reads player 2
  p1_done = 0;
  p2_done = 0;
  p1_current_letter = 0;
  p2_current_letter = 0;
  current_frame = 0; // reset current frame for sprite page
  t = 0; // reset global timer
  reset_names();
}

// resetting level vars and increasing difficulty
static void level_start()
{
  p1_init();
  vis_control_enable_p1(); // enable control reads player 1
  if (gamemode) {
    p2_init();
    vis_control_enable_p2(); // enable control reads player 2
  }
  screen_clear(); // black screen
  for (byte y = 0; y < 32; y++)
    vis_gd_copy(atxy(0, y), (uint16_t)(froggerbg_pic + y * 28), 28); // write background char bytes to screen (32 times 28 bytes)
  vis_gd_fill(atxy(0, 1), BG_BLUE, 28); // hide scoring numbers from static background
  vis_gd_wstart(RAM_SPR); // Hide all sprites
  for (uint16_t i = 0; i < 512; i++)
    vis_gd_xhide();
  vis_gd_end();
  draw_curtains(); // hide sides
  for (byte i = 0; i < 5; i++) // reset homes
    done[i] = 0;
  level++; // next level
  change_environment(); // make it progressively harder
  t = 0; // reset global timer, make sure this is after the change_environment random functions 
  level_timer = 0; // increments 72 times/sec, until 60 seconds
  insecthome = 0; // no visible insect
  crocheadhome = 0; // no visible croc head
  showtime_insect = 0; // no insect in home
  showtime_croc = 0; // no crocodile in home
  snakelogx = 0; // reset snake
  femalelogx = 0; // reset female frog
  for (uint8_t i = 0; i < 6; i++) { // reset achievements each level
    achiev[i] = 0;
  }
  msg_init(&msg_head, &msg_tail); // reset message queue each level
  current_msg.display_time = 0;
  strcpy(current_msg.display_text, "");
  MsgT go = { GAME_TEXTTIME, GAME_GO }; // new GO msg
  msg_enqueue(msgs, &msg_tail, go);
}

// builds the new top score list, from down to top, shifting and leaving a hole for player(s) spot
static void update_scores(int8_t *score_pos, byte *score_level, long *pscore) 
{
  // shift scores and make a hole
  for (uint8_t i = 9; i > *score_pos; i--)
    scores_updated[i] = scores_updated[i - 1];
  scores_updated[*score_pos].points = *pscore;
  scores_updated[*score_pos].level = *score_level;
}

// check if player's score reached highscores list
static void check_scores()
{
  game = 0; // disable game section
  saving = 0; // initial saving screen disabled, waiting for checks
  p1_score_pos = -1;
  p2_score_pos = -1;
  for (uint8_t i = 0; i < 10; i++) { // fill current scores 
    scores_updated[i] = scoreinfo.scores[i];
    if (p1_score >= scores_updated[i].points && p1_score_pos == -1)
      p1_score_pos = i;
    if (p2_score >= scores_updated[i].points && p2_score_pos == -1 && gamemode)
      p2_score_pos = i;
  }
  if (p1_score_pos != -1 && p2_score_pos != -1 && p1_score_pos == p2_score_pos) {
    if (p2_score >= p1_score) { // if equal scorelist spot, p2 has prio to be above
      p1_score_pos++; // lower in list by 1
      update_scores(&p2_score_pos, &p2_level, &p2_score);
      if (p1_score_pos < 10)
        update_scores(&p1_score_pos, &p1_level, &p1_score);
    } else if (p2_score < p1_score) {
      p2_score_pos++; // lower in list by 1
      update_scores(&p1_score_pos, &p1_level, &p1_score);
      if (p2_score_pos < 10)
        update_scores(&p2_score_pos, &p2_level, &p2_score);
    }
    saving = 1;
  } else if (p1_score_pos > -1) {
    update_scores(&p1_score_pos, &p1_level, &p1_score);
    saving = 1;
  } else if (p2_score_pos > -1) {
    update_scores(&p2_score_pos, &p2_level, &p2_score);
    saving = 1;
  }
  if (p1_achievs > 0 || p2_achievs > 0) // player(s) made some achievements
    saving = 1;
  if (!saving)
    intro = 1;
}

// contains all interaction functionality of the froggers with the environment
void player_start(byte player)
{
  static byte *frogspr;
  static uint16_t *frogx; 
  static uint16_t *frogy;
  static byte *frogface;
  static byte *leaping;
  static byte *dying;
  static byte *touched;
  static byte *touching;
  static uint16_t *frog_posx;
  static uint16_t *river_object_posx;
  static byte *deadly_river_object;
  static byte *deadly_river_object_pal;
  static uint16_t *river_sibling_object_posx_right;
  static uint16_t *river_sibling_object_posx_left;
  static byte *lives;
  static byte *prev_score_divisor;
  static long *score;
  static byte *female;
  static uint8_t *bonustime;
  static byte *joy;
  static byte p1_joy; // need these as intermediate state when calling this funtion for p1 and p2 alternatively
  static byte p2_joy;
  static byte *btn;
  static byte *lvl;
  if (player == GAME_PLAYER1) {
    frogspr = &p1_frogspr;
    frogx = &p1_frogx;
    frogy = &p1_frogy;
    frogface = &p1_frogface;
    leaping = &p1_leaping;
    dying = &p1_dying;
    touched = &p1_touched;
    touching = &p1_touching;
    frog_posx = &frog_p1_posx;
    river_object_posx = &river_object_posx_p1;
    deadly_river_object = &deadly_river_object_p1;
    deadly_river_object_pal = &deadly_river_object_pal_p1;
    river_sibling_object_posx_right = &river_sibling_object_posx_right_p1;
    river_sibling_object_posx_left = &river_sibling_object_posx_left_p1;
    lives = &p1_lives;
    prev_score_divisor = &p1_prev_score_divisor;
    score = &p1_score;
    female = &p1_female;
    bonustime = &p1_bonustime;
    joy_p1 = vis_control_read_joystick_p1();
    btn_p1 = vis_control_read_buttons_p1();
    btn = &btn_p1;
    lvl = &p1_level;
  } else {
    frogspr = &p2_frogspr;
    frogx = &p2_frogx;
    frogy = &p2_frogy;
    frogface = &p2_frogface;
    leaping = &p2_leaping;
    dying = &p2_dying;
    touched = &p2_touched;
    touching = &p2_touching;
    frog_posx = &frog_p2_posx;
    river_object_posx = &river_object_posx_p2;
    deadly_river_object = &deadly_river_object_p2;
    deadly_river_object_pal = &deadly_river_object_pal_p2;
    river_sibling_object_posx_right = &river_sibling_object_posx_right_p2;
    river_sibling_object_posx_left = &river_sibling_object_posx_left_p2;
    lives = &p2_lives;
    prev_score_divisor = &p2_prev_score_divisor;
    score = &p2_score;
    female = &p2_female;
    bonustime = &p2_bonustime;
    joy_p2 = vis_control_read_joystick_p2();
    btn_p2 = vis_control_read_buttons_p2();
    btn = &btn_p2;
    lvl = &p2_level;
  }
  // frogger drawing
  vis_gd_wstartspr((current_frame ? 0 + *frogspr : 256 + *frogspr)); // use fixed spr position for easier collision detection
  if (godmode && !gamemode) { // cheat mode in single-player, make frogger purple to indicate it
    *dying = 0; // keep setting this to 0, so you can't die
    draw_general_sprite(*frogx, *frogy, frog_anim_purple[*leaping / 2], *frogface, 0); // 8 cycles for leaping
  } else if (!(*dying)) {
    if (player == GAME_PLAYER1) {
      draw_general_sprite(*frogx, *frogy, frog_anim_green[*leaping / 2], *frogface, 0); // 8 cycles for leaping
    } else {
      draw_general_sprite(*frogx, *frogy, frog_anim_purple[*leaping / 2], *frogface, 0); // 8 cycles for leaping
    }
  } else { // dying animations
    if (*dying > 64) { // still watching the "game over" msg, at dying == 65
      if (player == GAME_PLAYER1) {
        vis_control_disable_p1();
      } else {
        vis_control_disable_p2();
      }
      draw_general_sprite(400, 300, die_drown_anim[0], 0, 0); // hide offscreen
    } else {
      if (location(*frogx, *frogy) == LOC_RIVER) {
        draw_general_sprite(*frogx, *frogy, die_drown_anim[(*dying) / 16], 0, 0); // 64 cycles for drowning (frogger can't swim?!)
      } else {
        draw_general_sprite(*frogx, *frogy, die_splash_anim[(*dying) / 16], 0, 0); // 64 cycles for splatting
      }
    }
  }
  vis_gd_end();
  if (level_timer == 3240) { // 45 secs passed
    MsgT timeup = { GAME_TEXTTIME, GAME_TIMEUP };
    msg_enqueue(msgs, &msg_tail, timeup);
    *dying = 1;
  }
  if (*btn == CONTROL_BTN2 && godmode && !gamemode)  // bail out of godmode, after you've reached the hi-score ofcourse :)
    godmode = 0;
  if (!(*dying) && (!(*leaping))) { // not dying and not leaping and joystick moved 
    if (player == GAME_PLAYER1 && joy_p1) {
      p1_joy = joy_p1;
      *leaping = 1;
      if (p1_joy == CONTROL_UP)
        *score += 10; // leaping forward score
    } else if (player == GAME_PLAYER2 && joy_p2) {
      p2_joy = joy_p2;
      *leaping = 1;
      if (p2_joy == CONTROL_UP)
        *score += 10; // leaping forward score
    }
  } else if (*leaping > 0) {
    *joy = (player == GAME_PLAYER1) ? p1_joy : p2_joy;
    if (*leaping <= 8) { // leaping is 8 times 2px
      if (*joy == CONTROL_LEFT) {
        *frogx -= 2;
        *frogface = 3;
      }
      if (*joy == CONTROL_RIGHT) {
        *frogx += 2;
        *frogface = 5;
      }
      if (*joy == CONTROL_UP) {
        *frogy -= 2;
        *frogface = 0;
      }
      if (*joy == CONTROL_DOWN) {
        *frogy += 2;
        *frogface = 6;
      }
      (*leaping)++;
    } else {
      *leaping = 0; // standing still again
    }
  }
  // Collision detection, Remember to wait for VBLANK == 1, else the collision detection is off!
  *touched = vis_gd_rd(COLLISION + *frogspr); // check for touch
  otter_touched = vis_gd_rd(COLLISION + otterspr); // check for touch
  *touching = (*touched != 0xff); // touching something
  if (*touching) {
    femalex = vis_gd_rd_spr_xpos(current_frame ? female_spr : 256 + female_spr);
    *river_object_posx = vis_gd_rd_spr_xpos(current_frame ? *touched : 256 + *touched);
    *frog_posx = vis_gd_rd_spr_xpos(current_frame ? *frogspr : 256 + *frogspr);
    *deadly_river_object = vis_gd_rd_spr_image(current_frame ? *touched : 256 + *touched);
    *deadly_river_object_pal = vis_gd_rd_spr_pal(current_frame ? *touched : 256 + *touched);
    *river_sibling_object_posx_right = vis_gd_rd_spr_xpos(current_frame ? *touched - 1: 256 + *touched - 1);
    *river_sibling_object_posx_left = vis_gd_rd_spr_xpos(current_frame ? *touched + 1 : 256 + *touched + 1);
  }
  if (*dying) {
    if (++(*dying) == 64) {
      (*lives)--;
      if (*lives) {
        if (player == GAME_PLAYER1) {
          p1_init();
        } else {
          p2_init();
        }
      } else {
        *lvl = level; // set max reached level for player
        if (!gamemode) {
          MsgT gameover = { GAME_TEXTTIME, GAME_OVER };
          msg_enqueue(msgs, &msg_tail, gameover);
        } else if (player == GAME_PLAYER1) {
          MsgT gameover = { GAME_TEXTTIME, GAME_OVER_P1 };
          msg_enqueue(msgs, &msg_tail, gameover);
        } else if (player == GAME_PLAYER2) {
          MsgT gameover = { GAME_TEXTTIME, GAME_OVER_P2 };
          msg_enqueue(msgs, &msg_tail, gameover);
        }
      }
    } else if (++(*dying) > 64) { // wait for messages to fade, then "game over"
      *dying = 65; // don't let this run too high 
      if (msg_empty(msg_head, msg_tail) && !displaymsg) {
        if (!gamemode || (gamemode && !p1_lives && !p2_lives && player == GAME_PLAYER1)) { // done, lets check for highscores
          check_scores();
        }
      }
    }
  } else if (location(*frogx, *frogy) == LOC_EDGE) { // can't touch edges
    *dying = 1;
  } else if (location(*frogx, *frogy) == LOC_ROAD) { // road section
    if (*touching) { // if touching something
      *dying = 1;
      // check for achievement 6: "too much seinfeld", hitting a truck
      if ((achiev[5] != 0xff) && ((*deadly_river_object == 0x02 && *deadly_river_object_pal == 6) || (*deadly_river_object == 0x03 && *deadly_river_object_pal == 4)))
        achiev[5]++;
      // update for achievement 6: "too much seinfeld"
      if (achiev[5] == 2) {
        achiev[5] = 0xff; // disable for now
        *score += 5000; // achievement score
        if (player == GAME_PLAYER1) {
          p1_achievs |= 0x20; // set bit 6
          p2_achievs &= 0xdf; // unset bit 6
        } else {
          p2_achievs |= 0x20; // set bit 6
          p1_achievs &= 0xdf; // unset bit 6
        }
        MsgT achiev6 = { GAME_TEXTTIME, GAME_ACHIEV11 };
        msg_enqueue(msgs, &msg_tail, achiev6);
      }
     // in making the last road jump to riverbank...
     // ...snake's tail is the only safe exception
      if ((location(*frogx, *frogy) <= 152) && (*deadly_river_object == 0x14 || *deadly_river_object == 0x15 || *deadly_river_object == 0x16) && (*deadly_river_object_pal == 6))
        *dying = 0;
    }
  } else if (location(*frogx, *frogy) == LOC_RIVER) { // river section
    if (!*leaping) { // done with leaping
      if (*touching) { // if touching something
        if (*deadly_river_object == 0x12) { // croc head
          *dying = 1;
        } else if (*deadly_river_object == 0x13) { // otter
          *dying = 1;
        } else if (*deadly_river_object == 0x14 || *deadly_river_object == 0x15 || *deadly_river_object == 0x16) {
            if (*deadly_river_object_pal == 4) { // snake head touching, tail is safe, which is same image but palette 6
              *dying = 1;
            }
        } else if (*touched == otter_touched) { // touching log piece that an otter just flanked
          *dying = 1;
        } else {
          // first check for female touch
          if (*deadly_river_object == 0x04 || *deadly_river_object == 0x05) {
            *female = 1;
          } else if (((*frogy == 104)) && (abs(*frog_posx - femalex) < 10)) { // also check if female is very close, to prevent her leaping over
            *female = 1;
          } else if // check if frogger is at least covering most of the river edge object sprites, else he slips off and dies
            (abs(*river_object_posx - *frog_posx) > 9) { // no more than 9 pixels away
            // check if there is a sibling river object on the left, i.e. 1 higher control sprite
            if (*frog_posx < *river_object_posx) {
              if (*river_sibling_object_posx_left != *river_object_posx - 16)  {
                *dying = 1;
              }
            } else if (*frog_posx > *river_object_posx) { // check if there is a sibling river object on the right, i.e. 1 lower control sprite
              if (*river_sibling_object_posx_right != *river_object_posx + 16)  {
                *dying = 1;
              }
            }
          }
        }
        // move frog according to lane speed: newx minus oldx
        *frogx += riverat(*frogy, t) - riverat(*frogy, t - 1);
      } else {
        *dying = 1;
      }
    }
  }
  else if (location(*frogx, *frogy) == LOC_MIDDLE_BANK) // river bank section
  {
    if (*touching) { // if touching something
      if (*deadly_river_object == 0x14 || *deadly_river_object == 0x15 || *deadly_river_object == 0x16) {
        if (*deadly_river_object_pal == 4) { // snake head touching, tail is safe, which is same image but palette 6
          *dying = 1;
        }
      }
    }
  }
  else if (location(*frogx, *frogy) == LOC_HOME) // near home
  {           
    if (!*leaping) {
      byte landed = 0; // reset
      for (byte i = 0; i < 5; i ++) {
        if (!done[i] && abs(homes[i] - *frogx) < 6) { // safe landing if within 6 pixels
          if (crocheadhome == (i + 1)) { // die by crocodile head in home slot
            *dying = 1;
          } else {
            done[i] = 1;
            landed = 1;
            if (achiev[0] != 0xff) {
              // update for achievement 1: "the right way"
              achiev[0] |= 1 << (7 - i); // set home slot bit from left to right
              achiev[0] &= ~((1 << (7 - i)) - 1); // always clear all bits on the right side, to ensure the right scoring order
              // check for achievement 1: "the right way"
              if (i == 4 && achiev[0] == 0xf8) { // gained achievement
                achiev[0] = 0xff; // disable for now
                MsgT achiev1 = { GAME_TEXTTIME, GAME_ACHIEV1 };
                msg_enqueue(msgs, &msg_tail, achiev1);
                *score += 5000; // achievement score
                if (player == GAME_PLAYER1) {
                  p1_achievs |= 0x01; // set bit 1
                  p2_achievs &= 0xfe; // unset bit 1
                } else {
                  p2_achievs |= 0x01; // set bit 1
                  p1_achievs &= 0xfe; // unset bit 1
                }
              }
            }
            // update for achievement 2: "speedy gonzales"
            if (achiev[1] != 0xff) {
              achiev[1] += (level_timer / 72);
            }
            // check for achievement 5: "calling me chicken?"
            if (achiev[4] == 1) {
              achiev[4] = 0xff; // disable for now
              MsgT achiev5 = { GAME_TEXTTIME, GAME_ACHIEV9 };
              msg_enqueue(msgs, &msg_tail, achiev5);
              *score += 5000; // achievement score
              if (player == GAME_PLAYER1) {
                p1_achievs |= 0x10; // set bit 5
                p2_achievs &= 0xef; // unset bit 5
              } else {
                p2_achievs |= 0x10; // set bit 5
                p1_achievs &= 0xef; // unset bit 5
              }
            }
            if (insecthome == (i + 1)) {
              *score += 200; // insect score
              bonusx = homes[i];
              // update for achievement 4: "the hungry pig"
              if (achiev[3] != 0xff) {
                achiev[3]++;
              }
              MsgT bonus = { GAME_TEXTTIMESHORT, GAME_INSECTBONUS };
              msg_enqueue(msgs, &msg_tail, bonus);
            }
            *score += 50; // home score
            if (*female) {
              *score += 200; // female carry score
              bonusx = homes[i];
              // update for achievement 3: "larry laffer style"
              if (achiev[2] != 0xff) {
                achiev[2]++;
              }
              MsgT bonus = { GAME_TEXTTIMESHORT, GAME_FEMALEBONUS };
              msg_enqueue(msgs, &msg_tail, bonus);
            }
          }
        }
      }
      if (landed) {
        char bonustxt[33];
        char timeleft[3];
        *bonustime = 45 - (level_timer / 72);
        *score +=  10 * (*bonustime);
        itoa(*bonustime, timeleft, 10); // convert time left to char
        MsgT bonusmsg;
        bonusmsg.display_time = GAME_TEXTTIME;
        strcpy(bonusmsg.display_text, strcat(strcpy(bonustxt, GAME_TIMEBONUS), timeleft)); // concat
        msg_enqueue(msgs, &msg_tail, bonusmsg);
        if (done[0] && done[1] && done[2] && done[3] && done[4]) { // level completed
          *score += 1000; // level score
          // check for achievement 2: "speedy gonzales"
          if (achiev[1] > 0 && achiev[1] < 60) {
            achiev[1] = 0xff; // disable for now
            MsgT achiev2 = { GAME_TEXTTIME, GAME_ACHIEV3 };
            msg_enqueue(msgs, &msg_tail, achiev2);
            *score += 5000; // achievement score
            if (player == GAME_PLAYER1) {
              p1_achievs |= 0x02; // set bit 2
              p2_achievs &= 0xfd; // unset bit 2
            } else {
              p2_achievs |= 0x02; // set bit 2
              p1_achievs &= 0xfd; // unset bit 2
            }
          }
          // check for achievement 3: "larry laffer style"
          if (achiev[2] == 5) {
            achiev[2] = 0xff; // disable for now
            MsgT achiev3 = { GAME_TEXTTIME, GAME_ACHIEV5 };
            msg_enqueue(msgs, &msg_tail, achiev3);
            *score += 5000; // achievement score
            if (player == GAME_PLAYER1) {
              p1_achievs |= 0x04; // set bit 3
              p2_achievs &= 0xfb; // unset bit 3
            } else {
              p2_achievs |= 0x04; // set bit 3
              p1_achievs &= 0xfb; // unset bit 3
            }
          }
          // check for achievement 4: "the hungry pig"
          if (achiev[3] == 5) {
            achiev[3] = 0xff; // disable for now
            MsgT achiev4 = { GAME_TEXTTIME, GAME_ACHIEV7 };
            msg_enqueue(msgs, &msg_tail, achiev4);
            *score += 5000; // achievement score
            if (player == GAME_PLAYER1) {
              p1_achievs |= 0x08; // set bit 4
              p2_achievs &= 0xf7; // unset bit 4
            } else {
              p2_achievs |= 0x08; // set bit 4
              p1_achievs &= 0xf7; // unset bit 4
            }
          }
          // move frog offscreen and wait for msgs
          *frogx = 200; // stay in the middle, so location isn't LOC_EDGE, but LOC_UNKNOWN
          *frogy = 300;
        } else {
          if (player == GAME_PLAYER1) {
            p1_init();
          } else {
            p2_init();
          }
        }
      } else // if frog did not land in a home, die! Also dies when home is already occupied.
        *dying = 1;
    }
  } else if (location(*frogx, *frogy) == LOC_BOTTOM_BANK) {
    // update for achievement 5: "calling me chicken?"
    if (level_timer >= 2592 && achiev[4] != 0xff) {
      if (gamemode) { // in 2-player mode, both need to be at the bottom
        achiev[4] = (location(p1_frogx, p1_frogy) == LOC_BOTTOM_BANK && location(p2_frogx, p2_frogy) == LOC_BOTTOM_BANK);
      } else {
        achiev[4] = 1;
      }
    } else if (level_timer < 2592 && achiev[4] != 0xff) {
        achiev[4] = 0;
    }
  } else if (location(*frogx, *frogy) == LOC_UNKNOWN) { // wait for screen msgs, then next level
    if (!msg_empty(msg_head, msg_tail) || displaymsg) {
      level_timer = 0;  // reset timer, to prevent dying while waiting
      if (player == GAME_PLAYER1) {
        vis_control_disable_p1();
      } else {
        vis_control_disable_p2();
      }
    } else {
      level_start(); // next level
    }
  }
  if (game) {
    check_score(score, prev_score_divisor, lives); // check if score is high enough to get another life
    if (player == GAME_PLAYER1) {
      draw_score(atxy(2, 1), *score);
    } else {
      draw_score(atxy(20, 1), *score);
    }
    if (!gamemode) { // single player mode
      if (!godmode) {
        vis_gd_putstr(11, 35, STR_MASK);
      } else {
        vis_gd_putstr(11, 35, GAME_GODMODE);
      }
      // hide p2 scoring
      vis_gd_fill(atxy(20, 0), BG_BLUE, 6);
      vis_gd_fill(atxy(20, 1), BG_BLUE, 6);
    }
    draw_lives();
    draw_level_timer();
    vis_gd_putstr(21, 33, GAME_LVLTXT);
    vis_gd_putstr(27, 33, itoa(level, levelstr, 10));
  }
}

// main function which contains the setup and the eternal loop
int main(void) {
	CPU_PRESCALE(0); // run at 16 MHz (external crystal)
#ifdef VIS_DEBUG
  init_usart(); // note that I'm using the UART1 rx/tx pins of the atmega1284p, not UART0
#endif
  _delay_ms(5000); // give Gameduino time to boot
  init_start();
  for(;;) { // eternal loop
    init_intro();
    while(intro) {
      intro_start(); // keep running the intro until button 2 is pressed
    }
    init_game();
    while (game) { // keep running game, until "game over"
      vis_gd_wstartspr((current_frame ? 0 : 256)); // start writing to sprite control, also resets current_sprite counter
      // road objects, bottom to top of road
      // Yellow cars
      set_cars(caramountrand[0], 3, carspacerand[0] / caramountrand[0], t, 216);
      // Dozers
      set_cars(caramountrand[1], 4, carspacerand[1] / caramountrand[1], t, 200);
      // Purple cars
      set_cars(caramountrand[2], 7, carspacerand[2] / caramountrand[2], t, 184);
      // Green and white racecars
      set_cars(caramountrand[3], 8, carspacerand[3] / caramountrand[3], t, 168);
      // Trucks (2 separate sprites for one truck)
      set_cars(caramountrand[4], 5, carspacerand[4] / caramountrand[4], t, 152);
      set_cars(caramountrand[4], 6, carspacerand[4] / caramountrand[4], t - (32 / speed_factor), 152);
      // river objects
      // bottom turtles 
      set_turtles(riveramountrand[0], 3, riverspacerand[0] / riveramountrand[0], t, 120);
      // top turtles
      set_turtles(riveramountrand[3], 2, riverspacerand[3] / riveramountrand[3], t, 72);
      // bottom logs
      set_logs(riveramountrand[1], 1, riverspacerand[1]/ riveramountrand[1], t, 104);
      // middle logs
      set_logs(riveramountrand[2], 4, riverspacerand[2] / riveramountrand[2], t, 88);
      // top logs
      set_logs(riveramountrand[4], 2, riverspacerand[4] / riveramountrand[4], t, 56);
      // draw completed homes
      for (byte i = 0; i < 5; i++) {
        if (done[i])
          draw_general_sprite(homes[i], 40, 63, 0, 0);
      }
      vis_gd_end();
      sound(); // check for sound events
      if (level > 2) { // above level 2, there is always a snake on the riverbank and a random otter
        draw_bank_snake();
        draw_otter();
      }
      draw_highscore();
      show_insect();
      show_croc();
      draw_log_snake();
      draw_female();
      display_msg();
      t++; // general timer, basically everything that moves at a certain speed is based on this
      level_timer++; // timer bar per level (45 secs)
      vis_gd_waitvblank(); // wait for vblanking (72 times/s refresh rate)
      current_frame = (t & 1); // swap frame buffer, modulus 2
      vis_gd_wr(SPR_PAGE, current_frame); // swap frame buffer
      if (gamemode)
        player_start(GAME_PLAYER2); // player 2 checks
      player_start(GAME_PLAYER1); // player 1 checks
    }
    init_saving();
    while(saving) {
      save_scores_start(); // keep running name entering and saving scores until button 2 is pressed
    }
  } // eternal loop
}

