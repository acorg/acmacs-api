export function url_prefix() {
    const url = new URL(document.location);
    if (/^\/ads\//.test(url.pathname))
        return "/ads/";
    else
        return "/";
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
