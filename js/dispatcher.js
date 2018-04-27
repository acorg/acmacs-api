export class Dispatcher {

    constructor() {
        const url = new URL(document.location);
        this.websocket_ = new WebSocket("wss://" + url.host + "/acmacs-api");
        this.websocket_.onopen = evt => this.onopen(evt);
        this.websocket_.onclose = evt => this.onclose(evt);
        this.websocket_.onmessage = evt => this.expect_hello(evt);
        this.websocket_.onerror = evt => this.onerror(evt);
        this.login_process_ = false;
        this.command_queue_ = []; // when waiting for login
        this.commands_sent_ = {};
        this.command_id_ = 1;
        this.setup_handlers_(url);
    }

    setup_handlers_(url) {
        this.handlers_ = {
        };
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
            if (this.login_process_) {
                this.command_queue_.push(data);
            }
            else {
                if (data.S === undefined) {
                    const session = this.store_session_();
                    if (!session) {
                        this.command_queue_.push(data);
                        this.login();
                    }
                    else {
                        data.S = session;
                        if (data.D === undefined) {
                            data.D = data.C + "#" + this.command_id_;
                            ++this.command_id_;
                            this.commands_sent_[data.D] = data;
                        }
                        this.websocket_.send(JSON.stringify(data));
                    }
                }
            }
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
        if (!message.E) {
            delete this.commands_sent_[message.D];
            const handler_name = message.D || message.C;
            if (this.handlers_[handler_name])
                this.handlers_[handler_name](message, this);
            else
                console.warn("Dispatcher.onmessage: unhandled", handler_name, this.handlers_, message);
        }
        else
            this.handle_error(message);
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

    handle_error(message) {
        switch (message.E) {
        case "no session":
            this.command_queue_.push(this.commands_sent_[message.D]);
            this.login();
            break;
        default:
            console.error(message);
            break;
        }
    }

    login() {
        this.login_process_ = true;
        this.login_widget_ = new LoginWidget(this);
    }

    store_session_(session) {
        const storage_key = "acmacs-d-session-id";
        try {
            if (!session)
                return window.localStorage.getItem(storage_key);
            else if (session === "remove")
                return window.localStorage.removeItem(storage_key);
            else
                return window.localStorage.setItem(storage_key, session);
        }
        catch(e) {
            return null;
        }
    }
}

// ----------------------------------------------------------------------

const LoginWidget_html = "\
<div class='login box-shadow-popup' id='login'>\
  <div class='title'>Acmacs-Web</div>\
  <form>\
    <div name='username-label'>Username</div>\
    <input autocomplete=username spellcheck=false tabIndex=1 name=username type=email></input>\
    <div class='separator' name=username></div>\
    <div name='password-label'>Password</div>\
    <input autocomplete=password spellcheck=false tabIndex=2 name=password type=password></input>\
    <div class='separator' name=password></div>\
    <div class='button box-shadow-button'>Login</div>\
    <div class='error-message'></div>\
  </form>\
</div>\
";

const LoginWidget_css = "\
<style name='login-widget'>\
  #login.box-shadow-popup { box-shadow: 0 2px 2px 0 rgba(0,0,0,0.14),0 3px 1px -2px rgba(0,0,0,0.12),0 1px 5px 0 rgba(0,0,0,0.2); }\
  #login .box-shadow-button {\
    border-radius: 3px;\
    box-shadow: 0 2px 2px 0 rgba(0,0,0,0.14),0 3px 1px -2px rgba(0,0,0,0.12),0 1px 5px 0 rgba(0,0,0,0.2);\
    transition: box-shadow .28s cubic-bezier(0.4,0.0,0.2,1);\
  }\
\
\
  #login { width: 20em; height: 20em; padding: 2em;\
          position: absolute; top: 0; bottom: 0; left: 0; right: 0; margin: auto;}\
  #login .title { font: 32px Helvetica,Arial,sans-serif; font-weight: bold; margin-bottom: 2em; color: $aw-orange; }\
  #login input { border: none; width: 100%; font: 400 16px Helvetica,Arial,sans-serif; }\
  #login input:focus { outline: none; }\
  #login div.separator { background-color: rgba(0,0,0,0.12); height: 1px; width: 100%; margin-bottom: 2em };\
  #login div.separator-focused { background-color: #54964e; height: 2px; transition: 0.2s; }\
  #login div.label-focused { color: #54964e; transition: 0.2s; }\
  #login .button { margin-left: auto; width: 4em; background-color: #54964e; color: white; font-size: 1.2em; padding: 0.2em 0.2em; text-align: center; font-weight: bold; cursor: pointer; }\
  #login .error-message { color: red; margin-top: 1em; }\
</style>\
";

class LoginWidget {

    constructor(dispatcher) {
        if ($("head").find("style[name='login-widget']").length === 0)
            $("head").append(LoginWidget_css);
        this.div = $(LoginWidget_html).appendTo($("body"));
        let username_input = this.div.find("input[name=username]");
        let username_separator = this.div.find("div.separator[name=username]");
        let username_label = this.div.find("div[name='username-label']");
        username_input.on("focus", () => {
            username_input.select();
            username_separator.addClass("separator-focused");
            username_label.addClass("label-focused");
        });
    }

    destroy() {
        this.div.remove();
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
