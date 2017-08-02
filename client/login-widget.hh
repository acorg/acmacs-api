#pragma once

#include "widget.hh"

// ----------------------------------------------------------------------

class Login;

class LoginWidget : public Widget
{
 public:
    LoginWidget(Login* aLogin);

    virtual void show();
    virtual void hide();

 private:
    using HTMLElement = client::HTMLElement;
    using HTMLInputElement = client::HTMLInputElement;

    Login* mLogin;
    HTMLElement* div;
    HTMLElement* username_label;
    HTMLInputElement* username_input;
    HTMLElement* username_separator;
    HTMLElement* password_label;
    HTMLInputElement* password_input;
    HTMLElement* password_separator;
    HTMLElement* login_button;
    HTMLElement* error_message;

    void create();
    virtual void attach();
    // virtual void dettach();
    void submit();

}; // class LoginWidget

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
