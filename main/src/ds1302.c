#include <assert.h>

#include "driver/gpio.h"
#include "esp_log.h"

#include "ds1302.h"


void ds1302_init(ds1302_t *device, int gpio_clk, int gpio_io, int gpio_rst) {
	device->gpio_clk = gpio_clk;
	device->gpio_io = gpio_io;
	device->gpio_rst = gpio_rst;
	gpio_set_direction(gpio_clk, GPIO_MODE_OUTPUT);
	gpio_set_direction(gpio_io, GPIO_MODE_OUTPUT);
	gpio_set_direction(gpio_rst, GPIO_MODE_OUTPUT);
	gpio_set_level(gpio_clk, 0);
	gpio_set_level(gpio_io, 0);
	gpio_set_level(gpio_rst, 0);
}

void ds1302_write_byte(ds1302_t *device, uint8_t data, ds1302_region_t region, size_t addr) {
	assert(region == REGION_RAM || region == REGION_CLOCK);
	assert(addr <= 0b11110);
	if (region == REGION_CLOCK) assert(addr <= REG_TRICLE_CHARGER);
	uint8_t cmd = 1 << 7 | addr << 1 | region << 6 | 0;

	gpio_set_level(device->gpio_rst, 1);
	for (size_t i = 0; i < 8; i++) {
		gpio_set_level(device->gpio_io, (cmd >> i) & 1);
		gpio_set_level(device->gpio_clk, 1);
		gpio_set_level(device->gpio_clk, 0);
	}

	for (size_t i = 0; i < 8; i++) {
		gpio_set_level(device->gpio_io, (data >> i) & 1);
		gpio_set_level(device->gpio_clk, 1);
		gpio_set_level(device->gpio_clk, 0);
	}
	gpio_set_level(device->gpio_rst, 0);
}

uint8_t ds1302_read_byte(ds1302_t *device, ds1302_region_t region, size_t addr) {
	assert(region == REGION_RAM || region == REGION_CLOCK);
	assert(addr <= 0b11110);
	if (region == REGION_CLOCK) assert(addr <= REG_TRICLE_CHARGER);
	uint8_t cmd = (1 << 7) | (addr << 1) | (region << 6) | 1;

	ESP_LOGI("ds1302", "reset on");
	gpio_set_level(device->gpio_rst, 1);
	for (size_t i = 0; i < 8; i++) {
		gpio_set_level(device->gpio_io, (cmd >> i) & 1);
		gpio_set_level(device->gpio_clk, 1);
		if (i < 7) gpio_set_level(device->gpio_clk, 0);
	}

	uint8_t result = 0;
	gpio_set_direction(device->gpio_io, GPIO_MODE_INPUT);
	for (size_t i = 0; i < 8; i++) {
		gpio_set_level(device->gpio_clk, 0);
		if (i < 7) gpio_set_level(device->gpio_clk, 1);
		result |= gpio_get_level(device->gpio_io) << i;
	}
	gpio_set_level(device->gpio_rst, 0);
	gpio_set_direction(device->gpio_io, GPIO_MODE_OUTPUT);
	gpio_set_level(device->gpio_io, 0);
	ESP_LOGI("ds1302", "reset off");
	ESP_LOGI("ds1302", "Read byte: %x", result);
	return result;
}

int ds1302_get_seconds(ds1302_t *device) {
	uint8_t reg = ds1302_read_byte(device, REGION_CLOCK, REG_SEC);
	int digit1 = reg & 0b1111;
	int digit10 = (reg >> 4) & 0b111;
	return digit10*10+digit1;
}

int ds1302_get_minutes(ds1302_t *device) {
	uint8_t reg = ds1302_read_byte(device, REGION_CLOCK, REG_MIN);
	int digit1 = reg & 0b1111;
	int digit10 = (reg >> 4) & 0b111;
	return digit10*10+digit1;
}

int ds1302_get_hours(ds1302_t *device) {
	uint8_t reg = ds1302_read_byte(device, REGION_CLOCK, REG_HR);
	int digit1 = reg & 0b1111;
	int digit10 = (reg >> 4) & 0b11;
	return digit10*10+digit1;
}

int ds1302_get_day(ds1302_t *device) {
	uint8_t reg = ds1302_read_byte(device, REGION_CLOCK, REG_DATE);
	int digit1 = reg & 0b1111;
	int digit10 = (reg >> 4) & 0b11;
	return digit10*10+digit1;
}

int ds1302_get_month(ds1302_t *device) {
	uint8_t reg = ds1302_read_byte(device, REGION_CLOCK, REG_MONTH);
	int digit1 = reg & 0b1111;
	int digit10 = (reg >> 4) & 0b1;
	return digit10*10+digit1;
}

int ds1302_get_year(ds1302_t *device) {
	uint8_t reg = ds1302_read_byte(device, REGION_CLOCK, REG_MONTH);
	int digit1 = reg & 0b1111;
	int digit10 = (reg >> 4) & 0b1111;
	return digit10*10+digit1+2000;
}

void ds1302_set_seconds(ds1302_t *device, int val) {
	assert(val <= 59 && val >= 0);
	uint8_t reg = (((val/10) << 4)&0b111) | ((val%10)&0b1111);
	ds1302_write_byte(device, reg, REGION_CLOCK, REG_SEC);
}

void ds1302_set_minutes(ds1302_t *device, int val) {
	assert(val <= 59 && val >= 0);
	uint8_t reg = (((val/10) << 4)&0b111) | ((val%10)&0b1111);
	ds1302_write_byte(device, reg, REGION_CLOCK, REG_MIN);
}

void ds1302_set_hours(ds1302_t *device, int val) {
	assert(val <= 23 && val >= 0);
	uint8_t reg = (((val/10) << 4)&0b11) | ((val%10)&0b1111);
	ds1302_write_byte(device, reg, REGION_CLOCK, REG_HR);
}

void ds1302_set_day(ds1302_t *device, int val) {
	assert(val <= 31 && val >= 0);
	uint8_t reg = (((val/10) << 4)&0b11) | ((val%10)&0b1111);
	ds1302_write_byte(device, reg, REGION_CLOCK, REG_DATE);
}

void ds1302_set_month(ds1302_t *device, int val) {
	assert(val <= 12 && val >= 0);
	uint8_t reg = (((val/10) << 4)&0b1) | ((val%10)&0b1111);
	ds1302_write_byte(device, reg, REGION_CLOCK, REG_MONTH);
}

void ds1302_set_year(ds1302_t *device, int val) {
	assert(val <= 2099 && val >= 2000);
	val -= 2000;
	uint8_t reg = (((val/10) << 4)&0b1111) | ((val%10)&0b1111);
	ds1302_write_byte(device, reg, REGION_CLOCK, REG_YEAR);
}
