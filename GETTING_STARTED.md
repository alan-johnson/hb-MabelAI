# Getting Started with Mabel AI

Mabel is an AI assistant for your Pebble smartwatch. Ask questions, get answers, and set reminders, alarms and timers — all from your wrist.

---

## What You'll Need

| Item | Notes |
|------|-------|
| Pebble smartwatch | Pebble Time, Pebble Time Round, or Pebble 2 Duo. Will support the Pebble Time 2 Round and Pebble Time 2 when release.|
| iPhone or Android phone | With Bluetooth |
| Pebble app + subscription | Free app; ~$3/month subscription required for voice |
| AI API key | At least one provider (Gemini has a free tier — see below) |

---

## Step 1 — Set Up Rebble

Rebble is the community-run replacement for Pebble's servers. You need it to connect your watch to your phone and to use voice dictation.

1. **Install the Pebble app** on your phone from the App Store or Google Play.
2. **Create a free Rebble account** at [rebble.io](https://rebble.io).
3. **Subscribe to Rebble** (~$3/month) to enable voice dictation on the watch.
4. Open the Pebble app and follow the setup wizard to pair your Pebble watch.

> If your watch was previously paired to the original Pebble app, the Rebble setup wizard will walk you through migrating.

---

## Step 2 — Install Mabel

1. Open the Pebble app on your phone.
2. Go to the **App Store** tab and search for **"hb MabelAI"**.
3. Tap **Install** — the app will appear on your watch within a few seconds.

---

## Step 3 — Get an AI API Key

Mabel works with three AI providers. You only need one key to get started.

### Gemini (Google) — Free Tier Available ✓

The easiest option if you want to start for free.

1. Go to [aistudio.google.com](https://aistudio.google.com).
2. Sign in with your Google account.
3. Click **Get API key** → **Create API key**.
4. Copy the key.

Gemini's free tier is generous and sufficient for typical watch use.

### Claude (Anthropic)

1. Go to [console.anthropic.com](https://console.anthropic.com).
2. Create an account and add a payment method.
3. Go to **API Keys** → **Create Key**.
4. Copy the key.

### ChatGPT (OpenAI)

1. Go to [platform.openai.com](https://platform.openai.com).
2. Create an account and add a payment method.
3. Go to **API Keys** → **Create new secret key**.
4. Copy the key.

---

## Step 4 — Configure Mabel

1. On your phone, open the Pebble app.
2. Go to **Apps** → find **hb MabelAI** → tap the **settings gear**.
3. Choose your **AI Provider** (Gemini, Claude, or ChatGPT).
4. Paste your **API key** into the corresponding field.
5. Tap **Save** — your watch will confirm "Settings saved!"

---

## Step 5 — Ask Your First Question

1. On your watch, open the **hb MabelAI** app.
2. Click the **middle button (MIC)** to start listening.
3. Speak your question clearly then click the **SELECT** button when done. Wait a moment for the dictation to complete.
4. Review the transcription on screen.
5. Press **SELECT** (middle button with the checkmark) to send it to Mabel.
6. Mabel's answer will appear on the response screen. Use **UP/DOWN** to scroll.
7. Press **BACK** to return to the main screen.

> **Tip:** Keep questions short and specific. "What's the capital of France?" works better than long, multi-part questions.

---

## Step 6 — Set a Reminder

Ask Mabel naturally, just like you would ask a person:

- *"Remind me to take my medication tomorrow at 9am"*
- *"Set a reminder to call Mom on Friday at 3pm"*
- *"Remind me about the meeting in 2 hours"*

Mabel will confirm the reminder and create a pin in your Pebble timeline. The reminder will appear as a notification on your watch at the scheduled time.

---

## Step 7 — View Your Reminders

1. From the hb MabelAI main screen, press the **bottom button (MENU)**.
2. Select **Reminders**.
3. Your upcoming reminders are listed in chronological order.
4. Use **UP/DOWN** to scroll. Press **BACK** to return.

> **Note:** The Reminders list shows reminders created by Mabel since you installed the app. You can also view all timeline events by pressing **UP** or **DOWN** from your watch face.

---

## Step 8 — Set a Timer

Ask Mabel naturally:

- *"Set a timer for 10 minutes"*
- *"Timer for 30 seconds"*
- *"Set a pasta timer for 12 minutes"*

Mabel will confirm the timer and create a pin in your Pebble timeline. The timer will fire as a notification when time is up.

**View your timers:**

1. Press the **bottom button (MENU)** from the main screen.
2. Select **Timers**.
3. Your upcoming timers are listed in order. Press **BACK** to return.

---

## Step 9 — Set an Alarm

Ask Mabel naturally:

- *"Set an alarm for 7am tomorrow"*
- *"Wake me up at 6:30"*
- *"Alarm for Monday at 8am"*

Mabel will confirm the alarm and pin it to your timeline. It will appear as a notification at the scheduled time.

**View your alarms:**

1. Press the **bottom button (MENU)** from the main screen.
2. Select **Alarms**.
3. Your upcoming alarms are listed in order. Press **BACK** to return.

---

## Step 10 — Check the Weather

Mabel can show you current weather conditions using your phone's GPS location — no API key required.

1. Press the **bottom button (MENU)** from the main screen.
2. Select **Weather**.
3. The weather screen shows:
   - Current temperature and conditions (Clear, Rain, Snow, etc.)
   - Feels-like temperature
   - Wind speed and direction
   - Relative humidity
   - +1 hour forecast

> **Temperature units** are chosen automatically — °F if your phone's timezone is in the US or its territories, °C everywhere else.

---

## Step 11 — Track Token Usage

Each AI provider tracks how many tokens you've used. You can view usage and reset the counter from the watch.

1. Press the **bottom button (MENU)** from the main screen.
2. Select **Tokens**.
3. The screen shows usage for each provider and your Rebble pin count.

This is handy for keeping an eye on API costs, especially on paid tiers.

---

## Troubleshooting

**"No API key. Open settings."**
You haven't entered an API key yet, or the wrong provider is selected. Go to Step 4.

**Dictation fails or never completes**
Your Rebble subscription may have lapsed, or your phone's Bluetooth connection dropped. Check that the Pebble app is running in the background and your subscription is active.

**"Phone not connected. Open the Pebble app."**
The Pebble app is not running or Bluetooth is off. Open the Pebble app on your phone and make sure Bluetooth is enabled.

**"API error 401"**
Your API key is invalid or has been revoked. Generate a new key and re-enter it in settings.

**"Timed out. Try again."**
The AI provider took too long to respond. This is usually temporary — try again in a moment.

**Reminder didn't appear on the timeline**
Timeline pins sync periodically. Navigate to the timeline on your watch (press UP or DOWN from the watch face) to trigger a sync. It may take a few minutes to appear.

---

## Quick Reference

| Button | Action |
|--------|--------|
| **MIC** (middle) | Start voice input |
| **SELECT** (middle, on confirm screen) | Send query to Mabel |
| **BACK** | Cancel / return to previous screen |
| **UP** (main screen) | Open Help |
| **DOWN** (main screen) | Open Menu (Reminders, etc.) |
| **UP / DOWN** (response screen) | Scroll answer |
