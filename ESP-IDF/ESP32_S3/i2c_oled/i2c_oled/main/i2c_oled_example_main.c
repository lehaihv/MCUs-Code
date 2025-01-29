/*
 * SPDX-FileCopyrightText: 2021-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "esp_lvgl_port.h"
#include "lvgl.h"

//BMP280 components
#include "bmp280_i2c.h"
#include "bmp280_i2c_hal.h"

#if CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
#include "esp_lcd_sh1107.h"
#else
#include "esp_lcd_panel_vendor.h"
#endif

static const char *TAG = "example";

#define I2C_BUS_PORT  0

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your LCD spec //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ    (400 * 1000)
#define EXAMPLE_PIN_NUM_SDA           7//4//7//3
#define EXAMPLE_PIN_NUM_SCL           15//5//15//4
#define EXAMPLE_PIN_NUM_RST           5//-1
#define EXAMPLE_I2C_HW_ADDR           0x3C

// The pixel number in horizontal and vertical
#if CONFIG_EXAMPLE_LCD_CONTROLLER_SSD1306
#define EXAMPLE_LCD_H_RES              128
#define EXAMPLE_LCD_V_RES              CONFIG_EXAMPLE_SSD1306_HEIGHT
#elif CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
#define EXAMPLE_LCD_H_RES              64
#define EXAMPLE_LCD_V_RES              128
#endif
// Bit number used to represent command and parameter
#define EXAMPLE_LCD_CMD_BITS           8
#define EXAMPLE_LCD_PARAM_BITS         8

extern void example_lvgl_demo_ui(lv_disp_t *disp);
char sample_txt[] = "UF 2024\nUF 2025\nUF 2026\nUF 2027";

void app_main(void)
{
    ESP_LOGI(TAG, "Initialize I2C bus");
    i2c_master_bus_handle_t i2c_bus = NULL;
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .i2c_port = I2C_BUS_PORT,
        .sda_io_num = EXAMPLE_PIN_NUM_SDA,
        .scl_io_num = EXAMPLE_PIN_NUM_SCL,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus));

    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = EXAMPLE_I2C_HW_ADDR,
        .scl_speed_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
        .control_phase_bytes = 1,               // According to SSD1306 datasheet
        .lcd_cmd_bits = EXAMPLE_LCD_CMD_BITS,   // According to SSD1306 datasheet
        .lcd_param_bits = EXAMPLE_LCD_CMD_BITS, // According to SSD1306 datasheet
#if CONFIG_EXAMPLE_LCD_CONTROLLER_SSD1306
        .dc_bit_offset = 6,                     // According to SSD1306 datasheet
#elif CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
        .dc_bit_offset = 0,                     // According to SH1107 datasheet
        .flags =
        {
            .disable_control_phase = 1,
        }
#endif
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c_bus, &io_config, &io_handle));

    ESP_LOGI(TAG, "Install SSD1306 panel driver");
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .bits_per_pixel = 1,
        .reset_gpio_num = EXAMPLE_PIN_NUM_RST,
    };
#if CONFIG_EXAMPLE_LCD_CONTROLLER_SSD1306
    esp_lcd_panel_ssd1306_config_t ssd1306_config = {
        .height = EXAMPLE_LCD_V_RES,
    };
    panel_config.vendor_config = &ssd1306_config;
    ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(io_handle, &panel_config, &panel_handle));
#elif CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
    ESP_ERROR_CHECK(esp_lcd_new_panel_sh1107(io_handle, &panel_config, &panel_handle));
#endif

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

#if CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
#endif

    ESP_LOGI(TAG, "Initialize LVGL");
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    lvgl_port_init(&lvgl_cfg);

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .buffer_size = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES,
        .double_buffer = true,
        .hres = EXAMPLE_LCD_H_RES,
        .vres = EXAMPLE_LCD_V_RES,
        .monochrome = true,
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        }
    };
    lv_disp_t *disp = lvgl_port_add_disp(&disp_cfg);

    /* Rotation of the screen */
    lv_disp_set_rotation(disp, LV_DISP_ROT_180);

    ESP_LOGI(TAG, "Display LVGL Scroll Text");
    // Lock the mutex due to the LVGL APIs are not thread-safe
    /* if (lvgl_port_lock(0)) {
        example_lvgl_demo_ui(disp);
        // Release the mutex
        //lv_label_set_text(*disp, sample_txt);
        lvgl_port_unlock();       
    } */

    // BMP280 
    /* esp_err_t err;
    uint8_t id = 0;

    bmp280_i2c_hal_init();

    err = bmp280_i2c_reset();
    if(err != BMP280_OK) ESP_LOGE(TAG, "Error setting the device!");

    err += bmp280_i2c_read_part_number(&id);
    if(err == ESP_OK){
        ESP_LOGI(TAG, "Part number: 0x%02x", id);
    } 
    else{
        ESP_LOGE(TAG, "Unable to read part number!");
    }

    err += bmp280_i2c_set_calib();
    ESP_LOGI(TAG, "Calibration data setting: %s", err == BMP280_OK ? "Successful" : "Failed");

    err += bmp280_i2c_write_power_mode(POWERMODE_NORMAL);
    ESP_LOGI(TAG, "Setting to normal mode: %s", err == BMP280_OK ? "Successful" : "Failed");

    //Config setting. We'll use suggested settings for elevation detection
    err += bmp280_i2c_write_config_filter(FILTER_4);
    bmp280_ctrl_meas_t ctrl_meas = {
        .osrs_press = OSRS_x4,
        .osrs_tmp = OSRS_x1,
    };
    err += bmp280_i2c_write_osrs(ctrl_meas);

    //uint8_t cfg;
    //bmp280_i2c_read_config(&cfg);
    //ESP_LOGW(TAG, "read_config: %d", cfg);

    if (err == BMP280_OK && id == 0x58)
    {
        ESP_LOGI(TAG, "BMP280 initialization successful");
        bmp280_data_t bmp280_dt;
        while(1)
        {
            //Reading here
            if(bmp280_i2c_read_data(&bmp280_dt) == BMP280_OK)
            {
                ESP_LOGI(TAG, "Pressure: %.01f Pa", (float)bmp280_dt.pressure/256);
                ESP_LOGI(TAG, "Temperature: %.01f °C", (float)bmp280_dt.temperature/100);
            }
            else{
                ESP_LOGE(TAG, "Error reading data!");
            }
            
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }
    else{
        ESP_LOGE(TAG, "BMP280 initialization failed!");
    } */

    // Oled
    lv_obj_t *scr = lv_scr_act();
    
    /* Task lock */
    lvgl_port_lock(0);

    /* Your LVGL objects code here .... */

    /* Label */
    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_recolor(label, true);
    lv_obj_set_width(label, EXAMPLE_LCD_H_RES);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, 0);  // LV_TEXT_ALIGN_CENTER
    lv_label_set_text_fmt(label, "Preceding with zeros: %010d \n", 1977);//"Preceding with blanks: %10d \n", 1977); //"floats: %4.2f %+.0e %E \n", 3.1416, 3.1416, 3.1416);
    //lv_label_set_text(label, sample_txt);//"#FF0000 "LV_SYMBOL_BELL" Hello world Espressif and LVGL "LV_SYMBOL_BELL"#\n#FF9400 "LV_SYMBOL_WARNING" For simplier initialization, use BSP "LV_SYMBOL_WARNING" #");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0); // -30 LV_ALIGN_TOP_MID // LV_ALIGN_CENTER

    //lv_label_set_text(label, sample_txt);//"#FF0000 "LV_SYMBOL_BELL" Hello world Espressif and LVGL "LV_SYMBOL_BELL"#\n#FF9400 "LV_SYMBOL_WARNING" For simplier initialization, use BSP "LV_SYMBOL_WARNING" #");
    //lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20); // -30 LV_ALIGN_TOP_MID // LV_ALIGN_CENTER

    /* Button */
    /* lv_obj_t *btn = lv_btn_create(scr);
    label = lv_label_create(btn);
    lv_label_set_text_static(label, "Rotate screen");
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_obj_add_event_cb(btn, _app_button_cb, LV_EVENT_CLICKED, NULL); */

    /* Task unlock */
    lvgl_port_unlock();
    ///////////
    /* esp_err_t err;
    uint8_t id = 0;

    bmp280_i2c_hal_init();

    err = bmp280_i2c_reset();
    if(err != BMP280_OK) ESP_LOGE(TAG, "Error setting the device!");

    err += bmp280_i2c_read_part_number(&id);
    if(err == ESP_OK){
        ESP_LOGI(TAG, "Part number: 0x%02x", id);
    } 
    else{
        ESP_LOGE(TAG, "Unable to read part number!");
    }

    err += bmp280_i2c_set_calib();
    ESP_LOGI(TAG, "Calibration data setting: %s", err == BMP280_OK ? "Successful" : "Failed");

    err += bmp280_i2c_write_power_mode(POWERMODE_NORMAL);
    ESP_LOGI(TAG, "Setting to normal mode: %s", err == BMP280_OK ? "Successful" : "Failed");

    //Config setting. We'll use suggested settings for elevation detection
    err += bmp280_i2c_write_config_filter(FILTER_4);
    bmp280_ctrl_meas_t ctrl_meas = {
        .osrs_press = OSRS_x4,
        .osrs_tmp = OSRS_x1,
    };
    err += bmp280_i2c_write_osrs(ctrl_meas);

    //uint8_t cfg;
    //bmp280_i2c_read_config(&cfg);
    //ESP_LOGW(TAG, "read_config: %d", cfg);

    if (err == BMP280_OK && id == 0x58)
    {
        ESP_LOGI(TAG, "BMP280 initialization successful");
        bmp280_data_t bmp280_dt;
        while(1)
        {
            //Reading here
            if(bmp280_i2c_read_data(&bmp280_dt) == BMP280_OK)
            {
                ESP_LOGI(TAG, "Pressure: %.01f Pa", (float)bmp280_dt.pressure/256);
                ESP_LOGI(TAG, "Temperature: %.01f °C", (float)bmp280_dt.temperature/100);
            }
            else{
                ESP_LOGE(TAG, "Error reading data!");
            }
            
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }
    else{
        ESP_LOGE(TAG, "BMP280 initialization failed!");
    } */
}
