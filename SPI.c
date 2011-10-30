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
#include "SPI.h"

// init the SPI
void vis_spi_begin() {
  SPI_DDR |= (1 << SPI_SS_PIN) | (1 << SPI_MOSI_PIN) | (1 << SPI_SCK_PIN); // set to output pins
  SPI_DDR &= ~(1 << SPI_MISO_PIN); // set to input pin
  SPI_PORT |= (1 << SPI_SS_PIN); // set SS to high, if this goes low, it will start talking to the slave
  SPI_PORT &= ~(1 << SPI_MOSI_PIN); // set to low
  SPI_PORT &= ~(1 << SPI_MISO_PIN); // set to low
  SPI_PORT &= ~(1 << SPI_SCK_PIN); // set to low
  SPCR |= (1 << MSTR); // set to master mode
  SPCR |= (1 << SPE); // enable all SPI operations
}

// disable all SPI operations
void vis_spi_end() {
  SPCR &= ~(1 << SPE);
}

// start the actual SPI byte transmission
byte vis_spi_transfer(byte data) {
  SPDR = data; // transmit the byte to be sent
  while (!(SPSR & (1 << SPIF))); // wait for the transfer to complete
  return SPDR; // then return the byte the slave just returned
}

// set SPI data order (lsb or msb first)
void vis_spi_setbitorder(uint8_t bitorder)
{
  if (bitorder == SPI_LSBFIRST) {
    SPCR |= (1 << DORD);
  } else {
    SPCR &= ~(1 << DORD);
  }
}

// set SPI mode (rising/falling sample/setup)
void vis_spi_setdatamode(uint8_t mode)
{
  SPCR = (SPCR & ~SPI_MODE_MASK) | mode; // clear 2nd/3rd bit and set new mode bits
}

// set SPI clock speed
void vis_spi_setclockdivider(uint8_t rate)
{
  SPCR = (SPCR & ~SPI_CLOCK_MASK) | (rate & SPI_CLOCK_MASK); // clear the last 2 lsb bits and set them to the rate constant
  SPSR = (SPSR & ~SPI_2XCLOCK_MASK) | ((rate >> 2) & SPI_2XCLOCK_MASK); // clear the last lsb bit and set new SPI2X bit by shifting rate 2 places to the right
}

// enable interrupt flag
void vis_spi_attachinterrupt() {
  SPCR |= (1 << SPIE);
}

// disable interrupt flag
void vis_spi_detachinterrupt() {
  SPCR &= ~(1 << SPIE);
}

