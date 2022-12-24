#include "pico/stdlib.h"
#include "dht22.h"

// timeout in us for state transitions
#define TIMEOUT 1000 // 1ms

// state transition helper, sets timestamp
static void state_transition(dht22_t *dht, dht22_state_t new_state) {
    dht->state = new_state;
    dht->state_since = time_us_32();
}

// check data using checksum
static bool is_valid_data(uint64_t data) {
    uint8_t acc = 0;
    for (int i = 8; i < 40; i += 8) {
        acc += (data >> i) & 0xFF;
    }
    return acc == (data & 0xFF);
}

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
    state_transition(dht, READY);
}

// return whether dht22 is ready for next measurement
bool dht22_is_ready(dht22_t *dht) {
    return dht->state == READY;
}

// driver task to be called in runloop
void dht22_task(dht22_t *dht) {
    switch (dht->state) {

    // idle, waiting for trigger
    case READY:
        break;

    // measurement triggered, pull signal low
    case TRIGGER_MEASUREMENT:
        // set signal pin as output and pull low
        if ((gpio_is_dir_out(dht->pin) != GPIO_OUT) || (gpio_get(dht->pin) != 0)) {
            gpio_put(dht->pin, 0);
            gpio_set_dir(dht->pin, GPIO_OUT);
        }
        // send signal for 10ms, then release and wait for response
        else if (time_us_32() - dht->state_since > 10000) {
            gpio_set_dir(dht->pin, GPIO_IN);
            state_transition(dht, WAIT_FOR_RESPONSE);
        }
        break;

    // after trigger release, wait for signal to be pulled low by sensor
    case WAIT_FOR_RESPONSE:
        // wait for low
        if (gpio_get(dht->pin) == 0) {
            state_transition(dht, RESPONSE_LOW);
        }
        // timeout
        else if (time_us_32() - dht->state_since > TIMEOUT) {
            state_transition(dht, ERROR);
            // printf("WAIT_FOR_RESPONSE timed out\n");
        }
        break;

    // low response should last 80us, then toggle to high
    case RESPONSE_LOW:
        // wait for high
        if (gpio_get(dht->pin) == 1) {
            state_transition(dht, RESPONSE_HIGH);
        }
        // timeout
        else if (time_us_32() - dht->state_since > TIMEOUT) {
            state_transition(dht, ERROR);
            // printf("RESPONSE_LOW timed out\n");
        }
        break;

    // high response should last 80us, then toggle to low for first bit
    case RESPONSE_HIGH:
        // wait for low
        if (gpio_get(dht->pin) == 0) {
            state_transition(dht, BIT_LOW_TIME);
        }
        // timeout
        else if (time_us_32() - dht->state_since > TIMEOUT) {
            state_transition(dht, ERROR);
            // printf("RESPONSE_HIGH timed out\n");
        }
        break;

    // for each bit, signal will be low for 50us, then toggle to high
    case BIT_LOW_TIME:
        // wait for high
        if (gpio_get(dht->pin) == 1) {
            state_transition(dht, BIT_HIGH_TIME);
        }
        // timeout
        else if (time_us_32() - dht->state_since > TIMEOUT) {
            state_transition(dht, ERROR);
            // printf("BIT_LOW_TIME timed out on bit %d\n", dht->bits_received);
        }
        break;

    // 0 bit lasts 26us high, 1 bit lasts 70us high, then toggle to low
    case BIT_HIGH_TIME:
        // wait for low, then infer bit value from time, finish after 40 bits
        if (gpio_get(dht->pin) == 0) {
            // if longer than 48us -> 1, else 0, MSB first
            dht->bits_buffer = (dht->bits_buffer << 1) | (time_us_32() - dht->state_since > 48);
            // count bits, finish after 40
            dht->bits_received++;
            if (dht->bits_received == 40) {
                // verify checksum and calculate results
                if (is_valid_data(dht->bits_buffer)) {
                    dht->humidity = (dht->bits_buffer >> 24) & 0xFFFF;
                    bool negative = (dht->bits_buffer >> 8) & 0x8000;
                    dht->temperature = ((dht->bits_buffer >> 8) & 0x7FFF) * (negative ? -1 : 1);
                    state_transition(dht, COOLDOWN);
                } else {
                    state_transition(dht, ERROR);
                    // printf("Checksum verification failed\n");
                }
            } else {
                state_transition(dht, BIT_LOW_TIME);
            }
        }
        // timeout
        else if (time_us_32() - dht->state_since > TIMEOUT) {
            state_transition(dht, ERROR);
            // printf("BIT_HIGH_TIME timed out on bit %d\n", dht->bits_received);
        }
        break;

    // cooldown period, need to wait 2s before next measurement
    case COOLDOWN:
        if (time_us_32() - dht->state_since > 2000 * 1000) {
            state_transition(dht, READY);
        }
        break;

    // error, waiting for reset
    case ERROR:
        break;

    }
}

// trigger measurement, returns false if not in READY state
bool dht22_trigger_measurement(dht22_t *dht) {
    if (dht->state != READY) return false;
    dht->bits_buffer = 0;
    dht->bits_received = 0;
    state_transition(dht, TRIGGER_MEASUREMENT);
    return true;
}

// reset driver, e.g. after error
void dht22_reset(dht22_t *dht) {
    state_transition(dht, READY);
}
