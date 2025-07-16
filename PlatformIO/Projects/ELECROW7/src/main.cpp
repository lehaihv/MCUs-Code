#include <Arduino.h>
#include <lvgl.h>
#include <LovyanGFX.hpp>
#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>
#include <Adafruit_GFX.h>
#include <Adafruit_BME280.h>
//#include <Adafruit_BMP280.h>
#include "ui.h"
#include "images.h"


// Change to your screen resolution.
#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 480

// Global variables for data series to be included in the chart.
lv_chart_series_t * ser_Temp;
lv_chart_series_t * ser_Hum;
int32_t * ser_Hum_y_points;
lv_obj_t * chart_bme_ref = NULL;
lv_chart_series_t * ser_Temp_ref = NULL;
lv_chart_series_t * ser_Hum_ref = NULL;



// Defines the number of series points on the chart.
#define number_of_points_series 100

// define pin for I2C module 1 ---> ADS1115
#define I2C_0_SDA 19
#define I2C_0_SCL 20

TwoWire I2C_BMP280 = TwoWire(0);  // "0", "1" instance of I2C module bus

Adafruit_BME280 bme280; // I2C

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
uint8_t pwm_value = 0;

unsigned long previousMillis_Update_UI = 0;
const long interval_Update_UI = 500;

// LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes.
#define DRAW_BUF_SIZE ((SCREEN_WIDTH * SCREEN_HEIGHT / 10) * (sizeof(uint16_t)))
static uint16_t draw_buf[DRAW_BUF_SIZE];

// Add global axis range variables for chart Y axes
int y_min_left = 0, y_max_left = 40;
int y_min_right = 0, y_max_right = 100;


