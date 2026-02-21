#pragma once
// hb-strings.h — all user-visible strings in one place for easy localization.
// To translate, replace the values below and rebuild.

// ---- Main window ----
#define STR_STATUS_IDLE        "Press MIC to ask Mabel"
#define STR_STATUS_LISTENING   "Listening..."
#define STR_STATUS_THINKING    "Thinking..."
#define STR_STATUS_NO_MIC      "No mic on this model."
#define STR_STATUS_NO_PHONE    "Phone not connected. Open the Pebble app."

// ---- Error messages ----
#define STR_ERR_SEND           "Send error. Try again."
#define STR_ERR_DICTATION      "Dictation failed. Try again."
#define STR_ERR_MSG_DROPPED    "Message dropped. Try again."
#define STR_ERR_OUTBOX_FAILED  "Send failed. Try again."

// ---- Help window ----
#define STR_HELP_TEXT \
  "How to use Mabel:\n\n" \
  "1. Press the MIC button (middle) to start listening.\n\n" \
  "2. Speak your question clearly.\n\n" \
  "3. Wait for Mabel to reply. This may take a few seconds.\n\n" \
  "4. Use the UP/DOWN buttons to scroll through long answers.\n\n" \
  "5. Press BACK to return to the main screen.\n\n" \
  "Tips:\n" \
  "- Keep questions short and clear.\n" \
  "- Make sure your phone is connected and the Pebble app is open.\n" \
  "- Change the AI provider in Settings."
