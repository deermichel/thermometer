#include <stdio.h>
#include "pico/stdlib.h"
#include "dht22.h"

#define DHT_PIN 28

int main() {
    stdio_init_all();

    dht22_t dht;
    dht22_init(&dht, DHT_PIN);

    bool trigger = dht22_trigger_measurement(&dht);
    printf("Triggered?: %d\n", trigger ? 1 : 0);

    while (!dht22_is_ready(&dht) && !dht22_has_error(&dht)) {
        dht22_task(&dht);
    }

    printf("Has error?: %d\n", dht22_has_error(&dht) ? 1 : 0);
    printf("Temperature: %d\n", dht22_get_temperature(&dht));
    printf("Humidity: %d\n", dht22_get_humidity(&dht));

    dht22_dispose(&dht);

    while (1);
    return 0;
}
