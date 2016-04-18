#ifndef PEBBLE_ARDUINO_SECURITY
#define PEBBLE_ARDUINO_SECURITY
#include <pebble.h>

/* This is called when the up button is clicked */
void security_up_click_handler(ClickRecognizerRef recognizer, void *context);
/* This is called when the select button is clicked */
void security_select_click_handler(ClickRecognizerRef recognizer, void *context);
/* This is called when the down button is clicked */
void security_down_click_handler(ClickRecognizerRef recognizer, void *context);

void security_config_provider(void *context);

void security_window_load(Window *window);

void security_window_unload(Window *window);

void arm_countdown(void *something);


#endif