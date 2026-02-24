# Getting Started with Mabel AI

Mabel is an AI assistant for your Pebble smartwatch. Ask questions, get answers, and set reminders — all from your wrist.

---

## What You'll Need

| Item | Notes |
|------|-------|
| Pebble smartwatch | Pebble Time, Pebble Time Round, Pebble Time 2, or Pebble 2 |
| iPhone or Android phone | With Bluetooth |
| Rebble app + subscription | Free app; ~$3/month subscription required for voice |
| AI API key | At least one provider (Gemini has a free tier — see below) |

---

## Step 1 — Set Up Rebble

Rebble is the community-run replacement for Pebble's servers. You need it to connect your watch to your phone and to use voice dictation.

1. **Install the Rebble app** on your phone from the App Store or Google Play.
2. **Create a free Rebble account** at [rebble.io](https://rebble.io).
3. **Subscribe to Rebble** (~$3/month) to enable voice dictation on the watch.
4. Open the Rebble app and follow the setup wizard to pair your Pebble watch.

> If your watch was previously paired to the original Pebble app, the Rebble setup wizard will walk you through migrating.

---

## Step 2 — Install Mabel

1. Open the Rebble app on your phone.
2. Go to the **App Store** tab and search for **"hb Mabel AI"**.
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

1. On your phone, open the Rebble app.
2. Go to **My Pebble** → **Apps** → find **hb Mabel AI** → tap the **settings gear**.
3. Choose your **AI Provider** (Gemini, Claude, or ChatGPT).
4. Paste your **API key** into the corresponding field.
5. Tap **Save** — your watch will confirm "Settings saved!"

---

## Step 5 — Ask Your First Question

1. On your watch, open the **Mabel** app.
2. Press the **middle button (MIC)** to start listening.
3. Speak your question clearly and wait a moment for the dictation to complete.
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

1. From the Mabel main screen, press the **bottom button (MENU)**.
2. Select **Reminders**.
3. Your upcoming reminders are listed in chronological order.
4. Use **UP/DOWN** to scroll. Press **BACK** to return.

> **Note:** The Reminders list shows reminders created by Mabel since you installed the app. You can also view all timeline events by pressing **UP** or **DOWN** from your watch face.

---

## Troubleshooting

**"No API key. Open settings."**
You haven't entered an API key yet, or the wrong provider is selected. Go to Step 4.

**Dictation fails or never completes**
Your Rebble subscription may have lapsed, or your phone's Bluetooth connection dropped. Check that the Rebble app is running in the background and your subscription is active.

**"Phone not connected. Open the Pebble app."**
The Rebble app is not running or Bluetooth is off. Open the Rebble app on your phone and make sure Bluetooth is enabled.

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
