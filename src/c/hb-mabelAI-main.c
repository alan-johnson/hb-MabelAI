#include "hb-mabelAI.h"

// Uncomment when running in the Pebble emulator (no real microphone).
// Comment out before deploying to a real watch.
//#define EMULATOR

#ifdef EMULATOR
  #define TEST_QUERY_REMINDER "Remind me to take my medication tomorrow at 9am"
  #define TEST_QUERY_TIMER    "Set a timer for 10 minutes"
  #define TEST_QUERY_ALARM    "Set an alarm for 7am tomorrow"
#endif

// ---- Private layer state ----
static BitmapLayer    *s_splash_layer;
static GBitmap        *s_splash_bitmap;
static Layer          *s_bubble_layer;
static GPath          *s_bubble_path;
static TextLayer      *s_prompt_layer;
static ActionBarLayer *s_main_action_bar;

// ---- Thinking animation ----
static AppTimer *s_thinking_timer = NULL;
static uint8_t   s_thinking_dots  = 0;
static char      s_thinking_buf[sizeof(STR_STATUS_THINKING) + 3];  // base + "..." + NUL

static void thinking_timer_callback(void *context) {
  s_thinking_timer = NULL;
  if (!s_is_loading) return;

  s_thinking_dots = (s_thinking_dots % 3) + 1;
  snprintf(s_thinking_buf, sizeof(s_thinking_buf), STR_STATUS_THINKING "%s",
           s_thinking_dots == 1 ? "."   :
           s_thinking_dots == 2 ? ".."  : "...");
  if (s_prompt_layer) {
    text_layer_set_text(s_prompt_layer, s_thinking_buf);
  }
  s_thinking_timer = app_timer_register(500, thinking_timer_callback, NULL);
}

static void start_thinking_animation(void) {
  if (s_thinking_timer) {
    app_timer_cancel(s_thinking_timer);
    s_thinking_timer = NULL;
  }
  s_thinking_dots = 1;
  snprintf(s_thinking_buf, sizeof(s_thinking_buf), STR_STATUS_THINKING ".");
  if (s_prompt_layer) {
    text_layer_set_text(s_prompt_layer, s_thinking_buf);
  }
  s_thinking_timer = app_timer_register(500, thinking_timer_callback, NULL);
}

// ---- Public: update prompt / status text ----

void set_status(const char *msg) {
  if (s_thinking_timer) {
    app_timer_cancel(s_thinking_timer);
    s_thinking_timer = NULL;
  }
  if (s_prompt_layer) {
    text_layer_set_text(s_prompt_layer, msg);
  }
}

// ---- Query dispatch ----

void send_query(const char *query) {
  start_thinking_animation();
  s_is_loading = true;
  DictionaryIterator *out_iter;
  AppMessageResult result = app_message_outbox_begin(&out_iter);
  if (result == APP_MSG_OK) {
    dict_write_cstring(out_iter, KEY_QUERY, query);
    app_message_outbox_send();
  } else {
    set_status(STR_ERR_SEND);
    s_is_loading = false;
  }
}

// ---- Voice Dictation ----

#if defined(PBL_MICROPHONE) && !defined(EMULATOR)
static DictationSession *s_dictation_session = NULL;

static void dictation_session_callback(DictationSession *session,
                                        DictationSessionStatus status,
                                        char *transcription,
                                        void *context) {
  s_dictation_session = NULL;
  if (status == DictationSessionStatusSuccess) {
    snprintf(s_response_buffer, sizeof(s_response_buffer), "...");
    if (window_stack_get_top_window() != s_result_window) {
#ifdef PBL_ROUND
      window_stack_push(s_result_window, false);
#else
      window_stack_push(s_result_window, true);
#endif
    }
    send_query(transcription);
  } else {
    set_status(STR_ERR_DICTATION);
    s_is_loading = false;
  }
  dictation_session_destroy(session);
}

static void start_dictation(void) {
  if (s_is_loading) return;
  if (!connection_service_peek_pebble_app_connection()) {
    set_status(STR_STATUS_NO_PHONE);
    return;
  }
  set_status(STR_STATUS_LISTENING);
  s_dictation_session = dictation_session_create(512,
                                                  dictation_session_callback,
                                                  NULL);
  dictation_session_start(s_dictation_session);
}
#endif  // PBL_MICROPHONE && !EMULATOR

// ---- Click handlers ----

