#include "session.hh"
#include "login.hh"
#include "login-widget.hh"
#include "argv.hh"

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

void Login::upon_transfer()
{
    if (!is_undefined_or_null(ARGV->session())) {
        send(new LoginSessionData{ARGV->session()});
    }
    else if (!is_undefined_or_null(ARGV->user())) {
        initiate_login(ARGV->user(), ARGV->password());
    }
    else {
        if (!mWidget)
            mWidget = new LoginWidget{this};
        mWidget->show();
    }

} // Login::upon_transfer

// ----------------------------------------------------------------------

void Login::process_message(Message* aMessage)
{
    if (eq(aMessage->get_C(), "login_nonce")) {
        auto* snonce = aMessage->get_login_nonce();
        auto* cnonce = make_cnonce();
        auto* digest_password = md5(concat(mUser, ";acmacs-web;", mPassword));
        auto* digest = md5(concat(snonce, ";", cnonce, ";", digest_password));
        send(new LoginPasswordCommandData{cnonce, digest});
    }
    else if (eq(aMessage->get_C(), "login_digest") || eq(aMessage->get_C(), "login_session")) {
        mWidget->hide();
        mPassword = nullptr;
        mUser = nullptr;
        session->set_id(aMessage->get_S());
        session->set_user(aMessage->get_user());
        session->set_display_name(aMessage->get_display_name());
          // console_log("Logged in: ", aMessage);
        transfer_to(mTransferTo);
    }
    else {
        process_error(concat("Unrecognized message from server: ", stringify(aMessage)));
    }

} // Login::process_message

// ----------------------------------------------------------------------

void Login::process_error(String* aError)
{
    console_error("ERROR:", aError);
    window.alert(aError);

} // Login::process_error

// ----------------------------------------------------------------------

void Login::initiate_login(String* aUser, String* aPassword)
{
    mUser = aUser;
    mPassword = aPassword;
    send(new GetNonceCommandData{aUser});

} // Login::initiate_login

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
