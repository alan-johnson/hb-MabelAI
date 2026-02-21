// AI provider implementations for hb-claudeAI
// Each provider receives the query, conversation history (common format), and
// a settings object: { apiKey, model, maxTokens, maxHistory }
// Callbacks: onSuccess(replyText), onError(statusMessage)

var SYSTEM_PROMPT =
  'You are a helpful assistant on a Pebble smartwatch. ' +
  'Responses must be concise (under 250 words) and plain text only — ' +
  'no markdown, no bullet symbols, no special characters. ' +
  'Be direct and informative.';

// ---- Claude (Anthropic) ----

function askClaude(query, history, settings, onSuccess, onError) {
  var messages = settings.maxHistory === 0
    ? [{ role: 'user', content: query }]
    : history;

  var body = JSON.stringify({
    model:      settings.model,
    max_tokens: settings.maxTokens,
    system:     SYSTEM_PROMPT,
    messages:   messages
  });

  var xhr = new XMLHttpRequest();
  xhr.open('POST', 'https://api.anthropic.com/v1/messages', true);
  xhr.setRequestHeader('Content-Type',      'application/json');
  xhr.setRequestHeader('x-api-key',         settings.apiKey);
  xhr.setRequestHeader('anthropic-version', '2023-06-01');

  // Manual timeout — xhr.timeout is not reliable in PebbleKit JS.
  var finished = false;
  var timer = setTimeout(function() {
    if (finished) return;
    finished = true;
    xhr.abort();
    onError('Timed out. Try again.');
  }, 30000);

  xhr.onreadystatechange = function() {
    if (xhr.readyState !== 4 || finished) return;
    finished = true;
    clearTimeout(timer);
    if (xhr.status === 200) {
      try {
        onSuccess(JSON.parse(xhr.responseText).content[0].text);
      } catch (e) {
        onError('Response parse error.');
      }
    } else {
      onError('API error ' + xhr.status + '. Check key/network.');
    }
  };
  xhr.onerror = function() {
    if (finished) return;
    finished = true;
    clearTimeout(timer);
    onError('Network error. Check connection.');
  };
  xhr.send(body);
}

// ---- Gemini (Google) ----
// History format: common {role:'user'|'assistant'} → Gemini {role:'user'|'model'}

function askGemini(query, history, settings, onSuccess, onError) {
  var contents = settings.maxHistory === 0
    ? [{ role: 'user', parts: [{ text: query }] }]
    : history.map(function(msg) {
        return {
          role:  msg.role === 'assistant' ? 'model' : 'user',
          parts: [{ text: msg.content }]
        };
      });

  var body = JSON.stringify({
    system_instruction: { parts: [{ text: SYSTEM_PROMPT }] },
    contents:           contents,
    generationConfig:   { maxOutputTokens: settings.maxTokens }
  });

  var url = 'https://generativelanguage.googleapis.com/v1beta/models/' +
    settings.model + ':generateContent?key=' + settings.apiKey;

  var xhr = new XMLHttpRequest();
  xhr.open('POST', url, true);
  xhr.setRequestHeader('Content-Type', 'application/json');

  xhr.onload = function() {
    if (xhr.status === 200) {
      try {
        onSuccess(JSON.parse(xhr.responseText).candidates[0].content.parts[0].text);
      } catch (e) {
        onError('Response parse error.');
      }
    } else {
      onError('API error ' + xhr.status + '. Check key/network.');
    }
  };
  xhr.onerror = function() { onError('Network error. Check phone connection.'); };
  xhr.send(body);
}

// ---- ChatGPT (OpenAI) ----

function askChatGPT(query, history, settings, onSuccess, onError) {
  var messages = settings.maxHistory === 0
    ? [{ role: 'system', content: SYSTEM_PROMPT }, { role: 'user', content: query }]
    : [{ role: 'system', content: SYSTEM_PROMPT }].concat(history);

  var body = JSON.stringify({
    model:      settings.model,
    max_tokens: settings.maxTokens,
    messages:   messages
  });

  var xhr = new XMLHttpRequest();
  xhr.open('POST', 'https://api.openai.com/v1/chat/completions', true);
  xhr.setRequestHeader('Content-Type',  'application/json');
  xhr.setRequestHeader('Authorization', 'Bearer ' + settings.apiKey);

  xhr.onload = function() {
    if (xhr.status === 200) {
      try {
        onSuccess(JSON.parse(xhr.responseText).choices[0].message.content);
      } catch (e) {
        onError('Response parse error.');
      }
    } else {
      onError('API error ' + xhr.status + '. Check key/network.');
    }
  };
  xhr.onerror = function() { onError('Network error. Check phone connection.'); };
  xhr.send(body);
}

// ---- Dispatcher ----

function ask(provider, query, history, settings, onSuccess, onError) {
  if (provider === 'gemini') {
    askGemini(query, history, settings, onSuccess, onError);
  } else if (provider === 'chatgpt') {
    askChatGPT(query, history, settings, onSuccess, onError);
  } else {
    askClaude(query, history, settings, onSuccess, onError);
  }
}

module.exports = { ask: ask };
