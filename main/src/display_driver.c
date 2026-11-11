#include "display_driver.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/ledc_types.h"
#include <assert.h>
#include <limits.h>
#include <time.h>
#include <string.h>

#define N_SEGMENTS	7
#define MAX_DUTY	130
#define PWM_CHANNEL	LEDC_CHANNEL_0

typedef int segments_mask_t[N_SEGMENTS];
static const segments_mask_t SEGMENTS_MASK_SYMBOLS[UCHAR_MAX + 1] =
{
	       /* A  B  C  D  E  F  G */
	['0'] = { 1, 1, 1, 1, 1, 1, 0 },
	['1'] = { 0, 1, 1, 0, 0, 0, 0 },
	['2'] = { 1, 1, 0, 1, 1, 0, 1 },
	['3'] = { 1, 1, 1, 1, 0, 0, 1 },
	['4'] = { 0, 1, 1, 0, 0, 1, 1 },
	['5'] = { 1, 0, 1, 1, 0, 1, 1 },
	['6'] = { 1, 0, 1, 1, 1, 1, 1 },
	['7'] = { 1, 1, 1, 0, 0, 0, 0 },
	['8'] = { 1, 1, 1, 1, 1, 1, 1 },
	['9'] = { 1, 1, 1, 1, 0, 1, 1 },

	/* ---- Uppercase letters ---- */
	['A'] = { 1, 1, 1, 0, 1, 1, 1 },
	['B'] = { 0, 0, 1, 1, 1, 1, 1 },   /* same as 'b' */
	['C'] = { 1, 0, 0, 1, 1, 1, 0 },
	['D'] = { 0, 1, 1, 1, 1, 0, 1 },   /* same as 'd' */
	['E'] = { 1, 0, 0, 1, 1, 1, 1 },
	['F'] = { 1, 0, 0, 0, 1, 1, 1 },
	['G'] = { 1, 0, 1, 1, 1, 1, 0 },
	['H'] = { 0, 1, 1, 0, 1, 1, 1 },
	['I'] = { 0, 1, 1, 0, 0, 0, 0 },   /* same as '1' */
	['J'] = { 0, 1, 1, 1, 1, 0, 0 },
	['L'] = { 0, 0, 0, 1, 1, 1, 0 },
	['O'] = { 1, 1, 1, 1, 1, 1, 0 },   /* same as '0' */
	['P'] = { 1, 1, 0, 0, 1, 1, 1 },
	['Q'] = { 1, 1, 1, 0, 0, 1, 1 },   /* same as 'q' */
	['S'] = { 1, 0, 1, 1, 0, 1, 1 },   /* same as '5' */
	['T'] = { 0, 0, 0, 1, 1, 1, 1 },   /* same as 't' */
	['U'] = { 0, 1, 1, 1, 1, 1, 0 },
	['Y'] = { 0, 1, 1, 1, 0, 1, 1 },   /* same as 'y' */

	/* ---- Lowercase letters ---- */
	['a'] = { 1, 1, 1, 1, 1, 0, 1 },
	['b'] = { 0, 0, 1, 1, 1, 1, 1 },
	['c'] = { 0, 0, 0, 1, 1, 0, 1 },
	['d'] = { 0, 1, 1, 1, 1, 0, 1 },
	['e'] = { 1, 1, 0, 1, 1, 1, 1 },
	['f'] = { 1, 0, 0, 0, 1, 1, 1 },   /* = 'F' */
	['g'] = { 1, 1, 1, 1, 0, 1, 1 },   /* = '9' */
	['h'] = { 0, 0, 1, 0, 1, 1, 1 },
	['i'] = { 0, 0, 1, 0, 0, 0, 0 },
	['j'] = { 0, 1, 1, 1, 1, 0, 0 },   /* = 'J' */
	['k'] = { 0, 0, 1, 0, 1, 1, 1 },   /* = 'h' */
	['l'] = { 0, 0, 0, 1, 1, 1, 0 },   /* = 'L' */
	['n'] = { 0, 0, 1, 0, 1, 0, 1 },
	['o'] = { 0, 0, 1, 1, 1, 0, 1 },
	['p'] = { 1, 1, 0, 0, 1, 1, 1 },   /* = 'P' */
	['q'] = { 1, 1, 1, 0, 0, 1, 1 },
	['r'] = { 0, 0, 0, 0, 1, 0, 1 },
	['s'] = { 1, 0, 1, 1, 0, 1, 1 },   /* = '5' */
	['t'] = { 0, 0, 0, 1, 1, 1, 1 },
	['u'] = { 0, 0, 1, 1, 1, 0, 0 },
	['y'] = { 0, 1, 1, 1, 0, 1, 1 },

	/* ---- Punctuation / symbols ---- */
	['-']  = { 0, 0, 0, 0, 0, 0, 1 },  /* dash (middle) */
	['_']  = { 0, 0, 0, 1, 0, 0, 0 },  /* underscore (bottom) */
	['=']  = { 0, 0, 0, 1, 0, 0, 1 },  /* equals (D + G) */
	['|']  = { 0, 1, 1, 0, 0, 0, 0 },
	['^']  = { 1, 1, 0, 0, 0, 1, 0 },
	['[']  = { 1, 0, 0, 1, 1, 1, 0 },  /* = 'C' */
	[']']  = { 0, 1, 1, 1, 0, 0, 0 },
	['"']  = { 0, 1, 0, 0, 0, 1, 0 },
};

