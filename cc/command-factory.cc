#include "acmacs-base/rapidjson.hh"

#include "command-factory.hh"
#include "acmacs-api-server.hh"

#include "command-session.hh"
#include "command-admin.hh"
#include "command-chart.hh"

// ----------------------------------------------------------------------

CommandFactory::CommandFactory()
    : mFactory{
    {"users", &CommandFactory::make<Command_users>},
    {"root_charts", &CommandFactory::make<Command_root_charts>},
    {"login_session", &CommandFactory::make<Command_login_session>},
    {"login_nonce", &CommandFactory::make<Command_login_nonce>},
    {"login_digest", &CommandFactory::make<Command_login_digest>},
}
{
}

// ----------------------------------------------------------------------

std::shared_ptr<Command> CommandFactory::find(std::string aMessage, AcmacsAPIServer& aServer, size_t aCommandNumber) const
{
    json_importer::Object msg{aMessage};
    std::shared_ptr<Command> result;
    const auto found = mFactory.find(msg.get_string("C"));
    if (found != mFactory.end())
        result = (this->*found->second)(std::move(msg), aServer, aCommandNumber);
    else
        result = make<Command_unknown>(std::move(msg), aServer, aCommandNumber);
    return result;

} // CommandFactory::find

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
