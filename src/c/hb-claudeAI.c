#include "hb-claudeAI.h"

// ---- Shared state definitions ----
char    s_response_buffer[2048];
bool    s_is_loading = false;
GBitmap *s_icon_mic;
GBitmap *s_icon_up;
GBitmap *s_icon_down;
Window  *s_main_window;
Window  *s_result_window;

// ---- AppMessage Callbacks ----

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  s_is_loading = false;

  Tuple *response_t = dict_find(iterator, KEY_RESPONSE);
  Tuple *status_t   = dict_find(iterator, KEY_STATUS);

  if (response_t) {
    show_response(response_t->value->cstring);
    set_status("Press MIC to ask Claude");
    // Push result window; if already on top, response is updated in place
    if (window_stack_get_top_window() != s_result_window) {
      window_stack_push(s_result_window, true);
    }
  }
  if (status_t) {
    set_status(status_t->value->cstring);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  s_is_loading = false;
  set_status("Message dropped. Try again.");
}

static void outbox_failed_callback(DictionaryIterator *iterator,
                                    AppMessageResult reason, void *context) {
  s_is_loading = false;
  set_status("Send failed. Try again.");
}

// ---- Init / Deinit ----

static void init(void) {
  s_response_buffer[0] = '\0';

  // Shared bitmaps loaded once — used by both windows
  s_icon_mic  = gbitmap_create_with_resource(RESOURCE_ID_ICON_MIC);
  s_icon_up   = gbitmap_create_with_resource(RESOURCE_ID_ICON_UP);
  s_icon_down = gbitmap_create_with_resource(RESOURCE_ID_ICON_DOWN);

  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_open(2048, 512);

  s_main_window = window_create();
  window_set_window_handlers(s_main_window, main_window_get_handlers());

  s_result_window = window_create();
  window_set_window_handlers(s_result_window, result_window_get_handlers());

  window_stack_push(s_main_window, true);
}

static void deinit(void) {
  window_destroy(s_main_window);
  window_destroy(s_result_window);
  gbitmap_destroy(s_icon_mic);
  gbitmap_destroy(s_icon_up);
  gbitmap_destroy(s_icon_down);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
