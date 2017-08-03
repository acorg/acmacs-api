#include "app-one.hh"

#include "toolkit-basic.hh"

// ----------------------------------------------------------------------

static void on_load();

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


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
