// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.3
// LVGL version: 8.3.6
// Project name: SquareLine_Test_Basic

#include "ui.h"

lv_chart_series_t * ser_Temp;   // Temperature series
lv_chart_series_t * ser_Hum;    // Humidity series
int16_t value = 0;

 
void button1_clicked(lv_event_t * e)
{
	// Your code here
	_ui_screen_change( &ui_Screen2, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Screen2_screen_init);
    _ui_screen_delete( &ui_Screen1);
}

void button2_clicked(lv_event_t * e)
{
	// Your code here
	_ui_screen_change( &ui_Screen1, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, &ui_Screen1_screen_init);
    _ui_screen_delete( &ui_Screen2);
}

void slider_change(lv_event_t * e)
{
	// Your code here
	char buf[30];
	lv_event_code_t code = lv_event_get_code(e); //--> Get the event code.
	if (code == LV_EVENT_VALUE_CHANGED) {
		lv_obj_t *target_slider = (lv_obj_t *)lv_event_get_target(e); //--> Slider that generated the event.
		value = lv_slider_get_value(target_slider);           //--> Get the slider value.
		// int16_t value = lv_slider_get_value(objects.slider_pwm);

		// Map the slider value to PWM range (0-255).
		//uint8_t pwm_value = value; //(value, 0, 100, 0, 255);

		// Set the LED brightness using PWM.
		//ledcWrite(LED_PIN, pwm_value);

		// Update the label with the current slider value.
		//lv_label_set_text_fmt(ui_Label5, "%d", value);
		lv_snprintf(buf, sizeof(buf), "Slider value: %d", value);
		lv_label_set_text(ui_Label5, buf);

		/* if (ui_Chart1 && ser_Temp) {
			lv_chart_set_next_value(ui_Chart1, ser_Temp, value);
			lv_chart_set_next_value(ui_Chart1, ser_Hum, value/15);
		} else {	
			printf("Chart or series is NULL!\n");
		} */

		//lv_chart_set_next_value(ui_Chart1, ui_Chart1_series_1, 70);
		//lv_chart_set_next_value(ui_Chart1, ui_Chart1_series_2, value/10);
		//lv_chart_refresh(ui_Chart1);

		//lv_chart_refresh(ui_Chart1);
		//Serial.print("Slider Value: ");		
		//Serial.println(value);
		//Serial.print("PWM Value: ");
		//Serial.println(pwm_value);
	}
}