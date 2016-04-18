#include "pebble-arduino-temperature.h"


extern Window *temperature_window;
TextLayer *temperature_text_layer;

void temperature_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, temperature_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, temperature_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, temperature_down_click_handler);
}

/* This is called when the up button is clicked */
void temperature_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  int key = 0;
  // send the message "hello?" to the phone, using key #0
  Tuplet value = TupletCString(key, "high");
  dict_write_tuplet(iter, &value);
  app_message_outbox_send();
}

/* This is called when the select button is clicked */
void temperature_select_click_handler(ClickRecognizerRef recognizer, void *context) {

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  int key = 0;
  // send the message "hello?" to the phone, using key #0
  Tuplet value = TupletCString(key, "average");
  dict_write_tuplet(iter, &value);
  app_message_outbox_send();
}

/* This is called when the down button is clicked */
void temperature_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  int key = 0;
  // send the message "hello?" to the phone, using key #0
  Tuplet value = TupletCString(key, "low");
  dict_write_tuplet(iter, &value);
  app_message_outbox_send();
}


void temperature_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  temperature_text_layer = text_layer_create(GRect(5,25,bounds.size.w - 5, bounds.size.h-25));
  text_layer_set_text_alignment(temperature_text_layer, GTextAlignmentLeft);
  text_layer_set_background_color(temperature_text_layer, GColorClear);
  text_layer_set_font(temperature_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text(temperature_text_layer, "Arduino Temp\nUp: High\nSelect: Avg\nDown: Low\nShake: Change");
  layer_add_child(window_layer, text_layer_get_layer(temperature_text_layer));
}

void temperature_window_unload(Window *window) {
  text_layer_destroy(temperature_text_layer);
}