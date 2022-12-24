#include <stdlib.h>
#include "pico/stdlib.h"
#include "dht22.h"

// dispose dht22 instance, releases all resources
void dht22_dispose(dht22_t *dht) {
    gpio_deinit(dht->pin);
}

// return latest humidity measurement * 10 (658 -> 65.8%)
int16_t dht22_get_humidity(dht22_t *dht) {
    return dht->humidity;
}

// return latest temperature measurement * 10 (269 -> 26.9C)
int16_t dht22_get_temperature(dht22_t *dht) {
    return dht->temperature;
}

// return whether an error has occurred
bool dht22_has_error(dht22_t *dht) {
    return dht->state == ERROR;
}

// init dht22 on pin X
void dht22_init(dht22_t *dht, uint pin) {
    gpio_init(pin);
    dht->pin = pin;
    dht->state = READY;
}

// return whether dht22 is ready for next measurement
bool dht22_is_ready(dht22_t *dht) {
    return dht->state == READY;
}

// driver task to be called in runloop
void dht22_task(dht22_t *dht);

// trigger measurement, returns false if not in READY state
bool dht22_trigger_measurement(dht22_t *dht);

// reset driver, e.g. after error
void dht22_reset(dht22_t *dht);
