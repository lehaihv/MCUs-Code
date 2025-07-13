#include <Arduino.h>
#include <lvgl.h>
#include <LovyanGFX.hpp>
#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>
#include <Adafruit_GFX.h>

#include "ui.h"
#include "images.h"


// Change to your screen resolution.
#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 480

//LovyanGFX library configuration.
class LGFX : public lgfx::LGFX_Device
{
public:
  lgfx::Bus_RGB     _bus_instance;
  lgfx::Panel_RGB   _panel_instance;

  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();
      cfg.panel = &_panel_instance;
      
      cfg.pin_d0  = GPIO_NUM_15; // B0
      cfg.pin_d1  = GPIO_NUM_7;  // B1
      cfg.pin_d2  = GPIO_NUM_6;  // B2
      cfg.pin_d3  = GPIO_NUM_5;  // B3
      cfg.pin_d4  = GPIO_NUM_4;  // B4
      
      cfg.pin_d5  = GPIO_NUM_9;  // G0
      cfg.pin_d6  = GPIO_NUM_46; // G1
      cfg.pin_d7  = GPIO_NUM_3;  // G2
      cfg.pin_d8  = GPIO_NUM_8;  // G3
      cfg.pin_d9  = GPIO_NUM_16; // G4
      cfg.pin_d10 = GPIO_NUM_1;  // G5
      
      cfg.pin_d11 = GPIO_NUM_14; // R0
      cfg.pin_d12 = GPIO_NUM_21; // R1
      cfg.pin_d13 = GPIO_NUM_47; // R2
      cfg.pin_d14 = GPIO_NUM_48; // R3
      cfg.pin_d15 = GPIO_NUM_45; // R4

      cfg.pin_henable = GPIO_NUM_41;
      cfg.pin_vsync   = GPIO_NUM_40;
      cfg.pin_hsync   = GPIO_NUM_39;
      cfg.pin_pclk    = GPIO_NUM_0;
      cfg.freq_write  = 15000000;

      cfg.hsync_polarity    = 0;
      cfg.hsync_front_porch = 40;
      cfg.hsync_pulse_width = 48;
      cfg.hsync_back_porch  = 40;
      
      cfg.vsync_polarity    = 0;
      cfg.vsync_front_porch = 1;
      cfg.vsync_pulse_width = 31;
      cfg.vsync_back_porch  = 13;

      cfg.pclk_active_neg   = 1;
      cfg.de_idle_high      = 0;
      cfg.pclk_idle_high    = 0;

      _bus_instance.config(cfg);
    }
    {
      auto cfg = _panel_instance.config();
      cfg.memory_width  = SCREEN_WIDTH;
      cfg.memory_height = SCREEN_HEIGHT;
      cfg.panel_width   = SCREEN_WIDTH;
      cfg.panel_height  = SCREEN_HEIGHT;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      _panel_instance.config(cfg);
    }
    _panel_instance.setBus(&_bus_instance);
    setPanel(&_panel_instance);
  }
};

// Declaring the "LGFX" object as "lcd".
LGFX lcd;
//---------------------------------------- 

// Library to access the touch screen.
#include "touch.h"

// Defines the Backlight PIN.
#define TFT_BL_PIN 2

// Defines the LED PIN.
#define LED_PIN 38

// Used to track the tick timer.
uint32_t lastTick = 0;

unsigned long previousMillis_Update_UI = 0;
const long interval_Update_UI = 5;

// LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes.
#define DRAW_BUF_SIZE ((SCREEN_WIDTH * SCREEN_HEIGHT / 10) * (sizeof(uint16_t)))
static uint16_t draw_buf[DRAW_BUF_SIZE];



// log_print()
// If logging is enabled, it will inform the user about what is happening in the library.
void log_print(lv_log_level_t level, const char * buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}


// update_UI()
// Subroutines to update screen displays or widgets.
void update_UI() {
  lv_tick_inc(millis() - lastTick); //--> Update the tick timer. Tick is new for LVGL 9.
  lastTick = millis();
  lv_timer_handler(); //--> Update the UI.
}


// my_disp_flush()
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, unsigned char* data) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  lv_draw_sw_rgb565_swap(data, w*h);

  #if (LV_COLOR_16_SWAP != 0)
    lcd.pushImageDMA(area->x1, area->y1, w, h, (uint16_t*) data);
  #else
    lcd.pushImageDMA(area->x1, area->y1, w, h, (uint16_t*) data);
  #endif

  lv_disp_flush_ready(disp);
}


