#pragma once

#include <cheerp/clientlib.h>

#include "string.hh"
#include "asm.hh"

// ----------------------------------------------------------------------

namespace toolkit
{
    using namespace client;

    inline void add_class(client::HTMLElement*) {}

    template <typename ... Args> inline void add_class(client::HTMLElement* aElement, String* aClass, Args&& ... args)
    {
        if (aClass->get_length()) {
            auto* present = aElement->get_className();
            if (present->get_length() == 0)
                aElement->set_className(aClass);
            else if (present->indexOf(aClass) == -1)
                aElement->set_className(concat_space(present, aClass));
        }
        add_class(aElement, std::forward<Args>(args) ...);
    }

    template <typename ... Args> inline void add_class(client::HTMLElement* aElement, const char* aClass, Args&& ... args)
    {
        add_class(aElement, new String{aClass});
        add_class(aElement, std::forward<Args>(args) ...);
    }

      // ----------------------------------------------------------------------

    inline void remove_class(client::HTMLElement*) {}
    template <typename S, typename ... Args> inline void remove_class(client::HTMLElement* aElement, S aClass, Args&& ... args)
    {
        aElement->set_className(aElement->get_className()->replace(aClass, ""));
        remove_class(aElement, std::forward<Args>(args) ...);
    }

// ----------------------------------------------------------------------

    namespace internal
    {
        class string_field
        {
         public:
            inline string_field(const char* aString) : mString{new client::String{aString}} {}
            inline string_field(client::String* aString) : mString{aString} {}
            inline operator client::String*() const { return mString; }
            inline operator client::String&() const { return *mString; }

         private:
            String* mString;
        }; // class string_field

    } // namespace internal

    class text : public internal::string_field { public: using internal::string_field::string_field; };
    class class_ : public internal::string_field { public: using internal::string_field::string_field; };

    namespace internal
    {
        inline void set(client::HTMLElement* aElement, text&& aText) { aElement->set_textContent(aText); }
        inline void set(client::HTMLElement* aElement, class_&& aClass) { add_class(aElement, aClass); }

        inline void set(client::HTMLElement*) {}
        template <typename Arg, typename ... Args> inline void set(client::HTMLElement* aElement, Arg&& first, Args&& ... args)
        {
            set(aElement, std::forward<Arg>(first));
            set(std::forward<Args>(args) ...);
        }
    } // namespace internal

    template <typename ... Args> inline client::HTMLElement* append_child(client::HTMLElement* parent, const char* aType, Args&& ... args)
    {
        auto* element = client::document.createElement(aType);
        internal::set(element, std::forward<Args>(args) ...);
        parent->appendChild(element);
        return element;
    }

} // namespace toolkit

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
