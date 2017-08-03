#pragma once

#include "widget.hh"

// ----------------------------------------------------------------------

class Login;

class LoginWidget : public Widget
{
 public:
    LoginWidget(Login* aLogin);

    void show() override;
    void hide() override;

    void show_error_message(String* aMessage);
    void hide_error_message();

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
