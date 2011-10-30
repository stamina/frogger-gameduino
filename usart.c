/*
 * USART functionality, based on Teensy code : www.pjrc.com by Paul Stoffregen
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
#include <avr/io.h>
#include <avr/interrupt.h>
#include  "usart.h"

//  Initialization
void init_usart()
{
  cli(); // Disable global interrupts
	UBRR1 = (F_CPU / 4 / USART_BAUDRATE - 1) / 2; // set baudrate
	UCSR1A = (1 << U2X1); // enable 2x speed
	UCSR1B = (1 << RXEN1) | (1 << TXEN1) | (1 << RXCIE1); // Turn on the reception and transmission circuitry and Reception Complete Interrupt
	UCSR1C = (1 << UCSZ11) | (1 << UCSZ10); // set 8 bits data size (default no parity bit, 1 stop bit)
  tx_buffer_head = tx_buffer_tail = 0; // init buffer
  rx_buffer_head = rx_buffer_tail = 0; // init buffer
  sei(); // Enable global interrupts
}

// Transmit a byte
void uart_putchar(uint8_t c)
{
	uint8_t i;
	i = tx_buffer_head + 1; // advance head
	if (i >= TX_BUFFER_SIZE) i = 0; // go to first index if buffer full
	while (tx_buffer_tail == i); // wait until space in buffer
	tx_buffer[i] = c; // put char in buffer
	tx_buffer_head = i; // set new head
	UCSR1B = (1<<RXEN1) | (1<<TXEN1) | (1<<RXCIE1) | (1<<UDRIE1);// Turn on the reception and transmission circuitry and Reception Complete and Transmit Complete Interrupt
}

// Receive a byte
uint8_t uart_getchar(void)
{
  uint8_t c, i;
	while (rx_buffer_head == rx_buffer_tail); // wait for character
  i = rx_buffer_tail + 1; // advance tail
  if (i >= RX_BUFFER_SIZE) i = 0; // got to first index if buffer full
  c = rx_buffer[i]; // get char from buffer
  rx_buffer_tail = i; // set new tail
  return c; // return char
}

// Writes a string from flash to the uart
void uart_print_P(const char *str)
{
	char c;
	while (1) {
		c = pgm_read_byte(str++);
		if (!c) break;
		uart_putchar(c);
	}
}

// Transmit Interrupt
ISR(USART1_UDRE_vect)
{
	uint8_t i;
	if (tx_buffer_head == tx_buffer_tail) {
		UCSR1B = (1<<RXEN1) | (1<<TXEN1) | (1<<RXCIE1); // buffer is empty, disable transmit interrupt
	} else { // fill transmit register with next byte to send
		i = tx_buffer_tail + 1; // get tail + 1
		if (i >= TX_BUFFER_SIZE) i = 0; // go to first index if buffer full
		UDR1 = tx_buffer[i]; // send byte
		tx_buffer_tail = i; // set new tail
	}
}

// Receive Interrupt
ISR(USART1_RX_vect)
{
	uint8_t c, i;
	c = UDR1; // receive byte
	i = rx_buffer_head + 1; // advance head 
	if (i >= RX_BUFFER_SIZE) i = 0; // go to first index if buffer full
	if (i != rx_buffer_tail) { // not empty
		rx_buffer[i] = c; // put in read buffer
		rx_buffer_head = i; // set new head
	}
}

// Return the number of bytes waiting in the receive buffer.
// Call this before uart_getchar() to check if it will need
// to wait for a byte to arrive.
uint8_t uart_available(void)
{
	uint8_t head, tail;
	head = rx_buffer_head;
	tail = rx_buffer_tail;
	if (head >= tail) return head - tail; // return count of bytes inbetween
	return RX_BUFFER_SIZE + head - tail; // head has rolled over to start, return count of bytes inbetween
}

