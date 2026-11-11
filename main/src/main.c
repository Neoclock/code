#include "display_driver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include <time.h>

display_t display = {};
SemaphoreHandle_t display_mutex;
int last_min = 0;

void display_process(void *) {
	while(1)
	{
		xSemaphoreTake(display_mutex, portMAX_DELAY);
		display_show(&display);
		xSemaphoreGive(display_mutex);
		//vTaskDelay(pdMS_TO_TICKS(500));
	}
}

void clock_process(void *) {
	while (1) {
		time_t current_time = time(NULL);
		struct tm *ts = localtime(&current_time);
		if (ts->tm_min-last_min >= 1) {
			display_panel_text_t panel_text;
			display_tube_text_t tube_text;
			snprintf(panel_text, 5, "%02i%02i", ts->tm_hour, ts->tm_min);
			snprintf(tube_text, 17, "%02i.%02i.%04i", 
					ts->tm_mday, ts->tm_mon+1, ts->tm_year+1900);
			xSemaphoreTake(display_mutex, portMAX_DELAY);
			display_panel_set_text(&display.panel, panel_text);
			display_tube_set_text(&display.tube, tube_text);
			xSemaphoreGive(display_mutex);
		}
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

void app_main(void)
{
	time_t tim = time(NULL);
	struct tm *ts = localtime(&tim);
	last_min = ts->tm_min;
	display_mutex = xSemaphoreCreateMutex();
	if (!display_mutex) {
		ESP_LOGE("main", "Failed to create mutex");
		return;
	}
	display = display_init((display_pinout_t){ .latch = 18, .clock = 19, .data = 21, .pwm = 2 });

	display_panel_set_mnemonic(&display.panel, 1, DISPLAY_PANEL_MNEMONIC_UDOT | DISPLAY_PANEL_MNEMONIC_LDOT);

	xTaskCreate(display_process, "display_process", 4096, NULL, 5, NULL);
	xTaskCreate(display_process, "clock_process", 4096, NULL, 6, NULL);
}
