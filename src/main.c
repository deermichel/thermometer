#include <stdio.h>
#include "pico/stdlib.h"
#include "lcd1602.h"

#define LED_PIN 25
#define DHT_PIN 28

int main() {
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    lcd1602_t lcd = {
        .en = 21,
        .rs = 16,
        .d4 = 20,
        .d5 = 19,
        .d6 = 18,
        .d7 = 17,
    };
    lcd1602_init(&lcd);
    lcd1602_set_display_on(&lcd, true, true, true);

    lcd1602_write_string(&lcd, "Hello");
    sleep_ms(1000);
    lcd1602_set_cursor_pos(&lcd, 1, 0);
    lcd1602_write_string(&lcd, "World!");

    while (1) {}

    lcd1602_dispose(&lcd);

    // dht22_t dht;
    // dht22_init(&dht, DHT_PIN);

    // while (1) {
    //     bool trigger = dht22_trigger_measurement(&dht);
    //     printf("\nTriggered?: %d\n", trigger ? 1 : 0);

    //     while (!dht22_is_ready(&dht) && !dht22_has_error(&dht)) {
    //         dht22_task(&dht);
    //     }

    //     printf("Has error?: %d\n", dht22_has_error(&dht) ? 1 : 0);
    //     if (dht22_has_error(&dht)) {
    //         dht22_reset(&dht);
    //         continue;
    //     }

    //     printf("Temperature: %d\n", dht22_get_temperature(&dht));
    //     printf("Humidity: %d\n", dht22_get_humidity(&dht));
    // }

    // dht22_dispose(&dht);
    return 0;
}
