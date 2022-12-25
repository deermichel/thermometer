#ifndef lcd1602_h
#define lcd1602_h

// --- lcd1602 driver public interface ---

// lcd1602 driver instance
typedef struct lcd1602 {
    uint en;    // enable pin
    uint rs;    // register select pin
    uint d4;    // d4 pin
    uint d5;    // d5 pin
    uint d6;    // d6 pin
    uint d7;    // d7 pin
} lcd1602_t;

// clear display
void lcd1602_clear_display(lcd1602_t *lcd);

// dispose, releases all resources
void lcd1602_dispose(lcd1602_t *lcd);

// init lcd1602 (set pins in struct before calling init)
void lcd1602_init(lcd1602_t *lcd);

// set cursor position
void lcd1602_set_cursor_pos(lcd1602_t *lcd, uint8_t row, uint8_t col);

// set display, cursor, blink on/off
void lcd1602_set_display_on(lcd1602_t *lcd, bool display, bool cursor, bool blink);

// write string
void lcd1602_write_string(lcd1602_t *lcd, const char *string);

#endif // lcd1602_h
