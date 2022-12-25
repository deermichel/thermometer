#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "dht22.h"
#include "lcd1602.h"

// char code for degree symbol on lcd1602
const char DEGREE_CHAR = 223;

// measurement interval in us
const uint32_t MEASUREMENT_INTERVAL = 20 * 1000 * 1000; // 20s

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

// display mode: current measurement or min-max values
enum { CURRENT, MINMAX } display_mode = CURRENT;

// thermometer data
struct {
    int16_t current_temp, min_temp, max_temp;
    uint16_t current_hum, min_hum, max_hum;
} data = {0};

// switch timestamp in us for debounce and longpress detection
uint32_t sw_pressed_since = 0;

// last measurement timestamp in us
uint32_t last_measurement = 0;

// whether display needs update
bool display_needs_update = true;

// update data with new measurement
bool update_data(int16_t temp, uint16_t hum) {
    if (temp == data.current_temp && hum == data.current_hum) return false; // no change
    data.current_temp = temp;
    if (temp < data.min_temp) data.min_temp = temp;
    if (temp > data.max_temp) data.max_temp = temp;
    data.current_hum = hum;
    if (hum < data.min_hum) data.min_hum = hum;
    if (hum > data.max_hum) data.max_hum = hum;
    return true;
}

// update display according to current mode
void update_display() {
    char buffer[17];
    switch (display_mode) {
        case CURRENT: // default screen - current measurements
            snprintf(buffer, 17, "Temp:   %3d.%d %cC", (data.current_temp / 10) % 100, abs(data.current_temp) % 10, DEGREE_CHAR);
            lcd1602_clear_display(&lcd);
            lcd1602_write_string(&lcd, buffer);
            snprintf(buffer, 17, "Hum:     %2d.%d %%", (data.current_hum / 10) % 100, data.current_hum % 10);
            lcd1602_set_cursor_pos(&lcd, 1, 0);
            lcd1602_write_string(&lcd, buffer);
            break;

        case MINMAX: // min-max values
            snprintf(buffer, 17, "Min %3d.%dC %2d.%d%%",
                (data.min_temp / 10) % 100, abs(data.min_temp) % 10,
                (data.min_hum / 10) % 100, data.min_hum % 10);
            lcd1602_clear_display(&lcd);
            lcd1602_write_string(&lcd, buffer);
            snprintf(buffer, 17, "Max %3d.%dC %2d.%d%%",
                (data.max_temp / 10) % 100, abs(data.max_temp) % 10,
                (data.max_hum / 10) % 100, data.max_hum % 10);
            lcd1602_set_cursor_pos(&lcd, 1, 0);
            lcd1602_write_string(&lcd, buffer);
            break;
    }
}

// gpio irq callback for switch
void switch_pressed() {
     // debounce 250ms
    if (time_us_32() - sw_pressed_since < 250000) return;
    sw_pressed_since = time_us_32();

    // change display mode
    display_mode = (display_mode == CURRENT) ? MINMAX : CURRENT;
    display_needs_update = true;
}

// main
int main() {
    stdio_init_all();

    // status led
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);

    // dht config
    dht22_t dht;
    dht22_init(&dht, DHT_PIN);

    // lcd config
    lcd1602_init(&lcd);
    lcd1602_set_display_on(&lcd, true, false, false);
    lcd1602_write_string(&lcd, "Hi there! :)");
    lcd1602_set_cursor_pos(&lcd, 1, 0);
    lcd1602_write_string(&lcd, "Switch: Min-Max");

    // switch config
    gpio_init(SW_PIN);
    gpio_set_dir(SW_PIN, GPIO_IN);
    gpio_pull_up(SW_PIN);
    gpio_set_irq_enabled_with_callback(SW_PIN, GPIO_IRQ_EDGE_FALL, true, switch_pressed);

    // initial measurement
    dht22_trigger_measurement(&dht);
    last_measurement = time_us_32();
    while (!dht22_is_ready(&dht) && !dht22_has_error(&dht)) dht22_task(&dht);
    data.min_temp = data.max_temp = data.current_temp = dht22_get_temperature(&dht);
    data.min_hum = data.max_hum = data.current_hum = dht22_get_humidity(&dht);

    // runloop
    while (1) {
        // status led
        gpio_put(LED_PIN, !dht22_is_ready(&dht));

        // clear min-max values on longpress (1s)
        if (gpio_get(SW_PIN) == 0 && time_us_32() - sw_pressed_since > 1000000) {
            data.min_temp = data.max_temp = data.current_temp;
            data.min_hum = data.max_hum = data.current_hum;
            display_needs_update = true;
            sw_pressed_since = time_us_32();
        }

        // trigger measurement
        if (time_us_32() - last_measurement > MEASUREMENT_INTERVAL) {
            dht22_trigger_measurement(&dht);
            last_measurement = time_us_32();
        }

        // update data once ready
        if (dht22_is_ready(&dht)) {
            bool got_new_data = update_data(dht22_get_temperature(&dht), dht22_get_humidity(&dht));
            if (got_new_data) display_needs_update = true;
        }

        // reset error
        if (dht22_has_error(&dht)) {
            dht22_reset(&dht);
        }

        // display needs update
        if (display_needs_update) {
            update_display();
            display_needs_update = false;
        }

        // driver task
        dht22_task(&dht);
    }

    // cleanup
    dht22_dispose(&dht);
    lcd1602_dispose(&lcd);
    return 0;
}
