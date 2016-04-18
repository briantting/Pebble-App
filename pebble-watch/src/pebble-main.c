#include <pebble.h>
#include "pebble-main.h"
#include "pebble-arduino-temperature.h"
#include "pebble-arduino-security.h"
Window *window, *temp_window, *temperature_window, *security_window;
TextLayer *main_text_layer, *msg_received_text_layer, *security_button_text, 
	*temperature_button_text;
char msg[200];
bool sent_msg = false;
AppTimer *app_timer;

void clear_recevied_message(void *data) {
	layer_remove_from_parent(data);
	window_stack_pop(true);
}

void out_sent_handler(DictionaryIterator *sent, void *context) {
  // outgoing message was delivered -- do nothing
  sent_msg = true;
}
void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  // outgoing message failed
  text_layer_set_text(main_text_layer, "Watch no longer connected to the server. Exiting program");
  // deinit();
}
void in_received_handler(DictionaryIterator *received, void *context) {
  // incoming message received
  // looks for key #0 in the incoming message
  int key = 2;
  temp_window = window_create();
  Layer *window_layer = window_get_root_layer(temp_window);
  GRect bounds = layer_get_bounds(window_layer);
  msg_received_text_layer = text_layer_create(GRect(5, 60, bounds.size.w - 15, bounds.size.h-60));
  TextLayer *degree_layer = text_layer_create(GRect(100, 55, 20, 20));
  
  text_layer_set_text_alignment(msg_received_text_layer, GTextAlignmentRight);
  text_layer_set_text_alignment(degree_layer, GTextAlignmentRight);
  text_layer_set_background_color(degree_layer, GColorClear);
  text_layer_set_font(degree_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  // text_layer_set_background_color(msg_received_text_layer, GColorClear);
  text_layer_set_font(msg_received_text_layer, fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD));


  Tuple *text_tuple = dict_find(received, key);
  if (text_tuple) {
    if (text_tuple->value) {
	    // put it in this global variable
	    strcpy(msg, text_tuple->value->cstring);
    }
    else {
    	strcpy(msg, "no value!"); 
    } 
    text_layer_set_text(msg_received_text_layer, msg);
    text_layer_set_text(degree_layer, "o");
    layer_add_child(window_layer, text_layer_get_layer(msg_received_text_layer));
    layer_add_child(window_layer, text_layer_get_layer(degree_layer));

  	window_stack_push(temp_window, true);
  	app_timer = app_timer_register(4000, clear_recevied_message, msg_received_text_layer);

  } 
  else {
  	//A message was sent, but none were received. 
  	if(sent_msg) {
  		text_layer_set_text(main_text_layer, "Watch no longer connected to the server. Exiting program");
  		app_timer = app_timer_register(4000, (AppTimerCallback) deinit, NULL);
  		
  	}
  	

  }

  sent_msg = false;



}

void in_dropped_handler(AppMessageResult reason, void *context) {
  // incoming message dropped
  text_layer_set_text(main_text_layer, "Error in!");
  // deinit();
}

/* This is called when the up button is clicked */
void main_up_click_handler(ClickRecognizerRef recognizer, void *context) {
	security_window = window_create();
  window_set_window_handlers(security_window, (WindowHandlers) { 
  		.load = security_window_load, 
  		.unload = security_window_unload, 
  });
	window_set_click_config_provider(security_window, security_config_provider);
	window_stack_push(security_window, true);
}


/* This is called when the down button is clicked */
void main_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  temperature_window = window_create();
  window_set_window_handlers(temperature_window, (WindowHandlers) { 
  		.load = temperature_window_load, 
  		.unload = temperature_window_unload, 
  });
	window_set_click_config_provider(temperature_window, temperature_config_provider);
	window_stack_push(temperature_window, true);
}

void tap_handler(AccelAxisType axis, int32_t direction) {
	vibes_long_pulse();
	DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  int key = 0;
  // send the message "hello?" to the phone, using key #0
  Tuplet value = TupletCString(key, "change");
  dict_write_tuplet(iter, &value);
  app_message_outbox_send();

}


/* this registers the appropriate function to the appropriate button */
void main_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, NULL);
  window_single_click_subscribe(BUTTON_ID_UP, main_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, main_down_click_handler);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  security_button_text = text_layer_create(GRect(0,10,bounds.size.w - 5, 20));
  temperature_button_text = text_layer_create(GRect(0,bounds.size.h - 30,bounds.size.w - 5, 20));
  main_text_layer = text_layer_create(GRect(0,0, bounds.size.w, bounds.size.h));
  text_layer_set_text_alignment(security_button_text, GTextAlignmentRight);
  text_layer_set_text_alignment(temperature_button_text, GTextAlignmentRight);
  text_layer_set_text_alignment(main_text_layer, GTextAlignmentCenter);
  text_layer_set_background_color(security_button_text, GColorClear);
  text_layer_set_background_color(temperature_button_text, GColorClear);
  text_layer_set_font(security_button_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_font(temperature_button_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_font(main_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text(security_button_text, "Security System ->");
  text_layer_set_text(temperature_button_text, "Arduino Temp ->");
  layer_add_child(window_layer, text_layer_get_layer(security_button_text));
  layer_add_child(window_layer, text_layer_get_layer(temperature_button_text));
}

static void main_window_unload(Window *window) {
  text_layer_destroy(temperature_button_text);
  text_layer_destroy(security_button_text);
  text_layer_destroy(main_text_layer);
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) { 
  		.load = main_window_load, 
  		.unload = main_window_unload, 
  });
    
  // for registering AppMessage handlers
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_sent(out_sent_handler);
  app_message_register_outbox_failed(out_failed_handler);
  const uint32_t inbound_size = 200;
  const uint32_t outbound_size = 200;
  app_message_open(inbound_size, outbound_size);

    // need this for adding the listener
  window_set_click_config_provider(window, main_config_provider);
  accel_tap_service_subscribe(tap_handler);
  
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
  window_destroy(temperature_window);
  window_stack_pop_all(true);
  // window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}