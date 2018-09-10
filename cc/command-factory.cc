#include "command-factory.hh"

#include "command-info.hh"
#include "command-session.hh"
#include "command-admin.hh"
#include "command-chart.hh"
#include "command-chain.hh"
#include "command-hidb.hh"

// ----------------------------------------------------------------------

const CommandFactory* CommandFactory::sFactory = nullptr;

CommandFactory::CommandFactory()
    : mFactory{
    {"chains",                data<Command_chains>()},
    {"chain_owners",          data<Command_chain_owners>()},
    {"chain_keywords",        data<Command_chain_keywords>()},
    {"chain_types",           data<Command_chain_types>()},

    {"doc",                   data<Command_doc>()},
    // {"map",                   data<Command_map>()},
    {"ace",                   data<Command_ace>()},
    {"pdf",                   data<Command_pdf>()},
    {"chart",                 data<Command_chart>()},
    {"root_charts",           data<Command_root_charts>()},
    {"chart_keywords",        data<Command_chart_keywords>()},
    {"chart_owners",          data<Command_chart_owners>()},
    {"sequences_of_chart",    data<Command_sequences_of_chart>()},
    {"download_sequences_of_chart_as_fasta",    data<Command_download_sequences_of_chart_as_fasta>()},
    {"download_layout_sequences_as_csv",        data<Command_download_layout_sequences_as_csv>()},
    {"hidb_antigen",          data<Command_hidb_antigen>()},
    {"hidb_serum",            data<Command_hidb_serum>()},

    {"download_ace",                          data<Command_download_ace>()},
    {"download_lispmds_save",                 data<Command_download_lispmds_save>()},
    {"download_layout",                       data<Command_download_layout>()},
    {"download_table_map_distances",          data<Command_download_table_map_distances>()},
    {"download_error_lines",                  data<Command_download_error_lines>()},
    {"download_distances_between_all_points", data<Command_download_distances_between_all_points>()},

    {"users",                 data<Command_users>()},

    {"version",               data<Command_version>()},
    {"list_commands",         data<Command_list_commands>()},

    {"login_session",         data<Command_login_session>()},
    {"login_nonce",           data<Command_login_nonce>()},
    {"login_digest",          data<Command_login_digest>()},
    {"logout",                data<Command_logout>()},
            },
      mCommandNumber{0}
{
    sFactory = this;
}

// ----------------------------------------------------------------------

std::shared_ptr<Command> CommandFactory::find(std::string aMessage, MongoAcmacsC2Access& aMongoAccess, ClientConnection& aClientConnection) const
{
    ++mCommandNumber;
    rjson::v1::object msg = rjson::v1::parse_string(aMessage);
    std::shared_ptr<Command> result;
    const auto found = mFactory.find(msg.get_string_or_throw("C"));
    if (found != mFactory.end())
        result = (this->*found->second.maker)(std::move(msg), aMongoAccess, aClientConnection, mCommandNumber);
    else
        result = make<Command_unknown>(std::move(msg), aMongoAccess, aClientConnection, mCommandNumber);
    return result;

} // CommandFactory::find

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
