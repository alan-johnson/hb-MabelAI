#include "hb-mabelAI.h"

// ---- Shared state definitions ----
char    s_response_buffer[2048];
static char s_status_buffer[128];   // persistent copy of inbox status strings
bool    s_is_loading = false;
GBitmap *s_icon_mic;
GBitmap *s_icon_up;
GBitmap *s_icon_down;
GBitmap *s_icon_help;
Window  *s_main_window;
Window  *s_result_window;
Window  *s_help_window;

// ---- Custom status bar shared state ----
char      s_status_bar_text[32]       = "";
TextLayer *s_main_status_bar_layer   = NULL;
TextLayer *s_result_status_bar_layer = NULL;
TextLayer *s_help_status_bar_layer   = NULL;

void refresh_custom_status_bars(void) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  BatteryChargeState batt = battery_state_service_peek();
  snprintf(s_status_bar_text, sizeof(s_status_bar_text),
           "%02d:%02d  BAT: %d%%", t->tm_hour, t->tm_min, batt.charge_percent);
  if (s_main_status_bar_layer) {
    layer_mark_dirty(text_layer_get_layer(s_main_status_bar_layer));
  }
  if (s_result_status_bar_layer) {
    layer_mark_dirty(text_layer_get_layer(s_result_status_bar_layer));
  }
  if (s_help_status_bar_layer) {
    layer_mark_dirty(text_layer_get_layer(s_help_status_bar_layer));
  }
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

  Tuple *response_t = dict_find(iterator, KEY_RESPONSE);
  Tuple *status_t   = dict_find(iterator, KEY_STATUS);

  if (response_t) {
    show_response(response_t->value->cstring);
    set_status(STR_STATUS_IDLE);
    // Push result window; if already on top, response is updated in place
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
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  s_is_loading = false;
  set_status(STR_ERR_MSG_DROPPED);
}

static void outbox_failed_callback(DictionaryIterator *iterator,
                                    AppMessageResult reason, void *context) {
  s_is_loading = false;
  set_status(STR_ERR_OUTBOX_FAILED);
}

// ---- Init / Deinit ----

static void init(void) {
  s_response_buffer[0] = '\0';

  // Shared bitmaps loaded once — used by both windows
  s_icon_mic  = gbitmap_create_with_resource(RESOURCE_ID_ICON_MIC);
  s_icon_up   = gbitmap_create_with_resource(RESOURCE_ID_ICON_UP);
  s_icon_down = gbitmap_create_with_resource(RESOURCE_ID_ICON_DOWN);
  s_icon_help = gbitmap_create_with_resource(RESOURCE_ID_ICON_HELP);

  // Subscribe once — callbacks update whichever status bar layers are live
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(battery_handler);
  refresh_custom_status_bars(); // populate buffer before any window loads

  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_open(2048, 512);

  s_main_window = window_create();
  window_set_window_handlers(s_main_window, main_window_get_handlers());

  s_result_window = window_create();
  window_set_window_handlers(s_result_window, result_window_get_handlers());

  s_help_window = window_create();
  window_set_window_handlers(s_help_window, help_window_get_handlers());

  // Chalk (round) animation engine does not support the slide-in animation;
  // using animated=true on PBL_ROUND produces "Animation does not exist" errors.
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
  window_destroy(s_help_window);
  gbitmap_destroy(s_icon_mic);
  gbitmap_destroy(s_icon_up);
  gbitmap_destroy(s_icon_down);
  gbitmap_destroy(s_icon_help);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
