#include "hb-claudeAI.h"

// ---- Private layer state ----
static BitmapLayer    *s_splash_layer;
static GBitmap        *s_splash_bitmap;
static TextLayer      *s_prompt_layer;
static ActionBarLayer *s_main_action_bar;

// ---- Public: update prompt / status text ----

void set_status(const char *msg) {
  if (s_prompt_layer) {
    text_layer_set_text(s_prompt_layer, msg);
  }
}

// ---- Voice Dictation ----

#ifdef PBL_MICROPHONE
static void dictation_session_callback(DictationSession *session,
                                        DictationSessionStatus status,
                                        char *transcription,
                                        void *context) {
  if (status == DictationSessionStatusSuccess) {
    set_status("Thinking...");
    s_is_loading = true;
    DictionaryIterator *out_iter;
    AppMessageResult result = app_message_outbox_begin(&out_iter);
    if (result == APP_MSG_OK) {
      dict_write_cstring(out_iter, KEY_QUERY, transcription);
      app_message_outbox_send();
    } else {
      set_status("Send error. Try again.");
      s_is_loading = false;
    }
  } else {
    set_status("Dictation failed. Try again.");
    s_is_loading = false;
  }
  dictation_session_destroy(session);
}

static void start_dictation(void) {
  if (s_is_loading) return;
  set_status("Listening...");
  DictationSession *session = dictation_session_create(512,
                                                        dictation_session_callback,
                                                        NULL);
  dictation_session_start(session);
}
#endif

// ---- Click handlers ----

static void main_select_click_handler(ClickRecognizerRef recognizer, void *context) {
#ifdef PBL_MICROPHONE
  start_dictation();
#else
  set_status("No mic on this model.");
#endif
}

static void main_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, main_select_click_handler);
}

// ---- Window lifecycle ----

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect  bounds       = layer_get_bounds(window_layer);

  // Action bar — mic icon on SELECT only
  s_main_action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(s_main_action_bar, window);
  action_bar_layer_set_click_config_provider(s_main_action_bar, main_click_config_provider);
  action_bar_layer_set_icon(s_main_action_bar, BUTTON_ID_SELECT, s_icon_mic);

  int content_w = bounds.size.w - ACTION_BAR_WIDTH;
  int half_h    = bounds.size.h / 2;

  // Top half: splash image
  s_splash_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CAPT_CLAUDE);
  s_splash_layer  = bitmap_layer_create(GRect(0, 0, content_w, half_h));
  bitmap_layer_set_bitmap(s_splash_layer, s_splash_bitmap);
  bitmap_layer_set_alignment(s_splash_layer, GAlignCenter);
  //bitmap_layer_set_compositing_mode(s_splash_layer, GCompOpAssign);
  bitmap_layer_set_compositing_mode(s_splash_layer, GCompOpSet);
  bitmap_layer_set_background_color(s_splash_layer, GColorDarkGray);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_splash_layer));

  // Bottom half: user prompt / status text
  s_prompt_layer = text_layer_create(GRect(4, half_h + 4, content_w - 8, half_h - 8));
  text_layer_set_font(s_prompt_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_color(s_prompt_layer, GColorBlack);
  text_layer_set_background_color(s_prompt_layer, GColorClear);
  text_layer_set_text_alignment(s_prompt_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(s_prompt_layer, GTextOverflowModeWordWrap);
  set_status("Press MIC to ask Mabel");
  layer_add_child(window_layer, text_layer_get_layer(s_prompt_layer));
}

static void main_window_unload(Window *window) {
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
