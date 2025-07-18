#include <PCA9557.h>
#include <lvgl.h>
//#include <LovyanGFX.hpp>
//#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
//#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>
#include <SPI.h>
//#include <Wire.h>
//#include <Adafruit_GFX.h>
//#include <demos/lv_demos.h>
//#include <examples/lv_examples.h>
#include "ui.h"

// UI
#define TFT_BL 2

// PCA9557 constants
#define IO0 0
#define IO1 1
#define IO2 2
#define IO3 3
#define IO4 4
#define IO5 5
#define IO6 6
#define IO7 7

#define IO_LOW 0
#define IO_HIGH 1
#define IO_INPUT 1
#define IO_OUTPUT 0

SPIClass& spi = SPI;
#include "gfx_conf.h"
//#include <DHT20.h>
//DHT20  dht20(&Wire1);
//#include <Crowbits_DHT20.h>
//Crowbits_DHT20 dht20;
/* Change to your screen resolution */
static uint32_t screenWidth;
static uint32_t screenHeight;
static lv_disp_draw_buf_t draw_buf;
//static lv_color_t *disp_draw_buf;
static lv_color_t disp_draw_buf[800 * 480 / 15];
static lv_disp_drv_t disp_drv;
float tem_float = 0;
float hum_float = 0;
/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{

  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  

  //lcd.fillScreen(TFT_WHITE);
#if (LV_COLOR_16_SWAP != 0)
 lcd.pushImageDMA(area->x1, area->y1, w, h,(lgfx::rgb565_t*)&color_p->full);
#else
  lcd.pushImageDMA(area->x1, area->y1, w, h,(lgfx::rgb565_t*)&color_p->full);//
#endif

  lv_disp_flush_ready(disp);

}

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  if (touch_has_signal())
  {
    if (touch_touched())
    {
      data->state = LV_INDEV_STATE_PR;

      /*Set the coordinates*/
      data->point.x = touch_last_x;
      data->point.y = touch_last_y;
      Serial.print( "Data x " );
      Serial.println( data->point.x );
      Serial.print( "Data y " );
      Serial.println( data->point.y );
      uint16_t touchX = data->point.x;
      uint16_t touchY = data->point.y;
      //lcd.fillCircle(touchX, touchY, 10, TFT_BLUE); // 使用正的坐标值绘制圆 
 
    }
    else if (touch_released())
    {
      data->state = LV_INDEV_STATE_REL;
    }
  }
  else
  {
    data->state = LV_INDEV_STATE_REL;
  }
  delay(15);
}

void my_timer(lv_timer_t * timer)
{
    char *tem       = (char *)malloc(24);
    char *hum       = (char *)malloc(24);
    char buff[16];

    //Get ambient temperature
    sprintf(tem, "Temperature: %.2f C ", tem_float = 20.0);
    //Serial.print(tem);
    lv_label_set_text(ui_Labeltemp, tem);
    //Get relative humidity
    //sprintf(hum, "Humidity: %.2f %RH", hum_float = 50.0);
    //Serial.println(hum);
    //lv_label_set_text(ui_Labelhum, hum);

    lv_chart_set_next_value(ui_Chart1, ui_Chart1_series_1, tem_float);
    //lv_chart_set_next_value(ui_Chart1, ui_Chart1_series_2, hum_float);
    if (slider_change_value) {
      sprintf(buff, "%.2f", lv_slider_get_value(ui_Slider1));
      lv_label_set_text(ui_Labelhum, buff);
      lv_chart_set_next_value(ui_Chart1, ui_Chart1_series_2, lv_slider_get_value(ui_Slider1));
      slider_change_value = 0;
    }
    lv_chart_refresh(ui_Chart1);
}


//PCA9557 Out;
void setup()
{
  
  Serial.begin(115200);
  

  //IO
  //pinMode(38, OUTPUT);
  //digitalWrite(38, LOW);
  


  
  //Wire.begin(19, 20);
  
  // Initialize PCA9557 - no begin() method needed, just Wire.begin() above
  
  /* // Set all pins as outputs initially
  Out.pinMode(IO0, OUTPUT);
  Out.pinMode(IO1, OUTPUT);
  Out.pinMode(IO2, OUTPUT);
  Out.pinMode(IO3, OUTPUT);
  Out.pinMode(IO4, OUTPUT);
  Out.pinMode(IO5, OUTPUT);
  Out.pinMode(IO6, OUTPUT);
  Out.pinMode(IO7, OUTPUT);

  // Set IO0 and IO1 to LOW
  Out.digitalWrite(IO0, LOW);
  Out.digitalWrite(IO1, LOW);
  delay(20);
  
  // Set IO0 to HIGH
  Out.digitalWrite(IO0, HIGH);
  delay(100);
  
  // Set IO1 as input
  Out.pinMode(IO1, INPUT); */
  
  //dht20.begin();


  
  // Init Display
  lcd.begin();
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextSize(2);
  delay(200);
  


  lv_init();


  // Init touch device
  touch_init();

  screenWidth = lcd.width();
  screenHeight = lcd.height();

  lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, screenWidth * screenHeight / 15); //4

  /* Initialize the display */
  lv_disp_drv_init(&disp_drv);
  /* Change the following line to your display resolution */
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /* Initialize the (dummy) input device driver */
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);
#ifdef TFT_BL
 
  //digitalWrite(TFT_BL, HIGH);
  ledcSetup(1, 300, 8);
  ledcAttachPin(TFT_BL, 1);
  ledcWrite(1, 0); /* Screen brightness can be modified by adjusting this parameter. (0-255) */
#endif
 
  #ifdef TFT_BL
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, LOW); 
  delay(500);
  digitalWrite(TFT_BL, HIGH);
  #endif
  lcd.fillScreen(TFT_BLACK);
  //lv_demo_widgets(); 
  ui_init();
  lv_timer_t * timer = lv_timer_create(my_timer, 2000,  NULL);
}

void loop()
{
  lv_timer_handler(); /* let the GUI do its work */
  delay( 10 );
   
}