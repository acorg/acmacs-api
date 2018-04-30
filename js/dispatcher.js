import "./md5.js";

export class Dispatcher {

    constructor() {
        const url = new URL(document.location);
        this.websocket_ = new WebSocket("wss://" + url.host + "/acmacs-api");
        this.websocket_.onopen = evt => this.onopen(evt);
        this.websocket_.onclose = evt => this.onclose(evt);
        this.websocket_.onmessage = evt => this.onmessage(evt);
        this.websocket_.onerror = evt => this.onerror(evt);
        this.login_process_ = false;
        this.session_ = this.store_value_("session-id");
        this.user_ = this.store_value_("user");
        this.user_name_ = this.store_value_("user_name");
        this.command_queue_ = []; // when waiting for login
        this.commands_sent_ = {};
        this.command_id_ = 1;
        this.setup_handlers_(url);
        this.modules = {};
    }

    setup_handlers_(url) {
        this.handlers_ = {
            "ERROR#no session": msg => this.no_session(msg),
            "ERROR#invalid session": msg => this.invalid_session(msg),
            "ERROR#could not parse Object ID string": msg => this.invalid_object_id(msg),
            "ERROR#invalid user or password": msg => this.invalid_user(msg),
            login_nonce: msg => this.login_nonce_response(msg),
            login_digest: msg => this.login_digest_response(msg),
            login_session: msg => this.login_session_response(msg),
            hello: msg => this.hello_handler(msg),

            list_commands: list_commands,
            chains: {import: "./chain.js", func: "chains"}
        };
        switch (url.pathname) {
        case "/list_commands":
        case "/list-commands":
        case "/lc":
            this.command_on_hello_ = {C: "list_commands"};
            break;
        case "/chains":
            this.command_on_hello_ = {C: "chains"}; // types: ["incremental"]
            break;
        }
    }

    handle(name, handler) {
        this.handlers_[name] = handler;
    }

    send(data) {
        // console.log("send", data);
        if (typeof(data) === "object" && data.C) {
            if (this.login_process_ && data.C.substr(0, 6) != "login_") {
                this.command_queue_.push(data);
            }
            else {
                if (data.D === undefined) {
                    data.D = data.C + "#" + this.command_id_;
                    ++this.command_id_;
                    this.commands_sent_[data.D] = data;
                }
                // console.log("send really", data);
                this.websocket_.send(JSON.stringify(data));
            }
        }
        else if (typeof(data) === "string" && data.indexOf('"C":') >= 0) {
            this.websocket_.send(data);
        }
        else {
            throw "invalid data passed to Dispatcher.send: " + JSON.stringify(data);
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
        console.log("command_on_hello_", this.command_on_hello_);
        if (this.command_on_hello_) {
            this.send(this.command_on_hello_);
            delete this.command_on_hello_;
        }
        else {
            console.warn("Dispatcher.hello_handler: no command_on_hello_");
        }
    }

    invoke_handler(key, message) {
        const handler = this.handlers_[key];
        if (handler) {
            if (typeof(handler) === "function") {
                handler(message, this);
            }
            else if (typeof(handler) === "object" && handler.import && handler.func) {
                import(handler.import).then(mod => mod[handler.func](message, this));
            }
            else {
                console.warn("Dispatcher.invoke_handler: handler not supported: ", JSON.stringify(key), message);
            }
        }
        else
            console.warn("Dispatcher.invoke_handler: handler not found: ", JSON.stringify(key), message);
    }

    // ----------------------------------------------------------------------

    onmessage(evt) {
        try {
            const message = JSON.parse(evt.data);
            if (!message.E) {
                delete this.commands_sent_[message.D];
                this.invoke_handler(message.C, message);
            }
            else {
                this.invoke_handler("ERROR#" + message.E, message);
            }
        }
        catch (err) {
            window.DDD = evt.data;
            console.error(err, evt, evt.data);
        }
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

    // ----------------------------------------------------------------------

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
    }

    invalid_user(message) {
        if (this.login_widget_)
            this.login_widget_.error(message.E);
        else
            console.error(message);
    }

    show_login_widget() {
        this.login_process_ = true;
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
        // setup title bar
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

        setTimeout(() => { username_input.focus(); }, 10);
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

function list_commands(data, dispatcher) {
    console.log(data);
    var ol = $("<ol></ol>").appendTo($("body"));
    data.commands.forEach(entry => {
        ol.append(`<li><span style="font-weight: bold">${entry.name}</span><pre style="margin-top: 0;">${entry.description}</pre></li>\n`);
    });
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
