#include "toolkit-basic.hh"
#include "app-one.hh"
#include "handler.hh"

// ----------------------------------------------------------------------

static void on_load();

// ----------------------------------------------------------------------

namespace client
{
    struct Command_chart : public CommandData
    {
        template <typename IdType> inline Command_chart(IdType id) : CommandData{"chart"_S} { set_id(id); }
        inline void set_id(const char* id) { set_id(to_String(id)); }
        void set_id(String*);
    };
}

class JsonPrinter : public Handler
{
 public:
    using Handler::Handler;

    inline void run()
        {
            using namespace toolkit;
            auto* div = append_child(document.get_body(), "div", class_{"material-design-box-shadow"});
            auto* button  = append_child(div, "button", text{"get chart"});
            pre = append_child(div, "pre", class_{"json-highlight"});
            send_command();
            button->addEventListener("click", cheerp::Callback([this](MouseEvent* aEvent) -> void {
                if (static_cast<int>(aEvent->get_button()) == 0)
                    send_command();
            }));
        }

    inline void send_command()
        {
            send(new client::Command_chart{"593a87ee48618fc1e72da4fe"});
        }

    void on_message(client::RawMessage* aMessage) override
        {
            client::console_log("JsonPrinter::on_message", aMessage);
            pre->set_innerHTML(json_syntax_highlight(stringify(aMessage, 2)));
        }

 private:
    using HTMLElement = client::HTMLElement;
    HTMLElement* pre;

}; // class JsonPrinter

// ----------------------------------------------------------------------

void webMain()
{
    client::window.set_onload(cheerp::Callback(on_load));
    make_asm_definitions();
}

// ----------------------------------------------------------------------

void ApplicationOne::run()
{
    make_connection();

} // ApplicationOne::run

// ----------------------------------------------------------------------

void ApplicationOne::logged_in()
{
    if (!mHandler) {
        auto* handler = new JsonPrinter{this};
        mHandler = handler;
        handler->run();
    }

} // ApplicationOne::logged_in

// ----------------------------------------------------------------------

void on_load()
{
    using namespace client;
    using namespace toolkit;

    console_log("app-one");

    append_child(document.get_body(), "div", text{"APP ONE"});
    auto* app = new ApplicationOne{};
    app->run();

    // static_cast<EventTarget&>(window).set_("session", new Session{});
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
