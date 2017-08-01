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
    auto* div1 = document.createElement("div");
    div1->set_className("login box-shadow-popup");
    div1->setAttribute("id", "login");

    auto* title = document.createElement("div");
    title->set_textContent("Acmacs-Web");
    title->set_className("title");
    div1->appendChild(title);

    auto* form = document.createElement("form");

    auto* username_label = document.createElement("div");
    username_label->set_textContent("Username");
    form->appendChild(username_label);
    auto* username_input = static_cast<HTMLInputElement*>(document.createElement("input"));
    username_input->setAttribute("autocomplete", "username");
    username_input->setAttribute("spellcheck", "false");
    username_input->setAttribute("tabIndex", "1");
    username_input->setAttribute("name", "username");
    username_input->setAttribute("type", "email");
    form->appendChild(username_input);

    auto* username_separator = document.createElement("div");
    username_separator->set_className("separator");
    form->appendChild(username_separator);

    auto* password_label = document.createElement("div");
    password_label->set_textContent("Password");
    form->appendChild(password_label);
    auto* password_input = static_cast<HTMLInputElement*>(document.createElement("input"));
    password_input->setAttribute("autocomplete", "password");
    password_input->setAttribute("spellcheck", "false");
    password_input->setAttribute("tabIndex", "2");
    password_input->setAttribute("name", "password");
    password_input->setAttribute("type", "password");
    form->appendChild(password_input);

    auto* password_separator = document.createElement("div");
    password_separator->set_className("separator");
    form->appendChild(password_separator);

    auto* login_button = document.createElement("div");
    login_button->set_textContent("Log In");
    login_button->set_className("button box-shadow-button");
    form->appendChild(login_button);

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
        toolkit::add_class(username_separator, "separator-focused");
        toolkit::add_class(username_label, "label-focused");
    }));

    username_input->addEventListener("blur", cheerp::Callback([username_label,username_separator](FocusEvent*) {
        toolkit::remove_class(username_separator, "separator-focused");
        toolkit::remove_class(username_label, "label-focused");
    }));

    password_input->addEventListener("focus", cheerp::Callback([password_input,password_separator,password_label](FocusEvent*) {
        password_input->select();
        toolkit::add_class(password_separator, "separator-focused");
        toolkit::add_class(password_label, "label-focused");
    }));

    password_input->addEventListener("blur", cheerp::Callback([password_label,password_separator](FocusEvent*) {
        toolkit::remove_class(password_separator, "separator-focused");
        toolkit::remove_class(password_label, "label-focused");
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
