#pragma once

#include <string>
#include <chrono>

#include "acmacs-base/rapidjson.hh"
#include "acmacs-webserver/websocketpp-asio.hh"

#include "mongo-access.hh"

// ----------------------------------------------------------------------

class AcmacsAPIServer;
class Session;

class Command : public json_importer::Object
{
 public:
    using time_point = decltype(std::chrono::high_resolution_clock::now());

    inline Command(json_importer::Object&& aSrc, AcmacsAPIServer& aServer, size_t aCommandNumber)
        : json_importer::Object{std::move(aSrc)}, mServer{aServer}, mCommandNumber{aCommandNumber}
        {
            set_command_start();
        }

    inline std::string command_name() const { return get_string("C"); }
    inline size_t command_number() const { return mCommandNumber; }

    virtual void run() = 0;

 protected:
    void send(std::string aMessage, websocketpp::frame::opcode::value op_code = websocketpp::frame::opcode::text);
    void send_error(std::string aMessage);
    mongocxx::database db();
    Session& session();

    inline time_point now() const { return std::chrono::high_resolution_clock::now(); }
    inline void set_command_start() { mCommandStart = now(); }
    inline auto command_start() const { return mCommandStart; }
    inline double command_duration() const { return std::chrono::duration<double>{now() - command_start()}.count(); }

      // mongo_operator: $in, $all
    inline void bson_in_for_optional_array_of_strings(MongodbAccess::bld_doc& append_to, const char* key, const char* mongo_operator, std::function<json_importer::ConstArray()> getter, std::function<std::string(const rapidjson::Value&)> transformer = &json_importer::get_string)
        {
            try {
                const auto array = getter();
                if (!array.Empty())
                    bson_append(append_to, key, bson_object(mongo_operator, bson_array(std::begin(array), std::end(array), transformer)));
            }
            catch (RapidjsonAssert&) {
            }
        }

 private:
    AcmacsAPIServer& mServer;
    const size_t mCommandNumber;
    time_point mCommandStart;

}; // class Command

// ----------------------------------------------------------------------

class Command_unknown : public Command
{
 public:
    using Command::Command;

    virtual inline void run()
        {
            send_error("unrecognized message");
        }

}; // class Command_users

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
