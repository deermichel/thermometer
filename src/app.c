#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "app.h"

// char code for degree symbol on lcd1602
const char DEGREE_CHAR = 223;

// display mode: current measurement or min-max values
static enum { CURRENT, MINMAX } display_mode = CURRENT;

// whether display needs update
static bool display_needs_update = false;

// switch pin
static uint sw_pin = 0;

// switch timestamp in us for debounce and longpress detection
static uint32_t sw_pressed_since = 0;

// thermometer data
static struct {
    int16_t current_temp, min_temp, max_temp;
    uint16_t current_hum, min_hum, max_hum;
} data = {0};

// gpio irq callback for switch
static void switch_pressed() {
     // debounce 250ms
    if (time_us_32() - sw_pressed_since < 250000) return;
    sw_pressed_since = time_us_32();

    // change display mode
    display_mode = (display_mode == CURRENT) ? MINMAX : CURRENT;
    display_needs_update = true;
}

// update display according to current mode
static void update_display(lcd1602_t *lcd) {
    char buffer[17];
    switch (display_mode) {
        case CURRENT: // default screen - current measurements
            snprintf(buffer, 17, "Temp:   %3d.%d %cC", (data.current_temp / 10) % 100, abs(data.current_temp) % 10, DEGREE_CHAR);
            lcd1602_clear_display(lcd);
            lcd1602_write_string(lcd, buffer);
            snprintf(buffer, 17, "Hum:     %2d.%d %%", (data.current_hum / 10) % 100, data.current_hum % 10);
            lcd1602_set_cursor_pos(lcd, 1, 0);
            lcd1602_write_string(lcd, buffer);
            break;

        case MINMAX: // min-max values
            snprintf(buffer, 17, "Min %3d.%dC %2d.%d%%",
                (data.min_temp / 10) % 100, abs(data.min_temp) % 10,
                (data.min_hum / 10) % 100, data.min_hum % 10);
            lcd1602_clear_display(lcd);
            lcd1602_write_string(lcd, buffer);
            snprintf(buffer, 17, "Max %3d.%dC %2d.%d%%",
                (data.max_temp / 10) % 100, abs(data.max_temp) % 10,
                (data.max_hum / 10) % 100, data.max_hum % 10);
            lcd1602_set_cursor_pos(lcd, 1, 0);
            lcd1602_write_string(lcd, buffer);
            break;
    }
}

// dispose app
void app_dispose() {
    gpio_deinit(sw_pin);
}

// init app with switch pin and initial values
void app_init(uint pin, int16_t temp, int16_t hum) {
    // switch config
    sw_pin = pin;
    gpio_init(sw_pin);
    gpio_set_dir(sw_pin, GPIO_IN);
    gpio_pull_up(sw_pin);
    gpio_set_irq_enabled_with_callback(sw_pin, GPIO_IRQ_EDGE_FALL, true, switch_pressed);

    // set initial values
    data.min_temp = data.max_temp = data.current_temp = temp;
    data.min_hum = data.max_hum = data.current_hum = hum;
    display_needs_update = true;
}

// app task to be called in runloop
void app_task(lcd1602_t *lcd) {
    // clear min-max values on longpress (1s)
    if (gpio_get(sw_pin) == 0 && time_us_32() - sw_pressed_since > 1000000) {
        data.min_temp = data.max_temp = data.current_temp;
        data.min_hum = data.max_hum = data.current_hum;
        display_needs_update = true;
        sw_pressed_since = time_us_32();
    }

    // display needs update
    if (display_needs_update) {
        update_display(lcd);
        display_needs_update = false;
    }
}

// update thermometer data
void app_update_data(int16_t temp, int16_t hum) {
    if (temp == data.current_temp && hum == data.current_hum) return; // no change
    data.current_temp = temp;
    if (temp < data.min_temp) data.min_temp = temp;
    if (temp > data.max_temp) data.max_temp = temp;
    data.current_hum = hum;
    if (hum < data.min_hum) data.min_hum = hum;
    if (hum > data.max_hum) data.max_hum = hum;
    display_needs_update = true;
}
