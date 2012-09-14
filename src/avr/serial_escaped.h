/*
 *  Escape aware serial functions. Useful for reading command payload
 *  data.
 *
 *  Copyright 2012 Elovalo project group 
 *  
 *  This file is part of Elovalo.
 *  
 *  Elovalo is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  Elovalo is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with Elovalo.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>

// Serial protocol fundamentals
#define ESCAPE              '~'  // Escape character. Go to command mode
#define LITERAL_ESCAPE      '\0' // Escape followed by this is literal escape.

typedef struct {
	uint8_t good;
	uint8_t byte;
} read_t;

/**
 * Sends a string to serial port. Pointer must point to program memory
 * pointing to the actual string data in program memory. So, double
 * pointing and kinda complex types.
 */
void send_string_from_pgm(const char * const* pgm_p);

/**
 * Send n bytes of data starting from SRAM pointer src in escaped form
 * to serial port.
 */
void sram_to_serial(void *src, uint16_t n);

/**
 * Reads n bytes of escaped data from serial port to given SRAM
 * location. If it contains a command, stop reading. If reading was
 * interrupted due to command, returns false, otherwise true.
 */
bool serial_to_sram(void *dest, uint16_t n);

/**
 * Sends a byte and escapes it if necessary.
 */
void send_escaped(uint8_t byte);

/**
 * Reads a byte. If it is a command, do not consume input. This uses
 * blocking reads.
 */
read_t read_escaped();
