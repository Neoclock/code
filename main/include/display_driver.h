#pragma once

#include "soc/gpio_num.h"
#include <stddef.h>
#ifdef __cplusplus
extern "C"
{
#endif

/*
 *    A
 *    --
 * F |  | B
 *    -- G
 * E |  | C
 *    --   . H
 *    D
 */

typedef enum display_mask_bit_t
{
	/* U5-Q5 pair */

	DISPLAY_MASK_BIT_TUBE_GRID_5,
	DISPLAY_MASK_BIT_TUBE_GRID_3,
	DISPLAY_MASK_BIT_TUBE_GRID_2,
	DISPLAY_MASK_BIT_TUBE_GRID_1,
	DISPLAY_MASK_BIT_TUBE_GRID_4,
	DISPLAY_MASK_BIT_TUBE_GRID_6,
	DISPLAY_MASK_BIT_TUBE_GRID_8,
	DISPLAY_MASK_BIT_TUBE_GRID_MNEMONIC,

	/* U4-Q4 pair */

	DISPLAY_MASK_BIT_TUBE_A,

	DISPLAY_MASK_BIT_PANEL_G,
	DISPLAY_MASK_BIT_TUBE_F = DISPLAY_MASK_BIT_PANEL_G,

	DISPLAY_MASK_BIT_PANEL_E,
	DISPLAY_MASK_BIT_TUBE_B = DISPLAY_MASK_BIT_PANEL_E,

	DISPLAY_MASK_BIT_PANEL_C,
	DISPLAY_MASK_BIT_TUBE_G = DISPLAY_MASK_BIT_PANEL_C,
	DISPLAY_MASK_BIT_TUBE_MINUS = DISPLAY_MASK_BIT_TUBE_G,

	DISPLAY_MASK_BIT_PANEL_D,
	DISPLAY_MASK_BIT_TUBE_E = DISPLAY_MASK_BIT_PANEL_D,
	DISPLAY_MASK_BIT_PANEL_CLOCK = DISPLAY_MASK_BIT_PANEL_D,
	DISPLAY_MASK_BIT_PANEL_LDOT = DISPLAY_MASK_BIT_PANEL_D,

	DISPLAY_MASK_BIT_PANEL_B,
	DISPLAY_MASK_BIT_TUBE_C = DISPLAY_MASK_BIT_PANEL_B,

	DISPLAY_MASK_BIT_PANEL_F,
	DISPLAY_MASK_BIT_TUBE_D = DISPLAY_MASK_BIT_PANEL_F,

	DISPLAY_MASK_BIT_PANEL_A,
	DISPLAY_MASK_BIT_TUBE_H = DISPLAY_MASK_BIT_PANEL_A,
	DISPLAY_MASK_BIT_PANEL_BELL = DISPLAY_MASK_BIT_PANEL_A,
	DISPLAY_MASK_BIT_PANEL_UDOT = DISPLAY_MASK_BIT_PANEL_A,
	DISPLAY_MASK_BIT_TUBE_CIRCLE = DISPLAY_MASK_BIT_TUBE_H,

	/* U6-Q6 pair */

	DISPLAY_MASK_BIT_NC_1,
	DISPLAY_MASK_BIT_TUBE_GRID_7,
	DISPLAY_MASK_BIT_PANEL_GRID_1,
	DISPLAY_MASK_BIT_PANEL_GRID_2,
	DISPLAY_MASK_BIT_PANEL_GRID_DOTS,
	DISPLAY_MASK_BIT_PANEL_GRID_4,
	DISPLAY_MASK_BIT_PANEL_GRID_3,
	DISPLAY_MASK_BIT_PANEL_GRID_MNEMONIC,

	/* that's all */
	DISPLAY_MASK_BIT_COUNT
} display_mask_bit_t;

typedef enum display_panel_mnemonic_t
{
	DISPLAY_PANEL_MNEMONIC_UDOT	= 1 << 0,
	DISPLAY_PANEL_MNEMONIC_LDOT	= 1 << 1,
	DISPLAY_PANEL_MNEMONIC_CLOCK	= 1 << 2,
	DISPLAY_PANEL_MNEMONIC_BELL	= 1 << 3
} display_panel_mnemonic_t;

typedef enum display_tube_mnemonic_t
{
	DISPLAY_TUBE_MNEMONIC_CIRCLE	= 1 << 0,
	DISPLAY_TUBE_MNEMONIC_MINUS	= 1 << 1
} display_tube_mnemonic_t;

#define DISPLAY_PANEL_N_DIGITS		6
#define DISPLAY_PANEL_N_NUMBER_DIGITS	4
#define DISPLAY_TUBE_N_DIGITS		9
#define DISPLAY_TUBE_N_NUMBER_DIGITS	8

typedef int display_mask_t[DISPLAY_MASK_BIT_COUNT];
typedef char display_panel_text_t[DISPLAY_PANEL_N_NUMBER_DIGITS + 1];
typedef char display_tube_text_t[DISPLAY_TUBE_N_NUMBER_DIGITS*2 + 1];

typedef struct display_pinout_t
{
	gpio_num_t latch, clock, data, pwm;
} display_pinout_t;

typedef struct display_digit_t
{
	display_mask_t mask;
} display_digit_t;

typedef struct display_tube_t
{
	display_digit_t digits[DISPLAY_TUBE_N_DIGITS];
	int _enable;
} display_tube_t;

typedef struct display_panel_t
{
	display_digit_t digits[DISPLAY_PANEL_N_DIGITS];
	int _enable;
} display_panel_t;

typedef struct display_t
{
	display_pinout_t pinout;
	display_panel_t panel;
	display_tube_t tube;

	size_t _showed_digit;
} display_t;

display_t display_init(display_pinout_t pinout);
void display_show(display_t *display);

void display_set_brightness(display_t *display, float brightness);

void display_panel_set_text(display_panel_t *panel, const display_panel_text_t text);
void display_panel_set_mnemonic(display_panel_t *panel, int enable, display_panel_mnemonic_t mnemonic);

void display_tube_set_text(display_tube_t *tube, const display_tube_text_t text);
void display_tube_set_mnemonic(display_tube_t *tube, int enable, display_tube_mnemonic_t mnemonic);

#ifdef __cplusplus
}
#endif

