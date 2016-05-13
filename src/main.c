#include <pebble.h>

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1

typedef struct {
  char temp_buffer[20];
} Temps;

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_weather_layer;
static Layer *some_layer;

static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static GFont s_time_font;
static GFont s_weather_font;

static void some_update_proc(Layer *layer, GContext *ctx) {
    Temps* dataPointer = layer_get_data(layer);
//     char buffer[20] = dataPointer->temp_buffer;
    GRect bounds = layer_get_bounds(layer);

      GRect frame = grect_inset(bounds, GEdgeInsets(10));

    APP_LOG(APP_LOG_LEVEL_ERROR, "arr: %s", dataPointer->temp_buffer);

    int now = dataPointer->temp_buffer[0];
    int max = dataPointer->temp_buffer[1];
    int min = dataPointer->temp_buffer[2];

    //get layer bounds, width and height
    double scale = (168.0 / 3.0) / (max - min); //pebble height top third
    graphics_context_set_fill_color(ctx, GColorWhite);
  
    for (int i = 1; i<8; i++) {
      int high = dataPointer->temp_buffer[i*2+1];
      int low = dataPointer->temp_buffer[i*2+2];
      APP_LOG(APP_LOG_LEVEL_ERROR, "high: %d, low: %d", high, low);
      GRect rect_bounds = GRect(16*i, (max - high)*scale , 14, (high - low)*scale);
      graphics_draw_rect(ctx, rect_bounds);
      int corner_radius = 2;
      graphics_fill_rect(ctx, rect_bounds, corner_radius, GCornersAll);

    }
  
    int x10 = ((min + 9) / 10) * 10;
      graphics_context_set_fill_color(ctx, GColorBlack);

  for (int j=x10; j < max; j+= 10 ) {
    float dy = (max - j) * scale;
    GPoint start = GPoint(0,dy);
    GPoint end = GPoint(144,dy);
    graphics_draw_line(ctx, start, end);

  }
  
    //Current temp
graphics_context_set_stroke_color(ctx, GColorRed);
GPoint startNow = GPoint(0, (max-now)*scale);
GPoint endNow = GPoint(20, (max-now)*scale);

char buffer[8];
  snprintf(buffer, 8, "%d°", now);
          APP_LOG(APP_LOG_LEVEL_ERROR, "now: %s", buffer);


  
  // Load the font
GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_14);


// Determine a reduced bounding box
GRect txtBounds = GRect(0,0,32, 56);

// Calculate the size of the text to be drawn, with restricted space
GSize text_size = graphics_text_layout_get_content_size(buffer, font, txtBounds, GTextOverflowModeWordWrap, GTextAlignmentCenter);

  
// Draw the temp
graphics_draw_line(ctx, startNow, endNow);
graphics_draw_text(ctx, buffer, font, txtBounds, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

  
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[20];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];

  // Read tuples for data
  Tuple *temp_tuple = dict_find(iterator, KEY_TEMPERATURE);
//   Tuple *conditions_tuple = dict_find(iterator, KEY_CONDITIONS);

  // If all data is available, use it  //   if(temp_tuple && conditions_tuple) {
  if(temp_tuple) {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%s", temp_tuple->value->cstring);
//     snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);

    // Assemble full string and display
    //snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
    text_layer_set_text(s_weather_layer, temperature_buffer);
    
    int max = temperature_buffer[0];
    int min = temperature_buffer[1];
    double scale = (168.0 / 3.0) / (max - min); //pebble height top third
        APP_LOG(APP_LOG_LEVEL_ERROR, "scale is %d", (int)(scale*100));

    for (int i = 1; i<5; i++) {
      int high = temperature_buffer[i*2];
      int low = temperature_buffer[i*2+1];
      APP_LOG(APP_LOG_LEVEL_ERROR, "high: %d, low: %d", high, low);
    }    

    // Give tempereature display layer the temperature data
//     char (*dataPointer)[20] = layer_get_data(some_layer);
    Temps* dataPointer = layer_get_data(some_layer);
    
   // dataPointer->temp_buffer = temperature_buffer;
    strcpy(dataPointer->temp_buffer, temperature_buffer);
    //dataPointer = &temperature_buffer;
    APP_LOG(APP_LOG_LEVEL_ERROR, "data pointer: %p", dataPointer);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();

  // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
}


static void main_window_load(Window *window) {  
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
GRect layer_bounds = bounds;
// layer_bounds.size.h /= 2;
  APP_LOG(APP_LOG_LEVEL_INFO, "Hieght: %d and width: %d",layer_bounds.size.h, layer_bounds.size.w );


  // Create GBitmap
//   s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);

  // Create BitmapLayer to display the GBitmap
//   s_background_layer = bitmap_layer_create(bounds);

  // Set the bitmap onto the layer and add to the window
//   bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
//   layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));

  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(90, 52), bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_48));

  // Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  // Create temperature Layer
  s_weather_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(140, 120), bounds.size.w, 25));

  // Style the text
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, GColorWhite);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_layer, "Loading...");

  // Create second custom font, apply it and add to Window
  s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_20));
  text_layer_set_font(s_weather_layer, s_weather_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));
  
     // Create a Layer and set the update_proc
  some_layer = layer_create_with_data(GRect(PBL_IF_ROUND_ELSE(18, 0),PBL_IF_ROUND_ELSE(90-56, 0), 144, 168), sizeof(Temps));
  layer_set_update_proc(some_layer, some_update_proc);
  layer_add_child(window_layer, some_layer);

  
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);

  // Unload GFont
  fonts_unload_custom_font(s_time_font);

  // Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);

  // Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);

  // Destroy weather elements
  text_layer_destroy(s_weather_layer);
  fonts_unload_custom_font(s_weather_font);
}


static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set the background color
  window_set_background_color(s_main_window, GColorBlack);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  update_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}