#pragma once

#include <string>
#include <map>
#include <memory>
#include <thread>
#include <atomic>

#pragma GCC diagnostic push
#include "mongo-diagnostics.hh"
#include <mongocxx/database.hpp>
#pragma GCC diagnostic pop

#include "acmacs-base/rjson.hh"

// ----------------------------------------------------------------------

class Command;
class MongoAcmacsC2Access;
class ClientConnection;

// ----------------------------------------------------------------------

class CommandFactory
{
 public:
    CommandFactory();

    std::shared_ptr<Command> find(std::string aMessage, MongoAcmacsC2Access& aMongoAccess, ClientConnection& aClientConnection) const;

    static const CommandFactory* sFactory; // global pointer for list_commands command
    const auto& commands() const { return mFactory; }

 private:
    using FactoryFunc = std::shared_ptr<Command> (CommandFactory::*)(rjson::v1::object&&, MongoAcmacsC2Access&, ClientConnection&, size_t) const;

    struct Data
    {
        Data(FactoryFunc aMaker, std::function<const char* ()> aDescription) : maker{aMaker}, description{aDescription} {}
        FactoryFunc maker;
        std::function<const char* ()> description;
    };

    template <typename Cmd> std::shared_ptr<Command> make(rjson::v1::object&& aSrc, MongoAcmacsC2Access& aMongoAccess, ClientConnection& aClientConnection, size_t aCommandNumber) const
        {
            return std::make_shared<Cmd>(std::move(aSrc), aMongoAccess, aClientConnection, aCommandNumber);
        }

    template <typename Cmd> inline static Data data()
        {
            return {&CommandFactory::make<Cmd>, &Cmd::description};
        }

    std::map<std::string, Data> mFactory;
    mutable std::atomic<size_t> mCommandNumber;

}; // class CommandFactory

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
