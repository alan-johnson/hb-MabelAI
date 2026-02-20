#pragma once
#include <pebble.h>

// AppMessage keys — must match hb-claudeAI.c and index.js
#define KEY_QUERY     0
#define KEY_RESPONSE  1
#define KEY_STATUS    2

// ---- Shared state (defined in hb-claudeAI.c) ----
extern char    s_response_buffer[2048];
extern bool    s_is_loading;
extern GBitmap *s_icon_mic;
extern GBitmap *s_icon_up;
extern GBitmap *s_icon_down;
extern Window  *s_main_window;
extern Window  *s_result_window;

// ---- API provided by hb-claudeAI-main.c ----
// Updates the prompt/status text in the bottom half of the main window.
void set_status(const char *msg);
// Returns the WindowHandlers struct for the main window.
WindowHandlers main_window_get_handlers(void);

// ---- API provided by hb-claudeAI-results.c ----
// Copies text into the response buffer and refreshes the result window if loaded.
void show_response(const char *text);
// Returns the WindowHandlers struct for the result window.
WindowHandlers result_window_get_handlers(void);
