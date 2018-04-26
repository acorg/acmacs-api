$(document).ready(function() {
    const url = new URL(document.location);
    // console.log(url);
    $("body").append("<p>pathname: " + url.pathname + "</p>");
    (new Set(url.searchParams.keys())).forEach(key => {
        $("body").append("<p>" + key + ": " + url.searchParams.getAll(key) + "</p>");
    });
    // var websocket = new WebSocket("ws://" + url.host + "/wss2");
    // websocket.onopen = evt => console.log("websocket opened", evt);
    // websocket.onclose = evt => console.log("websocket closed", evt);
    // websocket.onmessage = evt => console.log("websocket message", evt);
    // websocket.onerror = evt => console.log("websocket error", evt);
});
