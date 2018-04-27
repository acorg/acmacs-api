export class Dispatcher {

    constructor() {
        const url = new URL(document.location);
        this.websocket_ = new WebSocket("wss://" + url.host + "/acmacs-api");
        this.websocket_.onopen = evt => this.onopen(evt);
        this.websocket_.onclose = evt => this.onclose(evt);
        this.websocket_.onmessage = evt => this.expect_hello(evt);
        this.websocket_.onerror = evt => this.onerror(evt);
        this.setup_handlers_(url);
    }

    setup_handlers_(url) {
        this.handlers_ = {};
        switch (url.pathname) {
        case "/list_commands":
        case "/list-commands":
        case "/lc":
            this.handlers_.hello = list_commands;
            break;
        case "/chains":
            this.handlers_.hello = chains;
            break;
        }
    }

    handle(name, handler) {
        this.handlers_[name] = handler;
    }

    send(data) {
        if (typeof(data) === "object" && data.C) {
            this.websocket_.send(JSON.stringify(data));
        }
        else if (typeof(data) === "string" && data.indexOf('"C":') >= 0) {
            this.websocket_.send(data);
        }
        else {
            throw "invalid data passed to Dispatcher.send: " + JSON.stringify(data);
        }
    }

    expect_hello(evt) {
        this.websocket_.onmessage = evt => this.onmessage(evt);
        if (this.handlers_.hello)
            this.handlers_.hello(JSON.parse(evt.data), this);
        else
            console.warn("Dispatcher: no hello handler", evt);
    }

    onmessage(evt) {
        const message = JSON.parse(evt.data);
        const handler_name = message.D || message.C;
        if (this.handlers_[handler_name])
            this.handlers_[handler_name](message, this);
        else
            console.warn("Dispatcher.onmessage: unhandled", handler_name, this.handlers_, message);
    }

    onopen(evt) {
        console.log("websocket opened", evt);
    }

    onclose(evt) {
        console.log("websocket closed", evt);
    }

    onerror(evt) {
        console.log("websocket error", evt);
    }
}

// ----------------------------------------------------------------------

function list_commands(data, dispatcher) {
    if (data.D) {
        console.log(data);
        var ol = $("<ol></ol>").appendTo($("body"));
        data.commands.forEach(entry => {
            ol.append(`<li><span style="font-weight: bold">${entry.name}</span><pre style="margin-top: 0;">${entry.description}</pre></li>\n`);
        });
    }
    else {
        const message_name = "LC";
        dispatcher.handle(message_name, list_commands);
        dispatcher.send({C: "list_commands", D: message_name});
    }
}

// ----------------------------------------------------------------------

function chains(data, dispatcher) {
    if (data.D) {
        console.log(data);
    }
    else {
        const message_name = "chains";
        dispatcher.handle(message_name, chains);
        dispatcher.send({C: "chains", D: message_name, types: ["incremental"]});
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
