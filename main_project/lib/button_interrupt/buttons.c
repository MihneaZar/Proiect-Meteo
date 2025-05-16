#include <avr/io.h>
#include <avr/interrupt.h>
#include <buttons.h>

#include "timer.h"

volatile uint8_t blue_button = 0; 
volatile uint8_t red_button = 0;
uint32_t last_blue = 0;
uint32_t last_red = 0;

// PCINT18 / PD2 / 2 (RX) on Arduino Uno -> blue button
// PCINT19 / PD3 / 3 (TX) on Arduino UNO -> red button
void Buttons_init() {
	DDRD &= ~(1 << PD2) | ~(1 << PD3);
    PORTD |= (1 << PD2) | (1 << PD3);

    // interrupt on port D
    PCICR |= (1 << PCIE2);

    // interrupt on pins PCINT18 and PCINT19
    PCMSK2 |= (1 << PCINT18) | (1 << PCINT19);

    // any edge generates interrupt
    EICRA |= (1 << ISC00);
}

// intrerupere on D
ISR(PCINT2_vect) {
    // interrupt on PCINT18 (inverse logic)
    if (!(PIND & (1 << PD2)) && SYSTICKS_PASSED(last_blue, 200)) {
        last_blue = systicks;
        blue_button = 1;
    }
    // interrupt on PCINT19 (inverse logic)
    if (!(PIND & (1 << PD3)) && SYSTICKS_PASSED(last_red, 200)) {
        last_red = systicks;
        red_button = 1;
    }
}