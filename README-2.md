# Claude AI for Pebble

A Pebble watchapp that lets you speak questions to Claude AI and read the responses on your watch.

## Features
- Voice-to-text input using Pebble's built-in dictation (requires mic: Pebble Time, Time Round, Time Steel, 2/2HR)
- Responses displayed in a scrollable text view
- Multi-turn conversation memory (last 4 exchanges)
- Works on all Pebble platforms

## Controls
| Button | Action |
|--------|--------|
| SELECT (middle) | Start voice dictation |
| UP | Scroll response up |
| DOWN | Scroll response down |

## Setup

### 1. Get a Claude API Key
Sign up at https://console.anthropic.com and create an API key.

### 2. Add Your API Key
Open `src/pkjs/index.js` and replace:
```js
var CLAUDE_API_KEY = "YOUR_CLAUDE_API_KEY_HERE";
```
API Key = 
```
REDACTED_API_KEY
```

### 3. Add Icon Images
Place three 18x18 PNG images in `resources/images/`:
- `icon_mic.png`
- `icon_up.png`
- `icon_down.png`

### 4. Build & Install
```bash
cd pebble-claude
pebble build
pebble install --phone <YOUR_PHONE_IP>
```

## Architecture
```
[Pebble Watch]
    Voice → DictationSession → text
    AppMessage (KEY_QUERY) →
[Phone - PebbleKit JS]
    → POST https://api.anthropic.com/v1/messages
    ← JSON response
    AppMessage (KEY_RESPONSE) →
[Pebble Watch]
    Display in ScrollLayer
```

## Notes
- Model is set to `claude-haiku-4-5-20251001` for speed. Change to `claude-sonnet-4-6` for smarter responses.
- Responses truncated to ~1800 chars to fit watch memory.
- Requires Bluetooth + phone internet connection.
