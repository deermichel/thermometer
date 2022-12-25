#include "pico/stdlib.h"
#include "lcd1602.h"

// gpio init helper
static void gpio_init_out(uint pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, 0);
}

// toggle enable pin to commit data input
static void commit(lcd1602_t *lcd) {
    gpio_put(lcd->en, 1);
    sleep_us(40);
    gpio_put(lcd->en, 0);
    sleep_us(40);
}

// send 4 bit via 4 bit interface
static void send_4bit(lcd1602_t *lcd, bool rs, uint8_t data) {
    gpio_put(lcd->rs, rs);
    gpio_put(lcd->d4, data & 0x1);
    gpio_put(lcd->d5, data & 0x2);
    gpio_put(lcd->d6, data & 0x4);
    gpio_put(lcd->d7, data & 0x8);
    commit(lcd);
}

// send 8 bit via 4 bit interface
static void send_8bit(lcd1602_t *lcd, bool rs, uint8_t data) {
    send_4bit(lcd, rs, (data & 0xF0) >> 4);
    send_4bit(lcd, rs, data & 0x0F);
}

// clear display
void lcd1602_clear_display(lcd1602_t *lcd) {
    // RS D7 D6 D5 D4 D3 D2 D1 D0
    //  0  0  0  0  0  0  0  0  1
    send_8bit(lcd, 0, 0x01);
    sleep_ms(2); // sleep 2ms
}

// dispose, releases all resources
void lcd1602_dispose(lcd1602_t *lcd) {
    gpio_deinit(lcd->en);
    gpio_deinit(lcd->rs);
    gpio_deinit(lcd->d4);
    gpio_deinit(lcd->d5);
    gpio_deinit(lcd->d6);
    gpio_deinit(lcd->d7);
}

// init lcd1602 (set pins in struct before calling init)
void lcd1602_init(lcd1602_t *lcd) {
    // init gpio
    gpio_init_out(lcd->en);
    gpio_init_out(lcd->rs);
    gpio_init_out(lcd->d4);
    gpio_init_out(lcd->d5);
    gpio_init_out(lcd->d6);
    gpio_init_out(lcd->d7);

    // init sequence
    sleep_ms(20);               // sleep > 15ms
    send_4bit(lcd, 0, 0x3);     // function set: 8 bit
    sleep_ms(5);                // sleep > 4.1ms
    send_4bit(lcd, 0, 0x3);     // function set: 8 bit
    sleep_ms(1);                // sleep > 0.1ms
    send_4bit(lcd, 0, 0x3);     // function set: 8 bit
    send_4bit(lcd, 0, 0x2);     // function set: 4 bit
    send_8bit(lcd, 0, 0x28);    // function set: 4 bit, 2 lines, 5x8 font
    send_8bit(lcd, 0, 0x08);    // display off
    send_8bit(lcd, 0, 0x01);    // display clear
    sleep_ms(2);                // sleep 2ms
    send_8bit(lcd, 0, 0x06);    // entry mode set: move right, no shift
}

// set cursor position
void lcd1602_set_cursor_pos(lcd1602_t *lcd, uint8_t row, uint8_t col) {
    // RS D7 D6 D5 D4 D3 D2 D1 D0
    //  0  1 A6 A5 A4 A3 A2 A1 A0
    // 1st line: 0x00 - 0x27, 2nd line: 0x40 - 0x67
    uint8_t address = row * 0x40 + col;
    send_8bit(lcd, 0, 0x80 | address);
}

// set custom character
void lcd1602_set_custom_char(lcd1602_t *lcd, uint8_t index, const uint8_t *pattern) {
    // RS D7 D6 D5 D4 D3 D2 D1 D0
    //  0  0  1 A5 A4 A3 A2 A1 A0
    send_8bit(lcd, 0, 0x40 | (index * 8)); // set cgram address
    for (int i = 0; i < 8; i++) {
        send_8bit(lcd, 1, pattern[i]); // send cgram data
    }
    send_8bit(lcd, 0, 0x80); // reset cursor -> use ddram
}

// set display, cursor, blink on/off
void lcd1602_set_display_on(lcd1602_t *lcd, bool display, bool cursor, bool blink) {
    // RS D7 D6 D5 D4 D3 D2 D1 D0
    //  0  0  0  0  0  1  D  C  B
    send_8bit(lcd, 0, 0x08 | (display << 2) | (cursor << 1) | blink);
}

// write string
void lcd1602_write_string(lcd1602_t *lcd, const char *string) {
    for (int i = 0; string[i] != 0; i++) {
        send_8bit(lcd, 1, string[i]);
    }
}
