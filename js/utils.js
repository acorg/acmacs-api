import * as acv_utils from "../map-draw/ace-view/201805/utils.js";

// ----------------------------------------------------------------------

export function url_prefix() {
    const url = new URL(document.location);
    if (/^\/aw-chains\//.test(url.pathname))
        return "/aw-chains/";
    else
        return "/";
}

window.acv_url_prefix = url_prefix();

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
