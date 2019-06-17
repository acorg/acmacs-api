#include "command-session.hh"
#include "session.hh"

// ----------------------------------------------------------------------

void Command_login_session::run()
{
    make_session();
    session().use_session(session_id());
    send(to_json::v1::object("S", session().id(), "user", session().user(), "display_name", session().display_name()));

} // Command_login_session::run

// ----------------------------------------------------------------------

const char* Command_login_session::description()
{
    return "try to use the passed session_id, it it was not expired.\n  S :string - session id";

} // Command_login_session::description

// ----------------------------------------------------------------------

void Command_login_nonce::run()
{
    make_session();
    const auto nonce = session().login_nonce(user());
    send(to_json::v1::object("login_nonce", nonce));

} // Command_login_nonce::run

// ----------------------------------------------------------------------

const char* Command_login_nonce::description()
{
    return "the first stage of the login procedure\n  user :string - user name";

} // Command_login_nonce::description

// ----------------------------------------------------------------------

void Command_login_digest::run()
{
    session().login_with_password_digest(cnonce(), digest());
    send(to_json::v1::object("S", session().id(), "user", session().user(), "display_name", session().display_name()));

} // Command_login_digest::run

// ----------------------------------------------------------------------

const char* Command_login_digest::description()
{
    return "the second stage of the login procedure\n  cnonce :string\n  digest :string";

} // Command_login_digest::description

// ----------------------------------------------------------------------

void Command_logout::run()
{
    session().logout();
      // no reply to the client

} // Command_logout::run

// ----------------------------------------------------------------------

const char* Command_logout::description()
{
    return "removes session";

} // Command_logout::description

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
