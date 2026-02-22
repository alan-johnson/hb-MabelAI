#include "hb-mabelAI.h"

// ---- Private layer state ----
static StatusBarLayer *s_status_bar;
static ScrollLayer    *s_help_scroll_layer;
static TextLayer      *s_help_text_layer;

// ---- Window lifecycle ----

static void help_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect  bounds       = layer_get_bounds(window_layer);

  // Status bar — clock at the top
  s_status_bar = status_bar_layer_create();
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));

  // Scroll layer — full window width, no action bar (below status bar)
  s_help_scroll_layer = scroll_layer_create(GRect(4, STATUS_BAR_LAYER_HEIGHT + 4, bounds.size.w - 8, bounds.size.h - STATUS_BAR_LAYER_HEIGHT - 8));
  scroll_layer_set_click_config_onto_window(s_help_scroll_layer, window);
  layer_add_child(window_layer, scroll_layer_get_layer(s_help_scroll_layer));

  // Help text inside scroll layer
  int content_w = bounds.size.w - 8;
  s_help_text_layer = text_layer_create(GRect(0, 0, content_w, 2000));
  text_layer_set_font(s_help_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_color(s_help_text_layer, GColorBlack);
  text_layer_set_background_color(s_help_text_layer, GColorClear);
  text_layer_set_overflow_mode(s_help_text_layer, GTextOverflowModeWordWrap);
  text_layer_set_text(s_help_text_layer, STR_HELP_TEXT);

  // Size content to fit text
  GSize text_size = text_layer_get_content_size(s_help_text_layer);
  text_layer_set_size(s_help_text_layer, GSize(content_w, text_size.h + 10));
  scroll_layer_set_content_size(s_help_scroll_layer, GSize(content_w, text_size.h + 20));

  scroll_layer_add_child(s_help_scroll_layer, text_layer_get_layer(s_help_text_layer));
}

static void help_window_unload(Window *window) {
  status_bar_layer_destroy(s_status_bar);
  text_layer_destroy(s_help_text_layer);
  s_help_text_layer = NULL;
  scroll_layer_destroy(s_help_scroll_layer);
  s_help_scroll_layer = NULL;
}

// ---- Public: return handlers for use in init ----

WindowHandlers help_window_get_handlers(void) {
  return (WindowHandlers){
    .load   = help_window_load,
    .unload = help_window_unload,
  };
}
