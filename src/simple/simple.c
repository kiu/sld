#include "simple.h"

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

volatile uint16_t leds = 0;

// Interrupt for button press
ISR (INT1_vect) {
	leds = 0;
}

void sendSPI(uint16_t data) {
	
	cbi(PORTB, PB2);  // Latch

	SPDR = (uint8_t) ((data & 0xFF00) >> 8);
	while (!(SPSR & (1 << SPIF)));
	SPDR; // Clear SPIF

	SPDR = (uint8_t) (data & 0x00FF);
	while (!(SPSR & (1 << SPIF)));
	SPDR; // Clear SPIF

	sbi(PORTB, PB2);  // Latch

}

int main (void) {

	// Prepare Ports
	sbi(DDRB, PB1);  // LED Output Enable Out
	sbi(DDRB, PB2);  // LED Latch Out
	sbi(DDRB, PB3);  // LED MOSI Out
	sbi(DDRB, PB5);  // LED SCK Out

	cbi(DDRD, PD3);  // Button In
	sbi(PORTD, PD3);  // Button In Pullup

	// Prepare SPI
	SPCR = (1 << MSTR) | (1 << SPE); // clk/4
	SPSR |= (1 << SPI2X);

	cbi(PORTB, PB1);  // LED Output Enable On

	EIMSK = 1 << INT1; // Enable INT1
	sei(); // Enable interrupts

	// GO GO GO
	while (1) {
		sendSPI(leds);
		_delay_ms(1000);

		leds++;
		if (leds > 8191) {
			leds = 0;
		}

	}

	return 0;
}

