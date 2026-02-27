#include "hb-mabelAI.h"

// ---- Shared state definitions ----
char    s_response_buffer[1804];
char    s_query_buffer[512];
static char s_status_buffer[64];    // persistent copy of inbox status strings
bool    s_is_loading = false;
GBitmap *s_icon_mic;
GBitmap *s_icon_up;
GBitmap *s_icon_down;
GBitmap *s_icon_help;
GBitmap *s_icon_menu;
Window  *s_main_window;
Window  *s_result_window;
Window  *s_text_window;
Window  *s_menu_window;
Window  *s_reminders_window;
Window  *s_timers_window;
Window  *s_alarms_window;
Window  *s_weather_window;
Window  *s_tokens_window;

// ---- Custom status bar shared state ----
char   s_status_bar_text[32]    = "";
Layer *s_main_status_bar_layer      = NULL;
Layer *s_result_status_bar_layer    = NULL;
Layer *s_text_status_bar_layer      = NULL;
Layer *s_menu_status_bar_layer      = NULL;
Layer *s_reminders_status_bar_layer = NULL;
Layer *s_timers_status_bar_layer    = NULL;
Layer *s_alarms_status_bar_layer    = NULL;
Layer *s_weather_status_bar_layer   = NULL;
Layer *s_tokens_status_bar_layer    = NULL;

static void status_bar_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Background
  graphics_context_set_fill_color(ctx, COLOR_STATUSBAR_BG);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Time + battery text
  graphics_context_set_text_color(ctx, COLOR_STATUSBAR_FG);
  graphics_draw_text(ctx, s_status_bar_text,
                     fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                     GRect(0, 1, bounds.size.w, bounds.size.h - 2),
                     GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentCenter, NULL);

  // Dotted line along the bottom edge (2px on, 2px off)
  graphics_context_set_stroke_color(ctx, COLOR_STATUSBAR_FG);
  int y = bounds.size.h - 1;
  for (int x = 0; x < bounds.size.w; x += 4) {
    graphics_draw_pixel(ctx, GPoint(x,     y));
    graphics_draw_pixel(ctx, GPoint(x + 1, y));
  }
}

Layer *create_status_bar(Layer *window_layer, int width) {
  Layer *bar = layer_create(GRect(0, 0, width, STATUS_BAR_LAYER_HEIGHT + 2));
  layer_set_update_proc(bar, status_bar_update_proc);
  layer_add_child(window_layer, bar);
  return bar;
}

void refresh_custom_status_bars(void) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  BatteryChargeState batt = battery_state_service_peek();
  if (clock_is_24h_style()) {
    snprintf(s_status_bar_text, sizeof(s_status_bar_text),
             "%02d:%02d | %d%%", t->tm_hour, t->tm_min, batt.charge_percent);
  } else {
    int hour12 = t->tm_hour % 12;
    if (hour12 == 0) hour12 = 12;
    snprintf(s_status_bar_text, sizeof(s_status_bar_text),
             "%d:%02d %s | %d%%", hour12, t->tm_min,
             t->tm_hour < 12 ? "AM" : "PM", batt.charge_percent);
  }
  if (s_main_status_bar_layer)      layer_mark_dirty(s_main_status_bar_layer);
  if (s_result_status_bar_layer)    layer_mark_dirty(s_result_status_bar_layer);
  if (s_text_status_bar_layer)      layer_mark_dirty(s_text_status_bar_layer);
  if (s_menu_status_bar_layer)      layer_mark_dirty(s_menu_status_bar_layer);
  if (s_reminders_status_bar_layer) layer_mark_dirty(s_reminders_status_bar_layer);
  if (s_timers_status_bar_layer)    layer_mark_dirty(s_timers_status_bar_layer);
  if (s_alarms_status_bar_layer)    layer_mark_dirty(s_alarms_status_bar_layer);
  if (s_weather_status_bar_layer)   layer_mark_dirty(s_weather_status_bar_layer);
  if (s_tokens_status_bar_layer)    layer_mark_dirty(s_tokens_status_bar_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  refresh_custom_status_bars();
}

static void battery_handler(BatteryChargeState state) {
  refresh_custom_status_bars();
}

