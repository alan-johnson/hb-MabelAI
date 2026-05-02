# Clay Migration Plan — hb-MabelAI (revised)

## What Changed From the Previous Plan

The MLB-Live-Face watchface (https://github.com/alan-johnson/MLB-Live-Face) provides a working
reference implementation using `@rebble/clay` with Core Devices. Studying it overturns the main
assumption in the previous plan:

**`clay.generateUrl()` works directly with Core Devices on iOS — no GitHub Pages fallback is needed.**

The MLB app uses `Pebble.openURL(clay.generateUrl())` with no platform detection and no hosted-page
fallback. This means `docs/config.html` and `src/pkjs/config.js` (the URL builder) can both be
deleted. The entire iOS-fallback complexity from the previous plan is removed.

The wscript also needs updating — the older wscript API (`ctx.pbl_build`, `ctx.env = ...`) must be
replaced with the newer API (`ctx.pbl_program`, `ctx.set_env`) for Gabbro platform support to work.

---

## File Changeset Summary

| File | Action |
|---|---|
| `wscript` | **update** — new SDK API (4 changes) |
| `package.json` | **update** — add `@rebble/clay` dep; add `gabbro` to targetPlatforms if not already there |
| `src/pkjs/config.js` | **delete** (URL builder) → **replace** with new Clay config file |
| `src/pkjs/clay-config.js` | **create** — Clay items array (all settings) |
| `src/pkjs/clay-customfn.js` | **create** — manipulator for provider/model/key visibility |
| `src/pkjs/index.js` | **update** — Clay require, new showConfiguration + webviewclosed handlers |
| `docs/config.html` | **delete** — no longer needed |

---

## Step 1 — Update `wscript`

Four changes to the `build()` function. The new API is what the updated Pebble SDK requires to
recognise and compile Gabbro targets correctly.

```python
def build(ctx):
    ctx.load('pebble_sdk')

    build_worker = os.path.exists('worker_src')
    binaries = []

    for p in ctx.env.TARGET_PLATFORMS:
        ctx.set_env(ctx.all_envs[p])                           # was: ctx.env = ctx.all_envs[platform]
        ctx.set_group(ctx.env.PLATFORM_NAME)
        app_elf = '{}/pebble-app.elf'.format(ctx.env.BUILD_DIR)
        ctx.pbl_program(source=ctx.path.ant_glob('src/**/*.c'), target=app_elf)  # was: ctx.pbl_build(..., bin_type='app')

        if build_worker:
            worker_elf = '{}/pebble-worker.elf'.format(ctx.env.BUILD_DIR)
            binaries.append({'platform': p, 'app_elf': app_elf, 'worker_elf': worker_elf})
            ctx.pbl_worker(source=ctx.path.ant_glob('worker_src/**/*.c'), target=worker_elf)  # was: ctx.pbl_build(..., bin_type='worker')
        else:
            binaries.append({'platform': p, 'app_elf': app_elf})

    # Remove cached_env = ctx.env / ctx.env = cached_env — ctx.set_env() handles this
    ctx.set_group('bundle')
    ctx.pbl_bundle(binaries=binaries,
                   js=ctx.path.ant_glob(['src/pkjs/**/*.js', 'src/pkjs/**/*.json']),
                   js_entry_file='src/pkjs/index.js')
```

Changes:
1. `ctx.env = ctx.all_envs[p]` → `ctx.set_env(ctx.all_envs[p])`
2. `ctx.pbl_build(..., bin_type='app')` → `ctx.pbl_program(...)`
3. `ctx.pbl_build(..., bin_type='worker')` → `ctx.pbl_worker(...)`
4. Remove `cached_env = ctx.env` and `ctx.env = cached_env` (not needed with `set_env`)

---

## Step 2 — Install `@rebble/clay`

```bash
npm install --save @rebble/clay
```

v1.0.7 added Gabbro support. v1.0.8 rebuilt with the newer SDK. Use `^1.0.8`.

Also add `"gabbro"` to `targetPlatforms` in `package.json` if not already present.

---

## Step 3 — Create `src/pkjs/clay-config.js`

This replaces the old `src/pkjs/config.js` (URL builder). It is a JS module exporting the Clay
items array — following the exact same pattern as MLB-Live-Face's `config.js`.

All 8 settings map to Clay item types. The model `select` starts with an empty options array;
the manipulator (Step 4) populates it based on the selected provider.

```js
module.exports = [
  {
    "type": "heading",
    "defaultValue": "hb MabelAI Settings"
  },
  {
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "AI Provider", "size": 4 },
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
  {
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "API Keys", "size": 4 },
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
  {
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Model & Limits", "size": 4 },
      {
        "type": "select",
        "messageKey": "model",
        "label": "Model",
        "defaultValue": "claude-haiku-4-5-20251001",
        "options": []
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
  {
    "type": "section",
    "items": [
      { "type": "heading", "defaultValue": "Alarm Behavior", "size": 4 },
      {
        "type": "toggle",
        "messageKey": "alarmVibrate",
        "label": "Vibrate until dismissed",
        "description": "Alarms vibrate until you press a button. Quiet Time is always respected."
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];
```

---

## Step 4 — Create `src/pkjs/clay-customfn.js`

The manipulator handles two things the static config cannot: showing only the relevant API key
field for the active provider, and populating the model dropdown with provider-specific options.

```js
var MODELS = {
  claude: [
    { label: 'Haiku 4.5 — Fast',   value: 'claude-haiku-4-5-20251001' },
    { label: 'Sonnet 4.6 — Smart', value: 'claude-sonnet-4-6'         },
    { label: 'Opus 4.6 — Best',    value: 'claude-opus-4-6'           }
  ],
  gemini: [
    { label: 'Gemini 2.5 Flash — Default', value: 'gemini-2.5-flash'      },
    { label: 'Gemini 2.5 Flash — Fast',    value: 'gemini-2.5-flash-lite' },
    { label: 'Gemini 2.5 Pro — Best',      value: 'gemini-2.5-pro'        }
  ],
  chatgpt: [
    { label: 'GPT-4o Mini — Fast', value: 'gpt-4o-mini'  },
    { label: 'GPT-4o — Smart',     value: 'gpt-4o'       },
    { label: 'GPT-4 Turbo — Best', value: 'gpt-4-turbo'  }
  ]
};

module.exports = function(minifiedClay) {
  var clayConfig = this;

  function syncToProvider(provider) {
    ['claude', 'gemini', 'chatgpt'].forEach(function(p) {
      var item = clayConfig.getItemByMessageKey(p + 'ApiKey');
      if (item) { p === provider ? item.show() : item.hide(); }
    });
    var modelItem = clayConfig.getItemByMessageKey('model');
    if (modelItem) { modelItem.set('options', MODELS[provider] || []); }
  }

  clayConfig.on('showConfiguration', function() {
    var provider = clayConfig.getItemByMessageKey('provider').get() || 'claude';
    syncToProvider(provider);
  });

  clayConfig.getItemByMessageKey('provider').on('change', function() {
    syncToProvider(this.get());
  });
};
```

---

## Step 5 — Update `src/pkjs/index.js`

### Add at the top (replacing the old config require)

```js
// Replace:  var configPage = require('./config');
var Clay       = require('@rebble/clay');
var clayConfig = require('./clay-config');
var customFn   = require('./clay-customfn');
var clay       = new Clay(clayConfig, customFn, { autoHandleEvents: false });
```

### Replace `showConfiguration`

```js
// Before (2 lines using configPage):
Pebble.addEventListener('showConfiguration', function() {
  Pebble.openURL(configPage.getConfigPageUrl(loadSettings(), loadUsage()));
});

// After (1 line — Clay generates the URL directly, works on iOS and Android):
Pebble.addEventListener('showConfiguration', function() {
  Pebble.openURL(clay.generateUrl());
});
```

### Replace `webviewclosed`

Clay's response format wraps some values in `{ value: ... }` objects. Normalize them before
passing to `saveSettings()`, following the exact same pattern as MLB-Live-Face:

```js
Pebble.addEventListener('webviewclosed', function(e) {
  if (!e || !e.response) {
    console.log('No configuration received.');
    return;
  }
  // false = do not auto-save to localStorage; we use saveSettings() instead
  var raw = clay.getSettings(e.response, false);
  var settings = {};
  Object.keys(raw).forEach(function(key) {
    settings[key] = (raw[key] !== null && typeof raw[key] === 'object' && raw[key].hasOwnProperty('value'))
      ? raw[key].value
      : raw[key];
  });
  if (settings.resetUsage) { resetUsageStats(); }
  saveSettings(settings);
  console.log('Settings saved: ' + JSON.stringify(settings));
  armNextAlarmWakeup();
  var name = PROVIDER_NAMES[settings.provider] || 'AI';
  sendToWatch(KEY_STATUS, 'Settings saved! Press MIC to ask ' + name + '.');
});
```

---

## Step 6 — Delete the Old Files

- `src/pkjs/config.js` — the GitHub Pages URL builder; replaced by `clay-config.js`
- `docs/config.html` — the hosted page; no longer needed

---

## Usage Statistics Display

The current `docs/config.html` shows per-provider request and token counts. Clay does not have a
built-in item type for displaying dynamic read-only data. Options, in order of simplicity:

1. **Omit from the Clay config** (simplest). Usage tracking in `localStorage` continues to work;
   users view stats via the watch's Tokens menu screen, which already shows this data.
2. **Clay `text` item populated by the manipulator** on `showConfiguration`. Works on Android
   (pkjs and Clay page share the same localStorage context). Unreliable on iOS.
3. **Custom Clay component** — a self-contained HTML snippet registered with
   `clay.registerCustomComponents()`. Significant extra complexity.

Recommendation: go with option 1 for the initial migration. The Tokens screen on the watch
already surfaces this information. The Reset Stats action can be kept as a `button` item added
to the Clay config:

```json
{ "type": "button", "primary": false, "label": "Reset Usage Stats", "messageKey": "resetUsage" }
```

When clicked, `settings.resetUsage === true` in `webviewclosed`, which the handler already checks.

---

## Testing Checklist

- [ ] `npm install` — installs `@rebble/clay`, no errors
- [ ] `npm run build:prod` — all platform binaries present including `gabbro/`
- [ ] Config page opens from the watch settings menu
- [ ] Switching provider shows only that provider's API key field
- [ ] Switching provider updates the model dropdown to that provider's models
- [ ] Sliders (`maxTokens`, `maxHistory`) save and reload correctly
- [ ] `alarmVibrate` toggle saves and reloads correctly
- [ ] After save: watch shows "Settings saved! Press MIC to ask Claude." (or active provider)
- [ ] Reset Stats button clears usage counters
