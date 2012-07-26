/**
 * Led cube effects
 *
 * Not integrated to the main code, yet.
 */

#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "env.h"
#include "effects.h"
#include "effect_utils.h"

static void init_brownian(void);

const effect_t effects[] = {
	{ "brownian", &init_brownian, &effect_brownian, 1000, 0 },
	{ "sine", NULL, &effect_sine, 1000, 1 },
	{ "const", NULL, &effect_constant, 1000, 1 },
	{ "layers", NULL, &effect_layers_tester, 1000, 1 }
};

const int effects_len = sizeof(effects) / sizeof(effect_t);

/**
 * Brownian particle. Starts near center and then accumulates.
 * */
uint8_t brown_x;
uint8_t brown_y;
uint8_t brown_z;
static void init_brownian(void)
{
	brown_x = (uint8_t)(LEDS_X / 2);
	brown_y = (uint8_t)(LEDS_Y / 2);
	brown_z = (uint8_t)(LEDS_Z / 2);
}
void effect_brownian(void)
{
	// TODO: figure out how to accumulate
	set_led(brown_x, brown_y, brown_z, (1<<GS_DEPTH) - 1);

	brown_x = clamp(brown_x + randint(-2, 2), 0, LEDS_X - 1);
	brown_y = clamp(brown_y + randint(-2, 2), 0, LEDS_Y - 1);
	brown_z = clamp(brown_z + randint(-2, 2), 0, LEDS_Z - 1);
}

/**
 * Plot 2-dimensional sine waves which are moving. The parameters are
 * not tuned, this is just taken from my head. */
TWOD(effect_sine)
{
	const float sine_scaler = (float)MAX_2D_PLOT_INTENSITY/4;

	return sine_scaler * (2 + sin((float)x/2+(float)ticks/150) + sin((float)y/2+(float)ticks/300));
}

/**
 * Constant plot. Helps to spot errors in drawing code. Replace this
 * with something more useful in the future.
 */
TWOD(effect_constant)
{
	return 10000;
}

/**
 * Simple test function which draws voxel layers.
 */
void effect_layers_tester(void)
{
	clear_buffer();
	uint8_t z = (ticks/20 % LEDS_Z);

	for (uint8_t x=0; x<LEDS_X; x++) {
		for (uint8_t y=0; y<LEDS_Y; y++) {
			set_led(x, y, z, (1<<GS_DEPTH) - 1);
		}
	}
}