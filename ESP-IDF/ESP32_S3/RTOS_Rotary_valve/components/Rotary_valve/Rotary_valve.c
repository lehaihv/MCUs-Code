#include "Rotary_valve.h"

#include "driver/uart.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static const char *TAG = "PGValve_C";

// UART configuration (adjust pins/port/baud as needed)
static const uart_port_t UART_PORT_NUM = UART_NUM_1;
static const int UART_TX_PIN = 17;
static const int UART_RX_PIN = 16;
static const int UART_BUF_SIZE = 1024;

// Frame constants
static const uint8_t FRAME_STX = 0xCC;
static const uint8_t FRAME_ETX = 0xDD;
#define GENERAL_FRAME_SIZE 8
static const uint32_t DEFAULT_TIMEOUT_MS = 200;

static bool uart_initialized = false;

struct pgvalve_t {
    uint64_t handle;
    uint16_t port;
    uint16_t type;
    uint16_t mountid;
    bool initialized;
};

static void ensure_uart(void)
{
    if (uart_initialized) return;

    uart_config_t uart_config = {};
    uart_config.baud_rate = 9600;
    uart_config.data_bits = UART_DATA_8_BITS;
    uart_config.parity = UART_PARITY_DISABLE;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    uart_config.source_clk = UART_SCLK_APB;

    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, UART_BUF_SIZE, UART_BUF_SIZE, 0, NULL, 0));

    uart_flush(UART_PORT_NUM);
    uart_initialized = true;
}

static void compute_checksum6(const uint8_t first6[6], uint8_t *ckl, uint8_t *ckh)
{
    uint16_t sum = 0;
    for (int i = 0; i < 6; ++i) sum = (uint16_t)(sum + first6[i]);
    *ckl = (uint8_t)(sum & 0xFF);
    *ckh = (uint8_t)((sum >> 8) & 0xFF);
}

// return 0x00 success, otherwise error codes similar to previous implementation
static uint8_t send_frame(uint8_t addr, uint8_t func, uint8_t plo, uint8_t phi, uint8_t *resp, uint32_t timeout_ms)
{
    ensure_uart();
    if (timeout_ms == 0) timeout_ms = DEFAULT_TIMEOUT_MS;

    uint8_t p6[6] = { FRAME_STX, addr, func, plo, phi, FRAME_ETX };
    uint8_t ckl = 0, ckh = 0;
    compute_checksum6(p6, &ckl, &ckh);
    uint8_t frame[GENERAL_FRAME_SIZE] = { p6[0], p6[1], p6[2], p6[3], p6[4], p6[5], ckl, ckh };

    uart_flush(UART_PORT_NUM);

    int written = uart_write_bytes(UART_PORT_NUM, (const char *)frame, GENERAL_FRAME_SIZE);
    uart_wait_tx_done(UART_PORT_NUM, pdMS_TO_TICKS(100));
    if (written != (int)GENERAL_FRAME_SIZE) return 0x01; // FRAME ERROR

    if (!resp) return 0x00; // success with no response

    int64_t start_ms = esp_timer_get_time() / 1000;
    while ((esp_timer_get_time() / 1000) - start_ms < (int64_t)timeout_ms) {
        uint8_t b = 0;
        int len = uart_read_bytes(UART_PORT_NUM, &b, 1, pdMS_TO_TICKS(10));
        if (len <= 0) {
            vTaskDelay(pdMS_TO_TICKS(1));
            continue;
        }
        if (b == FRAME_STX) {
            resp[0] = FRAME_STX;
            size_t need = GENERAL_FRAME_SIZE - 1;
            size_t got = 0;
            int64_t inner_start = esp_timer_get_time() / 1000;
            while (got < need && ((esp_timer_get_time() / 1000) - inner_start) < (int64_t)timeout_ms) {
                uint8_t rb = 0;
                int rlen = uart_read_bytes(UART_PORT_NUM, &rb, 1, pdMS_TO_TICKS(10));
                if (rlen <= 0) {
                    vTaskDelay(pdMS_TO_TICKS(1));
                    continue;
                }
                resp[1 + got] = rb;
                ++got;
            }
            if (got != need) return 0x04; // BUSY/timeout
            if (resp[5] != FRAME_ETX) return 0x01; // FRAME error
            uint8_t rckl = 0, rckh = 0; compute_checksum6(resp, &rckl, &rckh);
            if (rckl != resp[6] || rckh != resp[7]) return 0x01; // checksum error
            return 0x00; // success
        }
    }
    return 0x04; // timeout waiting for STX
}

static bool probe_addr(uint8_t addr)
{
    uint8_t resp[GENERAL_FRAME_SIZE];
    return send_frame(addr, 0x3F, 0x00, 0x00, resp, DEFAULT_TIMEOUT_MS) == 0x00;
}

// Public API
pgvalve_t *pgvalve_create(void)
{
    pgvalve_t *v = (pgvalve_t *)malloc(sizeof(pgvalve_t));
    if (!v) return NULL;
    memset(v, 0, sizeof(*v));
    v->handle = 0;
    v->port = 0;
    v->type = 0;
    v->mountid = 0;
    v->initialized = false;
    return v;
}

