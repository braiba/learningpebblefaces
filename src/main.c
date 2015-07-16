#include <pebble.h>
  
static Window *s_main_window;
static GFont s_mono_font;
static char s_hour_buffer[] = "00";
static char s_minute_buffer[] = "00";
static char s_second_buffer[] = "00";
static Layer *s_hour_bar_layer;
static Layer *s_minute_bar_layer;
static Layer *s_second_bar_layer;
static TextLayer *s_hour_layer;
static TextLayer *s_minute_layer;
static TextLayer *s_second_layer;
static int s_hour_value;
static int s_minute_value;
static int s_second_value;

static void update_unit(TextLayer *text_layer, Layer *bar_layer, struct tm *tick_time, char *buffer, const char *format) {
  strftime(buffer, sizeof("00"), format, tick_time);
  text_layer_set_text(text_layer, buffer); 
  
  layer_mark_dirty(bar_layer);
}

static void update_time(struct tm *tick_time, TimeUnits units_changed) {
  if (units_changed >= HOUR_UNIT) {
    if (clock_is_24h_style() == true) {
      update_unit(s_hour_layer, s_hour_bar_layer, tick_time, s_hour_buffer, "%H");
    } else {
      update_unit(s_hour_layer, s_hour_bar_layer, tick_time, s_hour_buffer, "%I");
    }
    s_hour_value = tick_time->tm_hour;
  }
  
  if (units_changed >= MINUTE_UNIT) {
    s_minute_value = tick_time->tm_min;
    update_unit(s_minute_layer, s_minute_bar_layer, tick_time, s_minute_buffer, "%M");
  }
  
  if (units_changed >= SECOND_UNIT) {
    s_second_value = tick_time->tm_sec;
    update_unit(s_second_layer, s_minute_bar_layer, tick_time, s_second_buffer, "%S");
  }
}

static void init_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  
  // Make sure the time is displayed from the start
  update_time(tick_time, YEAR_UNIT);
}

static void init_text_layer(Window *window, TextLayer *text_layer) {
  text_layer_set_background_color(text_layer, GColorClear);
  text_layer_set_text_color(text_layer, GColorWhite);
  text_layer_set_text(text_layer, "00");
  
  text_layer_set_font(text_layer, s_mono_font);
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer));
}

static void bar_layer_update_callback(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorBlack);
  if (layer == s_hour_bar_layer) {
    #ifdef PBL_COLOR
      graphics_context_set_fill_color(ctx, GColorRed);
    #endif
    graphics_fill_rect(ctx, GRect(0, 0, (80 * s_hour_value) / 24, 52), 0, GCornerNone);
  } else if (layer == s_minute_bar_layer) {
    #ifdef PBL_COLOR
      graphics_context_set_fill_color(ctx, GColorGreen);
    #endif
    graphics_fill_rect(ctx, GRect(0, 0, (80 * s_minute_value) / 60, 52), 0, GCornerNone);
  } else if (layer == s_second_bar_layer) {
    #ifdef PBL_COLOR
      graphics_context_set_fill_color(ctx, GColorBlue);
    #endif
    graphics_fill_rect(ctx, GRect(0, 0, (80 * s_second_value) / 60, 52), 0, GCornerNone);
  }
}

static void init_bar_layer(Window *window, Layer *layer) {
  layer_add_child(window_get_root_layer(window), layer);
  layer_set_update_proc(layer, bar_layer_update_callback);
}

static void main_window_load(Window *window) {
  s_mono_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_UBUNTU_MONO_42));
  
  // Vertical: 3 + 52 + 3 + 52 + 3 + 52 + 3 = 168
  // Horizontal: 3 + 52 + 3 + 83 + 3 = 144
  s_hour_layer = text_layer_create(GRect(3, 3, 52, 52));
  init_text_layer(window, s_hour_layer);
  s_hour_bar_layer = layer_create(GRect(60, 3, 83, 52));
  init_bar_layer(window, s_hour_bar_layer);
  
  s_minute_layer = text_layer_create(GRect(3, 58, 52, 52));
  init_text_layer(window, s_minute_layer);
  s_minute_bar_layer = layer_create(GRect(60, 58, 83, 52));
  init_bar_layer(window, s_minute_bar_layer);
  
  s_second_layer = text_layer_create(GRect(3, 113, 52, 52));
  init_text_layer(window, s_second_layer);
  s_second_bar_layer = layer_create(GRect(60, 113, 83, 52));
  init_bar_layer(window, s_second_bar_layer);
    
  init_time();
}

static void main_window_unload(Window *window) {
  // Destroy TextLayers
  text_layer_destroy(s_hour_layer);
  text_layer_destroy(s_minute_layer);
  text_layer_destroy(s_second_layer);
  
  layer_destroy(s_hour_bar_layer);
  layer_destroy(s_minute_bar_layer);
  layer_destroy(s_second_bar_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time, units_changed);
}
  
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
    
  window_set_background_color(s_main_window, GColorBlack);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);    
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
