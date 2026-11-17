#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct {
	int gpio_clk;
	int gpio_io;
	int gpio_rst;
} ds1302_t;

typedef enum {
	REGION_CLOCK = 0,
	REGION_RAM = 1
} ds1302_region_t;

typedef enum {
	REG_SEC = 0,
	REG_MIN,
	REG_HR,
	REG_DATE,
	REG_MONTH,
	REG_DAY,
	REG_YEAR,
	REG_CONTROL,
	REG_TRICLE_CHARGER,
} ds1302_reg_t;

void ds1302_init(ds1302_t *device, int gpio_clk, int gpio_io, int gpio_rst);

void ds1302_write_byte(ds1302_t *device, uint8_t data, ds1302_region_t region, size_t addr);
uint8_t ds1302_read_byte(ds1302_t *device, ds1302_region_t region, size_t addr);

int ds1302_get_seconds(ds1302_t *device);
int ds1302_get_minutes(ds1302_t *device);
int ds1302_get_hours(ds1302_t *device);
int ds1302_get_day(ds1302_t *device);
int ds1302_get_month(ds1302_t *device);
int ds1302_get_year(ds1302_t *device);

void ds1302_set_seconds(ds1302_t *device, int val);
void ds1302_set_minutes(ds1302_t *device, int val);
void ds1302_set_hours(ds1302_t *device, int val);
void ds1302_set_day(ds1302_t *device, int val);
void ds1302_set_month(ds1302_t *device, int val);
void ds1302_set_year(ds1302_t *device, int val);
