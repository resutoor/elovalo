/* -*- mode: c; c-file-style: "linux" -*-
 *  vi: set shiftwidth=8 tabstop=8 noexpandtab:
 *
 *  Copyright 2012 Elovalo project group
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * debug.c
 *
 *  Contains all kind of debugging related helper functions for AVR.
 */

#include <util/atomic.h>

extern int __heap_start;

#define STACK_INIT_PATTERN 0xa5

void debug_fill_stack_area(void)
{
	uint8_t *stack_area_p;

	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		stack_area_p = (uint8_t *) ((SPH << 8) + SPL - 4);	//TODO: check this
	}

	while (stack_area_p >= (uint8_t *) & __heap_start) {
		*stack_area_p = STACK_INIT_PATTERN;
		stack_area_p--;
	}
}

uint16_t debug_get_unused_stack(void)
{
	register uint8_t *stack_area_p;
	register uint8_t *current_stack_p;

	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		current_stack_p = (uint8_t *) ((SPH << 8) + SPL);
	}

	/*
	 * Scan the stack from __heap_start upwards until stack watermark is found.
	 */
	stack_area_p = (uint8_t *) & __heap_start;
	while (stack_area_p <= current_stack_p) {
		if (*stack_area_p != STACK_INIT_PATTERN) {
			return (uint16_t) stack_area_p -
			    (uint16_t) & __heap_start;
			break;
		}
		stack_area_p++;
	}
	return 0;
}
