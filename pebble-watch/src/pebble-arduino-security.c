#include "pebble-arduino-security.h"
#define RESET_ALARM 5

extern Window *security_window;
TextLayer *arm_text_layer, *disarm_text_layer, *count_down_text_layer;
AppTimer *countdown = NULL;
int time_remaining = RESET_ALARM;
char security_buff[10];
bool is_armed = false;


void arm_countdown(void *something) {
	snprintf(security_buff, sizeof(security_buff), "%d", time_remaining);
  text_layer_set_text(count_down_text_layer, security_buff);
  time_remaining -= 1;


	if(time_remaining > -1) {
		countdown = app_timer_register(1000, arm_countdown, NULL);
	} 
	else {
  	text_layer_set_text(count_down_text_layer, "");
  	time_remaining = RESET_ALARM;

	}
}

void security_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, NULL);
  window_single_click_subscribe(BUTTON_ID_UP, security_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, security_down_click_handler);
}

/* This is called when the up button is clicked */
void security_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(!is_armed) {
  	is_armed = true;
  	DictionaryIterator *iter;
  	app_message_outbox_begin(&iter);
	  int key = 0;
	  // send the message "hello?" to the phone, using key #0
	  Tuplet value = TupletCString(key, "arm");
	  dict_write_tuplet(iter, &value);
	  app_message_outbox_send();
	  arm_countdown(NULL);
  }
  
}

/* This is called when the down button is clicked */
void security_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  int key = 0;
  // send the message "hello?" to the phone, using key #0
  Tuplet value = TupletCString(key, "disarm");
  dict_write_tuplet(iter, &value);
  app_message_outbox_send();
  if(!countdown) {
  	app_timer_cancel(countdown);
  	text_layer_set_text(count_down_text_layer, "");
  	time_remaining = RESET_ALARM;
  	is_armed = false;	
  }
  
}


void security_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  arm_text_layer = text_layer_create(GRect(0, 10, bounds.size.w, 20));
  disarm_text_layer = text_layer_create(GRect(0, bounds.size.h - 30, bounds.size.w - 5, 20));
  count_down_text_layer = text_layer_create(GRect(0,bounds.size.h / 2 - 20,bounds.size.w - 5, 40));
  text_layer_set_text_alignment(arm_text_layer, GTextAlignmentRight);
  text_layer_set_text_alignment(disarm_text_layer, GTextAlignmentRight);
  text_layer_set_text_alignment(count_down_text_layer, GTextAlignmentCenter);
  text_layer_set_background_color(count_down_text_layer, GColorClear);
  text_layer_set_font(arm_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_font(disarm_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_font(count_down_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text(arm_text_layer, "Arm Alarm ->");
  text_layer_set_text(disarm_text_layer, "Disarm Alarm ->");
  layer_add_child(window_layer, text_layer_get_layer(arm_text_layer));
  layer_add_child(window_layer, text_layer_get_layer(disarm_text_layer));
  layer_add_child(window_layer, text_layer_get_layer(count_down_text_layer));
}

void security_window_unload(Window *window) {
  text_layer_destroy(arm_text_layer);
  text_layer_destroy(disarm_text_layer);
  text_layer_destroy(count_down_text_layer);
}