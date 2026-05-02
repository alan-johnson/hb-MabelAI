module.exports = [
  {
    "type": "heading",
    "defaultValue": "hb MabelAI Settings"
  },

  // ── AI Provider ────────────────────────────────────────────────────────────
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "AI Provider",
        "size": 4
      },
      {
        "type": "radiogroup",
        "messageKey": "provider",
        "label": "Provider",
        "defaultValue": "claude",
        "options": [
          { "label": "Claude",  "value": "claude"  },
          { "label": "Gemini",  "value": "gemini"  },
          { "label": "ChatGPT", "value": "chatgpt" }
        ]
      }
    ]
  },

  // ── API Keys ───────────────────────────────────────────────────────────────
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "API Keys",
        "size": 4
      },
      {
        "type": "input",
        "messageKey": "claudeApiKey",
        "label": "Claude API Key",
        "attributes": { "placeholder": "sk-ant-api03-..." }
      },
      {
        "type": "input",
        "messageKey": "geminiApiKey",
        "label": "Gemini API Key",
        "attributes": { "placeholder": "AIza..." }
      },
      {
        "type": "input",
        "messageKey": "chatgptApiKey",
        "label": "ChatGPT API Key",
        "attributes": { "placeholder": "sk-..." }
      }
    ]
  },

  // ── Model & Limits ─────────────────────────────────────────────────────────
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Model & Limits",
        "size": 4
      },
      {
        "type": "select",
        "messageKey": "model",
        "label": "Model",
        "defaultValue": "claude-haiku-4-5-20251001",
        "options": [
          { "label": "Haiku 4.5 — Fast",   "value": "claude-haiku-4-5-20251001" },
          { "label": "Sonnet 4.6 — Smart", "value": "claude-sonnet-4-6"         },
          { "label": "Opus 4.6 — Best",    "value": "claude-opus-4-6"           }
        ]
      },
      {
        "type": "slider",
        "messageKey": "maxTokens",
        "label": "Max Response Tokens",
        "defaultValue": 300,
        "min": 100,
        "max": 600,
        "step": 50
      },
      {
        "type": "slider",
        "messageKey": "maxHistory",
        "label": "Conversation Memory (turns)",
        "defaultValue": 4,
        "min": 0,
        "max": 10,
        "step": 1
      }
    ]
  },

  // ── Alarm Behavior ─────────────────────────────────────────────────────────
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Alarm Behavior",
        "size": 4
      },
      {
        "type": "toggle",
        "messageKey": "alarmVibrate",
        "label": "Vibrate until dismissed",
        "description": "Alarms vibrate until you press a button. Quiet Time is always respected.",
        "defaultValue": false
      }
    ]
  },

  // ── Submit ─────────────────────────────────────────────────────────────────
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];
