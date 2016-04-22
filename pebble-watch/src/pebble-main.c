#include <pebble.h>
#include "pebble-main.h"
#include "pebble-arduino-temperature.h"
#include "pebble-arduino-security.h"

/* 
 * * * * * * * * * * *
 * GLOBAL VARIABLES  * 
 * * * * * * * * * * *
 */
Window *window, *temp_window, *temperature_window, *security_window;
TextLayer *main_text_layer, *msg_received_text_layer, *security_button_text, 
	*temperature_button_text, *toggle_button_text, *degree_layer;
char msg[200];
bool sent_msg = false;
AppTimer *app_timer;

/* 
 * * * * * * *
 * FUNCTIONS *
 * * * * * * *
 */

/* Remove the temporary message window from stack */
void clear_recevied_message(void *data) {
	window_stack_remove(temp_window, true);
}
/* Exit program by removing all windows from stack */
void exit_program(void *data) {
	window_stack_pop_all(true);
}

/* Display Exit message and remove windows above the base window */
void exit_message() {
	Layer *main_window_layer = window_get_root_layer(window);
	text_layer_set_text(main_text_layer, "Watch no longer connected to the server. Exiting program");
	layer_add_child(main_window_layer, text_layer_get_layer(main_text_layer));

	//Cancel app_timer if it is already in use
	if(app_timer) {
  	app_timer_cancel(app_timer);
  }

	if(temperature_window) {
		window_stack_remove(temperature_window, true);
	}
	if(security_window) {
		window_stack_remove(security_window, true);
	}
	if(temp_window) {
		window_stack_remove(temp_window, true);
	}
	//Wait 4 seconds before actually shutting down the program
	app_timer = app_timer_register(4000, (AppTimerCallback) exit_program, NULL);
}

/* Record that a message was successfully sent*/
void out_sent_handler(DictionaryIterator *sent, void *context) {
  // outgoing message was delivered
  sent_msg = true;
}

/* If outgoing message did not reach intended recipient, then shut down program */
void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  // outgoing message failed
	exit_message();
}

/* Handles incoming messages to the pebble watch */
void in_received_handler(DictionaryIterator *received, void *context) {

  //Creates a temporary window to display a message from sender
  temp_window = window_create();
  window_set_window_handlers(temp_window, (WindowHandlers) { 
  		.load = temp_window_load, 
  		.unload = temp_window_unload, 
  });

  // looks for key #2 in the incoming message
  int server_key = 2;
  Tuple *server_tuple = dict_find(received, server_key);

  if (server_tuple) {
  	sent_msg = false;
  	Layer *window_layer = window_get_root_layer(temp_window);
  	window_stack_push(temp_window, true);

    if (server_tuple->value) {
	    // put it in this global variable
	    strcpy(msg, server_tuple->value->cstring);
    }
    else {
    	strcpy(msg, "no value!"); 
    } 

    if(strncmp(msg, "Alarm", 5) == 0) {
    	text_layer_set_text(msg_received_text_layer, msg);
    }
    //Lost connection with Arduino 
    else if(strncmp(msg, "Lost", 4) == 0) {
    	window_stack_remove(temp_window, true);
    	exit_message();
    	return;
    } 
    //Confirmed that Arduino temp display was toggled
    else if (strncmp(msg, "Display", 7) == 0) {
    	window_stack_remove(temp_window, true);
    	return;
    } else {
    	text_layer_set_text(msg_received_text_layer, msg);
    	text_layer_set_text(degree_layer, "o");
    	layer_add_child(window_layer, text_layer_get_layer(degree_layer));
    }
    
    layer_add_child(window_layer, text_layer_get_layer(msg_received_text_layer));
  	
  	app_timer = app_timer_register(4000, clear_recevied_message, NULL);

  } 
  else {
  	//A message was sent, but none were received. Lost connection with server. 
  	if(sent_msg) {
  		exit_message();
  		
  	}

  }


}

void in_dropped_handler(AppMessageResult reason, void *context) {
  // incoming message dropped
  text_layer_set_text(main_text_layer, "Error in!");
  deinit();
}

