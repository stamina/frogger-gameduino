/*
 * SPI functionality for AVR's
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
#ifndef _SPI_H_INCLUDED
#define _SPI_H_INCLUDED

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdint.h>

// NOTE1: Extend these port/pin settings for other boards
#if defined(__AVR_ATmega1284P__) // my custom board
#define SPI_SS_PIN PB4 
#define SPI_MOSI_PIN PB5
#define SPI_MISO_PIN PB6
#define SPI_SCK_PIN PB7
#define SPI_DDR DDRB
#define SPI_PORT PORTB
#elif defined (__AVR_AT90USB1286__) // my Teensy++ 2.0
#define SPI_SS_PIN PB0 
#define SPI_MOSI_PIN PB2
#define SPI_MISO_PIN PB3
#define SPI_SCK_PIN PB1
#define SPI_DDR DDRB
#define SPI_PORT PORTB
#endif

#define SPI_LSBFIRST 0 // shift out least significant bit first
#define SPI_MSBFIRST 1 // shift out most significant bit first

// SPR [1:0] of SPCR  and SPI2X bit of SPSR register decides frequency of SCK. The combination of these three bits will be used to select SCK frequency
#define SPI_CLOCK_DIV4 0x00
#define SPI_CLOCK_DIV16 0x01
#define SPI_CLOCK_DIV64 0x02
#define SPI_CLOCK_DIV128 0x03
#define SPI_CLOCK_DIV2_2X 0x04 // SPI2x coming into play now
#define SPI_CLOCK_DIV8_2X 0x05
#define SPI_CLOCK_DIV32_2X 0x06
#define SPI_CLOCK_DIV64_2X 0x07

// SPI modes (rising/falling sample/setup)
#define SPI_MODE0 0x00
#define SPI_MODE1 0x04
#define SPI_MODE2 0x08
#define SPI_MODE3 0x0C

#define SPI_MODE_MASK 0x0C  // CPOL = bit 3, CPHA = bit 2 on SPCR
#define SPI_CLOCK_MASK 0x03  // SPR1 = bit 1, SPR0 = bit 0 on SPCR
#define SPI_2XCLOCK_MASK 0x01  // SPI2X = bit 0 on SPSR

typedef uint8_t boolean; // for convenience
typedef uint8_t byte; // for convenience
#define highByte(w) ((w) >> 8)
#define lowByte(w) ((uint8_t) ((w) & 0xff))

// proto's
void vis_spi_begin(void);
void vis_spi_end(void);
byte vis_spi_transfer(byte data);
void vis_spi_setbitorder(uint8_t bitorder);
void vis_spi_setdatamode(uint8_t mode);
void vis_spi_setclockdivider(uint8_t rate);
void vis_spi_attachinterrupt(void);
void vis_spi_detachinterrupt(void);

#endif