static const int PANEL_DIGIT_BIT_FROM_ORDER[DISPLAY_PANEL_N_DIGITS] =
{
	[0] = DISPLAY_MASK_BIT_PANEL_GRID_1,
	[1] = DISPLAY_MASK_BIT_PANEL_GRID_2,
	[2] = DISPLAY_MASK_BIT_PANEL_GRID_3,
	[3] = DISPLAY_MASK_BIT_PANEL_GRID_4,
	[4] = DISPLAY_MASK_BIT_PANEL_GRID_DOTS,
	[5] = DISPLAY_MASK_BIT_PANEL_GRID_MNEMONIC
};

static const int PANEL_DIGIT_ORDER_FROM_BIT[DISPLAY_MASK_BIT_COUNT] =
{
	[DISPLAY_MASK_BIT_PANEL_GRID_1] = 0,
	[DISPLAY_MASK_BIT_PANEL_GRID_2] = 1,
	[DISPLAY_MASK_BIT_PANEL_GRID_3] = 2,
	[DISPLAY_MASK_BIT_PANEL_GRID_4] = 3,
	[DISPLAY_MASK_BIT_PANEL_GRID_DOTS] = 4,
	[DISPLAY_MASK_BIT_PANEL_GRID_MNEMONIC] = 5
};

static const int TUBE_DIGIT_BIT_FROM_ORDER[DISPLAY_TUBE_N_DIGITS] =
{
	[0] = DISPLAY_MASK_BIT_TUBE_GRID_1,
	[1] = DISPLAY_MASK_BIT_TUBE_GRID_2,
	[2] = DISPLAY_MASK_BIT_TUBE_GRID_3,
	[3] = DISPLAY_MASK_BIT_TUBE_GRID_4,
	[4] = DISPLAY_MASK_BIT_TUBE_GRID_5,
	[5] = DISPLAY_MASK_BIT_TUBE_GRID_6,
	[6] = DISPLAY_MASK_BIT_TUBE_GRID_7,
	[7] = DISPLAY_MASK_BIT_TUBE_GRID_8,
	[8] = DISPLAY_MASK_BIT_TUBE_GRID_MNEMONIC
};

static const int TUBE_DIGIT_ORDER_FROM_BIT[DISPLAY_MASK_BIT_COUNT] =
{
	[DISPLAY_MASK_BIT_TUBE_GRID_1] = 0,
	[DISPLAY_MASK_BIT_TUBE_GRID_2] = 1,
	[DISPLAY_MASK_BIT_TUBE_GRID_3] = 2,
	[DISPLAY_MASK_BIT_TUBE_GRID_4] = 3,
	[DISPLAY_MASK_BIT_TUBE_GRID_5] = 4,
	[DISPLAY_MASK_BIT_TUBE_GRID_6] = 5,
	[DISPLAY_MASK_BIT_TUBE_GRID_7] = 6,
	[DISPLAY_MASK_BIT_TUBE_GRID_8] = 7,
	[DISPLAY_MASK_BIT_TUBE_GRID_MNEMONIC] = 8
};

