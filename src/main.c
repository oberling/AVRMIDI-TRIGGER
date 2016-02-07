#include "uart.h"
#include "midi_datatypes.h"
#include "midibuffer.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>

#define TRIGGER_PORT	PORTB
#define TRIGGER_DDR		DDRB
#define TRIGGER1		PB2
#define TRIGGER2		PB3
#define TRIGGER3		PB4
#define TRIGGER4		PB5
#define TRIGGER5		PB6
#define TRIGGER6		PB7
#define TRIGGER_PIN_OFFSET	2
#define NUM_TRIGGER_OUTPUTS		6
#ifndef TRIGGER_COUNTER_INIT
#pragma message "TRIGGER_COUTNER_INIT not defined - defaulting to 20"
#define TRIGGER_COUNTER_INIT	20
#endif
#ifndef MIDI_NOTE_OFFSET
#pragma message "MIDI_NOTE_OFFSET not defined - defaulting to 60"
// 60 is the middle C
#define MIDI_NOTE_OFFSET	60
#endif

midibuffer_t midi_buffer;
uint8_t midi_channel = 4;

uint8_t trigger_counter[NUM_TRIGGER_OUTPUTS];
bool must_update_trigger_output = false;

bool midi_handler_function(midimessage_t* m);
void update_trigger_output(void);
void init_io(void);
void init_variables(void);

bool midi_handler_function(midimessage_t* m) {
	// only return true if something of interest happened
	if(m->byte[0] == NOTE_ON(midi_channel) && m->byte[2] != 0) { // Velocity must not be 0
		if(m->byte[1]>=MIDI_NOTE_OFFSET && m->byte[1]<(MIDI_NOTE_OFFSET+NUM_TRIGGER_OUTPUTS)) { // if in range of triggering notes
			trigger_counter[m->byte[1]-MIDI_NOTE_OFFSET] = TRIGGER_COUNTER_INIT;
			return true;
		}
	}
	return false;
}

void update_trigger_output(void) {
	// only change output status - no decrement of counters here!
	uint8_t i=0;
	for(;i<NUM_TRIGGER_OUTPUTS;i++) {
		if(trigger_counter[i]) {
			TRIGGER_PORT |= (1<<(TRIGGER_PIN_OFFSET+i));
		} else {
			TRIGGER_PORT &= ~(1<<(TRIGGER_PIN_OFFSET+i));
		}
	}
}

void init_io(void) {
	TRIGGER_DDR |= (1<<TRIGGER1)|(1<<TRIGGER2)|(1<<TRIGGER3)|(1<<TRIGGER4)|(1<<TRIGGER5)|(1<<TRIGGER6);
	DDRD |= (1<<PD5);
	TCCR0B = (1<<CS01); // at 8000000Hz/8 = 1000000Hz -> 1000000Hz/256 = 3906.25Hz -> 1/3906.25Hz = 256µs
	TIMSK |= (1<<TOIE0);
	uart_init();
}

void init_variables(void) {
	midibuffer_init(&midi_buffer, &midi_handler_function);
}

ISR(TIMER0_OVF_vect) {
	// decrement counters here - trigger update of outputs if trigger is over
	uint8_t i=0;
	for(;i<NUM_TRIGGER_OUTPUTS;i++) {
		if(trigger_counter[i]) {
			trigger_counter[i]-=1;
			if(trigger_counter[i] == 0) {
				must_update_trigger_output = true;
			}
		}
	}
}

ISR(USART_RX_vect) {
	char a;
	uart_getc(&a);
	// this method only affects the writing position in the midibuffer
	// therefor it's ISR-save as long as the buffer does not run out of
	// space!!! prepare your buffers, everyone!
	midibuffer_put(&midi_buffer, a);
}

int main(int argc, char** argv) {
	cli();
	init_variables();
	init_io();
	sei();
	while(true) {
		if(midibuffer_tick(&midi_buffer)) {
			must_update_trigger_output = true;
		}
		if(must_update_trigger_output) {
			must_update_trigger_output = false;
			update_trigger_output();
		}
	}
	return 0;
}

