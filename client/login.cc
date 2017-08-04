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

void Login::reset()
{
    if (mWidget)
        mWidget->reset();
    mPassword = nullptr;

} // Login::reset

// ----------------------------------------------------------------------

inline void Login::show_widget()
{
    if (!mWidget)
        mWidget = new LoginWidget{this};
    mWidget->show();

} // Login::show_widget

// ----------------------------------------------------------------------

inline void Login::hide_widget()
{
    if (mWidget)
        mWidget->hide();

} // Login::hide_widget

// ----------------------------------------------------------------------

void Login::run(bool use_argv)
{
    if (use_argv && !is_undefined_or_null(ARGV->session())) {
        send(new LoginSessionData{ARGV->session()});
    }
    else if (use_argv && !is_undefined_or_null(ARGV->user())) {
        initiate_login(ARGV->user(), ARGV->password());
    }
    else {
        show_widget();
    }

} // Login::run

// ----------------------------------------------------------------------

void Login::use_session(bool cancel_existing_session)
{
    auto* sess = session();
    if (cancel_existing_session)
        sess->expired();
    if (sess->valid()) {
        send(new LoginSessionData{sess->id()});
    }
    else {
        run(!cancel_existing_session);
    }

} // Login::use_session

// ----------------------------------------------------------------------

void Login::initiate_login(String* aUser, String* aPassword)
{
    session()->user(aUser);
    mPassword = aPassword;
    send(new GetNonceCommandData{aUser});

} // Login::initiate_login

// ----------------------------------------------------------------------

void Login::on_message(client::RawMessage* aMessage)
{
    auto* sess = session();
    auto* msg = static_cast<client::LoginData*>(aMessage);
    if (eq("login_nonce", msg->get_C())) {
        auto* snonce = msg->get_login_nonce();
        auto* cnonce = make_cnonce();
        auto* digest_password = md5(concat(sess->user(), ";acmacs-web;", mPassword));
        mPassword = nullptr;
        auto* digest = md5(concat(snonce, ";", cnonce, ";", digest_password));
        send(new LoginPasswordCommandData{cnonce, digest});
    }
    else if (eq(msg->get_C(), "login_digest") || eq(msg->get_C(), "login_session")) {
        hide_widget();
        sess->id(msg->get_S());
        sess->user(msg->get_user());
        sess->display_name(msg->get_display_name());
        app()->logged_in();
    }
    else {
        on_error(concat("Unsupported message forwarded to Login: ", stringify(aMessage)));
    }

} // Login::on_message

// ----------------------------------------------------------------------

void Login::on_error(String* aMessage)
{
    if (eq(aMessage, "invalid user or password") || eq(aMessage, "invalid session")) {
        show_widget();
        mWidget->show_error_message(aMessage);
    }
    else {
        Handler::on_error(aMessage);
    }

} // Login::on_error

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
