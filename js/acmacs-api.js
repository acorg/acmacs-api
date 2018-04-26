$(document).ready(() => {
    show_document_location();
    test_websocket();
});

// ----------------------------------------------------------------------

function test_websocket() {
    const url = new URL(document.location);
    var websocket = new WebSocket("wss://" + url.host + "/acmacs-api");
    websocket.onopen = evt => console.log("websocket opened", evt);
    websocket.onclose = evt => console.log("websocket closed", evt);
    websocket.onmessage = evt => console.log("websocket message", evt);
    websocket.onerror = evt => console.log("websocket error", evt);
}

// ----------------------------------------------------------------------

function show_document_location() {
    const url = new URL(document.location);
    // console.log(url);
    $("body").append("<p>pathname: " + url.pathname + "</p>");
    (new Set(url.searchParams.keys())).forEach(key => {
        $("body").append("<p>" + key + ": " + url.searchParams.getAll(key) + "</p>");
    });
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
