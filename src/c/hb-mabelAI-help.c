#include "hb-mabelAI.h"

// ---- Private layer state ----
static ScrollLayer    *s_help_scroll_layer;
static TextLayer      *s_help_text_layer;

// ---- Chalk click handlers (replace scroll_layer_set_click_config_onto_window) ----
// scroll_layer_set_click_config_onto_window takes over the window click config,
// making it impossible to also subscribe to BACK. On Chalk we need a custom
// provider so we can pop without animation and avoid "Animation does not exist".

#ifdef PBL_ROUND
static void help_scroll_up(ClickRecognizerRef recognizer, void *context) {
  GPoint offset = scroll_layer_get_content_offset(s_help_scroll_layer);
  scroll_layer_set_content_offset(s_help_scroll_layer,
                                  GPoint(0, offset.y + 30), false);
}
static void help_scroll_down(ClickRecognizerRef recognizer, void *context) {
  GPoint offset = scroll_layer_get_content_offset(s_help_scroll_layer);
  scroll_layer_set_content_offset(s_help_scroll_layer,
                                  GPoint(0, offset.y - 30), false);
}
static void help_back_click(ClickRecognizerRef recognizer, void *context) {
  window_stack_pop(false);
}
static void help_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP,   help_scroll_up);
  window_single_click_subscribe(BUTTON_ID_DOWN, help_scroll_down);
  window_single_click_subscribe(BUTTON_ID_BACK, help_back_click);
}
#endif

// ---- Window lifecycle ----

static void help_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect  bounds       = layer_get_bounds(window_layer);

  // Scroll layer — full window width, no action bar (below status bar)
#ifdef PBL_ROUND
  int h_pad = 14;
#else
  int h_pad = 4;
#endif
  int content_w = bounds.size.w - h_pad * 2;
  s_help_scroll_layer = scroll_layer_create(GRect(h_pad, STATUS_BAR_LAYER_HEIGHT + 4, content_w, bounds.size.h - STATUS_BAR_LAYER_HEIGHT - 8));
#ifdef PBL_ROUND
  window_set_click_config_provider(window, help_click_config_provider);
#else
  scroll_layer_set_click_config_onto_window(s_help_scroll_layer, window);
#endif
  layer_add_child(window_layer, scroll_layer_get_layer(s_help_scroll_layer));

  // Help text inside scroll layer
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

  // Custom status bar — added last so it renders on top
  s_help_status_bar_layer = text_layer_create(GRect(0, 0, bounds.size.w, STATUS_BAR_LAYER_HEIGHT));
  text_layer_set_background_color(s_help_status_bar_layer, GColorBlack);
  text_layer_set_text_color(s_help_status_bar_layer, GColorWhite);
  text_layer_set_font(s_help_status_bar_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_alignment(s_help_status_bar_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(s_help_status_bar_layer, GTextOverflowModeTrailingEllipsis);
  text_layer_set_text(s_help_status_bar_layer, s_status_bar_text);
  layer_add_child(window_layer, text_layer_get_layer(s_help_status_bar_layer));
}

static void help_window_unload(Window *window) {
  text_layer_destroy(s_help_status_bar_layer);
  s_help_status_bar_layer = NULL;
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