display_t display_init(display_pinout_t pinout)
{
	display_t display =
	{
		.pinout = pinout,
	};

	/* enabling grids for corresponding digits */
	for(size_t i = 0; i < DISPLAY_PANEL_N_DIGITS; i++)
	{
		display.panel.digits[i].mask[PANEL_DIGIT_BIT_FROM_ORDER[i]] = 1;
	}
	for(size_t i = 0; i < DISPLAY_TUBE_N_DIGITS; i++)
	{
		display.tube.digits[i].mask[TUBE_DIGIT_BIT_FROM_ORDER[i]] = 1;
	}

	gpio_config(&(gpio_config_t)
			{
			.pin_bit_mask =
			(1 << display.pinout.latch) |
			(1 << display.pinout.clock) |
			(1 << display.pinout.data),
			.mode = GPIO_MODE_OUTPUT
			});

	ledc_channel_config(&(ledc_channel_config_t)
			{
			.gpio_num = display.pinout.pwm,
			.speed_mode = LEDC_HIGH_SPEED_MODE,
			.channel = PWM_CHANNEL,
			.timer_sel = LEDC_TIMER_0,
			.duty = MAX_DUTY
			});

	ledc_timer_config(&(const ledc_timer_config_t)
			{
			.speed_mode = LEDC_HIGH_SPEED_MODE,
			.duty_resolution = LEDC_TIMER_8_BIT,
			.timer_num = LEDC_TIMER_0,
			.freq_hz = 20000,
			.clk_cfg = RMT_CLK_SRC_APB
			});

	return display;
}

void display_set_brightness(display_t *display, float brightness) {
	assert(brightness >= 0.0f && brightness <= 1.0f);
	ledc_set_duty(LEDC_HIGH_SPEED_MODE, PWM_CHANNEL, MAX_DUTY * brightness);
	ledc_update_duty(LEDC_HIGH_SPEED_MODE, PWM_CHANNEL);
}

static void display_apply_mask(const display_pinout_t *pinout, const display_mask_t mask)
{
	assert(pinout); assert(mask);

	gpio_set_level(pinout->latch, 0);
	gpio_set_level(pinout->clock, 0);
	gpio_set_level(pinout->data, 0);

	for(size_t i = 0; i < DISPLAY_MASK_BIT_COUNT; i++)
	{
		gpio_set_level(pinout->data, !mask[DISPLAY_MASK_BIT_COUNT-1 - i]);
		gpio_set_level(pinout->clock, 1);
		gpio_set_level(pinout->clock, 0);
	}

	gpio_set_level(pinout->latch, 1);
	gpio_set_level(pinout->latch, 0);
}

void display_show(display_t *display)
{
	assert(display);

	/* setting duty */
	// FIXME: display sub-brightnesses
	// (tube.brightness, panel.brightness, ...) has no effect for now.

	if(display->panel._enable)
	{
		assert(display->_showed_digit < DISPLAY_PANEL_N_DIGITS);


		display_apply_mask(&display->pinout, display->panel.digits[display->_showed_digit++].mask);
		if(display->_showed_digit >= DISPLAY_PANEL_N_DIGITS)
		{
			display->_showed_digit = 0;
			display->panel._enable = 0;
			display->tube._enable = 1;
		}
	}
	else if(display->tube._enable)
	{
		assert(display->_showed_digit < DISPLAY_TUBE_N_DIGITS);

		display_apply_mask(&display->pinout, display->tube.digits[display->_showed_digit++].mask);
		if(display->_showed_digit >= DISPLAY_TUBE_N_DIGITS)
		{
			display->_showed_digit = 0;
			display->tube._enable = 0;
			display->panel._enable = 1;
		}
	}
	else
		display->panel._enable = 1;
}

void display_mask_panel_set_symbol(display_mask_t mask, unsigned char sym)
{
	assert(mask);

	mask[DISPLAY_MASK_BIT_PANEL_A] = SEGMENTS_MASK_SYMBOLS[sym][0];
	mask[DISPLAY_MASK_BIT_PANEL_B] = SEGMENTS_MASK_SYMBOLS[sym][1];
	mask[DISPLAY_MASK_BIT_PANEL_C] = SEGMENTS_MASK_SYMBOLS[sym][2];
	mask[DISPLAY_MASK_BIT_PANEL_D] = SEGMENTS_MASK_SYMBOLS[sym][3];
	mask[DISPLAY_MASK_BIT_PANEL_E] = SEGMENTS_MASK_SYMBOLS[sym][4];
	mask[DISPLAY_MASK_BIT_PANEL_F] = SEGMENTS_MASK_SYMBOLS[sym][5];
	mask[DISPLAY_MASK_BIT_PANEL_G] = SEGMENTS_MASK_SYMBOLS[sym][6];
}

