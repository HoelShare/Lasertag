#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <string.h>
#include <util/delay.h>
#include "i2c_slave_usi.h"

#define NO 0
#define YES 1

#define RED OCR1B
#define GREEN OCR1A
#define BLUE OCR0A

//UART http://www.mikrocontroller.net/articles/AVR-GCC-Tutorial/Der_UART
#define BAUD 1200UL      // Baudrate

// Berechnungen
#define UBRR_VAL ((F_CPU+BAUD*8)/(BAUD*16)-1)   // clever runden
#define BAUD_REAL (F_CPU/(16*(UBRR_VAL+1)))     // Reale Baudrate
#define BAUD_ERROR ((BAUD_REAL*1000)/BAUD) // Fehler in Promille, 1000 = kein Fehler.
 
#if ((BAUD_ERROR<990) || (BAUD_ERROR>1010))
  #error Systematischer Fehler der Baudrate grösser 1% und damit zu hoch! 
#endif

#define BIT_SET(port, mask, value) {if (value) {port |= (mask);} else {port &= ~(mask);}}

volatile uint8_t alive = YES;
volatile uint8_t color_buffer[8];

ISR(USART_RX_vect){
	static unsigned char lastbyte = 0;
	unsigned char ch = UDR;
	if(lastbyte == 'a' && ch == 'b'){
		alive = NO;
	}
	lastbyte = ch;
}

void long_delay(uint16_t ms)
{
	for(; ms>0; ms--) _delay_ms(1);
}

void i2c_slave_poll_buffer(unsigned char reg_addr, volatile unsigned char** buffer, volatile unsigned char* buffer_length){
	if (reg_addr < 6){
		*buffer = &color_buffer[reg_addr];
		*buffer_length = 6-reg_addr;
	} else {
// 		*buffer = i2c_buffer;
// 		*buffer_length = 16;
		*buffer_length = 0;
		*buffer = 0;
	}
}
void i2c_slave_write_complete(void){
	
}
void i2c_slave_read_complete(void){
	
}

int main(void) {
	usi_i2c_init(0x20);
	sei();                  // Interrupts global einschalten
	
	DDRB |= _BV(DDB2) | _BV(DDB3) | _BV(DDB4);//LEDs

	//PWM, Phase Correct, 8-bit, no presacler
	TCCR0A = _BV(WGM00) | _BV(COM0A1);
	TCCR0B = _BV(CS00);
	TCCR1A = _BV(WGM10) | _BV(COM1A1) | _BV(COM1B1);
	TCCR1B = _BV(CS00);
	
	RED = 0;
	GREEN = 0;
	BLUE = 0;
	//UART init
	UBRRH = UBRR_VAL >> 8;
	UBRRL = UBRR_VAL & 0xFF;
	UCSRC = _BV(UCSZ1) | _BV(UCSZ0);  // Asynchron 8N1 
	UCSRB |= _BV(RXEN)| _BV(RXCIE);  // UART RX und RX Interrupt einschalten
	
	while(1){
		if(alive){
			RED = 10;
		} else {
			RED = 0;
			long_delay(50);
			alive = YES;
		}
	}
	return 0;
}

