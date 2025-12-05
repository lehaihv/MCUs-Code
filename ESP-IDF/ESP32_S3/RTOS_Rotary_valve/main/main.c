#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"
#include "Rotary_valve.h"
#include "esp_log.h"

// RGB led control
#define BLINK_GPIO 38
static led_strip_handle_t led_strip;

// Rotary valve
static const char *TAG = "main_valve_task";
pgvalve_t *v = NULL;
uint16_t initial_port = 1;

// RTOS task 1
void task_RGB(void *pvParameter)
{
    while(1)
    {
        printf("Task 1 is running\n");
        /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
        led_strip_set_pixel(led_strip, 0, 0, 0, 255);
        /* Refresh the strip to send data */
        led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(500));    
        // Turn off led 
        /* Set all LED off to clear all pixels */
        led_strip_clear(led_strip);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// RTOS task 2
void task_led(void *pvParameter)
{
    while(1)
    {
        printf("Task 2 is running\n");
         gpio_set_level(GPIO_NUM_4, 1); // Turn on the led
        vTaskDelay(pdMS_TO_TICKS(100));    
        // Turn off led 
        /* Set all LED off to clear all pixels */
        gpio_set_level(GPIO_NUM_4, 0); // Turn off the led
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void valve_task(void *pvParameter)
{
    while(1)
    {
        //uint16_t i = 3;
        ESP_LOGI(TAG, "Switching to %d", initial_port);
        if (pgvalve_switchTo(v, (uint16_t)initial_port)) 
        {
            ESP_LOGI(TAG, "Switch OK");
            initial_port++; 
            if (initial_port > 10) initial_port = 1;
        } 
        else
        {
            ESP_LOGW(TAG, "Switch failed");
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
       
    pgvalve_destroy(v);
    vTaskDelete(NULL);
}

static void configure_led(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = 1, // at least one LED on board
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);

    // 2nd led at pin 4
    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_4, 1); // Turn off the led
}

static void detect_valve_port(void)
{
    //pgvalve_t *v = pgvalve_create();
    v = pgvalve_create();
    if (!v) {
        ESP_LOGE(TAG, "pgvalve_create failed");
        vTaskDelete(NULL);
        return;
    }
    if (pgvalve_detect(v)) {
        ESP_LOGI(TAG, "Valve detected on port %u", (unsigned)pgvalve_getDetectedPort(v));
        pgvalve_begin(v, pgvalve_getDetectedPort(v), 0, 0);

        uint16_t major = 0, minor = 0;
        if (pgvalve_getVersion(v, &major, &minor)) {
            ESP_LOGI(TAG, "Valve version %u.%u", major, minor);
        }

        uint16_t pos = 0;
        if (pgvalve_getCurrentPosition(v, &pos)) {
            ESP_LOGI(TAG, "Current position %u", pos);
        }
    } else {
        ESP_LOGI(TAG, "No valve found");
    }
}

void app_main(void)
{
    // Configure LED strip
    configure_led();

    // Detect valve port
    detect_valve_port();

    // Create RTOS task for RGB led control
    xTaskCreatePinnedToCore(&task_RGB, "task_RGB", 2048, NULL, 1, NULL, 0);

    // Create RTOS task for 2nd led 
    xTaskCreatePinnedToCore(&task_led, "task_led", 2048, NULL, 1, NULL, 0);

    // Create RTOS task for rotary valve
    xTaskCreatePinnedToCore(&valve_task, "valve_task", 3072, NULL, 1, NULL, 1);
}