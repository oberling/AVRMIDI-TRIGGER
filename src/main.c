#include "uart.h"
#include "midi_datatypes.h"
#include "midibuffer.h"
#include "trigger_flame.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
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

#define TRIGGER_FLAME_PRESCALER	4

midibuffer_t midi_buffer;
uint8_t midi_channel = 4;
uint8_t trigger_flame_counter = TRIGGER_FLAME_PRESCALER;

trigger_output_t trigger_output[NUM_TRIGGER_OUTPUTS];
bool must_update_trigger_output = false;

bool must_update_flames = false;

bool midi_handler_function(midimessage_t* m);
void update_trigger_output(void);
void update_flames(void);
void init_trigger_outputs(void);
void init_io(void);
void init_variables(void);

bool midi_handler_function(midimessage_t* m) {
	// only return true if something of interest happened
	if(m->byte[0] == NOTE_ON(midi_channel) && m->byte[2] != 0) { // Velocity must not be 0
		if(m->byte[1]>=MIDI_NOTE_OFFSET && m->byte[1]<(MIDI_NOTE_OFFSET+NUM_TRIGGER_OUTPUTS)) { // if in range of triggering notes
			trigger_output[m->byte[1]-MIDI_NOTE_OFFSET].output_counter = TRIGGER_COUNTER_INIT;
			// use velocity for flame selection - 127 being the normal single trigger
			uint8_t flame_type = m->byte[2]>>3; // simple scale down division by 8
			trigger_output[m->byte[1]-MIDI_NOTE_OFFSET].flame_type = flame_type;
			trigger_output[m->byte[1]-MIDI_NOTE_OFFSET].flame_sequence_position = 0;
			trigger_output[m->byte[1]-MIDI_NOTE_OFFSET].flame_sequence_counter = pgm_read_byte(&(trigger_flame[flame_type][0]));
			return true;
		}
	}
	return false;
}

void update_trigger_output(void) {
	// only change output status - no decrement of counters here!
	uint8_t i=0;
	for(;i<NUM_TRIGGER_OUTPUTS;i++) {
		if(trigger_output[i].output_counter) {
			TRIGGER_PORT |= (1<<(TRIGGER_PIN_OFFSET+i));
		} else {
			TRIGGER_PORT &= ~(1<<(TRIGGER_PIN_OFFSET+i));
		}
	}
}

void update_flames(void) {
	uint8_t i=0;
	for(;i<NUM_TRIGGER_OUTPUTS;i++) {
		if(trigger_output[i].flame_sequence_counter) {
			// count down our current sequence counter
			trigger_output[i].flame_sequence_counter -= 1;
			// current sequence step is over - see if there are remaining steps
			if(
				trigger_output[i].flame_sequence_counter == 0 &&
				trigger_output[i].flame_sequence_position < NUM_FLAME_STEPS-1 && 
				pgm_read_byte(&(trigger_flame[trigger_output[i].flame_type][trigger_output[i].flame_sequence_position+1]))
			) {
					// initiate next step of sequence - trigger the output
					trigger_output[i].flame_sequence_position += 1;
					trigger_output[i].flame_sequence_counter = pgm_read_byte(&(trigger_flame[trigger_output[i].flame_type][trigger_output[i].flame_sequence_position]));
					trigger_output[i].output_counter = TRIGGER_COUNTER_INIT;
					must_update_trigger_output = true;
			}
		}
	}
}

void init_trigger_outputs(void) {
	uint8_t i=0;
	for(;i<NUM_TRIGGER_OUTPUTS;i++) {
		trigger_output[i].output_counter = 0;
		trigger_output[i].flame_sequence_position = 0;
		trigger_output[i].flame_sequence_counter = 0;
		trigger_output[i].flame_type = 0;
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
	init_trigger_outputs();
}

ISR(TIMER0_OVF_vect) {
	// decrement counters here - trigger update of outputs if trigger is over
	uint8_t i=0;
	for(;i<NUM_TRIGGER_OUTPUTS;i++) {
		if(trigger_output[i].output_counter) {
			trigger_output[i].output_counter -= 1;
			if(trigger_output[i].output_counter == 0) {
				must_update_trigger_output = true;
			}
		}
	}
	trigger_flame_counter++;
	if(trigger_flame_counter > TRIGGER_FLAME_PRESCALER) {
		trigger_flame_counter = 0;
		must_update_flames = true;
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
		if(must_update_flames) {
			// could be done in the interrupt routine as well but has too many instructions
			must_update_flames = false;
			update_flames();
		}
	}
	return 0;
}

