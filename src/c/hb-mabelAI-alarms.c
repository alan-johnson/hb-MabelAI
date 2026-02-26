#include "hb-mabelAI.h"

// ---- Private state ----
static char            s_alarms_buffer[1804];
static TextLayer      *s_alarms_label;
static ScrollLayer    *s_scroll_layer;
static TextLayer      *s_alarms_layer;
static ActionBarLayer *s_alarms_action_bar;

// ---- Private: push buffer contents into the layers ----

static void refresh_alarms_layers(void) {
  if (!s_alarms_layer || !s_scroll_layer) return;
  text_layer_set_text(s_alarms_layer, s_alarms_buffer);
  GRect bounds = layer_get_bounds(scroll_layer_get_layer(s_scroll_layer));
  text_layer_set_size(s_alarms_layer, GSize(bounds.size.w, 8000));
  GSize text_size = text_layer_get_content_size(s_alarms_layer);
  text_layer_set_size(s_alarms_layer, GSize(bounds.size.w, text_size.h + 10));
  scroll_layer_set_content_size(s_scroll_layer, GSize(bounds.size.w, text_size.h + 20));
  scroll_layer_set_content_offset(s_scroll_layer, GPointZero, false);
}

// ---- Public: copy new alarms text into the buffer, then refresh ----

void show_alarms(const char *text) {
  snprintf(s_alarms_buffer, sizeof(s_alarms_buffer), "%s", text);
  refresh_alarms_layers();
}

// ---- Click handlers ----

static void alarms_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  GPoint offset = scroll_layer_get_content_offset(s_scroll_layer);
#ifdef PBL_ROUND
  scroll_layer_set_content_offset(s_scroll_layer, GPoint(0, offset.y + 30), false);
#else
  scroll_layer_set_content_offset(s_scroll_layer, GPoint(0, offset.y + 30), true);
#endif
}

static void alarms_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  GPoint offset = scroll_layer_get_content_offset(s_scroll_layer);
#ifdef PBL_ROUND
  scroll_layer_set_content_offset(s_scroll_layer, GPoint(0, offset.y - 30), false);
#else
  scroll_layer_set_content_offset(s_scroll_layer, GPoint(0, offset.y - 30), true);
#endif
}

static void alarms_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP,   alarms_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, alarms_down_click_handler);
}

// ---- Window lifecycle ----

static void alarms_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect  bounds       = layer_get_bounds(window_layer);

  s_alarms_action_bar = action_bar_layer_create();
  action_bar_layer_set_click_config_provider(s_alarms_action_bar, alarms_click_config_provider);
  action_bar_layer_set_icon(s_alarms_action_bar, BUTTON_ID_UP,   s_icon_up);
  action_bar_layer_set_icon(s_alarms_action_bar, BUTTON_ID_DOWN, s_icon_down);

  int content_w = bounds.size.w - ACTION_BAR_WIDTH;
  int content_h = bounds.size.h - STATUS_BAR_LAYER_HEIGHT;
#ifdef PBL_ROUND
  int h_pad = 36;
#else
  int h_pad = 2;
#endif
  int text_w   = content_w - h_pad - 2;
  int label_h  = 18;
  int scroll_h = content_h - label_h - 6;

  s_alarms_label = text_layer_create(
      GRect(h_pad, STATUS_BAR_LAYER_HEIGHT + 2, text_w, label_h));
  text_layer_set_font(s_alarms_label, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_color(s_alarms_label, GColorDarkGray);
  text_layer_set_background_color(s_alarms_label, GColorClear);
  text_layer_set_text(s_alarms_label, STR_SECTION_ALARMS);
  layer_add_child(window_layer, text_layer_get_layer(s_alarms_label));

  s_scroll_layer = scroll_layer_create(
      GRect(h_pad, STATUS_BAR_LAYER_HEIGHT + label_h + 4, text_w, scroll_h));
  layer_add_child(window_layer, scroll_layer_get_layer(s_scroll_layer));

  s_alarms_layer = text_layer_create(GRect(0, 0, text_w, 2000));
  text_layer_set_font(s_alarms_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_color(s_alarms_layer, GColorBlack);
  text_layer_set_background_color(s_alarms_layer, GColorClear);
  text_layer_set_overflow_mode(s_alarms_layer, GTextOverflowModeWordWrap);
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_alarms_layer));

  refresh_alarms_layers();

  s_alarms_status_bar_layer = create_status_bar(window_layer, bounds.size.w);
  action_bar_layer_add_to_window(s_alarms_action_bar, window);
}

static void alarms_window_unload(Window *window) {
  layer_destroy(s_alarms_status_bar_layer);
  s_alarms_status_bar_layer = NULL;
  text_layer_destroy(s_alarms_label);
  s_alarms_label = NULL;
  text_layer_destroy(s_alarms_layer);
  s_alarms_layer = NULL;
  scroll_layer_destroy(s_scroll_layer);
  s_scroll_layer = NULL;
  action_bar_layer_destroy(s_alarms_action_bar);
  s_alarms_action_bar = NULL;
}

// ---- Public: return handlers for use in init ----

WindowHandlers alarms_window_get_handlers(void) {
  return (WindowHandlers){
    .load   = alarms_window_load,
    .unload = alarms_window_unload,
  };
}