// my_touchpad_read()
void touchscreen_read(lv_indev_t *indev, lv_indev_data_t *data) {
  if (touch_has_signal()) {
    if (touch_touched()) {
      // Set the coordinates.
      data->point.x = touch_last_x;
      data->point.y = touch_last_y;
      //Serial.print( "Data x " );
      //Serial.println( data->point.x );
      //Serial.print( "Data y " );
      //Serial.println( data->point.y );
      
      data->state = LV_INDEV_STATE_PRESSED;
    } else if (touch_released()) {
      data->state = LV_INDEV_STATE_RELEASED;
    }
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
} 


// switch_led_event_handler()
// Callback that is triggered when "switch_led" is clicked/toggled.
static void switch_led_event_handler(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);                    //--> Get the event code.
  lv_obj_t * target_switch = (lv_obj_t*) lv_event_get_target(e);  //--> Switch that generated the event.
  
  if (code == LV_EVENT_VALUE_CHANGED) {
    LV_UNUSED(target_switch);

    bool checked = lv_obj_has_state(target_switch, LV_STATE_CHECKED);

    if (checked == true) {
      lv_img_set_src(objects.image_bulb, &img_img_light_bulb_on);
      digitalWrite(LED_PIN, HIGH);
    } else {
      lv_img_set_src(objects.image_bulb, &img_img_light_bulb_off);
      digitalWrite(LED_PIN, LOW);
    }
  }
}

// image_bulb_event_handler()
// Callback that is triggered when "image_bulb" is clicked/toggled.
static void image_bulb_event_handler(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);                    //--> Get the event code.
  
  if (code == LV_EVENT_CLICKED) {
    bool switch_led_checked = lv_obj_has_state(objects.switch_led, LV_STATE_CHECKED);

    if (switch_led_checked == true) {
      lv_img_set_src(objects.image_bulb, &img_img_light_bulb_off);
      lv_obj_remove_state(objects.switch_led, LV_STATE_CHECKED);
      digitalWrite(LED_PIN, LOW);
    } else {
      lv_img_set_src(objects.image_bulb, &img_img_light_bulb_on);
      lv_obj_add_state(objects.switch_led, LV_STATE_CHECKED);
      digitalWrite(LED_PIN, HIGH);
    }
  }
}


// VOID SETUP()
void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial.println();
  delay(1000);

  pinMode(TFT_BL_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(TFT_BL_PIN, HIGH);
  digitalWrite(LED_PIN, LOW);
  delay(500);

  // Init Display.
  Serial.println();
  Serial.println("LCD Begin.");
  lcd.begin();
  lcd.fillScreen(0x000000U);
  delay(100);

  // Init touch device.
  Serial.println();
  Serial.println("Touchscreen Begin.");
  touch_init();
  delay(100);

  //----------------------------------------LVGL setup.
  Serial.println();
  Serial.println("Start LVGL Setup...");
  delay(500);

  // Start LVGL.
  lv_init();

  // Register print function for debugging.
  lv_log_register_print_cb(log_print);

  // Initialize the display.
  lv_display_t * disp = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_display_set_buffers(disp, draw_buf, NULL, DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_resolution(disp, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_display_set_physical_resolution(disp, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_0);
  lv_display_set_flush_cb(disp, my_disp_flush);

  // Initialize the (dummy) input device driver.
  lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touchscreen_read);

  Serial.println("LVGL Setup Completed.");
  delay(500);
  //----------------------------------------

  //---------------------------------------- Integrate EEZ Studio GUI.
  ui_init();
  //---------------------------------------- 

  // Register "switch_led" event handler.
  lv_obj_add_event_cb(objects.switch_led, switch_led_event_handler, LV_EVENT_VALUE_CHANGED, NULL);

  // Register "image_bulb" event handler.
  lv_obj_add_event_cb(objects.image_bulb, image_bulb_event_handler, LV_EVENT_ALL, NULL);

  lv_timer_handler();
  delay(500);
}


// VOID LOOP()
void loop() {
  // put your main code here, to run repeatedly:
  
  unsigned long currentMillis_Update_UI = millis();
  if (currentMillis_Update_UI - previousMillis_Update_UI >= interval_Update_UI) {
    previousMillis_Update_UI = currentMillis_Update_UI;

    update_UI();
  }
}