// log_print()
// If logging is enabled, it will inform the user about what is happening in the library.
void log_print(lv_log_level_t level, const char * buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

void reset_chart(){
  // Reset chart.
  ser_Temp = lv_chart_get_series_next(objects.chart_bme280, NULL);
  ser_Hum = lv_chart_get_series_next(objects.chart_bme280, ser_Temp);
  for (int i = 0; i < number_of_points_series; i++) {
    lv_chart_set_next_value(objects.chart_bme280, ser_Temp, 0);
    lv_chart_set_next_value(objects.chart_bme280, ser_Hum, 0);
  }
  lv_chart_refresh(objects.chart_bme280);
}

void chart_setting(){
  //---------------------------------------- Chart settings.
  // Set the chart data display type.
  // For more details see here: https://docs.lvgl.io/9.2/widgets/chart.html#chart-type
  lv_chart_set_type(objects.chart_bme280, LV_CHART_TYPE_LINE);

  // Set the number of points in the series.
  // For more details see here: https://docs.lvgl.io/9.2/widgets/chart.html#number-of-points
  lv_chart_set_point_count(objects.chart_bme280, number_of_points_series);

  // Set the minimum and maximum values ​​in the y direction.
  // For more details see here: https://docs.lvgl.io/9.2/widgets/chart.html#vertical-range
  // LV_CHART_AXIS_PRIMARY_Y, 0, 80); --> 80 is the height of "chart_heartbeat". See in EEZ Studio project.
  //y_min_left = 0; y_max_left = 40;
  //y_min_right = 0; y_max_right = 100;
  lv_chart_set_range(objects.chart_bme280, LV_CHART_AXIS_PRIMARY_Y, y_min_left, y_max_left);
  lv_chart_set_range(objects.chart_bme280, LV_CHART_AXIS_SECONDARY_Y, y_min_right, y_max_right);

  lv_obj_set_style_pad_right(objects.chart_bme280, 20, 0);
  lv_obj_set_style_size(objects.chart_bme280, 6, 0, LV_PART_INDICATOR); // dots visible


  // Show horizontal and vertical grid lines
  lv_chart_set_div_line_count(objects.chart_bme280, 10, 10);
  lv_obj_set_style_line_opa(objects.chart_bme280, LV_OPA_40, LV_PART_ITEMS);
  lv_obj_set_style_line_width(objects.chart_bme280, 1, LV_PART_ITEMS);

  // (Optional) Customize series indicator lines
  lv_obj_set_style_line_opa(objects.chart_bme280, LV_OPA_COVER, LV_PART_INDICATOR);
  lv_obj_set_style_line_width(objects.chart_bme280, 2, LV_PART_INDICATOR);

  // Enable drawing of secondary Y-axis grid lines using items part
  lv_obj_set_style_opa(objects.chart_bme280, LV_OPA_COVER, LV_PART_ITEMS | LV_STATE_DEFAULT);  

  // Set to not display points in data.
  lv_obj_set_style_size(objects.chart_bme280, 0, 0, LV_PART_INDICATOR);

  // Add or insert series to chart.
  // For more details see here: https://docs.lvgl.io/9.2/widgets/chart.html#data-series
  ser_Temp = lv_chart_add_series(objects.chart_bme280, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
  ser_Hum = lv_chart_add_series(objects.chart_bme280, lv_palette_main(LV_PALETTE_GREEN), LV_CHART_AXIS_SECONDARY_Y);
  //ser_Hum_y_points = lv_chart_get_y_array(objects.chart_bme280, ser_Hum);

  // Set the initial data on the chart.
  for (int i = 0; i < number_of_points_series; i++) {
    lv_chart_set_next_value(objects.chart_bme280, ser_Temp, 0);
    lv_chart_set_next_value(objects.chart_bme280, ser_Hum, 0);
  }

  // The "lv_chart_refresh(chart_name)" command must be called after the external data source is updated to update the chart.
  // For more details see here: https://docs.lvgl.io/9.2/widgets/chart.html#data-series
  lv_chart_refresh(objects.chart_bme280);
}

void add_axis_labels(void) {
    lv_obj_t * chart = chart_bme_ref;
    lv_chart_series_t * ser_Temp = ser_Temp_ref;
    lv_chart_series_t * ser_Hum = ser_Hum_ref;
    if (!chart || !ser_Temp || !ser_Hum) return;

    lv_obj_t * parent = lv_obj_get_parent(chart);

    // Get the chart's inner drawing area (where data is plotted)
    lv_area_t content_area;
    lv_obj_get_content_coords(chart, &content_area);

    // Get the chart's absolute coordinates (position relative to parent)
    lv_area_t chart_coords;
    lv_obj_get_coords(chart, &chart_coords);
    lv_point_t chart_abs_pos = { chart_coords.x1, chart_coords.y1 };

    lv_coord_t chart_x = content_area.x1;
    lv_coord_t chart_y = content_area.y1;
    lv_coord_t chart_w = lv_area_get_width(&content_area);
    lv_coord_t chart_h = lv_area_get_height(&content_area);

    // ----- Y axis labels (left and right) -----
    // Get the actual ranges from the chart
    // Use y_min_left, y_max_left, y_min_right, y_max_right directly
    const int num_y_labels = 5;

    for (int i = 0; i < num_y_labels; i++) {
        // Left Y axis (primary)
        int val_left = y_min_left + i * (y_max_left - y_min_left) / (num_y_labels - 1);
        int y_pos_left = chart_y + chart_h - (val_left - y_min_left) * chart_h / (y_max_left - y_min_left);

        lv_obj_t * label_left = lv_label_create(parent);
        lv_label_set_text_fmt(label_left, "%d", val_left);
        lv_obj_set_style_text_align(label_left, LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_set_pos(label_left, chart_x - 35, y_pos_left - 8);

        // Right Y axis (secondary)
        int val_right = y_min_right + i * (y_max_right - y_min_right) / (num_y_labels - 1);
        int y_pos_right = chart_y + chart_h - (val_right - y_min_right) * chart_h / (y_max_right - y_min_right);

        lv_obj_t * label_right = lv_label_create(parent);
        lv_label_set_text_fmt(label_right, "%d", val_right);
        lv_obj_set_pos(label_right, chart_x + chart_w + 10, y_pos_right - 8);
    }

    // ----- X axis labels (unchanged) -----
    uint16_t point_count = lv_chart_get_point_count(chart);
    uint8_t num_x_labels = 5;

    for (uint8_t i = 0; i < num_x_labels; i++) {
        uint16_t point_index = i * (point_count - 1) / (num_x_labels - 1);
        lv_point_t pt;
        lv_chart_get_point_pos_by_id(chart, ser_Temp, point_index, &pt);
        int abs_x = chart_abs_pos.x + pt.x;
        lv_obj_t * label = lv_label_create(parent);
        lv_label_set_text_fmt(label, "%d", point_index);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_pos(label, abs_x - 10, content_area.y2 + 5);
    }
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

// slider_PWM_event_handler()
// Callback that is triggered when "slider_PWM" value change.
static void slider_PWM_event_handler(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e); //--> Get the event code.
  if (code == LV_EVENT_VALUE_CHANGED)
  {
    lv_obj_t *target_slider = (lv_obj_t *)lv_event_get_target(e); //--> Slider that generated the event.
    int16_t value = lv_slider_get_value(target_slider);           //--> Get the slider value.
    // int16_t value = lv_slider_get_value(objects.slider_pwm);

    // Map the slider value to PWM range (0-255).
    pwm_value = value; //(value, 0, 100, 0, 255);

    // Set the LED brightness using PWM.
    //ledcWrite(LED_PIN, pwm_value);
    analogWrite(LED_PIN, pwm_value);
    // Update the label with the current slider value.
    lv_label_set_text_fmt(objects.slider_value, "%d", value);

    Serial.print("Slider Value: ");
    Serial.println(value);
    Serial.print("PWM Value: ");
    Serial.println(pwm_value);
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

  //Wire.begin(19, 20);
  I2C_BMP280.begin(I2C_0_SDA, I2C_0_SCL);
  bme280.begin(0x76); // I2C address 0x76

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

  // Register "slider_PWM" event handler.
  lv_obj_add_event_cb(objects.slider_pwm, slider_PWM_event_handler, LV_EVENT_VALUE_CHANGED, NULL);

  lv_timer_handler();
  delay(500);
  // chart init
  chart_setting();
  reset_chart();
  chart_bme_ref = objects.chart_bme280;
  ser_Temp_ref = ser_Temp;
  ser_Hum_ref = ser_Hum;

  lv_timer_create([](lv_timer_t * t) {
      lv_obj_update_layout(chart_bme_ref);  // Ensure layout is updated
      add_axis_labels();
      lv_timer_del(t);  // Run once only
  }, 100, NULL);

  //ser_Temp = lv_chart_get_series_next(objects.chart_bme280, NULL);
  //ser_Hum = lv_chart_get_series_next(objects.chart_bme280, ser_Temp);

  Serial.println(F("BMP280 test"));
  unsigned status;
  status = bme280.begin(0x76);
  //status = bmp.begin();
  if (!status) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    Serial.print("SensorID was: 0x"); Serial.println(bme280.sensorID(),16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    while (1) delay(10);
  }

  /* Default settings from datasheet. */
  
  Serial.print(F("Temperature = "));
  Serial.print(bme280.readTemperature());
  Serial.println(" *C");

  Serial.print(F("Pressure = "));
  Serial.print(bme280.readPressure());
  Serial.println(" Pa");

  Serial.print(F("Approx altitude = "));
  Serial.print(bme280.readAltitude(1013.25)); /* Adjusted to local forecast! */
  Serial.println(" m");

  Serial.println();
}


// VOID LOOP()
void loop() {
  // put your main code here, to run repeatedly:
  
  unsigned long currentMillis_Update_UI = millis();
  if (currentMillis_Update_UI - previousMillis_Update_UI >= interval_Update_UI) {
    previousMillis_Update_UI = currentMillis_Update_UI;

    // Add new signal data to the chart.
    ser_Temp = lv_chart_get_series_next(objects.chart_bme280, NULL);
    ser_Hum = lv_chart_get_series_next(objects.chart_bme280, ser_Temp);
    lv_chart_set_next_value(objects.chart_bme280, ser_Temp, bme280.readTemperature());  
    lv_chart_set_next_value(objects.chart_bme280, ser_Hum, bme280.readHumidity());  

    /*Set the next points on 'ser1'*/
    //lv_chart_set_next_value(objects.chart_bme280, ser_Temp, lv_rand(30, 50));
    //lv_chart_set_next_value(objects.chart_bme280, ser_Hum, lv_rand(70, 90));

    /*Directly set points on 'ser2'*/

    //ser_Hum_y_points[i++] = lv_rand(50, 90);

    //lv_chart_refresh(objects.chart_bme280);
    /////////////////////////////

    update_UI();
    Serial.print(F("Temperature = "));
    Serial.print(bme280.readTemperature());
    Serial.println(" *C");

    Serial.print(F("Pressure = "));
    Serial.print(bme280.readPressure());
    Serial.println(" Pa");

    Serial.print(F("Approx altitude = "));
    Serial.print(bme280.readAltitude(1013.25)); /* Adjusted to local forecast! */
    Serial.println(" m");

    Serial.print("Humidity = ");
    Serial.print(bme280.readHumidity());
    Serial.println(" %");

    Serial.println(); 
  }
}