static void main_select_click_handler(ClickRecognizerRef recognizer, void *context) {
#if defined(EMULATOR)
  // Emulator has no real mic — cycle through test queries for full round-trip testing.
  // Press SELECT repeatedly to test: reminder → timer → alarm → repeat.
  static uint8_t s_test_index = 0;
  static const char *s_test_queries[] = {
    TEST_QUERY_REMINDER,
    TEST_QUERY_TIMER,
    TEST_QUERY_ALARM,
  };
  if (s_is_loading) return;
  if (!connection_service_peek_pebble_app_connection()) {
    set_status(STR_STATUS_NO_PHONE);
    return;
  }
  snprintf(s_response_buffer, sizeof(s_response_buffer), "...");
  if (window_stack_get_top_window() != s_result_window) {
#ifdef PBL_ROUND
    window_stack_push(s_result_window, false);
#else
    window_stack_push(s_result_window, true);
#endif
  }
  send_query(s_test_queries[s_test_index % 3]);
  s_test_index++;
#elif defined(PBL_MICROPHONE)
  start_dictation();
#else
  set_status(STR_STATUS_NO_MIC);
#endif
}

static void main_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  window_set_user_data(s_text_window, (void *)STR_HELP_TEXT);
#ifdef PBL_ROUND
  window_stack_push(s_text_window, false);
#else
  window_stack_push(s_text_window, true);
#endif
}

static void main_down_click_handler(ClickRecognizerRef recognizer, void *context) {
#ifdef PBL_ROUND
  window_stack_push(s_menu_window, false);
#else
  window_stack_push(s_menu_window, true);
#endif
}

static void main_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, main_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP,     main_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN,   main_down_click_handler);
}

// ---- Speech bubble drawing ----
// Complete bubble outline as a single closed polygon (15 pts), set in main_window_load.
// Rounded corners are approximated with a 45° midpoint (adequate at Pebble resolution).
// Tail base at 25% from left; tip at 50% toward Mabel.
static GPoint    s_bubble_pts[15];
static GPathInfo s_bubble_path_info = { .num_points = 15, .points = s_bubble_pts };

static void bubble_update_proc(Layer *layer, GContext *ctx) {
  if (!s_bubble_path) return;

#ifdef PBL_COLOR
  // Brass background behind bubble on color watches
  graphics_context_set_fill_color(ctx, GColorBrass);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
#endif

  // White interior fill
  graphics_context_set_fill_color(ctx, GColorWhite);
  gpath_draw_filled(ctx, s_bubble_path);

  // Black outline — single closed polygon, no horizontal base line at tail
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 2);
  gpath_draw_outline(ctx, s_bubble_path);
}

// ---- Window lifecycle ----

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect  bounds       = layer_get_bounds(window_layer);

  // Action bar — help top, mic middle, menu bottom (added to window after content layers)
  s_main_action_bar = action_bar_layer_create();
  action_bar_layer_set_click_config_provider(s_main_action_bar, main_click_config_provider);
  action_bar_layer_set_icon(s_main_action_bar, BUTTON_ID_UP,     s_icon_help);
  action_bar_layer_set_icon(s_main_action_bar, BUTTON_ID_SELECT, s_icon_mic);
  action_bar_layer_set_icon(s_main_action_bar, BUTTON_ID_DOWN,   s_icon_menu);

  int content_w = bounds.size.w - ACTION_BAR_WIDTH;
  int content_h = bounds.size.h - STATUS_BAR_LAYER_HEIGHT;
  int half_h    = content_h / 2;

  int body_h = half_h - 12;  // bubble body height (12px reserved for tail)
  int base_x = content_w / 4;   // tail base at 25% from left
  int tip_x  = content_w / 2;   // tip toward Mabel (centered)
  int w      = content_w;        // shorthand

  // Build the complete bubble polygon (clockwise from top-left).
  // Corners use one 45° midpoint each (radius ≈ 8px).
  // Points 0–2:  top-left corner
  s_bubble_pts[0]  = GPoint(1,     9);
  s_bubble_pts[1]  = GPoint(3,     3);
  s_bubble_pts[2]  = GPoint(9,     1);
  // Points 3–5:  top-right corner
  s_bubble_pts[3]  = GPoint(w - 9, 1);
  s_bubble_pts[4]  = GPoint(w - 3, 3);
  s_bubble_pts[5]  = GPoint(w - 1, 9);
  // Points 6–8:  bottom-right corner
  s_bubble_pts[6]  = GPoint(w - 1, body_h - 8);
  s_bubble_pts[7]  = GPoint(w - 3, body_h - 2);
  s_bubble_pts[8]  = GPoint(w - 9, body_h);
  // Points 9–11: tail (right base → tip → left base)
  s_bubble_pts[9]  = GPoint(base_x + 8, body_h);
  s_bubble_pts[10] = GPoint(tip_x,      body_h + 11);
  s_bubble_pts[11] = GPoint(base_x - 8, body_h);
  // Points 12–14: bottom-left corner
  s_bubble_pts[12] = GPoint(9,     body_h);
  s_bubble_pts[13] = GPoint(3,     body_h - 2);
  s_bubble_pts[14] = GPoint(1,     body_h - 8);

  // Top half: speech bubble
  s_bubble_path  = gpath_create(&s_bubble_path_info);
  s_bubble_layer = layer_create(GRect(0, STATUS_BAR_LAYER_HEIGHT, content_w, half_h));
  layer_set_update_proc(s_bubble_layer, bubble_update_proc);
  layer_add_child(window_layer, s_bubble_layer);

  // Status text inside bubble — transparent so bubble drawing shows through
