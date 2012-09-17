/* c-basic-offset: 8; tab-width: 8; indent-tabs-mode: nil
 * vi: set shiftwidth=8 tabstop=8 expandtab:
 * :indentSize=8:tabSize=8:noTabs=true:
 */
/*
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

/**
 * debug.h
 *
 *  Contains all kind of debugging related helper functions for AVR.
 */

#ifndef DEBUG_H_
#define DEBUG_H_

/**
 * Fill unused stack area between heap start and the current stack pointer with a specific pattern.
 * To be called in cstartup or in the start of main().
 */
void debug_fill_stack_area(void);

/**
 * Returns the number of unused bytes in the stack,
 * ie. the number of bytes between heap start and stack watermark.
 * Note: debug_fill_stack_area() must have been called before.
 */
uint16_t debug_get_unused_stack(void);

#endif /* DEBUG_H_ */
