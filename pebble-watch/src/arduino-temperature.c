#include <pebble.h>
static Window *window, *temp_window;
static TextLayer *main_text_layer, *msg_received_text_layer;
static char msg[200];
AppTimer *app_timer;

void clear_recevied_message(void *data) {
	layer_remove_from_parent(data);
	window_stack_pop(true);
}

void out_sent_handler(DictionaryIterator *sent, void *context) {
  // outgoing message was delivered -- do nothing
}
void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  // outgoing message failed
  text_layer_set_text(main_text_layer, "Error out!");
}
void in_received_handler(DictionaryIterator *received, void *context) {
  // incoming message received
  // looks for key #0 in the incoming message
  int key = 2;
  temp_window = window_create();
  Layer *window_layer = window_get_root_layer(temp_window);
  GRect bounds = layer_get_bounds(window_layer);
  msg_received_text_layer = text_layer_create(GRect(5,60,bounds.size.w - 5, bounds.size.h-60));
  text_layer_set_text_alignment(msg_received_text_layer, GTextAlignmentLeft);
  // text_layer_set_background_color(msg_received_text_layer, GColorClear);
  text_layer_set_font(msg_received_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));


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
    layer_add_child(window_layer, text_layer_get_layer(msg_received_text_layer));
  	window_stack_push(temp_window, true);
  	app_timer = app_timer_register(4000, clear_recevied_message, msg_received_text_layer);
  }



}

void in_dropped_handler(AppMessageResult reason, void *context) {
  // incoming message dropped
  text_layer_set_text(main_text_layer, "Error in!");
}

/* This is called when the up button is clicked */
void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  int key = 0;
  // send the message "hello?" to the phone, using key #0
  Tuplet value = TupletCString(key, "high");
  dict_write_tuplet(iter, &value);
  app_message_outbox_send();
}

/* This is called when the select button is clicked */
void select_click_handler(ClickRecognizerRef recognizer, void *context) {

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  int key = 0;
  // send the message "hello?" to the phone, using key #0
  Tuplet value = TupletCString(key, "average");
  dict_write_tuplet(iter, &value);
  app_message_outbox_send();
}

/* This is called when the down button is clicked */
void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  int key = 0;
  // send the message "hello?" to the phone, using key #0
  Tuplet value = TupletCString(key, "low");
  dict_write_tuplet(iter, &value);
  app_message_outbox_send();
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
void config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  main_text_layer = text_layer_create(GRect(5,25,bounds.size.w - 5, bounds.size.h-25));
  text_layer_set_text_alignment(main_text_layer, GTextAlignmentLeft);
  text_layer_set_background_color(main_text_layer, GColorClear);
  text_layer_set_font(main_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text(main_text_layer, "Arduino Temp\nUp: High\nSelect: Avg\nDown: Low\nShake: Change");
  layer_add_child(window_layer, text_layer_get_layer(main_text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(main_text_layer);
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) { 
  		.load = window_load, 
  		.unload = window_unload, 
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
  window_set_click_config_provider(window, config_provider);
  accel_tap_service_subscribe(tap_handler);
  
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}