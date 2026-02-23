# hb Mabel AI for Pebble

A Pebble watchapp that lets you speak questions to an AI and read the responses on your watch. Supports Claude, Gemini, and ChatGPT.

## Features

- Voice-to-text input using Pebble's built-in dictation (requires mic: Pebble Time, Time Round, Time Steel, 2/2HR)
- Confirmation window to review your transcription before sending
- Responses displayed in a split view — your request on top, Mabel's answer scrollable below
- Menu window for Reminders, Timers, Alarms, About, and Legal (stubs, forthcoming)
- Multi-turn conversation memory (configurable, default 4 exchanges)
- Usage statistics per AI provider with reset option
- Choice of AI provider: Claude, Gemini, or ChatGPT
- In-app settings UI for API keys, model, max tokens, and memory depth
- Supports Pebble platforms: basalt, chalk, diorite, emery

## Controls

**Main screen**
| Button | Action |
|--------|--------|
| UP | Open help page |
| SELECT (middle) | Start voice dictation |
| DOWN | Open menu |

**Confirm screen** *(review transcription before sending)*
| Button | Action |
|--------|--------|
| SELECT | Send to Mabel |
| BACK | Cancel — return to main screen |

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

**Menu screen**
| Button | Action |
|--------|--------|
| UP / DOWN | Navigate menu items |
| SELECT | Select item (stubs — no action yet) |
| BACK | Return to main screen |

## Setup

### 1. Get an API Key

Obtain a key for whichever provider(s) you want to use:
- **Claude** — https://console.anthropic.com
- **Gemini** — https://aistudio.google.com/apikey
- **ChatGPT** — https://platform.openai.com/api-keys

### 2. Configure API Keys

**Option A — Environment variables (recommended, keeps keys out of source)**

Set one or more `HB_*` variables, then generate the defaults file before building:
```bash
export HB_PROVIDER=gemini
export HB_GEMINI_API_KEY=AIza...
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
| `HB_MAX_TOKENS` | Max response tokens (0 = provider default) |
| `HB_MAX_HISTORY` | Conversation memory turns (0 = none, -1 = unlimited) |

**Option B — In-app settings**

Open the Pebble app on your phone → tap the gear icon next to hb Mabel AI → enter your key(s) and preferred provider in the settings page.

### 3. Build & Install
```bash
pebble build
pebble install --phone <YOUR_PHONE_IP>
```

### Emulator testing

Uncomment `#define EMULATOR` at the top of `src/c/hb-mabelAI-main.c` to bypass the microphone and send a fixed test query through the full confirm → response flow. Comment it out again before deploying to a real watch.

## Window flow

```
Main ──UP──────▶ Help
     ──SELECT──▶ Confirm ──SELECT──▶ Response
     ──DOWN────▶ Menu

BACK at every screen returns progressively to Main.
```

## Architecture

```
[Pebble Watch]
    Voice → DictationSession → transcription
    Confirm window → user presses SELECT
    AppMessage (KEY_QUERY) →
[Phone - PebbleKit JS]
    src/pkjs/index.js        — settings, conversation history, dispatch
    src/pkjs/ai-providers.js — Claude / Gemini / ChatGPT implementations
    src/pkjs/config.js       — in-app settings page (data URI HTML)
    → POST to provider API
    ← JSON response + token usage
    AppMessage (KEY_RESPONSE or KEY_STATUS) →
[Pebble Watch]
    Response window — request label (top) + scrollable Mabel answer (bottom)
```

## Models

| Provider | Available models |
|---|---|
| Claude | `claude-haiku-4-5-20251001` (default), `claude-sonnet-4-6`, `claude-opus-4-6` |
| Gemini | `gemini-2.5-flash`, `gemini-2.0-flash`, `gemini-1.5-flash`, `gemini-1.5-pro` |
| ChatGPT | `gpt-4o-mini`, `gpt-4o`, `gpt-4-turbo` |

## Source files

| File | Purpose |
|---|---|
| `src/c/hb-mabelAI.c` | Shared state, AppMessage callbacks, init/deinit |
| `src/c/hb-mabelAI.h` | Shared declarations and API surface |
| `src/c/hb-mabelAI-main.c` | Main window — splash image, status text, action bar |
| `src/c/hb-mabelAI-confirm.c` | Confirm window — transcription review before sending |
| `src/c/hb-mabelAI-results.c` | Response window — request/mabel split layout |
| `src/c/hb-mabelAI-help.c` | Help window — scrollable instructions |
| `src/c/hb-mabelAI-menu.c` | Menu window — Reminders, Timers, Alarms, About, Legal |
| `src/c/hb-strings.h` | All user-visible strings (one place for localization) |
| `src/pkjs/index.js` | PebbleKit JS — settings, usage tracking, AI dispatch |
| `src/pkjs/ai-providers.js` | Per-provider API call implementations |
| `src/pkjs/config.js` | Settings/config page builder (self-contained HTML) |
| `src/pkjs/env-defaults.js` | Build-time defaults generated by `npm run gen-env` |

## Notes

- Responses are truncated to ~1800 characters to fit watch memory.
- Requires Bluetooth connection and Pebble phone app running.
- Action bar icons are 16×16 px RGBA PNG: `icon_mic`, `icon_up`, `icon_down`, `icon_help`, `icon_menu`.