// ---- AppMessage Callbacks ----

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  s_is_loading = false;

  Tuple *response_t   = dict_find(iterator, KEY_RESPONSE);
  Tuple *status_t     = dict_find(iterator, KEY_STATUS);
  Tuple *reminders_t  = dict_find(iterator, KEY_REMINDERS_LIST);
  Tuple *timers_t     = dict_find(iterator, KEY_TIMERS_LIST);
  Tuple *alarms_t     = dict_find(iterator, KEY_ALARMS_LIST);
  Tuple *weather_t    = dict_find(iterator, KEY_WEATHER_DATA);
  Tuple *tokens_t     = dict_find(iterator, KEY_TOKENS_DATA);

  if (response_t) {
    show_response(response_t->value->cstring);
    set_status(STR_STATUS_IDLE);
    if (window_stack_get_top_window() != s_result_window) {
#ifdef PBL_ROUND
      window_stack_push(s_result_window, false);
#else
      window_stack_push(s_result_window, true);
#endif
    }
  }
  if (status_t) {
    snprintf(s_status_buffer, sizeof(s_status_buffer), "%s", status_t->value->cstring);
    set_status(s_status_buffer);
    show_response(s_status_buffer);  // also update the mabel section in result window
  }
  if (reminders_t) {
    show_reminders(reminders_t->value->cstring);
    if (window_stack_get_top_window() != s_reminders_window) {
#ifdef PBL_ROUND
      window_stack_push(s_reminders_window, false);
#else
      window_stack_push(s_reminders_window, true);
#endif
    }
  }
  if (timers_t) {
    show_timers(timers_t->value->cstring);
    if (window_stack_get_top_window() != s_timers_window) {
#ifdef PBL_ROUND
      window_stack_push(s_timers_window, false);
#else
      window_stack_push(s_timers_window, true);
#endif
    }
  }
  if (alarms_t) {
    show_alarms(alarms_t->value->cstring);
    if (window_stack_get_top_window() != s_alarms_window) {
#ifdef PBL_ROUND
      window_stack_push(s_alarms_window, false);
#else
      window_stack_push(s_alarms_window, true);
#endif
    }
  }
  if (weather_t) {
    show_weather(weather_t->value->cstring);
    if (window_stack_get_top_window() != s_weather_window) {
#ifdef PBL_ROUND
      window_stack_push(s_weather_window, false);
#else
      window_stack_push(s_weather_window, true);
#endif
    }
  }
  if (tokens_t) {
    show_tokens(tokens_t->value->cstring);
    if (window_stack_get_top_window() != s_tokens_window) {
#ifdef PBL_ROUND
      window_stack_push(s_tokens_window, false);
#else
      window_stack_push(s_tokens_window, true);
#endif
    }
  }
}

static void push_result_window(void) {
  if (window_stack_get_top_window() != s_result_window) {
#ifdef PBL_ROUND
    window_stack_push(s_result_window, false);
#else
    window_stack_push(s_result_window, true);
#endif
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  s_is_loading = false;
  set_status(STR_ERR_MSG_DROPPED);
  show_response(STR_ERR_MSG_DROPPED);
  push_result_window();
}

static void outbox_failed_callback(DictionaryIterator *iterator,
                                    AppMessageResult reason, void *context) {
  s_is_loading = false;
  set_status(STR_ERR_OUTBOX_FAILED);
  show_response(STR_ERR_OUTBOX_FAILED);
  push_result_window();
}

// ---- Init / Deinit ----

static void init(void) {
  s_response_buffer[0] = '\0';
  s_query_buffer[0]    = '\0';

  s_icon_mic  = gbitmap_create_with_resource(RESOURCE_ID_ICON_MIC);
  s_icon_up   = gbitmap_create_with_resource(RESOURCE_ID_ICON_UP);
  s_icon_down = gbitmap_create_with_resource(RESOURCE_ID_ICON_DOWN);
  s_icon_help = gbitmap_create_with_resource(RESOURCE_ID_ICON_HELP);
  s_icon_menu  = gbitmap_create_with_resource(RESOURCE_ID_ICON_MENU);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(battery_handler);
  refresh_custom_status_bars();

  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_open(1804, 512);

  s_main_window = window_create();
  window_set_window_handlers(s_main_window, main_window_get_handlers());

  s_result_window = window_create();
  window_set_window_handlers(s_result_window, result_window_get_handlers());

  s_text_window = window_create();
  window_set_window_handlers(s_text_window, text_window_get_handlers());

  s_menu_window = window_create();
  window_set_window_handlers(s_menu_window, menu_window_get_handlers());

  s_reminders_window = window_create();
  window_set_window_handlers(s_reminders_window, reminders_window_get_handlers());

  s_timers_window = window_create();
  window_set_window_handlers(s_timers_window, timers_window_get_handlers());

  s_alarms_window = window_create();
  window_set_window_handlers(s_alarms_window, alarms_window_get_handlers());

  s_weather_window = window_create();
  window_set_window_handlers(s_weather_window, weather_window_get_handlers());

  s_tokens_window = window_create();
  window_set_window_handlers(s_tokens_window, tokens_window_get_handlers());

#ifdef PBL_ROUND
  window_stack_push(s_main_window, false);
#else
  window_stack_push(s_main_window, true);
#endif
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  window_destroy(s_main_window);
  window_destroy(s_result_window);
  window_destroy(s_text_window);
  window_destroy(s_menu_window);
  window_destroy(s_reminders_window);
  window_destroy(s_timers_window);
  window_destroy(s_alarms_window);
  window_destroy(s_weather_window);
  window_destroy(s_tokens_window);
  gbitmap_destroy(s_icon_mic);
  gbitmap_destroy(s_icon_up);
  gbitmap_destroy(s_icon_down);
  gbitmap_destroy(s_icon_help);
  gbitmap_destroy(s_icon_menu);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
