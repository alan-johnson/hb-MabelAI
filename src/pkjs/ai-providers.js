// AI provider implementations for hb-claudeAI
// Each provider receives the query, conversation history (common format), and
// a settings object: { apiKey, model, maxTokens, maxHistory }
// Callbacks: onSuccess(replyText), onError(statusMessage)

var BASE_SYSTEM_PROMPT =
  'You are a helpful assistant on a Pebble smartwatch. ' +
  'Responses must be concise (under 250 words) and plain text only — ' +
  'no markdown, no bullet symbols, no special characters. ' +
  'Be direct and informative.';

function buildSystemPrompt(settings) {
  if (settings && settings.systemAddendum) {
    return BASE_SYSTEM_PROMPT + '\n\n' + settings.systemAddendum;
  }
  return BASE_SYSTEM_PROMPT;
}

// ---- Claude (Anthropic) ----

function askClaude(query, history, settings, onSuccess, onError) {
  var messages = settings.maxHistory === 0
    ? [{ role: 'user', content: query }]
    : history;

  var body = JSON.stringify({
    model:      settings.model,
    max_tokens: settings.maxTokens,
    system:     buildSystemPrompt(settings),
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
        var data  = JSON.parse(xhr.responseText);
        var usage = null;
        try { usage = { inputTokens: data.usage.input_tokens, outputTokens: data.usage.output_tokens }; } catch(e) {}
        onSuccess(data.content[0].text, usage);
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
    system_instruction: { parts: [{ text: buildSystemPrompt(settings) }] },
    contents:           contents,
    generationConfig:   { maxOutputTokens: settings.maxTokens }
  });

  var geminiModel = /^gemini/i.test(settings.model) ? settings.model : 'gemini-2.0-flash';
  var url = 'https://generativelanguage.googleapis.com/v1beta/models/' +
    geminiModel + ':generateContent?key=' + settings.apiKey;

  var xhr = new XMLHttpRequest();
  xhr.open('POST', url, true);
  xhr.setRequestHeader('Content-Type', 'application/json');

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
        var data  = JSON.parse(xhr.responseText);
        var usage = null;
        try { usage = { inputTokens: data.usageMetadata.promptTokenCount, outputTokens: data.usageMetadata.candidatesTokenCount }; } catch(e) {}
        onSuccess(data.candidates[0].content.parts[0].text, usage);
      } catch (e) {
        onError('Response parse error.');
      }
    } else {
      console.log('Gemini error ' + xhr.status + ': ' + xhr.responseText);
      var errMsg = 'API error ' + xhr.status + '. Check key/network.';
      try {
        var detail = JSON.parse(xhr.responseText).error.message;
        if (detail) {
          var dot = detail.indexOf('. ');
          errMsg = dot >= 0 ? detail.substring(0, dot + 1) : detail;
        }
      } catch (ex) { /* use default errMsg */ }
      onError(errMsg);
    }
  };
  xhr.onerror = function() {
    if (finished) return;
    finished = true;
    clearTimeout(timer);
    onError('Network error. Check phone connection.');
  };
  xhr.send(body);
}

// ---- ChatGPT (OpenAI) ----

function askChatGPT(query, history, settings, onSuccess, onError) {
  var sysPrompt = buildSystemPrompt(settings);
  var messages = settings.maxHistory === 0
    ? [{ role: 'system', content: sysPrompt }, { role: 'user', content: query }]
    : [{ role: 'system', content: sysPrompt }].concat(history);

  var body = JSON.stringify({
    model:      settings.model,
    max_tokens: settings.maxTokens,
    messages:   messages
  });

  var xhr = new XMLHttpRequest();
  xhr.open('POST', 'https://api.openai.com/v1/chat/completions', true);
  xhr.setRequestHeader('Content-Type',  'application/json');
  xhr.setRequestHeader('Authorization', 'Bearer ' + settings.apiKey);

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
        var data  = JSON.parse(xhr.responseText);
        var usage = null;
        try { usage = { inputTokens: data.usage.prompt_tokens, outputTokens: data.usage.completion_tokens }; } catch(e) {}
        onSuccess(data.choices[0].message.content, usage);
      } catch (e) {
        onError('Response parse error.');
      }
    } else {
      console.log('ChatGPT error ' + xhr.status + ': ' + xhr.responseText);
      onError('API error ' + xhr.status + '. Check key/network.');
    }
  };
  xhr.onerror = function() {
    if (finished) return;
    finished = true;
    clearTimeout(timer);
    onError('Network error. Check phone connection.');
  };
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
