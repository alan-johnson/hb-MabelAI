# hb MabelAI for Pebble

A Pebble watchapp that lets you speak questions to an AI and read the responses on your watch. Supports Claude, Gemini, and ChatGPT.

## Features

- Voice-to-text input using Pebble's built-in dictation (requires mic: Pebble Time, Time Round, Time Steel, 2/2HR)
- Responses displayed in a split view — your request on top, Mabel's answer scrollable below
- Multi-turn conversation memory (configurable, default 4 exchanges)
- Choice of AI provider: Claude, Gemini, or ChatGPT
- In-app settings UI for API keys, model, max tokens, and memory depth
- Usage statistics per AI provider with reset option
- Reminders, timers, and alarms set via AI and pinned to the Rebble timeline
- Current weather conditions and 1-hour forecast (no API key required)
- Token usage tracking per provider
- Supports all Pebble platforms: basalt, chalk, diorite, emery, flint

## Controls

**Main screen**
| Button | Action |
|--------|--------|
| UP | Open help page |
| SELECT (middle) | Start voice dictation |
| DOWN | Open menu |

**Response screen**
| Button | Action |
|--------|--------|
| UP | Scroll response up |
| DOWN | Scroll response down |
| BACK | Return to main screen |

**Menu**
| Item | Description |
|------|-------------|
| Reminders | Upcoming reminders set via AI |
| Timers | Upcoming timers set via AI |
| Alarms | Upcoming alarms set via AI |
| Weather | Current conditions + 1-hour forecast |
| Tokens | Usage vs limit for each AI provider + Rebble pin count |
| About | Version and credits |
| Legal | AI content disclaimer |

## Weather

Weather data is fetched from **[Open-Meteo](https://open-meteo.com)** — a free, open-source weather API that requires no API key.

**What is shown:**
- Current temperature and weather description (Clear, Rain, Snow, etc.)
- Feels-like temperature
- Wind speed and direction
- Relative humidity
- +1 hour forecast (temperature and description)

**Temperature units** are chosen automatically based on the phone's timezone:
- **°F** for the US, its territories, and a small number of other regions
- **°C** everywhere else

**Data source:** [Open-Meteo Forecast API](https://api.open-meteo.com/v1/forecast) using the device's GPS coordinates.

## Setup

### 1. Get an API Key

Obtain a key for whichever provider(s) you want to use:
- **Claude** — https://console.anthropic.com
- **Gemini** — https://aistudio.google.com/apikey
- **ChatGPT** — https://platform.openai.com/api-keys

### 2. Configure API Keys

Open the Pebble app on your phone → tap the gear icon next to hb MabelAI → enter your key(s) and preferred provider in the settings page.

### 3. Build & Install
```bash
pebble build
pebble install --phone <YOUR_PHONE_IP>
```

### Emulator testing

Uncomment `#define EMULATOR` at the top of `src/c/hb-mabelAI-main.c` to bypass the microphone and cycle through fixed test queries. Comment it out again before deploying to a real watch.

## Window flow

```
Main ──UP──────▶ Help
     ──SELECT──▶ Response
     ──DOWN────▶ Menu ──▶ Reminders / Timers / Alarms / Weather / Tokens / About / Legal

BACK at every screen returns progressively to Main.
```

## Architecture

```
[Pebble Watch]
    Voice → DictationSession → transcription
    AppMessage (KEY_QUERY) →
[Phone - PebbleKit JS]
    src/pkjs/index.js        — settings, conversation history, AI dispatch
    src/pkjs/ai-providers.js — Claude / Gemini / ChatGPT implementations
    src/pkjs/config.js       — in-app settings page (data URI HTML)
    → POST to provider API
    ← JSON response + token usage
    AppMessage (KEY_RESPONSE or KEY_STATUS) →
[Pebble Watch]
    Response window — request label (top) + scrollable Mabel answer (bottom)

[Pebble Watch]
    AppMessage (KEY_WEATHER_REQUEST) →
[Phone - PebbleKit JS]
    navigator.geolocation → GPS coordinates
    → GET https://api.open-meteo.com/v1/forecast
    ← JSON (temperature, humidity, wind, weather code, hourly forecast)
    AppMessage (KEY_WEATHER_DATA) →
[Pebble Watch]
    Weather window — current conditions + 1-hour forecast
```

## Models

| Provider | Available models |
|---|---|
| Claude | `claude-haiku-4-5-20251001` (default), `claude-sonnet-4-6`, `claude-opus-4-6` |
| Gemini | `gemini-2.5-flash` (default), `gemini-2.5-flash-lite`, `gemini-2.5-pro` |
| ChatGPT | `gpt-4o-mini` (default), `gpt-4o`, `gpt-4-turbo` |

## Source files

| File | Purpose |
|---|---|
| `src/c/hb-mabelAI.c` | Shared state, AppMessage callbacks, init/deinit |
| `src/c/hb-mabelAI.h` | Shared declarations, color scheme defines, API surface |
| `src/c/hb-mabelAI-main.c` | Main window — splash image, status text, action bar |
| `src/c/hb-mabelAI-results.c` | Response window — request/mabel split layout |
| `src/c/hb-mabelAI-menu.c` | Menu window |
| `src/c/hb-mabelAI-reminders.c` | Reminders list window |
| `src/c/hb-mabelAI-timers.c` | Timers list window |
| `src/c/hb-mabelAI-alarms.c` | Alarms list window |
| `src/c/hb-mabelAI-weather.c` | Weather window |
| `src/c/hb-mabelAI-tokens.c` | Token usage window |
| `src/c/hb-mabelAI-textwindow.c` | Shared scrollable text window (Help, About, Legal) |
| `src/c/hb-strings.h` | All user-visible strings (one place for localization) |
| `src/pkjs/index.js` | PebbleKit JS — settings, usage tracking, AI dispatch, weather |
| `src/pkjs/ai-providers.js` | Per-provider API call implementations |
| `src/pkjs/config.js` | Settings/config page builder (self-contained HTML) |
| `src/pkjs/env-defaults.js` | Build-time defaults generated by `npm run gen-env` |

## Notes

- Responses are truncated to ~1800 characters to fit watch memory.
- Requires Bluetooth connection and the Pebble phone app running.
- Rebble subscription required for voice dictation and timeline pins (reminders, timers, alarms).
- Action bar icons are 16×16 px RGBA PNG.
