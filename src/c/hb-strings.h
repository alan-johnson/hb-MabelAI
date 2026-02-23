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

// ---- Confirm window ----
#define STR_CONFIRM_TITLE      "Send to Mabel?"
#define STR_CONFIRM_HINT       "SELECT: send  BACK: cancel"

// ---- Results window section labels ----
#define STR_SECTION_REQUEST    "request"
#define STR_SECTION_MABEL      "mabel"

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
