// Configuration page URL builder for hb-MabelAI
//
// iOS 14+ blocks data: URIs in WKWebView, so we use a hosted GitHub Pages URL.
// Settings and usage stats are passed via the URL hash (fragment) so API keys
// are never sent to any server.
//
// Setup: set CONFIG_PAGE_BASE_URL to your deployed GitHub Pages URL.
// See docs/config.html for the hosted page.

var CONFIG_PAGE_BASE_URL = 'https://alan-johnson.github.io/hb-MabelAI/config.html';

function getConfigPageUrl(settings, usage) {
  var payload = encodeURIComponent(JSON.stringify({
    settings: settings || {},
    usage:    usage    || {}
  }));
  return CONFIG_PAGE_BASE_URL + '#' + payload;
}

module.exports = { getConfigPageUrl: getConfigPageUrl };
