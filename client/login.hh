#pragma once

#include "handler.hh"

// ----------------------------------------------------------------------

namespace client
{
    struct LoginSessionData : public CommandData
    {
        inline LoginSessionData(String* aS) : CommandData{"login_session"_S} { set_S(aS); }
        void set_S(String*);
    };

    struct LoginData : public ResponseData
    {
        String* get_login_nonce();
        String* get_S();
        String* get_user();
        String* get_display_name();
    };

} // namespace client

// ----------------------------------------------------------------------

class LoginWidget;

class Login : public Handler
{
 public:
    inline Login(Application* aApp)
        : Handler{aApp}, mWidget{nullptr}
        {
            client::console_log("Login");
        }
      //inline Login(const Login&) = default;
    inline ~Login() override { client::console_log("~Login"); }

    void run();
    void use_session();
    void on_message(client::RawMessage* aMessage) override;
    void on_error(String* aMessage) override;

 private:
    LoginWidget* mWidget;
    String* mPassword;

    friend class LoginWidget;

    void initiate_login(String* aUser, String* aPassword);
    void show_widget();
    void hide_widget();

}; // class Login

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
