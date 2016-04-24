#ifndef PEBBLE_ARDUINO_TEMP
#define PEBBLE_ARDUINO_TEMP
#include <pebble.h>

/* This is called when the up button is clicked */
void temperature_up_click_handler(ClickRecognizerRef recognizer, void *context);
/* This is called when the select button is clicked */
void temperature_select_click_handler(ClickRecognizerRef recognizer, void *context);
/* This is called when the down button is clicked */
void temperature_down_click_handler(ClickRecognizerRef recognizer, void *context);

void temperature_config_provider(void *context);

void temperature_window_load(Window *window);

void temperature_window_unload(Window *window);


#endif