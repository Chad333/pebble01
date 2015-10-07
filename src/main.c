#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static GFont s_time_font;
static GFont s_batt_font;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static BitmapLayer *s_background_layer, *s_bt_icon_layer;
static GBitmap *s_background_bitmap, *s_bt_icon_bitmap;
static BitmapLayer *s_background_layer, *s_batt_icon_layer;
static GBitmap *s_background_bitmap, *s_batt_icon_bitmap;
static TextLayer *s_batt_layer;
static AppTimer *s_timer;

static void handle_battery(BatteryChargeState charge_state) {
  static char buffer[] = "00";
  snprintf(buffer, sizeof(buffer), "%d", charge_state.charge_percent);
  text_layer_set_text(s_batt_layer, buffer);
}

static void timer_callback(void *data) {
  layer_set_hidden(bitmap_layer_get_layer(s_batt_icon_layer),true);
  text_layer_set_text(s_batt_layer,"");
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
  layer_set_hidden(bitmap_layer_get_layer(s_batt_icon_layer),false);
  handle_battery(battery_state_service_peek());
  s_timer = app_timer_register(1700, timer_callback, NULL);
}

static void bluetooth_callback(bool connected) {
  // Show icon if disconnected
  layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected);

  if(!connected) {
    vibes_double_pulse();
  }
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }
  text_layer_set_text(s_time_layer, buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window){
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND3);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  
  s_time_layer = text_layer_create(GRect(0,59,139,50));
  text_layer_set_background_color(s_time_layer,GColorClear);
  text_layer_set_text_color(s_time_layer,GColorBlack);
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_CALVIN_40));
  s_batt_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_CALVIN_18));
  
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_ICON);
  s_batt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BUBBLE2);

  s_bt_icon_layer = bitmap_layer_create(GRect(107, 1, 35, 65));
  bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
  s_batt_icon_layer = bitmap_layer_create(GRect(107, 110, 35, 65));
  bitmap_layer_set_bitmap(s_batt_icon_layer, s_batt_icon_bitmap);
  s_batt_layer = text_layer_create(GRect(108,137,40,60));
  text_layer_set_background_color(s_batt_layer,GColorClear);
  text_layer_set_text_color(s_batt_layer,GColorBlack);
  text_layer_set_font(s_batt_layer, s_batt_font);
  text_layer_set_text_alignment(s_batt_layer, GTextAlignmentCenter);

  layer_add_child(window_get_root_layer(window),text_layer_get_layer(s_time_layer));
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_icon_layer));
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_batt_icon_layer));
  layer_set_hidden(bitmap_layer_get_layer(s_batt_icon_layer),true);
  layer_add_child(window_get_root_layer(window),text_layer_get_layer(s_batt_layer));
  bluetooth_callback(bluetooth_connection_service_peek());
  battery_state_service_subscribe(handle_battery);
  
}
static void main_window_unload(Window *window){
  gbitmap_destroy(s_background_bitmap);
  bitmap_layer_destroy(s_background_layer);
  text_layer_destroy(s_time_layer);
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_batt_font);
  gbitmap_destroy(s_bt_icon_bitmap);
  bitmap_layer_destroy(s_bt_icon_layer);
  gbitmap_destroy(s_batt_icon_bitmap);
  bitmap_layer_destroy(s_batt_icon_layer);
}
static void init() {
  s_main_window = window_create();
  
  window_set_window_handlers(s_main_window,(WindowHandlers){
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  bluetooth_connection_service_subscribe(bluetooth_callback);
  accel_tap_service_subscribe(tap_handler);
  
  window_stack_push(s_main_window, true);
  update_time();
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}