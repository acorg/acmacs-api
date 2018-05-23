import * as acv_utils from "../map-draw/ace-view-1/utils.js";
import * as acv_toolkit from "../map-draw/ace-view-1/toolkit.js";
import "./md5.js";

// ----------------------------------------------------------------------

export class Dispatcher {

    constructor() {
        const url = new URL(document.location);
        this.websocket_url_ = `wss://${url.host}/acmacs-api`;
        this.websocket_reconnection_delay_ = 5 * 1000;
        this.login_process_ = false;
        this.session_ = this.store_value_("session-id");
        this.user_ = this.store_value_("user");
        this.user_name_ = this.store_value_("user_name");
        this.command_queue_ = []; // when waiting for login
        this.commands_sent_ = {};
        this.command_id_ = 1;
        this.setup_handlers_();
        this.modules = {};
        this.reconnect();
    }

    setup_handlers_() {
        this.handlers_ = {
            "ERROR#no session": msg => this.no_session(msg),
            "ERROR#invalid session": msg => this.invalid_session(msg),
            "ERROR#could not parse Object ID string": msg => this.invalid_object_id(msg),
            "ERROR#invalid user or password": msg => this.invalid_user(msg),
            "ERROR#": msg => this.default_error_handler(msg),
            login_nonce: msg => this.login_nonce_response(msg),
            login_digest: msg => this.login_digest_response(msg),
            login_session: msg => this.login_session_response(msg),
            hello: msg => this.hello_handler(msg),

            list_commands: list_commands,
            chains: {import: "./chain.js", func: "chains"},
            doc: doc_info
        };

        const url_pathname_fields = new URL(document.location).pathname.replace(/^\/ads/, "").split("/");
        // console.log("url_pathname_fields", url_pathname_fields);
        switch (url_pathname_fields[1]) {
        case "list_commands":
        case "list-commands":
        case "lc":
            this.command_on_hello_ = {C: "list_commands"};
            break;
        case "chains":
            switch(url_pathname_fields[2]) {
            case "whocc":
                this.command_on_hello_ = {C: "chains", keywords: ["whocc"]};
                break;
            default:
                this.command_on_hello_ = {C: "chains"}; // types: ["incremental"]
                break;
            }
            break;
        case "doc":
            this.command_on_hello_ = {C: "doc", id: url_pathname_fields[2], chain_source_data: true};
            break;
        case "chain":
            if (url_pathname_fields.length > 3)
                this.command_on_hello_ = {C: "doc", id: url_pathname_fields[2], chain_source_data: false, add_to_response: {step_no: url_pathname_fields[3]}};
            else
                this.command_on_hello_ = {C: "doc", id: url_pathname_fields[2], chain_source_data: true};
            break;
        default:
            this.command_on_hello_ = {C: "chains", keywords: ["whocc"]};
            break;
        }
    }

    handle(name, handler) {
        this.handlers_[name] = handler;
    }

    send(data) {
        // console.log("send", data);
        if (typeof(data) === "object" && data.C) {
            this.make_D_(data);
            this.commands_sent_[data.D] = data;
            if (this.login_process_ && data.C.substr(0, 6) != "login_")
                this.command_queue_.push(data);
            else
                this.websocket_.send(JSON.stringify(data));
        }
        else if (typeof(data) === "string" && data.indexOf('"C":') >= 0) {
            this.websocket_.send(data);
        }
        else {
            throw "invalid data passed to Dispatcher.send: " + JSON.stringify(data);
        }
    }

    send_receive(data, handler) {
        if (typeof(data) !== "object" || !data.C)
            throw "invalid data passed to Dispatcher.send_receive: " + JSON.stringify(data);
        this.make_D_(data);
        this.handle(data.D, handler);
        this.send(data);
    }

    async send_receive_async(data) {
        if (typeof(data) !== "object" || !data.C)
            throw "invalid data passed to Dispatcher.send_receive: " + JSON.stringify(data);
        this.make_D_(data);
        const result = new Promise(resolve => this.handle(data.D, resolve));
        this.send(data);
        return result;
    }

    make_D_(data) {
        if (typeof(data) === "object" && data.C && data.D === undefined) {
            data.D = data.C + "#" + this.command_id_;
            ++this.command_id_;
        }
    }

