#pragma once
// hb-strings.h — all user-visible strings in one place for easy localization.
// To translate, replace the values below and rebuild.

// ---- Main window ----
#define STR_STATUS_IDLE        "Press MIC to ask Mabel"
#define STR_STATUS_LISTENING   "Listening..."
#define STR_STATUS_THINKING    "Thinking"
#define STR_STATUS_NO_MIC      "No mic on this model."
#define STR_STATUS_NO_PHONE    "Phone not connected. Open the Pebble app."

// ---- Error messages ----
#define STR_ERR_SEND           "Send error. Try again."
#define STR_ERR_DICTATION      "Dictation failed. Try again."
#define STR_ERR_MSG_DROPPED    "Message dropped. Try again."
#define STR_ERR_OUTBOX_FAILED  "Send failed. Try again."

// ---- Results window section labels ----
#define STR_SECTION_REQUEST    "request"
#define STR_SECTION_MABEL      "mabel"

// ---- Reminders window ----
#define STR_SECTION_REMINDERS  "reminders"
#define STR_NO_REMINDERS       "No reminders yet.\n\nAsk Mabel to set one!"

// ---- Timers window ----
#define STR_SECTION_TIMERS     "timers"

// ---- Alarms window ----
#define STR_SECTION_ALARMS     "alarms"

// ---- Menu window ----
#define STR_MENU_ITEM_REMINDERS  "Reminders"
#define STR_MENU_ITEM_TIMERS     "Timers"
#define STR_MENU_ITEM_ALARMS     "Alarms"
#define STR_MENU_ITEM_ABOUT      "About"
#define STR_MENU_ITEM_LEGAL      "Legal"

// ---- About window ----
#define STR_ABOUT_TEXT \
  "Mabel AI\n\n" \
  "Version 1.0\n\n" \
  "Your AI assistant on your wrist.\n\n" \
  "More details coming soon."

// ---- Legal window ----
#define STR_LEGAL_TEXT \
  "Terms & Licenses\n\n" \
  "Legal information and third-party license notices will appear here.\n\n" \
  "More details coming soon."

// ---- Help window ----
#define STR_HELP_TEXT \
  "How to use Mabel:\n\n" \
  "MIC (middle):\n" \
  "Start listening. Review the transcription, then press SELECT to send.\n\n" \
  "MENU (bottom):\n" \
  "Reminders, Timers, Alarms, and more.\n\n" \
  "HELP (top):\n" \
  "This screen.\n\n" \
  "Response screen:\n" \
  "UP/DOWN: scroll answer.\n" \
  "BACK: return to previous screen.\n\n" \
  "Tips:\n" \
  "- Keep questions short and clear.\n" \
  "- Phone must be connected.\n" \
  "- Change AI provider in Settings."
