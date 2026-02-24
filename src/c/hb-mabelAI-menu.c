#include "hb-mabelAI.h"

// ---- Private state ----
static SimpleMenuLayer  *s_menu_layer;
static SimpleMenuSection s_menu_sections[1];
static SimpleMenuItem    s_menu_items[5];

// ---- Callbacks ----

static void send_reminders_request(void) {
  DictionaryIterator *iter;
  AppMessageResult result = app_message_outbox_begin(&iter);
  if (result != APP_MSG_OK) return;
  int32_t val = 1;
  dict_write_int(iter, KEY_REMINDERS_REQUEST, &val, sizeof(int32_t), true);
  app_message_outbox_send();
}

static void menu_select_callback(int index, void *context) {
  (void)context;
  if (index == 0) {
    send_reminders_request();
  } else if (index == 3) {
#ifdef PBL_ROUND
    window_stack_push(s_about_window, false);
#else
    window_stack_push(s_about_window, true);
#endif
  } else if (index == 4) {
#ifdef PBL_ROUND
    window_stack_push(s_legal_window, false);
#else
    window_stack_push(s_legal_window, true);
#endif
  }
}

// ---- Window lifecycle ----

static void menu_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect  bounds       = layer_get_bounds(window_layer);

  s_menu_items[0] = (SimpleMenuItem){ .title = STR_MENU_ITEM_REMINDERS, .callback = menu_select_callback };
  s_menu_items[1] = (SimpleMenuItem){ .title = STR_MENU_ITEM_TIMERS,    .callback = menu_select_callback };
  s_menu_items[2] = (SimpleMenuItem){ .title = STR_MENU_ITEM_ALARMS,    .callback = menu_select_callback };
  s_menu_items[3] = (SimpleMenuItem){ .title = STR_MENU_ITEM_ABOUT,     .callback = menu_select_callback };
  s_menu_items[4] = (SimpleMenuItem){ .title = STR_MENU_ITEM_LEGAL,     .callback = menu_select_callback };

  s_menu_sections[0] = (SimpleMenuSection){
    .items     = s_menu_items,
    .num_items = ARRAY_LENGTH(s_menu_items),
  };

  // Place menu below the status bar so items aren't hidden under it
  GRect menu_rect = GRect(0, STATUS_BAR_LAYER_HEIGHT,
                          bounds.size.w, bounds.size.h - STATUS_BAR_LAYER_HEIGHT);
  s_menu_layer = simple_menu_layer_create(menu_rect, window, s_menu_sections, 1, NULL);
  layer_add_child(window_layer, simple_menu_layer_get_layer(s_menu_layer));

  // Custom status bar — added last so it renders on top
  s_menu_status_bar_layer = create_status_bar(window_layer, bounds.size.w);
}

static void menu_window_unload(Window *window) {
  text_layer_destroy(s_menu_status_bar_layer);
  s_menu_status_bar_layer = NULL;
  simple_menu_layer_destroy(s_menu_layer);
  s_menu_layer = NULL;
}

// ---- Public: return handlers for use in init ----

WindowHandlers menu_window_get_handlers(void) {
  return (WindowHandlers){
    .load   = menu_window_load,
    .unload = menu_window_unload,
  };
}
