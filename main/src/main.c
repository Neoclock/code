#include "display_driver.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"

void app_main(void)
{
	display_t display = display_init((display_pinout_t){ .latch = 18, .clock = 19, .data = 21, .pwm = 2 });

	display_panel_text_t text = "NNNN";
	display_panel_set_text(&display.panel, text);
	display_panel_set_mnemonic(&display.panel, 1, DISPLAY_PANEL_MNEMONIC_CLOCK | DISPLAY_PANEL_MNEMONIC_BELL | DISPLAY_PANEL_MNEMONIC_UDOT);

	while(1)
	{
		display_show(&display);
		vTaskDelay(pdMS_TO_TICKS(5));
	}
}

