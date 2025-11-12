// PGValve C API for ESP-IDF
#ifndef ROTARY_VALVE_H
#define ROTARY_VALVE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pgvalve_t pgvalve_t;

// Lifecycle
pgvalve_t *pgvalve_create(void);
void pgvalve_destroy(pgvalve_t *v);

bool pgvalve_detect(pgvalve_t *v); // if v == NULL, uses a temporary instance
bool pgvalve_begin(pgvalve_t *v, uint16_t port, uint16_t type, uint16_t id);
void pgvalve_end(pgvalve_t *v);

// Commands
bool pgvalve_reset(pgvalve_t *v);
bool pgvalve_switchTo(pgvalve_t *v, uint16_t pos);
bool pgvalve_getCurrentPosition(pgvalve_t *v, uint16_t *pos);

// Info
bool pgvalve_getPortCount(pgvalve_t *v, uint16_t *count);
bool pgvalve_getVersion(pgvalve_t *v, uint16_t *major, uint16_t *minor);
bool pgvalve_getMaxRPM(pgvalve_t *v, uint16_t *rpm);
bool pgvalve_setMaxRPM(pgvalve_t *v, uint16_t rpm);

uint16_t pgvalve_getDetectedPort(pgvalve_t *v);
uint16_t pgvalve_getDetectedType(pgvalve_t *v);
uint16_t pgvalve_getDetectedMountID(pgvalve_t *v);

#ifdef __cplusplus
}
#endif

#endif // ROTARY_VALVE_H
