#pragma once
#include <pebble.h>
#include "hb-strings.h"

// ---- Color scheme — change these to retheme the app ----
// Non-platform-conditional (used on all watches)
#define COLOR_TEXT          GColorBlack    // primary body text
#define COLOR_TEXT_LABEL    GColorDarkGray // section label text (muted)
#define COLOR_BUBBLE_FILL   GColorWhite    // speech bubble interior
#define COLOR_BUBBLE_STROKE GColorBlack    // speech bubble outline
#define COLOR_DIVIDER       GColorBlack    // horizontal divider line
#define COLOR_ACCENT        GColorBrass    // primary accent (color watches)
// Platform-conditional colors
#ifdef PBL_COLOR
  #define COLOR_STATUSBAR_BG  GColorBrass
  #define COLOR_STATUSBAR_FG  GColorBlack
  #define COLOR_SPLASH_BG     GColorBrass
#else
  #define COLOR_STATUSBAR_BG  GColorBlack
  #define COLOR_STATUSBAR_FG  GColorWhite
  #define COLOR_SPLASH_BG     GColorWhite
#endif

// AppMessage keys — must match hb-mabelAI.c and index.js
#define KEY_QUERY             0
#define KEY_RESPONSE          1
#define KEY_STATUS            2
#define KEY_REMINDERS_REQUEST 3
#define KEY_REMINDERS_LIST    4
#define KEY_TIMERS_REQUEST    5
#define KEY_TIMERS_LIST       6
#define KEY_ALARMS_REQUEST    7
#define KEY_ALARMS_LIST       8
#define KEY_WEATHER_REQUEST   9
#define KEY_WEATHER_DATA      10
#define KEY_TOKENS_REQUEST    11
#define KEY_TOKENS_DATA       12
// Vibrating alarm keys (phone→watch: arm wakeup; watch→phone: dismissed)
#define KEY_ALARM_WAKEUP      13
#define KEY_ALARM_LABEL       14
#define KEY_ALARM_DISMISSED   15
// Delete keys (watch→phone: index of item to remove from sorted list)
#define KEY_REMINDER_DELETE   16
#define KEY_TIMER_DELETE      17
#define KEY_ALARM_DELETE      18

// Persist keys for vibrating alarm state
#define PERSIST_KEY_ALARM_WAKEUP_ID  1
#define PERSIST_KEY_ALARM_LABEL      2

// ---- Shared state (defined in hb-mabelAI.c) ----
extern char    s_response_buffer[1804];
extern char    s_query_buffer[512];
extern bool    s_is_loading;
extern GBitmap *s_icon_mic;
extern GBitmap *s_icon_up;
extern GBitmap *s_icon_down;
extern GBitmap *s_icon_help;
extern GBitmap *s_icon_menu;
extern Window  *s_main_window;
extern Window  *s_result_window;
extern Window  *s_text_window;
extern Window  *s_menu_window;
extern Window  *s_reminders_window;
extern Window  *s_timers_window;
extern Window  *s_alarms_window;
extern Window  *s_weather_window;
extern Window  *s_tokens_window;
extern Window  *s_alarm_notify_window;

// ---- Shared custom status bar ----
// Shared text buffer — all windows' status bar layers read from this.
extern char   s_status_bar_text[32];
// Each window's status bar Layer — set in _load, cleared in _unload.
extern Layer *s_main_status_bar_layer;
extern Layer *s_result_status_bar_layer;
extern Layer *s_text_status_bar_layer;
extern Layer *s_menu_status_bar_layer;
extern Layer *s_reminders_status_bar_layer;
extern Layer *s_timers_status_bar_layer;
extern Layer *s_alarms_status_bar_layer;
extern Layer *s_weather_status_bar_layer;
extern Layer *s_tokens_status_bar_layer;
extern Layer *s_alarm_notify_status_bar_layer;
// Formats time + battery into s_status_bar_text and redraws all live layers.
void refresh_custom_status_bars(void);
// Creates a full-width status bar Layer (with dotted bottom line), adds it to window_layer, and returns it.
Layer *create_status_bar(Layer *window_layer, int width);

// ---- API provided by hb-mabelAI-main.c ----
// Updates the prompt/status text in the bottom half of the main window.
void set_status(const char *msg);
// Sends a query to the AI via AppMessage.
void send_query(const char *query);
// Returns the WindowHandlers struct for the main window.
WindowHandlers main_window_get_handlers(void);

// ---- API provided by hb-mabelAI-results.c ----
// Copies text into the response buffer and refreshes the result window if loaded.
void show_response(const char *text);
// Returns the WindowHandlers struct for the result window.
WindowHandlers result_window_get_handlers(void);

// ---- API provided by hb-mabelAI-textwindow.c ----
// Shared scrollable text window. Set text via window_set_user_data before pushing.
WindowHandlers text_window_get_handlers(void);

// ---- API provided by hb-mabelAI-menu.c ----
// Returns the WindowHandlers struct for the menu window.
WindowHandlers menu_window_get_handlers(void);

// ---- API provided by hb-mabelAI-reminders.c ----
// Copies text into the reminders buffer and refreshes the window if loaded.
void show_reminders(const char *text);
// Returns the WindowHandlers struct for the reminders window.
WindowHandlers reminders_window_get_handlers(void);

// ---- API provided by hb-mabelAI-timers.c ----
void show_timers(const char *text);
WindowHandlers timers_window_get_handlers(void);

// ---- API provided by hb-mabelAI-alarms.c ----
void show_alarms(const char *text);
WindowHandlers alarms_window_get_handlers(void);

// ---- API provided by hb-mabelAI-weather.c ----
void show_weather(const char *text);
WindowHandlers weather_window_get_handlers(void);

// ---- API provided by hb-mabelAI-tokens.c ----
void show_tokens(const char *text);
WindowHandlers tokens_window_get_handlers(void);

// ---- API provided by hb-mabelAI-alarm-notify.c ----
void alarm_notify_set_label(const char *label);
WindowHandlers alarm_notify_window_get_handlers(void);