void pgvalve_destroy(pgvalve_t *v)
{
    if (!v) return;
    free(v);
}

bool pgvalve_detect(pgvalve_t *v)
{
    // If no instance provided, use a temporary one
    bool result = false;
    if (!v) {
        pgvalve_t tmp;
        memset(&tmp, 0, sizeof(tmp));
        ensure_uart();
        for (uint16_t addr = 0x42; addr <= 0x43; ++addr) {    //0x00--0x7F
            if (probe_addr((uint8_t)addr)) {
                result = true;
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(2));
        }
        return result;
    }

    ensure_uart();
    for (uint16_t addr = 0x42; addr <= 0x43; ++addr) {  //0x00--0x7F
        if (probe_addr((uint8_t)addr)) {
            v->port = (unsigned short)addr;
            v->type = 0;
            v->mountid = 0;
            return true;
        }
        vTaskDelay(pdMS_TO_TICKS(2));
    }
    return false;
}

bool pgvalve_begin(pgvalve_t *v, uint16_t port, uint16_t type, uint16_t id)
{
    if (!v) return false;
    ensure_uart();
    v->port = port;
    v->type = type;
    v->mountid = id;
    v->handle = (uint64_t)(port & 0xFFu);
    v->initialized = true;
    return true;
}

void pgvalve_end(pgvalve_t *v)
{
    if (!v) return;
    v->initialized = false;
}

bool pgvalve_reset(pgvalve_t *v)
{
    if (!v) return false;
    uint8_t resp[GENERAL_FRAME_SIZE];
    return send_frame((uint8_t)(v->handle & 0xFFu), 0x45, 0x00, 0x00, resp, DEFAULT_TIMEOUT_MS) == 0x00;
}

bool pgvalve_switchTo(pgvalve_t *v, uint16_t pos)
{
    if (!v) return false;
    uint8_t resp[GENERAL_FRAME_SIZE];
    ESP_LOGI(TAG, "Sending switchTo command: addr=0x%02X, func=0x44, pos=%d", (unsigned)((uint8_t)(v->handle & 0xFFu)), (int)pos);
    uint8_t result = send_frame((uint8_t)(v->handle & 0xFFu), 0x44, (uint8_t)(pos & 0xFFu), 0x00, resp, DEFAULT_TIMEOUT_MS);
    ESP_LOGI(TAG, "switchTo result: 0x%02X", result);
    if (result == 0x00) {
        char buf[64];
        int off = 0;
        off += snprintf(buf + off, sizeof(buf) - off, "switchTo response frame:");
        for (int i = 0; i < (int)GENERAL_FRAME_SIZE && off < (int)sizeof(buf) - 4; i++) {
            off += snprintf(buf + off, sizeof(buf) - off, " %02X", resp[i]);
        }
        ESP_LOGI(TAG, "%s", buf);
    }
    return result == 0x00;
}

bool pgvalve_getCurrentPosition(pgvalve_t *v, uint16_t *pos)
{
    if (!v || !pos) return false;
    uint8_t resp[GENERAL_FRAME_SIZE];
    if (send_frame((uint8_t)(v->handle & 0xFFu), 0x3E, 0x00, 0x00, resp, DEFAULT_TIMEOUT_MS) != 0x00) return false;

    char buf[64];
    int off = 0;
    off += snprintf(buf + off, sizeof(buf) - off, "Position response frame:");
    for (int i = 0; i < (int)GENERAL_FRAME_SIZE && off < (int)sizeof(buf) - 4; i++) {
        off += snprintf(buf + off, sizeof(buf) - off, " %02X", resp[i]);
    }
    ESP_LOGI(TAG, "%s", buf);

    *pos = (uint16_t)resp[3];
    ESP_LOGI(TAG, "Raw bytes: resp[2]=%d, resp[3]=%d, calculated pos=%d", resp[2], resp[3], *pos);
    return true;
}

bool pgvalve_getPortCount(pgvalve_t *v, uint16_t *count)
{
    (void)v; (void)count;
    return false;
}

bool pgvalve_getVersion(pgvalve_t *v, uint16_t *major, uint16_t *minor)
{
    if (!v || !major || !minor) return false;
    uint8_t resp[GENERAL_FRAME_SIZE];
    if (send_frame((uint8_t)(v->handle & 0xFFu), 0x3F, 0x00, 0x00, resp, DEFAULT_TIMEOUT_MS) != 0x00) return false;
    *major = resp[3];
    *minor = resp[4];
    return true;
}

bool pgvalve_getMaxRPM(pgvalve_t *v, uint16_t *rpm)
{
    (void)v; (void)rpm;
    return false;
}

bool pgvalve_setMaxRPM(pgvalve_t *v, uint16_t rpm)
{
    (void)v; (void)rpm;
    return false;
}

uint16_t pgvalve_getDetectedPort(pgvalve_t *v)
{
    if (!v) return 0;
    return v->port;
}

uint16_t pgvalve_getDetectedType(pgvalve_t *v)
{
    if (!v) return 0;
    return v->type;
}

uint16_t pgvalve_getDetectedMountID(pgvalve_t *v)
{
    if (!v) return 0;
    return v->mountid;
}
