#include "hb-mabelAI.h"

// ---- Private layer state ----
static TextLayer      *s_request_label;   // "request" section header
static TextLayer      *s_request_text;    // query content (fixed, truncated)
static TextLayer      *s_divider_layer;   // 1px horizontal separator
static TextLayer      *s_mabel_label;     // "mabel" section header
static ScrollLayer    *s_scroll_layer;
static TextLayer      *s_response_layer;  // response content inside scroll layer
static ActionBarLayer *s_result_action_bar;

// ---- Private: push buffer contents into the layers ----

static void refresh_result_layers(void) {
  if (!s_response_layer || !s_scroll_layer) return;
  if (s_request_text) {
    text_layer_set_text(s_request_text, s_query_buffer);
  }
  text_layer_set_text(s_response_layer, s_response_buffer);
  GRect bounds = layer_get_bounds(scroll_layer_get_layer(s_scroll_layer));
  // Reset to unconstrained height before measuring so word-wrap height is accurate
  text_layer_set_size(s_response_layer, GSize(bounds.size.w, 8000));
  GSize text_size = text_layer_get_content_size(s_response_layer);
  text_layer_set_size(s_response_layer, GSize(bounds.size.w, text_size.h + 10));
  scroll_layer_set_content_size(s_scroll_layer, GSize(bounds.size.w, text_size.h + 20));
  scroll_layer_set_content_offset(s_scroll_layer, GPointZero, false);
}

// ---- Public: copy new response text into the buffer, then refresh ----

void show_response(const char *text) {
  snprintf(s_response_buffer, sizeof(s_response_buffer), "%s", text);
  refresh_result_layers();
}

// ---- Click handlers ----

static void result_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  GPoint offset = scroll_layer_get_content_offset(s_scroll_layer);
#ifdef PBL_ROUND
  scroll_layer_set_content_offset(s_scroll_layer, GPoint(0, offset.y + 30), false);
#else
  scroll_layer_set_content_offset(s_scroll_layer, GPoint(0, offset.y + 30), true);
#endif
}

static void result_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  GPoint offset = scroll_layer_get_content_offset(s_scroll_layer);
#ifdef PBL_ROUND
  scroll_layer_set_content_offset(s_scroll_layer, GPoint(0, offset.y - 30), false);
#else
  scroll_layer_set_content_offset(s_scroll_layer, GPoint(0, offset.y - 30), true);
#endif
}

static void result_back_click_handler(ClickRecognizerRef recognizer, void *context) {
#ifdef PBL_ROUND
  window_stack_pop(false);
#else
  window_stack_pop(true);
#endif
}

static void result_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP,   result_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, result_down_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, result_back_click_handler);
}

// ---- Window lifecycle ----

static void result_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect  bounds       = layer_get_bounds(window_layer);

  // Action bar — up/down scroll
  s_result_action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(s_result_action_bar, window);
  action_bar_layer_set_click_config_provider(s_result_action_bar, result_click_config_provider);
  action_bar_layer_set_icon(s_result_action_bar, BUTTON_ID_UP,   s_icon_up);
  action_bar_layer_set_icon(s_result_action_bar, BUTTON_ID_DOWN, s_icon_down);

  int content_w = bounds.size.w - ACTION_BAR_WIDTH;
  int content_h = bounds.size.h - STATUS_BAR_LAYER_HEIGHT;
#ifdef PBL_ROUND
  int h_pad = 36;  // request label at y≈18 — circle clips x < 36, so 36 clears the edge
#else
  int h_pad = 2;
#endif
  int text_w   = content_w - h_pad - 2;
  int label_h  = 18;  // GOTHIC_14_BOLD renders ~18px tall; 14 clips the bottom
  int req_h    = content_h * 2 / 5;   // ~60px Basalt, ~65px Chalk
  int div_y    = STATUS_BAR_LAYER_HEIGHT + req_h;
  int scroll_h = content_h - req_h - label_h - 6;

  // ---- REQUEST section ----
  s_request_label = text_layer_create(
      GRect(h_pad, STATUS_BAR_LAYER_HEIGHT + 2, text_w, label_h));
  text_layer_set_font(s_request_label, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_color(s_request_label, GColorDarkGray);
  text_layer_set_background_color(s_request_label, GColorClear);
  text_layer_set_text(s_request_label, STR_SECTION_REQUEST);
  layer_add_child(window_layer, text_layer_get_layer(s_request_label));

  s_request_text = text_layer_create(
      GRect(h_pad, STATUS_BAR_LAYER_HEIGHT + label_h + 2, text_w, req_h - label_h - 4));
  text_layer_set_font(s_request_text, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_color(s_request_text, GColorBlack);
  text_layer_set_background_color(s_request_text, GColorClear);
  text_layer_set_overflow_mode(s_request_text, GTextOverflowModeTrailingEllipsis);
  layer_add_child(window_layer, text_layer_get_layer(s_request_text));

  // Thin horizontal divider
  s_divider_layer = text_layer_create(GRect(h_pad, div_y, text_w, 1));
  text_layer_set_background_color(s_divider_layer, GColorBlack);
  layer_add_child(window_layer, text_layer_get_layer(s_divider_layer));

  // ---- MABEL section ----
  s_mabel_label = text_layer_create(
      GRect(h_pad, div_y + 2, text_w, label_h));
  text_layer_set_font(s_mabel_label, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_color(s_mabel_label, GColorDarkGray);
  text_layer_set_background_color(s_mabel_label, GColorClear);
  text_layer_set_text(s_mabel_label, STR_SECTION_MABEL);
  layer_add_child(window_layer, text_layer_get_layer(s_mabel_label));

  s_scroll_layer = scroll_layer_create(
      GRect(h_pad, div_y + label_h + 4, text_w, scroll_h));
  layer_add_child(window_layer, scroll_layer_get_layer(s_scroll_layer));

  s_response_layer = text_layer_create(GRect(0, 0, text_w, 2000));
  text_layer_set_font(s_response_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_color(s_response_layer, GColorBlack);
  text_layer_set_background_color(s_response_layer, GColorClear);
  text_layer_set_overflow_mode(s_response_layer, GTextOverflowModeWordWrap);
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_response_layer));

  // Populate layers from the shared buffers
  refresh_result_layers();

  // Custom status bar — added last so it renders on top of the action bar
  s_result_status_bar_layer = create_status_bar(window_layer, bounds.size.w);
}

static void result_window_unload(Window *window) {
  text_layer_destroy(s_result_status_bar_layer);
  s_result_status_bar_layer = NULL;
  text_layer_destroy(s_request_label);
  s_request_label = NULL;
  text_layer_destroy(s_request_text);
  s_request_text = NULL;
  text_layer_destroy(s_divider_layer);
  s_divider_layer = NULL;
  text_layer_destroy(s_mabel_label);
  s_mabel_label = NULL;
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
