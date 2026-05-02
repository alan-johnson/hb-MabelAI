#include "hb-mabelAI.h"

// Vibration pattern: three short pulses (on/off/on/off/on ms)
#define VIBE_INTERVAL_MS   9000

static char        s_alarm_label_buffer[64];
static TextLayer  *s_alarm_title_layer;
static TextLayer  *s_alarm_label_layer;
static TextLayer  *s_alarm_hint_layer;
static AppTimer   *s_vibe_timer = NULL;

// ---- Vibration loop ----

static void vibe_timer_callback(void *context);

static void start_vibe_burst(void) {
  if (quiet_time_is_active()) return;
  static const uint32_t segs[] = {200, 100, 200, 100, 400};
  VibePattern pat = { .durations = segs, .num_segments = ARRAY_LENGTH(segs) };
  vibes_enqueue_custom_pattern(pat);
  s_vibe_timer = app_timer_register(VIBE_INTERVAL_MS, vibe_timer_callback, NULL);
}

static void vibe_timer_callback(void *context) {
  s_vibe_timer = NULL;
  start_vibe_burst();
}

static void stop_vibration(void) {
  if (s_vibe_timer) {
    app_timer_cancel(s_vibe_timer);
    s_vibe_timer = NULL;
  }
  vibes_cancel();
}

// ---- Dismiss logic ----

static void notify_phone_dismissed(void) {
  DictionaryIterator *iter;
  if (app_message_outbox_begin(&iter) != APP_MSG_OK) return;
  int32_t val = 1;
  dict_write_int(iter, KEY_ALARM_DISMISSED, &val, sizeof(int32_t), true);
  app_message_outbox_send();
}

static void dismiss_click_handler(ClickRecognizerRef recognizer, void *context) {
  stop_vibration();
  notify_phone_dismissed();
  window_stack_pop(true);
}

static void alarm_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP,   dismiss_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, dismiss_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, dismiss_click_handler);
}

// ---- Window lifecycle ----

static void alarm_notify_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect  bounds       = layer_get_bounds(window_layer);

  window_set_click_config_provider(window, alarm_click_config_provider);

  int w        = bounds.size.w;
  int h        = bounds.size.h;
  int status_h = STATUS_BAR_LAYER_HEIGHT + 2;

#ifdef PBL_ROUND
  int h_pad = 20;
#else
  int h_pad = 6;
#endif

  // "alarm" section label
  s_alarm_title_layer = text_layer_create(
      GRect(h_pad, status_h + 6, w - h_pad * 2, 20));
  text_layer_set_font(s_alarm_title_layer,
      fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_color(s_alarm_title_layer, COLOR_TEXT_LABEL);
  text_layer_set_background_color(s_alarm_title_layer, GColorClear);
  text_layer_set_text(s_alarm_title_layer, STR_ALARM_NOTIFY_TITLE);
  layer_add_child(window_layer, text_layer_get_layer(s_alarm_title_layer));

  // Alarm description — large, prominent
  int label_y = status_h + 30;
  int hint_h  = 18;
  int label_h = h - label_y - hint_h - 6;
  s_alarm_label_layer = text_layer_create(
      GRect(h_pad, label_y, w - h_pad * 2, label_h));
  text_layer_set_font(s_alarm_label_layer,
      fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_color(s_alarm_label_layer, COLOR_TEXT);
  text_layer_set_background_color(s_alarm_label_layer, GColorClear);
  text_layer_set_overflow_mode(s_alarm_label_layer, GTextOverflowModeWordWrap);
  text_layer_set_text(s_alarm_label_layer, s_alarm_label_buffer);
  layer_add_child(window_layer, text_layer_get_layer(s_alarm_label_layer));

  // Dismiss hint anchored to bottom
  s_alarm_hint_layer = text_layer_create(
      GRect(h_pad, h - hint_h - 2, w - h_pad * 2, hint_h));
  text_layer_set_font(s_alarm_hint_layer,
      fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_color(s_alarm_hint_layer, COLOR_TEXT_LABEL);
  text_layer_set_background_color(s_alarm_hint_layer, GColorClear);
  text_layer_set_text(s_alarm_hint_layer, STR_ALARM_NOTIFY_DISMISS);
  layer_add_child(window_layer, text_layer_get_layer(s_alarm_hint_layer));

  s_alarm_notify_status_bar_layer = create_status_bar(window_layer, w);

  start_vibe_burst();
}

static void alarm_notify_window_unload(Window *window) {
  stop_vibration();
  layer_destroy(s_alarm_notify_status_bar_layer);
  s_alarm_notify_status_bar_layer = NULL;
  text_layer_destroy(s_alarm_title_layer);
  s_alarm_title_layer = NULL;
  text_layer_destroy(s_alarm_label_layer);
  s_alarm_label_layer = NULL;
  text_layer_destroy(s_alarm_hint_layer);
  s_alarm_hint_layer = NULL;
}

// ---- Public API ----

void alarm_notify_set_label(const char *label) {
  snprintf(s_alarm_label_buffer, sizeof(s_alarm_label_buffer), "%s", label);
  if (s_alarm_label_layer) {
    text_layer_set_text(s_alarm_label_layer, s_alarm_label_buffer);
  }
}

WindowHandlers alarm_notify_window_get_handlers(void) {
  return (WindowHandlers){
    .load   = alarm_notify_window_load,
    .unload = alarm_notify_window_unload,
  };
}
