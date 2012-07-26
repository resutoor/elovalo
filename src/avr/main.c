/**
 * Ledikuutio Main...
 * TODO: 	Make sure that SRAM usage stays as low as possible,
 * 			avoid using local variables or declaring variables at runtime.
 *
 * TODO: 	Check if there's way to monitor the stack usage at runtime
 * 			to avoid stack overflow when pushing close the SRAM limit.
 *
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include "pinMacros.h"
#include "main.h"
#include "init.h"
#include "tlc5940.h"
#include "serial.h"
#include "../cube.h"

#define ESCAPE 0x7e

#define DEFAULT 0x00
#define STANDALONE 0x01
#define SERIAL_SLAVE 0x02

uint8_t state=0x00;

int main() {

	cli();

	disableWDT();
	initPorts();
	initSPI();

	initTLC5940();
	initBLANKTimer();

	initUSART();
	sei();

	InitGScycle(); //TODO: Send first byte to the SPI bus...

	while(1){

		if(serial_available()){

			serial_send(0xf5);

			if(state==DEFAULT){
				//wait command from usart and process it

				processCommand();
			}
			else{
				stateMachine();
			}

		}

		if(state == STANDALONE){
			animSnake();
		}

		if(state == SERIAL_SLAVE){
			//put the current byte to the backbuffer unelss it's literal escape
		}

	}

	return 0;
}

void animSnake() {
	// Some variables used over multiple invocations
	static uint8_t i = 1;
	static uint8_t apu = 1; /* we need this in order to determine
				 * if the non-skipped number is odd */

	// Clear backbuffer once every frame...
	if (isAfterFlip) {
		clearArray(gs_buf_back, 24*TLC5940);
		
		if (i < (25*TLC5940)) {
			
			if(i%3==0) { //Skip!
				i++;
				apu=1; //we need to reset the helper
			}

			if(apu==1) { //Odd
				gs_buf_back[i-1]=0xff;
				gs_buf_back[i]=0xf0;
			} else { //even
				gs_buf_back[i-1]=0x0f;
				gs_buf_back[i]=0xff;
			}
			apu++;
		}
		
		i++;
		
		if(i==24*TLC5940){ //Ending cell, reset EVERYTHING
			i=1;
			apu=1;
			//clearArray(BackBuffer, 24*TLC5940);
		}
		
		isAfterFlip = 0;
	}
}

void processCommand(){
	switch (serial_read()) {
		case ESCAPE:

				state = DEFAULT;
			break;

		case STANDALONE:

				state = STANDALONE;
			break;

		case SERIAL_SLAVE:

				state = SERIAL_SLAVE;
			break;

		default:
			//keep the current state
			break;
	}
}

void stateMachine(){
	switch (serial_read()) {
		case ESCAPE:
			while(!serial_available());
			if(serial_read()==0x00){
				//literal escape
				//state = 0x00
			}
			else{
				state = 0x00;
			}
			break;

		default:
			//do whatever in the current state
			break;
	}
}

void clearArray(volatile uint8_t *arr, uint8_t len) {

	for (uint8_t r = 0; r < len; r++) {
		arr[r] = 0x00;
	}

}

////Generic SPI master transmission
////Return slave data.
//void SPI_Transfer(uint8_t cData)
//{
//	/* Start transmission */
//	PORTB |= (1<<PB1);
//	SPDR = cData; //Send byte
//	/* Wait for transmission complete */
//	while(!(SPSR & (1<<SPIF)));
//	PORTB &= ~(1<<PB1);
//}

////SPI Transfer for TLC5940
//void SPI_Transfer_TLC5940(uint8_t FrontBuffer[])
//{
//	GSdataCounter=0;
//	/* Start transmission */
//	PORTB &= ~(1<<PB1);
//
//	//notTransferring=0; //we're transferring data...
//	SPDR = FrontBuffer[0]; //Send first byte to initialize the ISR managed transfer
//
//	/* Wait for transmission complete */
//	//while(!(SPSR & (1<<SPIF)));
//	//PORTB &= ~(1<<PB2);
//}


//If an interrupt happens and there isn't an interrupt handler, we go here!
ISR(BADISR_vect)
{
	pin_high(DEBUG_LED); //Give us an indication about an error condition...
	while(1){
		}
}