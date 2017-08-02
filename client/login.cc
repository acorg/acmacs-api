#include "toolkit-basic.hh"
#include "session.hh"
#include "argv.hh"
#include "login.hh"

// ----------------------------------------------------------------------

namespace client
{
    String* md5(String*);

      // --------------------------------------------------

    struct GetNonceCommandData : public CommandData
    {
        inline GetNonceCommandData(String* aUser) : CommandData{"login_nonce"_S} { set_user(aUser); }
        void set_user(String*);
    };

    struct LoginPasswordCommandData : public CommandData
    {
        inline LoginPasswordCommandData(String* cnonce, String* digest) : CommandData{"login_digest"_S} { set_cnonce(cnonce); set_digest(digest); }
        void set_cnonce(String*);
        void set_digest(String*);
    };

}

using namespace client;

// ----------------------------------------------------------------------

void login(client::WebSocket* aWS, OnMessageBase::TransferTo aTransferTo)
{
    aWS->set_onmessage(cheerp::Callback(Login(aWS, aTransferTo)));

} // login

// ----------------------------------------------------------------------

void Login::process_message(Message* aMessage)
{
    auto server_version = aMessage->get_hello();
    if ("acmacs-api-server-v1"_S == server_version) {
        if (!is_undefined_or_null(ARGV->session())) {
            this->template transfer_send<LoggedIn>(new LoginSessionData{ARGV->session()}, this->pass_transfer_to());
        }
        else if (!is_undefined_or_null(ARGV->user())) {
            initiate_login(ARGV->user(), ARGV->password());
        }
        else {
            widget();
              // window.alert("Cannot login: no cridentials");
        }
    }
    else {
        window.alert(concat("Unsupported server version: ", server_version));
    }

} // Login::process_message

// ----------------------------------------------------------------------

void Login::initiate_login(String* aUser, String* aPassword)
{
    this->template transfer_send<LoginNonce>(new GetNonceCommandData{aUser}, aUser, aPassword, this->pass_transfer_to());

} // Login::initiate_login

// ----------------------------------------------------------------------

void Login::widget()
{
    using namespace toolkit;

    auto* div1 = document.createElement("div");
    div1->set_className("login box-shadow-popup");
    div1->setAttribute("id", "login");

      // auto* title =
    append_child(div1, "div", text{"Acmacs-Web"}, class_{"title"});

    auto* form = document.createElement("form");

    auto* username_label = append_child(form, "div", text{"Username"});
    auto* username_input = static_cast<HTMLInputElement*>(append_child(form, "input", attr{"autocomplete", "username"}, attr{"spellcheck", "false"}, attr{"tabIndex", "1"}, attr{"name", "username"}, attr{"type", "email"}));
    auto* username_separator = append_child(form, "div", class_{"separator"});

    auto* password_label = append_child(form, "div", text{"Password"});
    auto* password_input = static_cast<HTMLInputElement*>(append_child(form, "input", attr{"autocomplete", "password"}, attr{"spellcheck", "false"}, attr{"tabIndex", "2"}, attr{"name", "password"}, attr{"type", "password"}));
    auto* password_separator = append_child(form, "div", class_{"separator"});
    auto* login_button = append_child(form, "div", text{"Log In"}, class_{"button box-shadow-button"});

    auto submit = [username_input, password_input, this]() -> void {
                      if (username_input->get_value()->get_length()) {
                          initiate_login(username_input->get_value(), password_input->get_value());
                      }
                      else {
                          username_input->focus();
                      }
                  };

    username_input->addEventListener("focus", cheerp::Callback([username_input,username_separator,username_label](FocusEvent*) {
        username_input->select();
        add_class(username_separator, "separator-focused");
        add_class(username_label, "label-focused");
    }));

    username_input->addEventListener("blur", cheerp::Callback([username_label,username_separator](FocusEvent*) {
        remove_class(username_separator, "separator-focused");
        remove_class(username_label, "label-focused");
    }));

    password_input->addEventListener("focus", cheerp::Callback([password_input,password_separator,password_label](FocusEvent*) {
        password_input->select();
        add_class(password_separator, "separator-focused");
        add_class(password_label, "label-focused");
    }));

    password_input->addEventListener("blur", cheerp::Callback([password_label,password_separator](FocusEvent*) {
        remove_class(password_separator, "separator-focused");
        remove_class(password_label, "label-focused");
    }));

    username_input->addEventListener("keydown", cheerp::Callback([password_input](KeyboardEvent* aEvent) -> void {
        if (eq(aEvent->get_key(), "Enter")) {
            password_input->focus();
        }
    }));

    password_input->addEventListener("keydown", cheerp::Callback([submit](KeyboardEvent* aEvent) -> void {
        if (eq(aEvent->get_key(), "Enter")) {
            submit();
        }
    }));

    login_button->addEventListener("click", cheerp::Callback([submit](MouseEvent* aEvent) -> void {
        if (static_cast<int>(aEvent->get_button()) == 0)
            submit();
    }));

    div1->appendChild(form);
    document.get_body()->appendChild(div1);

    username_input->focus();

} // Login::widget

// ----------------------------------------------------------------------

void LoginNonce::process_message(Message* aMessage)
{
    if (is_undefined(aMessage->get_E())) {
        auto* snonce = aMessage->get_login_nonce();
        auto* cnonce = make_cnonce();
        auto* digest_password = md5(concat(mUser, ";acmacs-web;", mPassword));
        auto* digest = md5(concat(snonce, ";", cnonce, ";", digest_password));
        this->template transfer_send<LoggedIn>(new LoginPasswordCommandData{cnonce, digest}, this->pass_transfer_to());
    }
    else {
        window.alert(concat("Login failed: ", aMessage->get_E()));
    }

} // LoginNonce::process_message

// ----------------------------------------------------------------------

void LoggedIn::process_message(Message* aMessage)
{
    if (!is_undefined(aMessage->get_E()) || (aMessage->get_C() != "login_digest"_S && aMessage->get_C() != "login_session"_S)) {
        window.alert(concat("Login failed: ", aMessage->get_E()));
    }
    else {
        session->set_id(aMessage->get_S());
        session->set_user(aMessage->get_user());
        session->set_display_name(aMessage->get_display_name());
        console_log("Logged in: ", aMessage);
          // console_log("Logged in: ", session->get_user(), session->get_display_name());
    }
    transfer_to();

} // LoggedIn::process_message

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
