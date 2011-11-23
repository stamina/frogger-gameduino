/*
 * Controller code for "Brugmania" Arcade Cabinet
 *
 * Created by Bas Brugman
 * http:// www.visionnaire.nl 2011
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
#include "control.h"

// setup all button and joystick pins to input and pull-up, so that
// we'll get a read when a button/joystick switch is connected to ground (pulled low)
void vis_control_init() {
#if defined(__AVR_ATmega1284P__)
  // joystick p1 and 4 buttons p2 (4x p1, 4x p2)
  DDRA = 0x00; // set as input
  PORTA = 0xFF; // set pull-ups
  // joystick p2 and 4 buttons p1 (4x p2, 4x p1)
  DDRC = 0x00; // set as input
  PORTC = 0xFF; // set pull-ups
  // 1 button p1 and 1 button p2 ( 1x p1, 1x p2)
  DDRD |= 0x80; // heartbeat led as output
  DDRD &= 0x9F; // set as 2 inputs (1001 1111)
  PORTD |= 0xE0; // set as 2 pull-ups (1110 0000) + heartbeat led
#elif defined (__AVR_AT90USB1286__) // my Teensy++ 2.0
  // 8 pins for 2 joysticks
  DDRF = 0x00; // set as input
  PORTF = 0xFF; // set pull-ups
  // 8 buttons (3x p1, 5x p2)
  DDRC = 0x00; // set as input
  PORTC = 0xFF; // set pull-ups
  // first 2 buttons from p1
  DDRE &= 0xFC; // set as 2 inputs (1111 1100)
  PORTE |= 0x03; // set as 2 pull-ups (0000 0011)
  DDRB |= 0x80; // led shifter as output
  PORTB |= 0x80; // high
#endif
  prev_joy_p1 = 0;
  prev_btn_p1 = 0;
  prev_joy_p2 = 0;
  prev_btn_p2 = 0;
}

// enable button/joystick control for player 1
void vis_control_enable_p1() {
  control_disabled_p1 = 0;
}

// enable button/joystick control for player 2
void vis_control_enable_p2() {
  control_disabled_p2 = 0;
}

// disable button/joystick control for player 1
void vis_control_disable_p1() {
  control_disabled_p1 = 1;
}

// disable button/joystick control for player 2
void vis_control_disable_p2() {
  control_disabled_p2 = 1;
}

//Set read bits from control
byte vis_control_read_buttons_p1()
{
  if (control_disabled_p1)
    return 0;
  byte r = 0;
  if (~PIN_PLAYER1_BTN1)
    r |= CONTROL_BTN1;
  if (~PIN_PLAYER1_BTN2)
    r |= CONTROL_BTN2;
  if (~PIN_PLAYER1_BTN3)
    r |= CONTROL_BTN3;
  if (~PIN_PLAYER1_BTN4)
    r |= CONTROL_BTN4;
  if (~PIN_PLAYER1_BTN5)
    r |= CONTROL_BTN5;
  byte edge = r & ~prev_btn_p1; // disable all previous reads, to prevent bouncing
  prev_btn_p1 = r;
  return edge;
}

// Set read bits from control
byte vis_control_read_joystick_p1()
{
  if (control_disabled_p1)
    return 0;
  byte r = 0;
  if (~PIN_PLAYER1_JOY_UP)
    r |= CONTROL_UP;
  if (~PIN_PLAYER1_JOY_DOWN)
    r |= CONTROL_DOWN;
  if (~PIN_PLAYER1_JOY_LEFT)
    r |= CONTROL_LEFT;
  if (~PIN_PLAYER1_JOY_RIGHT)
    r |= CONTROL_RIGHT;
  byte edge = r & ~prev_joy_p1; // disable all previous reads, to prevent bouncing
  prev_joy_p1 = r;
  return edge;
}

// Set read bits from control
byte vis_control_read_buttons_p2()
{
  if (control_disabled_p2)
    return 0;
  byte r = 0;
  if (~PIN_PLAYER2_BTN2)
    r |= CONTROL_BTN2;
  if (~PIN_PLAYER2_BTN2)
    r |= CONTROL_BTN2;
  if (~PIN_PLAYER2_BTN3)
    r |= CONTROL_BTN3;
  if (~PIN_PLAYER2_BTN4)
    r |= CONTROL_BTN4;
  if (~PIN_PLAYER2_BTN5)
    r |= CONTROL_BTN5;
  byte edge = r & ~prev_btn_p2; // disable all previous reads, to prevent bouncing
  prev_btn_p2 = r;
  return edge;
}

// Set read bits from control
byte vis_control_read_joystick_p2()
{
  if (control_disabled_p2)
    return 0;
  byte r = 0;
  if (~PIN_PLAYER2_JOY_UP)
    r |= CONTROL_UP;
  if (~PIN_PLAYER2_JOY_DOWN)
    r |= CONTROL_DOWN;
  if (~PIN_PLAYER2_JOY_LEFT)
    r |= CONTROL_LEFT;
  if (~PIN_PLAYER2_JOY_RIGHT)
    r |= CONTROL_RIGHT;
  byte edge = r & ~prev_joy_p2; // disable all previous reads, to prevent bouncing
  prev_joy_p2 = r;
  return edge;
}

