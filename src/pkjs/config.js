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
