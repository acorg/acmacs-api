#pragma once

#include "widget.hh"

// ----------------------------------------------------------------------

class Login;

class LoginWidget : public Widget
{
 public:
    LoginWidget(Login* aLogin);

    void show();
    void hide();

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

    void attach();
    void dettach();

}; // class LoginWidget

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
