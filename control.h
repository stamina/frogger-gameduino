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
// ATMEL ATMEGA1284p
//
//                                      +---\/---+
//              LED Shifter         PB0 |        |  PA0 ->   player 2 btn 1
//                                  PB1 |        |  PA1 ->   player 2 btn 2
//  GD pin 2                        PB2 |        |  PA2 ->   player 2 btn 3
//                                  PB3 |        |  PA3 ->   player 2 btn 4
//                              SS  PB4 |        |  PA4 ->   player 1 joy UP  
//                            MOSI  PB5 |        |  PA5 ->   player 1 joy DOWN
//                            MISO  PB6 |        |  PA6 ->   player 1 joy LEFT
//                             SCK  PB7 |        |  PA7 ->   player 1 joy RIGHT
//                                  RST |        |  AREF
//                                  VCC |        |  GND 
//                                  GND |        |  AVCC
//                                XTAL2 |        |  PC7 ->   player 2 joy UP  
//                                XTAL1 |        |  PC6 ->   player 2 joy DOWN
//                       USART RX0  PD0 |        |  PC5 ->   player 2 joy LEFT
//                       USART TX0  PD1 |        |  PC4 ->   player 2 joy RIGHT
//                       USART RX1  PD2 |        |  PC3 ->   player 1 btn 1
//                       USART TX1  PD3 |        |  PC2 ->   player 1 btn 2
//                                  PD4 |        |  PC1 ->   player 1 btn 3
// player 1 btn 5 ->                PD5 |        |  PC0 ->   player 1 btn 4
// player 2 btn 5 ->                PD6 |        |  PD7 ->   heartbeat led
//                                      +--------+
//
// Teensy ++ 2.0 AT90USB1286
//
//                        +---\/---+
//                     GND|        |+5V
// LED shifter         PB7|        |PB6
//                     PD0|        |PB5
//                     PD1|        |PB4
// RX USART/GD pin2    PD2|        |PB3 -> MISO
// TX USART            PD3|        |PB2 -> MOSI
//                     PD4|        |PB1 -> SCK
//                     PD5|        |PB0 -> SS  
//                     PD6|        |PE7
//                     PD7|        |PE6
// player 1 btn 1 ->   PE0|        |GND
// player 1 btn 2 ->   PE1|        |AREF
// player 1 btn 3 ->   PC0|        |PF0  ->  player 1 joy UP  
// player 1 btn 4 ->   PC1|        |PF1  ->  player 1 joy DOWN
// player 1 btn 5 ->   PC2|        |PF2  ->  player 1 joy LEFT
// player 2 btn 1 ->   PC3|        |PF3  ->  player 1 joy RIGHT
// player 2 btn 2 ->   PC4|        |PF4  ->  player 2 joy UP  
// player 2 btn 3 ->   PC5|        |PF5  ->  player 2 joy DOWN
// player 2 btn 4 ->   PC6|        |PF6  ->  player 2 joy LEFT
// player 2 btn 5 ->   PC7|        |PF7  ->  player 2 joy RIGHT
//                        +--------+
//
#ifndef _CONTROL_H_INCLUDED
#define __CONTROL_H_INCLUDED

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <util/delay.h>
#include "SPI.h"
#include "usart.h"

#if defined(__AVR_ATmega1284P__)                        // Brugmania ATMEGA1284p
#define PIN_PLAYER1_BTN1          PINC & (1 << PINC3) 
#define PIN_PLAYER1_BTN2          PINC & (1 << PINC2)
#define PIN_PLAYER1_BTN3          PINC & (1 << PINC1)
#define PIN_PLAYER1_BTN4          PINC & (1 << PINC0)
#define PIN_PLAYER1_BTN5          PIND & (1 << PIND5)
#define PIN_PLAYER2_BTN1          PINA & (1 << PINA0)
#define PIN_PLAYER2_BTN2          PINA & (1 << PINA1)
#define PIN_PLAYER2_BTN3          PINA & (1 << PINA2)
#define PIN_PLAYER2_BTN4          PINA & (1 << PINA3)
#define PIN_PLAYER2_BTN5          PIND & (1 << PIND6)
#define PIN_PLAYER1_JOY_UP        PINA & (1 << PINA4)   
#define PIN_PLAYER1_JOY_DOWN      PINA & (1 << PINA5)   
#define PIN_PLAYER1_JOY_LEFT      PINA & (1 << PINA6)   
#define PIN_PLAYER1_JOY_RIGHT     PINA & (1 << PINA7)   
#define PIN_PLAYER2_JOY_UP        PINC & (1 << PINC7)
#define PIN_PLAYER2_JOY_DOWN      PINC & (1 << PINC6)
#define PIN_PLAYER2_JOY_LEFT      PINC & (1 << PINC5)
#define PIN_PLAYER2_JOY_RIGHT     PINC & (1 << PINC4)
#define PIN_GAMEDUINO             PINB & (1 << PINB2)
#define PIN_LEDSHIFTER            PINB & (1 << PINB0)
#elif defined (__AVR_AT90USB1286__)                     // my Teensy++ 2.0
#define PIN_PLAYER1_BTN1          PINE & (1 << PINE0)
#define PIN_PLAYER1_BTN2          PINE & (1 << PINE1)
#define PIN_PLAYER1_BTN3          PINC & (1 << PINC0)
#define PIN_PLAYER1_BTN4          PINC & (1 << PINC1)
#define PIN_PLAYER1_BTN5          PINC & (1 << PINC2)
#define PIN_PLAYER2_BTN1          PINC & (1 << PINC3)
#define PIN_PLAYER2_BTN2          PINC & (1 << PINC4)
#define PIN_PLAYER2_BTN3          PINC & (1 << PINC5)
#define PIN_PLAYER2_BTN4          PINC & (1 << PINC6)
#define PIN_PLAYER2_BTN5          PINC & (1 << PINC7)
#define PIN_PLAYER1_JOY_UP        PINF & (1 << PINF0)
#define PIN_PLAYER1_JOY_DOWN      PINF & (1 << PINF1) 
#define PIN_PLAYER1_JOY_LEFT      PINF & (1 << PINF2)
#define PIN_PLAYER1_JOY_RIGHT     PINF & (1 << PINF3)
#define PIN_PLAYER2_JOY_UP        PINF & (1 << PINF4)
#define PIN_PLAYER2_JOY_DOWN      PINF & (1 << PINF5)
#define PIN_PLAYER2_JOY_LEFT      PINF & (1 << PINF6)
#define PIN_PLAYER2_JOY_RIGHT     PINF & (1 << PINF7)
#define PIN_GAMEDUINO             PIND & (1 << PIND2)
#define PIN_LEDSHIFTER            PINB & (1 << PINB7)
#endif

#define CONTROL_UP    1
#define CONTROL_DOWN  2
#define CONTROL_LEFT  4
#define CONTROL_RIGHT 8
#define CONTROL_BTN1  1
#define CONTROL_BTN2  2
#define CONTROL_BTN3  4
#define CONTROL_BTN4  8
#define CONTROL_BTN5  16

// proto's
void vis_control_init(void);
void vis_control_enable_p1(void);
void vis_control_disable_p1(void);
void vis_control_enable_p2(void);
void vis_control_disable_p2(void);
byte vis_control_read_buttons_p1(void);
byte vis_control_read_joystick_p1(void);
byte vis_control_read_buttons_p2(void);
byte vis_control_read_joystick_p2(void);

// globals
byte prev_joy_p1;
byte prev_btn_p1;
byte prev_joy_p2;
byte prev_btn_p2;
byte control_disabled_p1;
byte control_disabled_p2;

#endif

