#include "login-widget.hh"
#include "login.hh"

// ----------------------------------------------------------------------

LoginWidget::LoginWidget(Login* aLogin)
    : mLogin(aLogin)
{
    create();
    attach();

} // LoginWidget::LoginWidget

// ----------------------------------------------------------------------

void LoginWidget::show()
{
    // attach();
    toolkit::remove_class(div, "hidden");
    username_input->focus();

} // LoginWidget::show

// ----------------------------------------------------------------------

void LoginWidget::hide()
{
    // dettach();
    toolkit::add_class(div, "hidden");

} // LoginWidget::hide

// ----------------------------------------------------------------------

void LoginWidget::create()
{
    using namespace toolkit;

    div = append_child(document.get_body(), "div", class_{"login box-shadow-popup hidden"}, attr{"id", "login"});

      // auto* title =
    append_child(div, "div", text{"Acmacs-Web"}, class_{"title"});

    auto* form = append_child(div, "form");

    username_label = append_child(form, "div", text{"Username"});
    username_input = static_cast<HTMLInputElement*>(append_child(form, "input", attr{"autocomplete", "username"}, attr{"spellcheck", "false"}, attr{"tabIndex", "1"}, attr{"name", "username"}, attr{"type", "email"}));
    username_separator = append_child(form, "div", class_{"separator"});

    password_label = append_child(form, "div", text{"Password"});
    password_input = static_cast<HTMLInputElement*>(append_child(form, "input", attr{"autocomplete", "password"}, attr{"spellcheck", "false"}, attr{"tabIndex", "2"}, attr{"name", "password"}, attr{"type", "password"}));
    password_separator = append_child(form, "div", class_{"separator"});
    login_button = append_child(form, "div", text{"Log In"}, class_{"button box-shadow-button"});
    error_message = append_child(form, "div", class_{"error-message"});

} // LoginWidget::create

// ----------------------------------------------------------------------

void LoginWidget::attach()
{
    using namespace client;
    using namespace toolkit;

    username_input->addEventListener("focus", cheerp::Callback([this](FocusEvent*) {
        username_input->select();
        add_class(username_separator, "separator-focused");
        add_class(username_label, "label-focused");
    }));

    username_input->addEventListener("blur", cheerp::Callback([this](FocusEvent*) {
        remove_class(username_separator, "separator-focused");
        remove_class(username_label, "label-focused");
    }));

    password_input->addEventListener("focus", cheerp::Callback([this](FocusEvent*) {
        password_input->select();
        add_class(password_separator, "separator-focused");
        add_class(password_label, "label-focused");
    }));

    password_input->addEventListener("blur", cheerp::Callback([this](FocusEvent*) {
        remove_class(password_separator, "separator-focused");
        remove_class(password_label, "label-focused");
    }));

    username_input->addEventListener("keydown", cheerp::Callback([this](KeyboardEvent* aEvent) -> void {
        hide_error_message();
        if (eq(aEvent->get_key(), "Enter")) {
            password_input->focus();
        }
    }));

    password_input->addEventListener("keydown", cheerp::Callback([this](KeyboardEvent* aEvent) -> void {
        hide_error_message();
        if (eq(aEvent->get_key(), "Enter")) {
            submit();
        }
    }));

    login_button->addEventListener("click", cheerp::Callback([this](MouseEvent* aEvent) -> void {
        if (static_cast<int>(aEvent->get_button()) == 0)
            submit();
    }));

} // LoginWidget::attach

// ----------------------------------------------------------------------

void LoginWidget::submit()
{
    if (username_input->get_value()->get_length()) {
        mLogin->initiate_login(username_input->get_value(), password_input->get_value());
    }
    else {
        show_error_message("Username cannot be empty"_S);
        username_input->focus();
    }

} // LoginWidget::submit

// ----------------------------------------------------------------------

void LoginWidget::show_error_message(String* aMessage)
{
    error_message->set_textContent(aMessage);

} // LoginWidget::show_error_message

// ----------------------------------------------------------------------

void LoginWidget::hide_error_message()
{
    error_message->set_textContent("");

} // LoginWidget::hide_error_message

// ----------------------------------------------------------------------

// void LoginWidget::dettach()
// {

// } // LoginWidget::dettach

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
