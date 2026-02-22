# hb-claudeAI for Pebble

A Pebble watchapp that lets you speak questions to an AI and read the responses on your watch. Supports Claude, Gemini, and ChatGPT.

## Features
- Voice-to-text input using Pebble's built-in dictation (requires mic: Pebble Time, Time Round, Time Steel, 2/2HR)
- Responses displayed in a scrollable text view
- Multi-turn conversation memory (configurable, default 4 exchanges)
- Choice of AI provider: Claude, Gemini, or ChatGPT
- In-app settings UI for API keys, model, and memory
- Works on all Pebble platforms (basalt, chalk, diorite, emery, flint)

## Controls

**Main screen**
| Button | Action |
|--------|--------|
| SELECT (middle) | Start voice dictation |
| UP | Open help page |

**Response screen**
| Button | Action |
|--------|--------|
| UP | Scroll response up |
| DOWN | Scroll response down |
| BACK | Return to main screen |

**Help screen**
| Button | Action |
|--------|--------|
| UP / DOWN | Scroll help text |
| BACK | Return to main screen |

## Setup

### 1. Get an API Key
Obtain a key for whichever provider(s) you want to use:
- **Claude** — https://console.anthropic.com
    API key = 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
- **Gemini** — https://aistudio.google.com/apikey
    API key = AIzaSyCjOfhC7F1hkH8Eq3d1fft6BsclrOQCoWQ
- **ChatGPT** — https://platform.openai.com/api-keys

### 2. Configure API Keys

**Option A — Environment variables (recommended, keeps keys out of source)**

Set one or more `HB_*` variables, then generate the defaults file before building:
```bash
export HB_PROVIDER=claude
export HB_CLAUDE_API_KEY=sk-ant-...
npm run gen-env
pebble build
```

Supported variables:

| Variable | Description |
|---|---|
| `HB_PROVIDER` | Active provider: `claude`, `gemini`, or `chatgpt` |
| `HB_CLAUDE_API_KEY` | Anthropic API key |
| `HB_GEMINI_API_KEY` | Google Gemini API key |
| `HB_CHATGPT_API_KEY` | OpenAI API key |
| `HB_MODEL` | Model ID (must match the provider) |
| `HB_MAX_TOKENS` | Max response tokens |
| `HB_MAX_HISTORY` | Conversation memory turns (0 = none) |

**Option B — In-app settings**

Open the Pebble app on your phone → tap the gear icon next to hb-claudeAI → enter your key(s) and preferred provider in the settings page.

### 3. Build & Install
```bash
pebble build
pebble install --phone <YOUR_PHONE_IP>
```

## Architecture
```
[Pebble Watch]
    Voice → DictationSession → text
    AppMessage (KEY_QUERY) →
[Phone - PebbleKit JS]
    src/pkjs/index.js       — settings, conversation history, dispatch
    src/pkjs/ai-providers.js — Claude / Gemini / ChatGPT implementations
    src/pkjs/config.js      — in-app settings page (data URI HTML)
    → POST to provider API
    ← JSON response
    AppMessage (KEY_RESPONSE) →
[Pebble Watch]
    Display in ScrollLayer
```

## Models

| Provider | Model IDs |
|---|---|
| Claude | `claude-haiku-4-5-20251001` (default), `claude-sonnet-4-6`, `claude-opus-4-6` |
| Gemini | `gemini-2.0-flash` (default), `gemini-1.5-flash`, `gemini-1.5-pro` |
| ChatGPT | `gpt-4o-mini` (default), `gpt-4o`, `gpt-4-turbo` |

## Notes
- Responses are truncated to ~1800 chars to fit watch memory.
- Requires Bluetooth + phone internet connection.
- Icon images (`icon_mic.png`, `icon_up.png`, `icon_down.png`, `icon_help.png`) are 16×16 px, 1-bit grayscale.
