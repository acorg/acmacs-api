$(document).ready(() => {
    make_header();
    run();
});

// ----------------------------------------------------------------------

function run() {
    import("./dispatcher.js").then(dispatcher_m => {
        window.acmacs_api_dispatcher = new dispatcher_m.Dispatcher();
    });
}

// ----------------------------------------------------------------------

function make_header() {
    $("body").append("<div class='acmacs-web-header'><div class='acmacs-web-title'>Acmacs-Web</div><div class='acmacs-web-user'></div><div style='clear: both;'></div></div>");
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
