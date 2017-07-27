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

#include "send-func.hh"

// ----------------------------------------------------------------------

namespace json_importer { class Object; }

class Command;
class Session;

// ----------------------------------------------------------------------

class CommandFactory
{
 public:
    CommandFactory();

    std::shared_ptr<Command> find(std::string aMessage, mongocxx::database aDb, Session& aSession, SendFunc aSendFunc) const;

    static const CommandFactory* sFactory; // global pointer for list_commands command
    const auto& commands() const { return mFactory; }

 private:
    using FactoryFunc = std::shared_ptr<Command> (CommandFactory::*)(json_importer::Object&&, mongocxx::database, Session&, SendFunc, size_t) const;

    struct Data
    {
        inline Data(FactoryFunc aMaker, std::function<const char* ()> aDescription) : maker{aMaker}, description{aDescription} {}
        FactoryFunc maker;
        std::function<const char* ()> description;
    };

    template <typename Cmd> inline std::shared_ptr<Command> make(json_importer::Object&& aSrc, mongocxx::database aDb, Session& aSession, SendFunc aSendFunc, size_t aCommandNumber) const
        {
            return std::make_shared<Cmd>(std::move(aSrc), aDb, aSession, aSendFunc, aCommandNumber);
        }

    template <typename Cmd> inline static Data data()
        {
            return {&CommandFactory::make<Cmd>, &Cmd::description};
        }

      // std::map<std::string, std::function<std::shared_ptr<Command> (std::string)>> mFactory;
    std::map<std::string, Data> mFactory;
    mutable std::atomic<size_t> mCommandNumber;

}; // class CommandFactory


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
