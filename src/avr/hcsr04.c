/**
 * Device driver for two HC-SR04 ultrasound sensors
 * Sensor 0 is connected to PC4 and PC5.
 * Sensor 1 is connected to PC2 and PC3.
 */

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <inttypes.h>
#include "hcsr04.h"

//values for state
#define ST_IDLE 0
#define ST_SENDING_START_PULSE 1
#define ST_WAITING_RESPONSE_PULSE 2
#define ST_MEASURING_RESPONSE_PULSE 3
#define ST_WAITING_ECHO_FADING_AWAY 4

volatile uint8_t state = ST_IDLE;

//values for oper_mode
#define SINGLE_SHOT 0
#define CONTINUOUS 1

volatile uint8_t oper_mode = SINGLE_SHOT;

#define SENSOR_COUNT 2 //can not be bigger than 2 without code changes
volatile uint8_t current_sensor = 0;

//the result of the latest measurement
volatile uint16_t resp_pulse_length[SENSOR_COUNT];

int send_pulse(void);

/**
 * Initialize the timer for a new measurement cycle
 */
void timer_init(void)
{
	//TCCR1A &= ~(1<<WGM11 | 1<<WGM10); //no need to do this, bits are cleared by default
	//TCCR1B &= ~(1<<WGM13 | 1<<WGM12); //no need to do this, bits are cleared by default
	TCCR1B |= (1 << WGM12);	// sets compare and reset to "top" mode
	TCCR1B |= (1 << CS10);	// set clock divider to 1

	OCR1A = 160;		// set "top" for 10 us (16 MHz sysclock in use)
	// TODO: the actual pulse length is 10 ms! Why?
	TCNT1 = 0;
	TIFR1 |= (1 << OCF1A);	// clear possible pending int
	TIMSK1 |= (1 << OCIE1A);	// enable int
}

/**
 * This int handler is called twice during measurement cycle
 *  - when it is time to generate the trailing edge of the start pulse
 *  - when the measurement cycle ends
 */
ISR(TIMER1_COMPA_vect)
{

	if (state == ST_SENDING_START_PULSE) {

		PORTC &= ~(1 << (current_sensor == 0 ? PC5 : PC3));	// force trailing edge of the pulse

		//re-init TIMER1 for measuring response pulse length
		TCCR1B &= ~(1 << CS12 | 1 << CS11 | 1 << CS10);	// stop timer
		OCR1A = 6000;	// set "top" for 96 ms (16 MHz sysclock in use)
		TCNT1 = 0;
		TCCR1B |= (1 << CS12);	// start timer, set clock divider to 256

		state = ST_WAITING_RESPONSE_PULSE;
	} else {
		//normally the state should be ST_WAITING_ECHO_FADING_AWAY here
		//but clear state machine also in all other cases

		if (state == ST_MEASURING_RESPONSE_PULSE) {
			// This situation happens when there is no
			// object in the beam area
			resp_pulse_length[current_sensor] = HCSR04_MEAS_FAIL;
		}

		TIMSK1 &= ~(1 << OCIE1A);	// disable timer int
		PCICR &= ~(1 << PCIE1);	// disable PIN Change int
		state = ST_IDLE;

		if (oper_mode == CONTINUOUS) {
			current_sensor++;
			if (current_sensor >= SENSOR_COUNT)
				current_sensor = 0;
			send_pulse();
		}
	}
}

/**
 * Initialize the I/O pins for a new measurement cycle
 */
void pin_init(void)
{
	DDRC |= (1 << PC5);	//output
	DDRC &= ~(1 << PC4);	//input

	//Enable PIN Change Interrupt 1 - This enables interrupts on pins
	//PCINT14...8, see doc8161.pdf Rev. 8161D – 10/09, ch 12
	PCICR |= (1 << PCIE1);

	//Set the mask on Pin change interrupt 1 so that only right pin triggers
	//the interrupt. see doc8161.pdf Rev. 8161D – 10/09, ch 12.2.1
	PCMSK1 |= (1 << (current_sensor == 0 ? PCINT12 : PCINT10));
	return;
}

/**
 * This int handler is called twice during measurement cycle
 *  - when it is time to generate the trailing edge of the start pulse
 *  - when the measurement cycle ends
 */
ISR(PCINT1_vect)
{

	register uint8_t leading_edge = PINC & (1 << (current_sensor == 0 ? PINC4 : PINC2));
	if (state == ST_WAITING_RESPONSE_PULSE) {
		if (leading_edge) {
			TCNT1 = 0; //restart counting
			state = ST_MEASURING_RESPONSE_PULSE;
		}
		//else{
		//trailing edge, just fall through
		//}
	} else if (state == ST_MEASURING_RESPONSE_PULSE) {
		if (!leading_edge) {
			resp_pulse_length[current_sensor] = TCNT1;
			PCICR &= ~(1 << PCIE1);	//Disable PIN Change Interrupt 1
			state = ST_WAITING_ECHO_FADING_AWAY;
		}
		//else{
		//leading edge, just fall through
		//}
	}
	//else{
	//should not happen
	//}
}

int send_pulse(void)
{
	if (state != ST_IDLE)
		return 0;

	pin_init();
	PORTC |= (1 << (current_sensor == 0 ? PC5 : PC3));	// the leading edge of the pulse
	timer_init();

	state = ST_SENDING_START_PULSE;

	return 1;
}

int hcsr04_send_pulse(uint8_t sensor)
{
	if (oper_mode == CONTINUOUS)
		return 0;
	if (sensor >= SENSOR_COUNT)
		return 0;

	current_sensor = sensor;
	return send_pulse();
}

int hcsr04_start_continuous_meas(void)
{
	int ret_val = send_pulse();

	if (ret_val)
		oper_mode = CONTINUOUS;

	return ret_val;
}

void hcsr04_stop_continuous_meas(void)
{
	oper_mode = SINGLE_SHOT;
}

uint16_t hcsr04_get_pulse_length(uint8_t sensor)
{
	//Note! input param not checked
	return resp_pulse_length[sensor];
}

uint16_t hcsr04_get_distance_in_cm(uint8_t sensor)
{
	//Note! input param not checked
	if (resp_pulse_length[sensor] == HCSR04_MEAS_FAIL)
		return resp_pulse_length[sensor];

	// 36 pulses per 10 cm, when clock divider is 256 in TCCR1B
	return (10 * resp_pulse_length[sensor]) / 36;
}
