#include "hb-mabelAI.h"

// ---- Private layer state ----
static StatusBarLayer *s_status_bar;
static ScrollLayer    *s_scroll_layer;
static TextLayer      *s_response_layer;
static ActionBarLayer *s_result_action_bar;

// ---- Private: push buffer contents into the layers ----

static void refresh_result_layers(void) {
  if (!s_response_layer || !s_scroll_layer) return;
  text_layer_set_text(s_response_layer, s_response_buffer);
  GRect bounds    = layer_get_bounds(scroll_layer_get_layer(s_scroll_layer));
  GSize text_size = text_layer_get_content_size(s_response_layer);
  text_layer_set_size(s_response_layer, GSize(bounds.size.w, text_size.h + 10));
  scroll_layer_set_content_size(s_scroll_layer, GSize(bounds.size.w, text_size.h + 20));
  scroll_layer_set_content_offset(s_scroll_layer, GPointZero, false);
}

// ---- Public: copy new text into the buffer, then refresh layers ----

void show_response(const char *text) {
  snprintf(s_response_buffer, sizeof(s_response_buffer), "%s", text);
  refresh_result_layers();
}

// ---- Click handlers ----

static void result_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  GPoint offset = scroll_layer_get_content_offset(s_scroll_layer);
  scroll_layer_set_content_offset(s_scroll_layer, GPoint(0, offset.y + 30), true);
}

static void result_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  GPoint offset = scroll_layer_get_content_offset(s_scroll_layer);
  scroll_layer_set_content_offset(s_scroll_layer, GPoint(0, offset.y - 30), true);
}

static void result_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP,   result_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, result_down_click_handler);
}

// ---- Window lifecycle ----

static void result_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect  bounds       = layer_get_bounds(window_layer);

  // Status bar — clock at the top
  s_status_bar = status_bar_layer_create();
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));

  // Action bar — up/down scroll; BACK button pops automatically
  s_result_action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(s_result_action_bar, window);
  action_bar_layer_set_click_config_provider(s_result_action_bar, result_click_config_provider);
  action_bar_layer_set_icon(s_result_action_bar, BUTTON_ID_UP,   s_icon_up);
  action_bar_layer_set_icon(s_result_action_bar, BUTTON_ID_DOWN, s_icon_down);

  int content_w = bounds.size.w - ACTION_BAR_WIDTH;

  // Scroll layer (below status bar)
  s_scroll_layer = scroll_layer_create(GRect(2, STATUS_BAR_LAYER_HEIGHT + 2, content_w - 4, bounds.size.h - STATUS_BAR_LAYER_HEIGHT - 4));
  layer_add_child(window_layer, scroll_layer_get_layer(s_scroll_layer));

  // Response text inside scroll layer
  s_response_layer = text_layer_create(GRect(0, 0, content_w - 4, 2000));
  text_layer_set_font(s_response_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_color(s_response_layer, GColorBlack);
  text_layer_set_background_color(s_response_layer, GColorClear);
  text_layer_set_overflow_mode(s_response_layer, GTextOverflowModeWordWrap);
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_response_layer));

  // Populate layers from the shared buffer (already filled before this window was pushed)
  refresh_result_layers();
}

static void result_window_unload(Window *window) {
  status_bar_layer_destroy(s_status_bar);
  text_layer_destroy(s_response_layer);
  s_response_layer = NULL;
  scroll_layer_destroy(s_scroll_layer);
  s_scroll_layer = NULL;
  action_bar_layer_destroy(s_result_action_bar);
}

// ---- Public: return handlers for use in init ----

WindowHandlers result_window_get_handlers(void) {
  return (WindowHandlers){
    .load   = result_window_load,
    .unload = result_window_unload,
  };
}
