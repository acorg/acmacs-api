$(document).ready(() => {
    run();
    // show_document_location();
    //test_websocket();
});

// ----------------------------------------------------------------------

function run() {
    import("./dispatcher.js").then(dispatcher_m => {
        window.acmacs_api_dispatcher = new dispatcher_m.Dispatcher();
    });
}

// function test_websocket() {
//     const url = new URL(document.location);
//     var websocket = new WebSocket("wss://" + url.host + "/acmacs-api");
//     websocket.onopen = evt => console.log("websocket opened", evt);
//     websocket.onclose = evt => console.log("websocket closed", evt);
//     var num_messages = 0;
//     websocket.onmessage = evt => {
//         ++num_messages;
//         const message = JSON.parse(evt.data);
//         console.log("websocket message", message, num_messages);
//         if (message.hello) {
//             websocket.send('{"C": "version"}');
//         }
//         else if (message.C === "version") {
//             websocket.send('{"C": "list_commands"}');
//         }
//     }
//     websocket.onerror = evt => console.log("websocket error", evt);
// }

// // ----------------------------------------------------------------------

// function show_document_location() {
//     const url = new URL(document.location);
//     // console.log(url);
//     $("body").append("<p>pathname: " + url.pathname + "</p>");
//     (new Set(url.searchParams.keys())).forEach(key => {
//         $("body").append("<p>" + key + ": " + url.searchParams.getAll(key) + "</p>");
//     });
// }

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
