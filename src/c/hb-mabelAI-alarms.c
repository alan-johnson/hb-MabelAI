#include "hb-mabelAI.h"

#define MAX_LIST_ITEMS 20

typedef struct {
  char title[80];
  char subtitle[24];
} ListItem;

static char              s_alarms_buffer[1804];
static ListItem          s_items[MAX_LIST_ITEMS];
static int               s_item_count = 0;
static SimpleMenuItem    s_menu_items[MAX_LIST_ITEMS];
static SimpleMenuSection s_sections[1];
static SimpleMenuLayer  *s_menu_layer;

static void alarm_selected(int index, void *context) {
  if (index < 0 || index >= s_item_count) return;
  if (!strchr(s_items[index].subtitle, '/')) return;
  DictionaryIterator *iter;
  if (app_message_outbox_begin(&iter) != APP_MSG_OK) return;
  int32_t val = (int32_t)index;
  dict_write_int(iter, KEY_ALARM_DELETE, &val, sizeof(int32_t), true);
  app_message_outbox_send();
}

static void build_menu(void) {
  s_item_count = 0;
  char *p = s_alarms_buffer;
  while (*p && s_item_count < MAX_LIST_ITEMS) {
    char *sep = strstr(p, "\n\n");
    int chunk_len = sep ? (int)(sep - p) : (int)strlen(p);
    char *nl = memchr(p, '\n', (size_t)chunk_len);
    if (nl) {
      int tlen = (int)(nl - p);
      int slen = chunk_len - tlen - 1;
      if (tlen < 0) tlen = 0;
      if (slen < 0) slen = 0;
      snprintf(s_items[s_item_count].title,    sizeof(s_items[0].title),    "%.*s", tlen, p);
      snprintf(s_items[s_item_count].subtitle, sizeof(s_items[0].subtitle), "%.*s", slen, nl + 1);
    } else {
      snprintf(s_items[s_item_count].title, sizeof(s_items[0].title), "%.*s", chunk_len, p);
      s_items[s_item_count].subtitle[0] = '\0';
    }
    s_item_count++;
    p = sep ? sep + 2 : p + chunk_len;
    if (!sep) break;
  }

  bool has_real_items = false;
  for (int i = 0; i < s_item_count; i++) {
    s_menu_items[i] = (SimpleMenuItem){
      .title    = s_items[i].title,
      .subtitle = s_items[i].subtitle[0] ? s_items[i].subtitle : NULL,
      .callback = alarm_selected,
    };
    if (strchr(s_items[i].subtitle, '/')) has_real_items = true;
  }
  s_sections[0] = (SimpleMenuSection){
    .title     = has_real_items ? "select to delete" : NULL,
    .items     = s_menu_items,
    .num_items = (uint32_t)s_item_count,
  };
}

void show_alarms(const char *text) {
  snprintf(s_alarms_buffer, sizeof(s_alarms_buffer), "%s", text);
  build_menu();
  if (s_menu_layer) {
    menu_layer_reload_data(simple_menu_layer_get_menu_layer(s_menu_layer));
  }
}

static void alarms_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect  bounds       = layer_get_bounds(window_layer);
  build_menu();
  s_menu_layer = simple_menu_layer_create(
      GRect(0, STATUS_BAR_LAYER_HEIGHT, bounds.size.w, bounds.size.h - STATUS_BAR_LAYER_HEIGHT),
      window, s_sections, 1, NULL);
  layer_add_child(window_layer, simple_menu_layer_get_layer(s_menu_layer));
  s_alarms_status_bar_layer = create_status_bar(window_layer, bounds.size.w);
}

static void alarms_window_unload(Window *window) {
  layer_destroy(s_alarms_status_bar_layer);
  s_alarms_status_bar_layer = NULL;
  simple_menu_layer_destroy(s_menu_layer);
  s_menu_layer = NULL;
}

WindowHandlers alarms_window_get_handlers(void) {
  return (WindowHandlers){
    .load   = alarms_window_load,
    .unload = alarms_window_unload,
  };
}
