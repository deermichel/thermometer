#ifndef dht22_h
#define dht22_h

// --- dht22 driver public interface ---

// driver states
typedef enum dht22_state {
    READY,
    TRIGGER_MEASUREMENT,
    WAIT_FOR_RESPONSE,
    RESPONSE_LOW,
    RESPONSE_HIGH,
    BIT_LOW_TIME,
    BIT_HIGH_TIME,
    COOLDOWN,
    ERROR,
} dht22_state_t;

// dht22 driver instance
typedef struct dht22 {
    dht22_state_t state;    // current state
    uint32_t state_since;   // timestamp in us of last state change
    uint pin;               // gpio pin of dht22 signal
    int16_t humidity;       // latest humidity measurement
    int16_t temperature;    // latest temperature measurement
    uint64_t bits_buffer;   // internal buffer for received bits
    uint8_t bits_received;  // internal counter for received bits
} dht22_t;

// dispose dht22 instance, releases all resources
void dht22_dispose(dht22_t *dht);

// return latest humidity measurement * 10 (658 -> 65.8%)
int16_t dht22_get_humidity(dht22_t *dht);

// return latest temperature measurement * 10 (269 -> 26.9C)
int16_t dht22_get_temperature(dht22_t *dht);

// return whether an error has occurred
bool dht22_has_error(dht22_t *dht);

// init dht22 on pin X
void dht22_init(dht22_t *dht, uint pin);

// return whether dht22 is ready for next measurement
bool dht22_is_ready(dht22_t *dht);

// driver task to be called in runloop
void dht22_task(dht22_t *dht);

// trigger measurement, returns false if not in READY state
bool dht22_trigger_measurement(dht22_t *dht);

// reset driver, e.g. after error
void dht22_reset(dht22_t *dht);

#endif // dht22_h
