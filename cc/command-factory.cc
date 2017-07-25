#include "acmacs-base/rapidjson.hh"

#include "command-factory.hh"
#include "acmacs-api-server.hh"

#include "command-info.hh"
#include "command-session.hh"
#include "command-admin.hh"
#include "command-chart.hh"

// ----------------------------------------------------------------------

const CommandFactory* CommandFactory::sFactory = nullptr;

CommandFactory::CommandFactory()
    : mFactory{
    {"root_charts", data<Command_root_charts>()},
    {"chart_keywords", data<Command_chart_keywords>()},
    {"chart_owners", data<Command_chart_owners>()},

    {"users", data<Command_users>()},

    {"version", data<Command_version>()},
    {"list_commands", data<Command_list_commands>()},

    {"login_session", data<Command_login_session>()},
    {"login_nonce", data<Command_login_nonce>()},
    {"login_digest",data<Command_login_digest>()},
}
{
    sFactory = this;
}

// ----------------------------------------------------------------------

std::shared_ptr<Command> CommandFactory::find(std::string aMessage, AcmacsAPIServer& aServer, size_t aCommandNumber) const
{
    json_importer::Object msg{aMessage};
    std::shared_ptr<Command> result;
    const auto found = mFactory.find(msg.get_string("C"));
    if (found != mFactory.end())
        result = (this->*found->second.maker)(std::move(msg), aServer, aCommandNumber);
    else
        result = make<Command_unknown>(std::move(msg), aServer, aCommandNumber);
    return result;

} // CommandFactory::find

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
