#include "session.hh"
#include "argv.hh"

// ----------------------------------------------------------------------

namespace client
{
    String* md5(String*);
}

using namespace client;

// ----------------------------------------------------------------------

void LoggedIn::process_message(Message* aMessage)
{
    if (!is_undefined(aMessage->get_E()) || aMessage->get_R() != "session"_S) {
        window.alert(concat("Login failed: ", aMessage->get_E()));
    }
    else {
        session->set_id(aMessage->get_S());
        session->set_user(aMessage->get_user());
        session->set_display_name(aMessage->get_display_name());
        console_log("Logged in: ", session->get_user(), session->get_display_name());
    }
    this->send("{\"C\":\"echo\"}");
    transfer_to();

} // LoggedIn::process_message

// ----------------------------------------------------------------------

void LoginNonce::process_message(Message* aMessage)
{
    if (is_undefined(aMessage->get_E())) {
        auto* snonce = aMessage->get_login_nonce();
        auto* cnonce = make_cnonce();
        auto* digest_password = md5(concat(ARGV->user(), ";acmacs-web;", ARGV->password()));
        auto* digest = md5(concat(snonce, ";", cnonce, ";", digest_password));
        this->template transfer_send<LoggedIn>(new LoginPasswordCommandData{cnonce, digest}, this->pass_transfer_to());
    }
    else {
        window.alert(concat("Login failed: ", aMessage->get_E()));
    }

} // LoginNonce::process_message

// ----------------------------------------------------------------------

void Login::process_message(Message* aMessage)
{
    auto server_version = aMessage->get_hello();
    if ("acmacs-api-server-v1"_S == server_version) {
        if (!is_undefined(ARGV->session())) {
            this->template transfer_send<LoggedIn>(new LoginSessionData{ARGV->session()}, this->pass_transfer_to());
        }
        else if (!is_undefined(ARGV->user())) {
            this->template transfer_send<LoginNonce>(new GetNonceCommandData{ARGV->user()}, this->pass_transfer_to());
        }
        else {
            window.alert("Cannot login: no cridentials");
        }
    }
    else {
        window.alert(concat("Unsupported server version: ", server_version));
    }

} // Login::process_message

// ----------------------------------------------------------------------

void login(client::WebSocket* aWS, OnMessageBase::TransferTo aTransferTo)
{
    aWS->set_onmessage(cheerp::Callback(Login(aWS, aTransferTo)));

} // login

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End: