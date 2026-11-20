#include "display_driver.h"
#include "esp_err.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include <time.h>
#include <sys/time.h>
#include "protocol_examples_common.h"
#include "nvs_flash.h"
#include "esp_sntp.h"

display_t display = {};
SemaphoreHandle_t display_mutex;
int last_min = -200;

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
		display_panel_text_t panel_text;
		display_tube_text_t tube_text;
		snprintf(panel_text, 5, "%02i%02i", ts->tm_hour, ts->tm_min);
		snprintf(tube_text, 17, "%02i.%02i.%04i", 
				ts->tm_mday, ts->tm_mon+1, ts->tm_year+1900);
		xSemaphoreTake(display_mutex, portMAX_DELAY);
		display_panel_set_text(&display.panel, panel_text);
		display_tube_set_text(&display.tube, tube_text);
		display_panel_set_mnemonic(&display.panel, ts->tm_sec&1, DISPLAY_PANEL_MNEMONIC_UDOT | DISPLAY_PANEL_MNEMONIC_LDOT);
		xSemaphoreGive(display_mutex);
		last_min = ts->tm_min;
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

void time_update(void *) {
	ESP_ERROR_CHECK( nvs_flash_init() );
	ESP_ERROR_CHECK( esp_netif_init() );
	ESP_ERROR_CHECK( esp_event_loop_create_default() );
	ESP_ERROR_CHECK( example_connect() );

	esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
	esp_sntp_setservername(0, "pool.ntp.org");
	esp_sntp_set_sync_interval(60*60*1000);
	ESP_LOGI("time_update", "running SNTP init");
	esp_sntp_init();
	struct timeval tv;
	ESP_LOGI("time_update", "running time sync");
	esp_sntp_sync_time(&tv);
	while (1) { vTaskDelay(pdMS_TO_TICKS(10000)); };
}

#define DS_CLK 19
#define DS_IO 18
#define DS_RST 5

void app_main(void)
{
	//ds1302_t device;
	//ds1302_init(&device, DS_CLK, DS_IO, DS_RST);
	////while (1) {
	////	gpio_set_level(DS_RST, 0);
	////	gpio_set_level(DS_RST, 1);
	////}
	//ds1302_set_seconds(&device, 0);
	//while (1) {
	//	ESP_LOGI("main", "Seconds: %i", ds1302_get_seconds(&device));
	//	vTaskDelay(pdMS_TO_TICKS(1000));
	//}
    setenv("TZ", "MSK-3", 1);
	tzset();
	struct tm ts = {
		.tm_hour = 18,
		.tm_min = 30,
		.tm_year = 2026 - 1900,
		.tm_mon = 11 - 1,
		.tm_mday = 20,
	};
	struct timeval tv = { mktime(&ts), 0 };
	settimeofday(&tv, NULL);
	display_mutex = xSemaphoreCreateMutex();
	if (!display_mutex) {
		ESP_LOGE("main", "Failed to create mutex");
		return;
	}
	display = display_init((display_pinout_t){ .latch = 14, .clock = 12, .data = 27, .pwm = 13 });

	display_panel_set_mnemonic(&display.panel, 1, DISPLAY_PANEL_MNEMONIC_UDOT | DISPLAY_PANEL_MNEMONIC_LDOT);

	ESP_LOGI("main", "clock_process run");
	xTaskCreate(clock_process, "clock_process", 4096, NULL, 6, NULL);
	ESP_LOGI("main", "time_update_run");
	xTaskCreate(time_update, "time_update", 4096, NULL, 7, NULL);
	ESP_LOGI("main", "display_process run");
	xTaskCreate(display_process, "display_process", 4096, NULL, 5, NULL);
}
