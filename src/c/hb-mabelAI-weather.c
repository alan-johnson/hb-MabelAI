#include "hb-mabelAI.h"

// ---- Private state ----
static char            s_weather_buffer[512];
static TextLayer      *s_weather_label;
static ScrollLayer    *s_scroll_layer;
static TextLayer      *s_weather_layer;
static ActionBarLayer *s_weather_action_bar;

// ---- Private: push buffer contents into the layers ----

static void refresh_weather_layers(void) {
  if (!s_weather_layer || !s_scroll_layer) return;
  text_layer_set_text(s_weather_layer, s_weather_buffer);
  GRect bounds = layer_get_bounds(scroll_layer_get_layer(s_scroll_layer));
  text_layer_set_size(s_weather_layer, GSize(bounds.size.w, 8000));
  GSize text_size = text_layer_get_content_size(s_weather_layer);
  text_layer_set_size(s_weather_layer, GSize(bounds.size.w, text_size.h + 10));
  scroll_layer_set_content_size(s_scroll_layer, GSize(bounds.size.w, text_size.h + 20));
  scroll_layer_set_content_offset(s_scroll_layer, GPointZero, false);
}

// ---- Public: copy new weather text into the buffer, then refresh ----

void show_weather(const char *text) {
  snprintf(s_weather_buffer, sizeof(s_weather_buffer), "%s", text);
  refresh_weather_layers();
}

// ---- Click handlers ----

static void weather_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  GPoint offset = scroll_layer_get_content_offset(s_scroll_layer);
#ifdef PBL_ROUND
  scroll_layer_set_content_offset(s_scroll_layer, GPoint(0, offset.y + 30), false);
#else
  scroll_layer_set_content_offset(s_scroll_layer, GPoint(0, offset.y + 30), true);
#endif
}

static void weather_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  GPoint offset = scroll_layer_get_content_offset(s_scroll_layer);
#ifdef PBL_ROUND
  scroll_layer_set_content_offset(s_scroll_layer, GPoint(0, offset.y - 30), false);
#else
  scroll_layer_set_content_offset(s_scroll_layer, GPoint(0, offset.y - 30), true);
#endif
}

static void weather_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP,   weather_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, weather_down_click_handler);
}

// ---- Window lifecycle ----

static void weather_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect  bounds       = layer_get_bounds(window_layer);

  s_weather_action_bar = action_bar_layer_create();
  action_bar_layer_set_click_config_provider(s_weather_action_bar, weather_click_config_provider);
  action_bar_layer_set_icon(s_weather_action_bar, BUTTON_ID_UP,   s_icon_up);
  action_bar_layer_set_icon(s_weather_action_bar, BUTTON_ID_DOWN, s_icon_down);

  int content_w = bounds.size.w - ACTION_BAR_WIDTH;
  int content_h = bounds.size.h - STATUS_BAR_LAYER_HEIGHT;
#ifdef PBL_PLATFORM_GABBRO
  int h_pad = 66, right_pad = 38;
#elif defined(PBL_ROUND)
  int h_pad = 36, right_pad = 2;
#else
  int h_pad = 2,  right_pad = 2;
#endif
  int text_w  = content_w - h_pad - right_pad;
  int label_h = 18;
  int scroll_h = content_h - label_h - 6;

  s_weather_label = text_layer_create(
      GRect(h_pad, STATUS_BAR_LAYER_HEIGHT + 2, text_w, label_h));
  text_layer_set_font(s_weather_label, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_color(s_weather_label, COLOR_TEXT_LABEL);
  text_layer_set_background_color(s_weather_label, GColorClear);
  text_layer_set_text(s_weather_label, STR_SECTION_WEATHER);
  layer_add_child(window_layer, text_layer_get_layer(s_weather_label));

  s_scroll_layer = scroll_layer_create(
      GRect(h_pad, STATUS_BAR_LAYER_HEIGHT + label_h + 4, text_w, scroll_h));
  layer_add_child(window_layer, scroll_layer_get_layer(s_scroll_layer));

  s_weather_layer = text_layer_create(GRect(0, 0, text_w, 2000));
  text_layer_set_font(s_weather_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_color(s_weather_layer, COLOR_TEXT);
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_overflow_mode(s_weather_layer, GTextOverflowModeWordWrap);
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_weather_layer));

  refresh_weather_layers();

  s_weather_status_bar_layer = create_status_bar(window_layer, bounds.size.w);
  action_bar_layer_add_to_window(s_weather_action_bar, window);
}

static void weather_window_unload(Window *window) {
  layer_destroy(s_weather_status_bar_layer);
  s_weather_status_bar_layer = NULL;
  text_layer_destroy(s_weather_label);
  s_weather_label = NULL;
  text_layer_destroy(s_weather_layer);
  s_weather_layer = NULL;
  scroll_layer_destroy(s_scroll_layer);
  s_scroll_layer = NULL;
  action_bar_layer_destroy(s_weather_action_bar);
  s_weather_action_bar = NULL;
}

// ---- Public: return handlers for use in init ----

WindowHandlers weather_window_get_handlers(void) {
  return (WindowHandlers){
    .load   = weather_window_load,
    .unload = weather_window_unload,
  };
}
