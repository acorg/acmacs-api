#pragma once

#include "application.hh"

// ----------------------------------------------------------------------

class Handler
{
 public:
    inline Handler(Application* aApp) : mApp{aApp} {}
    virtual ~Handler();

    virtual inline void reset() {}
    virtual void on_message(client::RawMessage* aMessage) = 0;
    virtual void on_error(String* aMessage);

 protected:
    inline Application* app() { return mApp; }
    inline Session* session() { return mApp->session(); }
    inline void send(client::CommandData* aCommand) { mApp->send(aCommand, this); }

 private:
    Application* mApp;

}; // class Handler

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
