#include "pico/stdlib.h"
#include "dht22.h"

#define DHT_PIN 23

int main() {
    stdio_init_all();

    dht22_t dht;
    dht22_init(&dht, DHT_PIN);

    dht22_dispose(&dht);
    return 0;
}
