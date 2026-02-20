// PebbleKit JS — runs on the phone, bridges Pebble <-> Claude API

var configPage = require('./config');

// Message key constants (must match hb-claudeAI.c)
var KEY_QUERY    = 0;
var KEY_RESPONSE = 1;
var KEY_STATUS   = 2;

// ---- Settings ----

var DEFAULT_SETTINGS = {
  apiKey:     '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',
  model:      'claude-haiku-4-5-20251001',
  maxTokens:  300,
  maxHistory: 4
};

function loadSettings() {
  var maxTokens  = parseInt(localStorage.getItem('maxTokens'));
  var maxHistory = parseInt(localStorage.getItem('maxHistory'));
  return {
    apiKey:     localStorage.getItem('apiKey')  || DEFAULT_SETTINGS.apiKey,
    model:      localStorage.getItem('model')   || DEFAULT_SETTINGS.model,
    maxTokens:  isNaN(maxTokens)  ? DEFAULT_SETTINGS.maxTokens  : maxTokens,
    maxHistory: isNaN(maxHistory) ? DEFAULT_SETTINGS.maxHistory : maxHistory
  };
}

function saveSettings(settings) {
  localStorage.setItem('apiKey',     settings.apiKey);
  localStorage.setItem('model',      settings.model);
  localStorage.setItem('maxTokens',  settings.maxTokens);
  localStorage.setItem('maxHistory', settings.maxHistory);
}

// ---- Utility ----

function truncateForWatch(text, maxChars) {
  maxChars = maxChars || 1800;
  if (text.length <= maxChars) return text;
  return text.substring(0, maxChars - 3) + '...';
}

function sendToWatch(key, value) {
  Pebble.sendAppMessage(
    { [key]: value },
    function() { console.log('Message sent to watch OK'); },
    function(e) { console.log('Failed to send to watch: ' + JSON.stringify(e)); }
  );
}

// ---- Claude API ----

var conversationHistory = [];

function askClaude(userQuery) {
  var settings = loadSettings();

  if (!settings.apiKey) {
    sendToWatch(KEY_STATUS, 'No API key. Open settings.');
    return;
  }

  sendToWatch(KEY_STATUS, 'Thinking...');

  // Build message list, respecting conversation memory setting
  var messages;
  if (settings.maxHistory === 0) {
    messages = [{ role: 'user', content: userQuery }];
  } else {
    conversationHistory.push({ role: 'user', content: userQuery });
    // Each "turn" = 1 user + 1 assistant message
    while (conversationHistory.length > settings.maxHistory * 2) {
      conversationHistory.shift();
    }
    messages = conversationHistory;
  }

  var requestBody = JSON.stringify({
    model:      settings.model,
    max_tokens: settings.maxTokens,
    system: 'You are a helpful assistant on a Pebble smartwatch. ' +
            'Responses must be concise (under 250 words) and plain text only — ' +
            'no markdown, no bullet symbols, no special characters. ' +
            'Be direct and informative.',
    messages: messages
  });

  var xhr = new XMLHttpRequest();
  xhr.open('POST', 'https://api.anthropic.com/v1/messages', true);
  xhr.setRequestHeader('Content-Type',      'application/json');
  xhr.setRequestHeader('x-api-key',         settings.apiKey);
  xhr.setRequestHeader('anthropic-version', '2023-06-01');

  xhr.onload = function() {
    if (xhr.status === 200) {
      try {
        var response = JSON.parse(xhr.responseText);
        var reply    = response.content[0].text;
        if (settings.maxHistory > 0) {
          conversationHistory.push({ role: 'assistant', content: reply });
        }
        sendToWatch(KEY_RESPONSE, truncateForWatch(reply));
      } catch (e) {
        console.log('Parse error: ' + e);
        sendToWatch(KEY_STATUS, 'Response parse error.');
      }
    } else {
      console.log('API error: ' + xhr.status + ' ' + xhr.responseText);
      sendToWatch(KEY_STATUS, 'API error ' + xhr.status + '. Check key/network.');
    }
  };

  xhr.onerror = function() {
    console.log('Network error calling Claude API');
    sendToWatch(KEY_STATUS, 'Network error. Check phone connection.');
  };

  xhr.send(requestBody);
}

// ---- PebbleKit JS Events ----

Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready');
  var settings = loadSettings();
  if (!settings.apiKey) {
    sendToWatch(KEY_STATUS, 'No API key. Open settings.');
  }
  // TEMP: auto-fire a test query after 2 seconds
  setTimeout(function() { askClaude('What is the capital of France?'); }, 2000);
});

Pebble.addEventListener('showConfiguration', function() {
  Pebble.openURL(configPage.getConfigPageUrl(loadSettings()));
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (!e.response) return;
  try {
    var settings = JSON.parse(decodeURIComponent(e.response));
    saveSettings(settings);
    console.log('Settings saved: ' + JSON.stringify(settings));
    sendToWatch(KEY_STATUS, 'Settings saved! Press MIC to ask Claude.');
  } catch (err) {
    console.log('Failed to parse config response: ' + err);
  }
});

Pebble.addEventListener('appmessage', function(e) {
  var payload = e.payload;
  console.log('Received from watch: ' + JSON.stringify(payload));
  if (payload[KEY_QUERY] !== undefined) {
    askClaude(payload[KEY_QUERY]);
  }
});