#ifdef PBL_ROUND
  s_prompt_layer = text_layer_create(GRect(14, STATUS_BAR_LAYER_HEIGHT + 5, content_w - 28, body_h - 10));
#else
  s_prompt_layer = text_layer_create(GRect(6, STATUS_BAR_LAYER_HEIGHT + 4, content_w - 12, body_h - 8));
#endif
  text_layer_set_font(s_prompt_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_color(s_prompt_layer, GColorBlack);
  text_layer_set_background_color(s_prompt_layer, GColorClear);
  text_layer_set_text_alignment(s_prompt_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(s_prompt_layer, GTextOverflowModeWordWrap);
  set_status(STR_STATUS_IDLE);
  layer_add_child(window_layer, text_layer_get_layer(s_prompt_layer));

  // Bottom half: Mabel image
#ifdef PBL_COLOR
  s_splash_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MABEL_DOG_CLR);
#else
  s_splash_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MABEL_DOG_BW);
#endif
#ifdef PBL_ROUND
  s_splash_layer = bitmap_layer_create(GRect(0, STATUS_BAR_LAYER_HEIGHT + half_h, bounds.size.w, content_h - half_h));
#else
  s_splash_layer = bitmap_layer_create(GRect(0, STATUS_BAR_LAYER_HEIGHT + half_h, content_w, content_h - half_h));
#endif
  bitmap_layer_set_bitmap(s_splash_layer, s_splash_bitmap);
  bitmap_layer_set_alignment(s_splash_layer, GAlignCenter);
  bitmap_layer_set_compositing_mode(s_splash_layer, GCompOpSet);
  #ifdef PBL_COLOR
    bitmap_layer_set_background_color(s_splash_layer, GColorBrass);
  #else
    bitmap_layer_set_background_color(s_splash_layer, GColorWhite);
  #endif
  layer_add_child(window_layer, bitmap_layer_get_layer(s_splash_layer));

  // Status bar added before action bar — action bar renders on top in the overlap corner
  s_main_status_bar_layer = create_status_bar(window_layer, bounds.size.w);
  action_bar_layer_add_to_window(s_main_action_bar, window);
}

static void main_window_unload(Window *window) {
  if (s_thinking_timer) {
    app_timer_cancel(s_thinking_timer);
    s_thinking_timer = NULL;
  }
#if defined(PBL_MICROPHONE) && !defined(EMULATOR)
  if (s_dictation_session) {
    dictation_session_destroy(s_dictation_session);
    s_dictation_session = NULL;
  }
#endif
  layer_destroy(s_main_status_bar_layer);
  s_main_status_bar_layer = NULL;
  layer_destroy(s_bubble_layer);
  s_bubble_layer = NULL;
  gpath_destroy(s_bubble_path);
  s_bubble_path = NULL;
  bitmap_layer_destroy(s_splash_layer);
  gbitmap_destroy(s_splash_bitmap);
  text_layer_destroy(s_prompt_layer);
  s_prompt_layer = NULL;
  action_bar_layer_destroy(s_main_action_bar);
}

// ---- Public: return handlers for use in init ----

WindowHandlers main_window_get_handlers(void) {
  return (WindowHandlers){
    .load   = main_window_load,
    .unload = main_window_unload,
  };
}
