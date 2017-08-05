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

    virtual inline ~JsonPrinter()
        {
            div->get_parentNode()->removeChild(div);
            div = nullptr;
            pre = nullptr;
        }

    inline void run()
        {
            using namespace toolkit;
            div = append_child(document.get_body(), "div", class_{"material-design-box-shadow"});
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

    inline void on_message(client::RawMessage* aMessage) override
        {
            client::console_log("JsonPrinter::on_message", aMessage);
            pre->set_innerHTML(json_syntax_highlight(stringify(aMessage, 2)));
        }

 private:
    using HTMLElement = client::HTMLElement;
    HTMLElement* div;
    HTMLElement* pre;

}; // class JsonPrinter

// ----------------------------------------------------------------------

void webMain()
{
    client::window.set_onload(cheerp::Callback(on_load));
    make_asm_definitions();
}

// ----------------------------------------------------------------------

ApplicationOne::ApplicationOne()
    : Application{}, mHandler{nullptr}
{
    using namespace toolkit;

    auto* header = append_child(document.get_body(), "table", attr{"id", "page-header"});
    auto* tbody = append_child(header, "tbody");
    auto* tr = append_child(tbody, "tr");
    append_child(tr, "td", text{"Acmacs-Web"}, attr{"id", "acmacs-web-logo"});
    h_display_name = append_child(tr, "td", attr{"id", "display-name"});
    h_display_name->addEventListener("click", cheerp::Callback([this](MouseEvent* aEvent) -> void {
        if (static_cast<int>(aEvent->get_button()) == 0)
            ask_logout();
    }));

} // ApplicationOne::ApplicationOne

// ----------------------------------------------------------------------

void ApplicationOne::run()
{
    make_connection();

} // ApplicationOne::run

// ----------------------------------------------------------------------

void ApplicationOne::logged_in()
{
    Application::logged_in();
    h_display_name->set_textContent(session()->display_name());
    if (!mHandler) {
        auto* handler = new JsonPrinter{this};
        mHandler = handler;
        handler->run();
    }

} // ApplicationOne::logged_in

// ----------------------------------------------------------------------

void ApplicationOne::ask_logout()
{
    auto result = client::window.confirm("Really want to logout?");
    if (result)
        logout();

} // ApplicationOne::ask_logout

// ----------------------------------------------------------------------

void ApplicationOne::reset()
{
    Application::reset();
    delete mHandler;
    mHandler = nullptr;
    h_display_name->set_textContent(""_S);

} // ApplicationOne::reset

// ----------------------------------------------------------------------

void on_load()
{
    using namespace client;

    console_log("app-one");

    auto* app = new ApplicationOne{};
    app->run();

    // static_cast<EventTarget&>(window).set_("session", new Session{});
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