    process_command_queue() {
        // console.log("process_command_queue", this.command_queue_);
        while (this.command_queue_.length > 0 && !this.login_process_) {
            let cmd = this.command_queue_.shift();
            this.send(cmd);
        }
    }

    hello_handler(evt) {
        this.login_session();
        // console.log("command_on_hello_", this.command_on_hello_);
        if (this.command_on_hello_) {
            this.send(this.command_on_hello_);
            this.command_on_hello_ = null;
        }
        else if (this.command_on_hello_ === undefined) {
            console.warn("Dispatcher.hello_handler: no command_on_hello_");
        }
    }

    invoke_handler(key1, key2, message) {
        let handler = this.handlers_[key1] || this.handlers_[key2] || (key2 && key2.substr(0, 6) === "ERROR#" ? this.handlers_["ERROR#"] : undefined);
        if (handler) {
            if (typeof(handler) === "function") {
                handler(message, this);
            }
            else if (typeof(handler) === "object" && handler.import && handler.func) {
                import(handler.import).then(mod => mod[handler.func](message, this));
            }
            else {
                console.warn("Dispatcher.invoke_handler: handler not supported: ", JSON.stringify([key1, key2]), message);
            }
            if (key1 && key1.indexOf("#") > 0)
                delete this.handlers_[key1];
        }
        else
            console.warn("Dispatcher.invoke_handler: handler not found: ", JSON.stringify([key1, key2]), message);
    }

    // ----------------------------------------------------------------------
    // WebSocket
    // ----------------------------------------------------------------------

    reconnect() {
        this.websocket_ = new WebSocket(this.websocket_url_);
        this.websocket_.onopen = evt => this.onopen(evt);
        this.websocket_.onclose = evt => this.onclose(evt);
        this.websocket_.onmessage = evt => this.onmessage(evt);
        this.websocket_.onerror = evt => this.onerror(evt);
    }

    onmessage(evt) {
        try {
            const message = JSON.parse(evt.data);
            if (!message.E) {
                delete this.commands_sent_[message.D];
                this.invoke_handler(message.D, message.C, message);
            }
            else {
                this.invoke_handler(null, "ERROR#" + message.E, message);
            }
        }
        catch (err) {
            console.error(err, evt, evt.data);
        }
    }

    onopen(evt) {
        // console.log("connected", evt);
    }

    onclose(evt) {
        console.log("websocket closed", evt);
        window.setTimeout(() => this.reconnect(), (this.websocket_closed_time_ && (new Date() - this.websocket_closed_time_) < this.websocket_reconnection_delay_) ? this.websocket_reconnection_delay_ : 0);
        this.websocket_closed_time_ = new Date();
    }

    onerror(evt) {
        // console.error("websocket error", evt);
    }

    // ----------------------------------------------------------------------

    default_error_handler(message) {
        acv_toolkit.movable_window_with_error(message, "center", "ERROR");
    }

    invalid_object_id(message) {
        if (message.C.substr(0, 6) === "login_") {
            this.invalid_session(message);
            this.show_login_widget();
        }
        else
            console.warn("Dispatcher.invalid_object_id: unhandled ERROR#", message.E, message);
    }

    // ----------------------------------------------------------------------
    // Login

    no_session(message) {
        this.command_queue_.push(this.commands_sent_[message.D]);
        this.show_login_widget();
    }

    invalid_session(message) {
        this.session_ = undefined;
        this.user_ = undefined;
        this.user_name_ = undefined;
        this.store_value_("session-id", "#remove");
        this.store_value_("user", "#remove");
        this.store_value_("user_name", "#remove");
        this.show_login_widget();
    }

    invalid_user(message) {
        if (this.login_widget_)
            this.login_widget_.error(message.E);
        else
            console.error(message);
    }

    show_login_widget() {
        this.login_process_ = true;
        if (this.login_widget_ === undefined)
            this.login_widget_ = new LoginWidget(this);
    }

    initiate_login(user, password) {
        this.user_ = user;
        this.password_ = password;
        this.send({C: "login_nonce", S: "login", user: user});
    }

    login_nonce_response(message) {
        const cnonce = Math.floor(Math.random() * 0xFFFFFFFF).toString(16);
        const digest = md5(message.login_nonce + ";" + cnonce + ";" + md5(this.user_ + ";acmacs-web;" + this.password_));
        delete this.password_;
        this.send({C: "login_digest", S: "login", cnonce: cnonce, digest: digest});
    }

