#pragma once

#include <string>
#include <map>
#include <memory>

// ----------------------------------------------------------------------

class AcmacsAPIServer;
class Command;
namespace json_importer { class Object; }

class CommandFactory
{
 public:
    CommandFactory();

    std::shared_ptr<Command> find(std::string aMessage, AcmacsAPIServer& aServer, size_t aCommandNumber) const;

    static const CommandFactory* sFactory; // global pointer for list_commands command
    const auto& commands() const { return mFactory; }

 private:
    using FactoryFunc = std::shared_ptr<Command> (CommandFactory::*)(json_importer::Object&&, AcmacsAPIServer&, size_t aCommandNumber) const;

    struct Data
    {
        inline Data(FactoryFunc aMaker, std::function<const char* ()> aDescription) : maker{aMaker}, description{aDescription} {}
        FactoryFunc maker;
        std::function<const char* ()> description;
    };

    template <typename Cmd> inline std::shared_ptr<Command> make(json_importer::Object&& aSrc, AcmacsAPIServer& aServer, size_t aCommandNumber) const
        {
            return std::make_shared<Cmd>(std::move(aSrc), aServer, aCommandNumber);
        }

    template <typename Cmd> inline static Data data()
        {
            return {&CommandFactory::make<Cmd>, &Cmd::description};
        }

      // std::map<std::string, std::function<std::shared_ptr<Command> (std::string)>> mFactory;
    std::map<std::string, Data> mFactory;

}; // class CommandFactory


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
