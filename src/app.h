#ifndef app_h
#define app_h

#include "lcd1602.h"

// --- thermometer app public interface ---

// dispose app
void app_dispose();

// init app with switch pin and initial values
void app_init(uint sw_pin, int16_t temperature, int16_t humidity);

// app task to be called in runloop
void app_task(lcd1602_t *lcd);

// update thermometer data
void app_update_data(int16_t temperature, int16_t humidity);

#endif // app_h
