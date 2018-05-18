import * as acv_utils from "../map-draw/ace-view-1/utils.js";

// ----------------------------------------------------------------------

export function url_prefix() {
    const url = new URL(document.location);
    if (/^\/ads\//.test(url.pathname))
        return "/ads/";
    else
        return "/";
}

window.acv_url_prefix = url_prefix();

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
