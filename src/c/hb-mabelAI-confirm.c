#include "hb-mabelAI.h"

// ---- Private layer state ----
static ActionBarLayer *s_confirm_action_bar;
static TextLayer      *s_confirm_title_layer;
static TextLayer      *s_confirm_text_layer;
static char            s_confirm_buf[512];
static char            s_confirm_display_buf[512 + sizeof("\n\n" STR_CONFIRM_HINT)];

// ---- Public: store transcription text and refresh if window is live ----

void show_confirmation(const char *text) {
  snprintf(s_confirm_buf, sizeof(s_confirm_buf), "%s", text);
  snprintf(s_confirm_display_buf, sizeof(s_confirm_display_buf),
           "%s\n\n" STR_CONFIRM_HINT, text);
  if (s_confirm_text_layer) {
    text_layer_set_text(s_confirm_text_layer, s_confirm_display_buf);
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

static void confirm_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, confirm_select_click_handler);
}

// ---- Window lifecycle ----

static void confirm_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect  bounds       = layer_get_bounds(window_layer);

  // Action bar — checkmark on SELECT to send
  s_confirm_action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(s_confirm_action_bar, window);
  action_bar_layer_set_click_config_provider(s_confirm_action_bar, confirm_click_config_provider);
  action_bar_layer_set_icon(s_confirm_action_bar, BUTTON_ID_SELECT, s_icon_check);

  int content_w = bounds.size.w - ACTION_BAR_WIDTH;
#ifdef PBL_ROUND
  int h_pad = 34;  // title at y≈22 — circle clips x < 31, so 34 clears the edge
#else
  int h_pad = 4;
#endif
  int text_w = content_w - h_pad;
  int y = STATUS_BAR_LAYER_HEIGHT + 6;

  // Title: "Send to Mabel?"
  s_confirm_title_layer = text_layer_create(GRect(h_pad, y, text_w, 22));
  text_layer_set_font(s_confirm_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_color(s_confirm_title_layer, GColorBlack);
  text_layer_set_background_color(s_confirm_title_layer, GColorClear);
  text_layer_set_text_alignment(s_confirm_title_layer, GTextAlignmentCenter);
  text_layer_set_text(s_confirm_title_layer, STR_CONFIRM_TITLE);
  layer_add_child(window_layer, text_layer_get_layer(s_confirm_title_layer));
  y += 26;

  // Transcription text — fills remaining space below title
  int text_h = bounds.size.h - y - 4;
  s_confirm_text_layer = text_layer_create(GRect(h_pad, y, text_w, text_h));
  text_layer_set_font(s_confirm_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_color(s_confirm_text_layer, GColorBlack);
  text_layer_set_background_color(s_confirm_text_layer, GColorClear);
  text_layer_set_text_alignment(s_confirm_text_layer, GTextAlignmentLeft);
  text_layer_set_overflow_mode(s_confirm_text_layer, GTextOverflowModeWordWrap);
  text_layer_set_text(s_confirm_text_layer, s_confirm_display_buf);
  layer_add_child(window_layer, text_layer_get_layer(s_confirm_text_layer));

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
  action_bar_layer_destroy(s_confirm_action_bar);
}

// ---- Public: return handlers for use in init ----

WindowHandlers confirm_window_get_handlers(void) {
  return (WindowHandlers){
    .load   = confirm_window_load,
    .unload = confirm_window_unload,
  };
}
