#ifndef PEBBLE_MAIN
#define PEBBLE_MAIN
#include <pebble.h>

void clear_recevied_message(void *data);

void out_sent_handler(DictionaryIterator *sent, void *context);
void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context);
void in_received_handler(DictionaryIterator *received, void *context);
void in_dropped_handler(AppMessageResult reason, void *context);

/* This is called when the up button is clicked */
void main_up_click_handler(ClickRecognizerRef recognizer, void *context);
/* This is called when the down button is clicked */
void main_down_click_handler(ClickRecognizerRef recognizer, void *context);
void tap_handler(AccelAxisType axis, int32_t direction);

/* this registers the appropriate function to the appropriate button */
void main_config_provider(void *context);

static void main_window_load(Window *window);
static void main_window_unload(Window *window);

static void init(void);
static void deinit(void);

#endif