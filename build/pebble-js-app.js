/******/ (function(modules) { // webpackBootstrap
/******/ 	// The module cache
/******/ 	var installedModules = {};
/******/
/******/ 	// The require function
/******/ 	function __webpack_require__(moduleId) {
/******/
/******/ 		// Check if module is in cache
/******/ 		if(installedModules[moduleId])
/******/ 			return installedModules[moduleId].exports;
/******/
/******/ 		// Create a new module (and put it into the cache)
/******/ 		var module = installedModules[moduleId] = {
/******/ 			exports: {},
/******/ 			id: moduleId,
/******/ 			loaded: false
/******/ 		};
/******/
/******/ 		// Execute the module function
/******/ 		modules[moduleId].call(module.exports, module, module.exports, __webpack_require__);
/******/
/******/ 		// Flag the module as loaded
/******/ 		module.loaded = true;
/******/
/******/ 		// Return the exports of the module
/******/ 		return module.exports;
/******/ 	}
/******/
/******/
/******/ 	// expose the modules object (__webpack_modules__)
/******/ 	__webpack_require__.m = modules;
/******/
/******/ 	// expose the module cache
/******/ 	__webpack_require__.c = installedModules;
/******/
/******/ 	// __webpack_public_path__
/******/ 	__webpack_require__.p = "";
/******/
/******/ 	// Load entry module and return exports
/******/ 	return __webpack_require__(0);
/******/ })
/************************************************************************/
/******/ ([
/* 0 */
/***/ (function(module, exports, __webpack_require__) {

	__webpack_require__(1);
	module.exports = __webpack_require__(2);


/***/ }),
/* 1 */
/***/ (function(module, exports) {

	/**
	 * Copyright 2024 Google LLC
	 *
	 * Licensed under the Apache License, Version 2.0 (the "License");
	 * you may not use this file except in compliance with the License.
	 * You may obtain a copy of the License at
	 *
	 *     http://www.apache.org/licenses/LICENSE-2.0
	 *
	 * Unless required by applicable law or agreed to in writing, software
	 * distributed under the License is distributed on an "AS IS" BASIS,
	 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	 * See the License for the specific language governing permissions and
	 * limitations under the License.
	 */
	
	(function(p) {
	  if (!p === undefined) {
	    console.error('Pebble object not found!?');
	    return;
	  }
	
	  // Aliases:
	  p.on = p.addEventListener;
	  p.off = p.removeEventListener;
	
	  // For Android (WebView-based) pkjs, print stacktrace for uncaught errors:
	  if (typeof window !== 'undefined' && window.addEventListener) {
	    window.addEventListener('error', function(event) {
	      if (event.error && event.error.stack) {
	        console.error('' + event.error + '\n' + event.error.stack);
	      }
	    });
	  }
	
	})(Pebble);


/***/ }),
/* 2 */
/***/ (function(module, exports, __webpack_require__) {

	// PebbleKit JS — runs on the phone, bridges Pebble <-> Claude API
	
	var configPage = __webpack_require__(3);
	
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


/***/ }),
/* 3 */
/***/ (function(module, exports) {

	// Configuration page builder for hb-claudeAI
	// Returns a self-contained data URI that opens in the Pebble app's webview.
	
	function getConfigPageUrl(settings) {
	  var apiKey     = settings.apiKey     || '';
	  var model      = settings.model      || 'claude-haiku-4-5-20251001';
	  var maxTokens  = settings.maxTokens  || 300;
	  var maxHistory = settings.maxHistory != null ? settings.maxHistory : 4;
	
	  function esc(str) {
	    return String(str)
	      .replace(/&/g, '&amp;')
	      .replace(/"/g, '&quot;')
	      .replace(/</g, '&lt;')
	      .replace(/>/g, '&gt;');
	  }
	
	  function opt(value, label) {
	    return '<option value="' + value + '"' + (model === value ? ' selected' : '') + '>' + label + '</option>';
	  }
	
	  var histLabel = maxHistory + (maxHistory === 1 ? ' turn' : ' turns');
	
	  var html = [
	    '<!DOCTYPE html><html><head>',
	    '<meta charset="utf-8">',
	    '<meta name="viewport" content="width=device-width,initial-scale=1">',
	    '<title>Claude AI Settings</title>',
	    '<style>',
	    '*{box-sizing:border-box;margin:0;padding:0}',
	    'body{font-family:-apple-system,BlinkMacSystemFont,sans-serif;background:#111;color:#eee;padding:16px 16px 40px}',
	    'h1{font-size:20px;font-weight:700;color:#f90;text-align:center;margin-bottom:20px;letter-spacing:.5px}',
	    '.card{background:#1c1c1c;border-radius:10px;padding:14px 14px 18px;margin-bottom:14px}',
	    'label{display:block;font-size:11px;color:#888;text-transform:uppercase;letter-spacing:.8px;margin-bottom:6px}',
	    '.field{margin-top:14px}',
	    'input[type=password],select{width:100%;padding:9px 10px;background:#2a2a2a;color:#eee;border:1px solid #3a3a3a;border-radius:6px;font-size:15px;-webkit-appearance:none;appearance:none}',
	    'select{background-image:url("data:image/svg+xml,%3Csvg xmlns=\'http://www.w3.org/2000/svg\' width=\'12\' height=\'8\'%3E%3Cpath d=\'M0 0l6 8 6-8z\' fill=\'%23888\'/%3E%3C/svg%3E");background-repeat:no-repeat;background-position:right 10px center;padding-right:28px}',
	    'input[type=range]{width:100%;margin:8px 0 4px;accent-color:#f90}',
	    '.range-row{display:flex;justify-content:space-between;align-items:baseline}',
	    '.range-val{color:#f90;font-size:13px;font-weight:600}',
	    'button{display:block;width:100%;padding:14px;background:#f90;color:#000;font-size:17px;font-weight:700;border:none;border-radius:10px;cursor:pointer;margin-top:20px}',
	    'button:active{background:#d70}',
	    '.hint{font-size:11px;color:#555;margin-top:5px}',
	    '</style></head><body>',
	
	    '<h1>Claude AI Settings</h1>',
	
	    '<div class="card">',
	    '<label>API Key</label>',
	    '<input type="password" id="apiKey" placeholder="sk-ant-api03-..." value="' + esc(apiKey) + '">',
	    '<p class="hint">Found at console.anthropic.com</p>',
	    '</div>',
	
	    '<div class="card">',
	    '<label>Model</label>',
	    '<select id="model">',
	    opt('claude-haiku-4-5-20251001', 'Haiku 4.5 \u2014 Fast'),
	    opt('claude-sonnet-4-6',         'Sonnet 4.6 \u2014 Smart'),
	    opt('claude-opus-4-6',           'Opus 4.6 \u2014 Best'),
	    '</select>',
	
	    '<div class="field">',
	    '<div class="range-row"><label>Max Response Tokens</label><span class="range-val" id="tokVal">' + maxTokens + '</span></div>',
	    '<input type="range" id="maxTokens" min="100" max="600" step="50" value="' + maxTokens + '" oninput="updateTok(this.value)">',
	    '<p class="hint">Higher = longer answers, slower response</p>',
	    '</div>',
	
	    '<div class="field">',
	    '<div class="range-row"><label>Conversation Memory</label><span class="range-val" id="histVal">' + esc(histLabel) + '</span></div>',
	    '<input type="range" id="maxHistory" min="0" max="10" step="1" value="' + maxHistory + '" oninput="updateHist(this.value)">',
	    '<p class="hint">How many back-and-forth exchanges to remember (0 = none)</p>',
	    '</div>',
	    '</div>',
	
	    '<button onclick="save()">Save Settings</button>',
	
	    '<script>',
	    'function updateTok(v){document.getElementById("tokVal").textContent=v;}',
	    'function updateHist(v){document.getElementById("histVal").textContent=v+(v==1?" turn":" turns");}',
	    'function save(){',
	    '  var cfg={',
	    '    apiKey:document.getElementById("apiKey").value,',
	    '    model:document.getElementById("model").value,',
	    '    maxTokens:parseInt(document.getElementById("maxTokens").value),',
	    '    maxHistory:parseInt(document.getElementById("maxHistory").value)',
	    '  };',
	    '  location.href="pebblejs://close#"+encodeURIComponent(JSON.stringify(cfg));',
	    '}',
	    '<\/script>',
	    '</body></html>'
	  ].join('');
	
	  return 'data:text/html,' + encodeURIComponent(html);
	}
	
	module.exports = { getConfigPageUrl: getConfigPageUrl };


/***/ })
/******/ ]);
//# sourceMappingURL=pebble-js-app.js.map