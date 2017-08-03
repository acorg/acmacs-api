#pragma once

#include "application.hh"

// ----------------------------------------------------------------------

class Handler
{
 public:
    inline Handler(Application* aApp) : mApp{aApp} {}
    virtual ~Handler();

 protected:
    inline Application* app() { return mApp; }
    inline void send(client::CommandData* aCommand) { mApp->send(aCommand, this); }

 private:
    Application* mApp;

}; // class Handler

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
