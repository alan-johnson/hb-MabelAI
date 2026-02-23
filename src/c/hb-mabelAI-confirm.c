#include "hb-mabelAI.h"

// ---- Private layer state ----
static TextLayer *s_confirm_title_layer;
static TextLayer *s_confirm_text_layer;
static TextLayer *s_confirm_hint_layer;
static char       s_confirm_buf[512];

// ---- Public: store transcription text and refresh if window is live ----

void show_confirmation(const char *text) {
  snprintf(s_confirm_buf, sizeof(s_confirm_buf), "%s", text);
  if (s_confirm_text_layer) {
    text_layer_set_text(s_confirm_text_layer, s_confirm_buf);
  }
}

// ---- Click handlers ----

static void confirm_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_is_loading) return;
  snprintf(s_query_buffer, sizeof(s_query_buffer), "%s", s_confirm_buf);
  snprintf(s_response_buffer, sizeof(s_response_buffer), "...");
  if (window_stack_get_top_window() != s_result_window) {
#ifdef PBL_ROUND
    window_stack_push(s_result_window, false);
#else
    window_stack_push(s_result_window, true);
#endif
  }
  send_query(s_query_buffer);
}

#ifdef PBL_ROUND
static void confirm_back_click_handler(ClickRecognizerRef recognizer, void *context) {
  window_stack_pop(false);
}
#endif

static void confirm_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, confirm_select_click_handler);
#ifdef PBL_ROUND
  window_single_click_subscribe(BUTTON_ID_BACK, confirm_back_click_handler);
#endif
}

// ---- Window lifecycle ----

static void confirm_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect  bounds       = layer_get_bounds(window_layer);

#ifdef PBL_ROUND
  int h_pad = 14;
#else
  int h_pad = 6;
#endif
  int content_w = bounds.size.w - h_pad * 2;
  int y = STATUS_BAR_LAYER_HEIGHT + 6;

  // Title: "Send to Mabel?"
  s_confirm_title_layer = text_layer_create(GRect(h_pad, y, content_w, 22));
  text_layer_set_font(s_confirm_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_color(s_confirm_title_layer, GColorBlack);
  text_layer_set_background_color(s_confirm_title_layer, GColorClear);
  text_layer_set_text_alignment(s_confirm_title_layer, GTextAlignmentCenter);
  text_layer_set_text(s_confirm_title_layer, STR_CONFIRM_TITLE);
  layer_add_child(window_layer, text_layer_get_layer(s_confirm_title_layer));
  y += 26;

  // Transcription text — fills space between title and hint
  int hint_h = 28;
  int text_h = bounds.size.h - y - hint_h - 4;
  s_confirm_text_layer = text_layer_create(GRect(h_pad, y, content_w, text_h));
  text_layer_set_font(s_confirm_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_color(s_confirm_text_layer, GColorBlack);
  text_layer_set_background_color(s_confirm_text_layer, GColorClear);
  text_layer_set_text_alignment(s_confirm_text_layer, GTextAlignmentLeft);
  text_layer_set_overflow_mode(s_confirm_text_layer, GTextOverflowModeTrailingEllipsis);
  text_layer_set_text(s_confirm_text_layer, s_confirm_buf);
  layer_add_child(window_layer, text_layer_get_layer(s_confirm_text_layer));

  // Hint at bottom
  s_confirm_hint_layer = text_layer_create(
      GRect(h_pad, bounds.size.h - hint_h, content_w, hint_h));
  text_layer_set_font(s_confirm_hint_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_color(s_confirm_hint_layer, GColorDarkGray);
  text_layer_set_background_color(s_confirm_hint_layer, GColorClear);
  text_layer_set_text_alignment(s_confirm_hint_layer, GTextAlignmentCenter);
  text_layer_set_text(s_confirm_hint_layer, STR_CONFIRM_HINT);
  layer_add_child(window_layer, text_layer_get_layer(s_confirm_hint_layer));

  window_set_click_config_provider(window, confirm_click_config_provider);

  // Custom status bar — added last so it renders on top
  s_confirm_status_bar_layer = create_status_bar(window_layer, bounds.size.w);
}

static void confirm_window_unload(Window *window) {
  text_layer_destroy(s_confirm_status_bar_layer);
  s_confirm_status_bar_layer = NULL;
  text_layer_destroy(s_confirm_title_layer);
  s_confirm_title_layer = NULL;
  text_layer_destroy(s_confirm_text_layer);
  s_confirm_text_layer = NULL;
  text_layer_destroy(s_confirm_hint_layer);
  s_confirm_hint_layer = NULL;
}

// ---- Public: return handlers for use in init ----

WindowHandlers confirm_window_get_handlers(void) {
  return (WindowHandlers){
    .load   = confirm_window_load,
    .unload = confirm_window_unload,
  };
}