void display_mask_tube_set_symbol(display_mask_t mask, unsigned char sym)
{
	assert(mask);

	mask[DISPLAY_MASK_BIT_TUBE_A] = SEGMENTS_MASK_SYMBOLS[sym][0];
	mask[DISPLAY_MASK_BIT_TUBE_B] = SEGMENTS_MASK_SYMBOLS[sym][1];
	mask[DISPLAY_MASK_BIT_TUBE_C] = SEGMENTS_MASK_SYMBOLS[sym][2];
	mask[DISPLAY_MASK_BIT_TUBE_D] = SEGMENTS_MASK_SYMBOLS[sym][3];
	mask[DISPLAY_MASK_BIT_TUBE_E] = SEGMENTS_MASK_SYMBOLS[sym][4];
	mask[DISPLAY_MASK_BIT_TUBE_F] = SEGMENTS_MASK_SYMBOLS[sym][5];
	mask[DISPLAY_MASK_BIT_TUBE_G] = SEGMENTS_MASK_SYMBOLS[sym][6];
}

void display_panel_set_text(display_panel_t *panel, const display_panel_text_t text)
{
	assert(panel); assert(text);
	assert(strlen(text) == DISPLAY_PANEL_N_NUMBER_DIGITS);

	for(size_t i = 0; i < DISPLAY_PANEL_N_NUMBER_DIGITS; i++)
		display_mask_panel_set_symbol(panel->digits[i].mask, text[i]);
}

void display_tube_set_text(display_tube_t *tube, const display_tube_text_t text)
{
	assert(tube); assert(text);
	assert(strlen(text) >= DISPLAY_TUBE_N_NUMBER_DIGITS);

	size_t j = 0;
	for(size_t i = 0; i < DISPLAY_TUBE_N_NUMBER_DIGITS; i++, j++)
	{
		if(text[j] == '.')
		{
			assert(i > 0 && "Invalid text");
			tube->digits[i-1].mask[DISPLAY_MASK_BIT_TUBE_H] = 1;
			j++;
		}
		else if(i > 0)
			tube->digits[i-1].mask[DISPLAY_MASK_BIT_TUBE_H] = 0;

		display_mask_tube_set_symbol(tube->digits[i].mask, text[j]);
	}
}

void display_panel_set_mnemonic(display_panel_t *panel, int enable, display_panel_mnemonic_t mnemonic)
{
	assert(panel);

	if(mnemonic & DISPLAY_PANEL_MNEMONIC_UDOT)
		panel->digits[PANEL_DIGIT_ORDER_FROM_BIT[DISPLAY_MASK_BIT_PANEL_GRID_DOTS]].mask[DISPLAY_MASK_BIT_PANEL_UDOT] = enable;
	if(mnemonic & DISPLAY_PANEL_MNEMONIC_LDOT)
		panel->digits[PANEL_DIGIT_ORDER_FROM_BIT[DISPLAY_MASK_BIT_PANEL_GRID_DOTS]].mask[DISPLAY_MASK_BIT_PANEL_LDOT] = enable;
	if(mnemonic & DISPLAY_PANEL_MNEMONIC_BELL)
		panel->digits[PANEL_DIGIT_ORDER_FROM_BIT[DISPLAY_MASK_BIT_PANEL_GRID_MNEMONIC]].mask[DISPLAY_MASK_BIT_PANEL_BELL] = enable;
	if(mnemonic & DISPLAY_PANEL_MNEMONIC_CLOCK)
		panel->digits[PANEL_DIGIT_ORDER_FROM_BIT[DISPLAY_MASK_BIT_PANEL_GRID_MNEMONIC]].mask[DISPLAY_MASK_BIT_PANEL_CLOCK] = enable;
}

void display_tube_set_mnemonic(display_tube_t *tube, int enable, display_tube_mnemonic_t mnemonic)
{
	assert(tube);

	if(mnemonic & DISPLAY_TUBE_MNEMONIC_MINUS)
		tube->digits[TUBE_DIGIT_ORDER_FROM_BIT[DISPLAY_MASK_BIT_TUBE_GRID_MNEMONIC]].mask[DISPLAY_MASK_BIT_TUBE_MINUS] = enable;
	if(mnemonic & DISPLAY_TUBE_MNEMONIC_CIRCLE)
		tube->digits[TUBE_DIGIT_ORDER_FROM_BIT[DISPLAY_MASK_BIT_TUBE_GRID_MNEMONIC]].mask[DISPLAY_MASK_BIT_TUBE_CIRCLE] = enable;
}

