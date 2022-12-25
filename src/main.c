#include "pico/stdlib.h"
#include "app.h"
#include "dht22.h"
#include "lcd1602.h"

// measurement interval in us
const uint32_t MEASUREMENT_INTERVAL = 30 * 1000 * 1000; // 30s

// pinout
const uint LED_PIN = 25;
const uint DHT_PIN = 9;
const uint SW_PIN = 14;
const uint LCD_EN = 21;
const uint LCD_RS = 16;
const uint LCD_D4 = 20;
const uint LCD_D5 = 19;
const uint LCD_D6 = 18;
const uint LCD_D7 = 17;

// lcd handle
lcd1602_t lcd = {
    .en = LCD_EN,
    .rs = LCD_RS,
    .d4 = LCD_D4,
    .d5 = LCD_D5,
    .d6 = LCD_D6,
    .d7 = LCD_D7,
};

// last measurement timestamp in us
uint32_t last_measurement = 0;

// main
int main() {
    stdio_init_all();

    // status led config
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);

    // lcd config
    lcd1602_init(&lcd);
    lcd1602_set_display_on(&lcd, true, false, false);
    lcd1602_write_string(&lcd, "Hi there! :) v1");
    lcd1602_set_cursor_pos(&lcd, 1, 0);
    lcd1602_write_string(&lcd, "Switch: Min-Max");

    // dht config
    dht22_t dht;
    dht22_init(&dht, DHT_PIN);
    sleep_ms(1000); // 1s cooldown

    // initial measurement
    dht22_trigger_measurement(&dht);
    last_measurement = time_us_32();
    while (!dht22_is_ready(&dht) && !dht22_has_error(&dht)) dht22_task(&dht);

    // app init
    app_init(SW_PIN, dht22_get_temperature(&dht), dht22_get_humidity(&dht));

    // runloop
    while (1) {
        // status led
        gpio_put(LED_PIN, !dht22_is_ready(&dht));

        // trigger measurement
        if (time_us_32() - last_measurement > MEASUREMENT_INTERVAL) {
            dht22_trigger_measurement(&dht);
            last_measurement = time_us_32();
        }

        // update data once ready
        if (dht22_is_ready(&dht)) {
            app_update_data(dht22_get_temperature(&dht), dht22_get_humidity(&dht));
        }

        // reset error
        if (dht22_has_error(&dht)) {
            dht22_reset(&dht);
        }

        // tasks
        app_task(&lcd);
        dht22_task(&dht);
    }

    // if we get down here, cleanup is the least of our problems ...
    app_dispose();
    dht22_dispose(&dht);
    lcd1602_dispose(&lcd);
    return 0;
}
