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

#include "acmacs-base/from-json.hh"
#include "send-func.hh"

// ----------------------------------------------------------------------

class Command;
class Session;
class MongoAcmacsC2Access;

// ----------------------------------------------------------------------

class CommandFactory
{
 public:
    CommandFactory();

    std::shared_ptr<Command> find(std::string aMessage, MongoAcmacsC2Access& aMongoAccess, SendFunc aSendFunc) const;

    static const CommandFactory* sFactory; // global pointer for list_commands command
    const auto& commands() const { return mFactory; }

 private:
    using FactoryFunc = std::shared_ptr<Command> (CommandFactory::*)(from_json::object&&, MongoAcmacsC2Access&, SendFunc, size_t) const;

    struct Data
    {
        inline Data(FactoryFunc aMaker, std::function<const char* ()> aDescription) : maker{aMaker}, description{aDescription} {}
        FactoryFunc maker;
        std::function<const char* ()> description;
    };

    template <typename Cmd> inline std::shared_ptr<Command> make(from_json::object&& aSrc, MongoAcmacsC2Access& aMongoAccess, SendFunc aSendFunc, size_t aCommandNumber) const
        {
            return std::make_shared<Cmd>(std::move(aSrc), aMongoAccess, aSendFunc, aCommandNumber);
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
