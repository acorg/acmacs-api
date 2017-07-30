#include "command-factory.hh"

#include "command-info.hh"
#include "command-session.hh"
#include "command-admin.hh"
#include "command-chart.hh"
#include "command-chain.hh"

// ----------------------------------------------------------------------

const CommandFactory* CommandFactory::sFactory = nullptr;

CommandFactory::CommandFactory()
    : mFactory{
    {"chains",         data<Command_chains>()},
    {"chain_owners",   data<Command_chain_owners>()},
    {"chain_keywords", data<Command_chain_keywords>()},
    {"chain_types",    data<Command_chain_types>()},

    {"chart",          data<Command_chart>()},
    {"root_charts",    data<Command_root_charts>()},
    {"chart_keywords", data<Command_chart_keywords>()},
    {"chart_owners",   data<Command_chart_owners>()},

    {"users",          data<Command_users>()},

    {"version",        data<Command_version>()},
    {"list_commands",  data<Command_list_commands>()},

    {"login_session",  data<Command_login_session>()},
    {"login_nonce",    data<Command_login_nonce>()},
    {"login_digest",   data<Command_login_digest>()},
            },
      mCommandNumber{0}
{
    sFactory = this;
}

// ----------------------------------------------------------------------

std::shared_ptr<Command> CommandFactory::find(std::string aMessage, MongoAcmacsC2Access& aMongoAccess, SendFunc aSendFunc) const
{
    ++mCommandNumber;
    from_json::object msg{aMessage};
    std::shared_ptr<Command> result;
    const auto found = mFactory.find(msg.get_string("C"));
    if (found != mFactory.end())
          // result = (this->*found->second.maker)(std::move(msg), aDb, aSession, aSendFunc, mCommandNumber);
        result = (this->*found->second.maker)(std::move(msg), aMongoAccess, aSendFunc, mCommandNumber);
    else
          // result = make<Command_unknown>(std::move(msg), aDb, aSession, aSendFunc, mCommandNumber);
        result = make<Command_unknown>(std::move(msg), aMongoAccess, aSendFunc, mCommandNumber);
    return result;

} // CommandFactory::find

// ----------------------------------------------------------------------

// std::shared_ptr<Command> CommandFactory::find(std::string aMessage, mongocxx::database aDb, Session& aSession, SendFunc aSendFunc) const
// {
//     ++mCommandNumber;
//     from_json::object msg{aMessage};
//     std::shared_ptr<Command> result;
//     const auto found = mFactory.find(msg.get_string("C"));
//     if (found != mFactory.end())
//         result = (this->*found->second.maker)(std::move(msg), aDb, aSession, aSendFunc, mCommandNumber);
//     else
//         result = make<Command_unknown>(std::move(msg), aDb, aSession, aSendFunc, mCommandNumber);
//     return result;

// } // CommandFactory::find

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
