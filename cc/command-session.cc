#include "command-session.hh"
#include "session.hh"

// ----------------------------------------------------------------------

void Command_login_session::run()
{
    session().use_session(session_id());
    send(json_object("S", session().id(), "user", session().user(), "display_name", session().display_name()));

} // Command_login_session::run

// ----------------------------------------------------------------------

void Command_login_nonce::run()
{
    const auto nonce = session().login_nonce(user());
    send(json_object("login_nonce", nonce));

} // Command_login_nonce::run

// ----------------------------------------------------------------------

void Command_login_digest::run()
{
    session().login_with_password_digest(cnonce(), digest());
    send(json_object("S", session().id(), "user", session().user(), "display_name", session().display_name()));

} // Command_login_digest::run

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
