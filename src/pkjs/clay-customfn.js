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

  // Items are not accessible until after clayConfig.build() runs.
  // AFTER_BUILD is the earliest point where getItemByMessageKey() works.
  clayConfig.on('AFTER_BUILD', function() {
    var provider = clayConfig.getItemByMessageKey('provider').get() || 'claude';
    syncToProvider(provider);

    clayConfig.getItemByMessageKey('provider').on('change', function() {
      syncToProvider(this.get());
    });
  });
};
