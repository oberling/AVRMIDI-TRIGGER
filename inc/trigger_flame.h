#ifndef _TRIGGER_FLAME_H_
#define _TRIGGER_FLAME_H_

#include <stdint.h>
#include <avr/pgmspace.h>

#define NUM_FLAME_STEPS	5

static const uint8_t trigger_flame[16][NUM_FLAME_STEPS] PROGMEM = 
{
	{10,20,30,40,50},
	{10,15,20,40,80},
	{80,80,80,80,80},
	{40,40,40,40,40},
	{20,20,20,20,20},
	{10,10,10,10,10},
	{40,40,40, 0, 0},
	{20,20,20, 0, 0},
	{10,10,10, 0, 0},
	{80,80, 0, 0, 0},
	{40,40, 0, 0, 0},
	{20,20, 0, 0, 0},
	{10,10, 0, 0, 0},
	{40, 0, 0, 0, 0},
	{10, 0, 0, 0, 0},
	{ 0, 0, 0, 0, 0}
};

typedef struct {
	// usually one would also store the PORT and Pin here - but that takes up too much
	// valuable data space
	// volatile uint8_t* output_port;
	// uint8_t output_pin;
	uint8_t output_counter;
	uint8_t flame_sequence_position;
	uint8_t flame_sequence_counter;
	uint8_t flame_type;
} trigger_output_t;

#endif