/* This is called when the up button is clicked */
void main_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  printf("main up button: %d\n", heap_bytes_used());
	
	security_window = window_create();
  window_set_window_handlers(security_window, (WindowHandlers) { 
  		.load = security_window_load, 
  		.unload = security_window_unload, 
  });
	window_set_click_config_provider(security_window, security_config_provider);
	window_stack_push(security_window, true);
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
  int key = 0;
  // send the message "hello?" to the phone, using key #0
  Tuplet value = TupletCString(key, "off");
  dict_write_tuplet(iter, &value);
  app_message_outbox_send();
  printf("main up button: %d\n", heap_bytes_used());

}

/* This is called when the select button is clicked */
void main_select_click_handler(ClickRecognizerRef recognizer, void *context) {
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
  int key = 0;
  // send the message "hello?" to the phone, using key #0
  Tuplet value = TupletCString(key, "latest");
  dict_write_tuplet(iter, &value);
  app_message_outbox_send();

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
  window_single_click_subscribe(BUTTON_ID_SELECT, main_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, main_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, main_down_click_handler);
}

static void temp_window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  msg_received_text_layer = text_layer_create(GRect(5, 60, bounds.size.w - 15, bounds.size.h-60));
  text_layer_set_text_alignment(msg_received_text_layer, GTextAlignmentRight);
  text_layer_set_font(msg_received_text_layer, fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD));

  degree_layer = text_layer_create(GRect(100, 55, 20, 20));
  text_layer_set_text_alignment(degree_layer, GTextAlignmentRight);
  text_layer_set_background_color(degree_layer, GColorClear);
  text_layer_set_font(degree_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));

}

static void temp_window_unload(Window *window) {
  text_layer_destroy(msg_received_text_layer);
  text_layer_destroy(degree_layer);
}

static void main_window_load(Window *window) {
	printf("main window load: %d\n", heap_bytes_used());
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  security_button_text = text_layer_create(GRect(0,10,bounds.size.w - 5, 20));
  temperature_button_text = text_layer_create(GRect(0,bounds.size.h - 30,bounds.size.w - 5, 20));
  toggle_button_text = text_layer_create(GRect(0,bounds.size.h/2-10,bounds.size.w - 5, 20));
  main_text_layer = text_layer_create(GRect(0,0, bounds.size.w, bounds.size.h));
  text_layer_set_text_alignment(security_button_text, GTextAlignmentRight);
  text_layer_set_text_alignment(temperature_button_text, GTextAlignmentRight);
  text_layer_set_text_alignment(toggle_button_text, GTextAlignmentRight);
  text_layer_set_text_alignment(main_text_layer, GTextAlignmentCenter);
  text_layer_set_background_color(security_button_text, GColorClear);
  text_layer_set_background_color(temperature_button_text, GColorClear);
  text_layer_set_background_color(toggle_button_text, GColorClear);
  text_layer_set_font(security_button_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_font(temperature_button_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_font(main_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_font(toggle_button_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text(security_button_text, "Security System ->");
  text_layer_set_text(temperature_button_text, "High / Avg / Low ->");
  text_layer_set_text(toggle_button_text, "Latest Temp ->");
  layer_add_child(window_layer, text_layer_get_layer(security_button_text));
  layer_add_child(window_layer, text_layer_get_layer(temperature_button_text));
  layer_add_child(window_layer, text_layer_get_layer(toggle_button_text));
  printf("main window load: %d\n", heap_bytes_used());

}

static void main_window_unload(Window *window) {
  printf("main window unload: %d\n", heap_bytes_used());
  text_layer_destroy(temperature_button_text);
  text_layer_destroy(security_button_text);
  text_layer_destroy(main_text_layer);
  text_layer_destroy(toggle_button_text);
  printf("main window unload: %d\n", heap_bytes_used());

}

static void init(void) {
	printf("init: %d\n", heap_bytes_used());
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
  printf("init: %d\n", heap_bytes_used());
}

/* Destroys all windows as program closes */
static void deinit(void) {
		printf("%d", heap_bytes_used());
  	window_destroy(window);
  	window_destroy(temperature_window);
  	window_destroy(temp_window);
  	window_destroy(security_window);
  	app_message_deregister_callbacks();
		printf("%d", heap_bytes_used());
  
}

/*
 * * * * * *
 * M A I N *
 * * * * * *
*/

int main(void) {
  init();
  app_event_loop();
  deinit();
}