    login_digest_response(message) {
        this.login_widget_.destroy();
        delete this.login_widget_;
        this.session_ = message.S;
        this.user_ = message.user;
        this.user_name_ = message.display_name;
        this.store_value_("session-id", this.session_);
        this.store_value_("user", this.user_);
        this.store_value_("user_name", this.user_name_);
        this.logged_in();
    }

    login_session() {
        if (this.session_) {
            this.login_process_ = true;
            this.send({C: "login_session", S: this.session_});
        }
    }

    login_session_response(message) {
        this.logged_in();
    }

    logged_in() {
        this.login_process_ = false;
        $("body .acmacs-web-header .acmacs-web-user").empty().append(this.user_name_);
        this.process_command_queue();
    }

    store_value_(key, value) {
        key = "acmacs-d-" + key;
        try {
            if (!value)
                return window.localStorage.getItem(key);
            else if (value === "#remove")
                return window.localStorage.removeItem(key);
            else
                return window.localStorage.setItem(key, value);
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
    <div class='button box-shadow-button' name='login-button'>Login</div>\
    <div class='error-message'></div>\
  </form>\
</div>\
";

class LoginWidget {

    constructor(dispatcher) {
        this.div = $(LoginWidget_html).appendTo($("body"));

        let username_input = this.div.find("input[name=username]");
        let username_separator = this.div.find("div.separator[name=username]");
        let username_label = this.div.find("div[name='username-label']");
        let password_input = this.div.find("input[name=password]");
        let password_separator = this.div.find("div.separator[name=password]");
        let password_label = this.div.find("div[name='password-label']");
        let login_button = this.div.find("div[name='login-button']");

        username_input.on("focus", evt => {
            username_input.select();
            username_separator.addClass("separator-focused");
            username_label.addClass("label-focused");
        });
        username_input.on("blur", evt => {
            username_separator.removeClass("separator-focused");
            username_label.removeClass("label-focused");
        });
        username_input.on("keydown", evt => {
            this.hide_error_message();
            if (evt.key === "Enter")
                password_input.focus();
        });

        password_input.on("focus", evt => {
            password_input.select();
            password_separator.addClass("separator-focused");
            password_label.addClass("label-focused");
        });
        password_input.on("blur", evt => {
            password_separator.removeClass("separator-focused");
            password_label.removeClass("label-focused");
        });
        password_input.on("keydown", evt => {
            this.hide_error_message();
            if (evt.key === "Enter")
                this.submit(dispatcher);
        });

        login_button.on("click", evt => {
            if (evt.button === 0)
                this.submit(dispatcher);
        });

        window.setTimeout(() => username_input.focus(), 10);
    }

    destroy() {
        this.div.remove();
    }

    show_error_message(message) {
        this.div.find(".error-message").append(message);
    }

    hide_error_message() {
        this.div.find(".error-message").empty();
    }

    error(message) {
        this.show_error_message(message);
        this.div.find("input[name=username]").focus();
    }

    submit(dispatcher) {
        let username_input = this.div.find("input[name=username]");
        let password_input = this.div.find("input[name=password]");
        if (username_input.val().length > 0) {
            dispatcher.initiate_login(username_input.val(), password_input.val());
        }
        else {
            this.show_error_message("Username cannot be empty");
            username_input.focus();
        }
        password_input.val("");
    }
}

// ----------------------------------------------------------------------

function list_commands(message, dispatcher) {
    // console.log("list_commands", message);
    var ol = $("<ol></ol>").appendTo($("body"));
    message.commands.forEach(entry => {
        ol.append(`<li><span style="font-weight: bold">${entry.name}</span><pre style="margin-top: 0;">${entry.description}</pre></li>\n`);
    });
}

// ----------------------------------------------------------------------

function doc_info(message, dispatcher) {
    switch (message.doc._t) {
    case "acmacs.inspectors.routine_diagnostics.IncrementalChainForked":
    case "acmacs.inspectors.routine_diagnostics.IncrementalChain":
        if (message.add_to_response && message.add_to_response.step_no !== undefined)
            import("./chain.js").then(mod => mod.chain_step(message.doc, message.add_to_response.step_no, dispatcher));
        else
            import("./chain.js").then(mod => mod.chain(message.doc, dispatcher));
        break;
    default:
        console.error("doc_info: unsupported doc type " + message.doc._t, message.doc);
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
