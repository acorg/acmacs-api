#pragma once

#include <cheerp/clientlib.h>

#include "string.hh"
#include "asm.hh"

// ----------------------------------------------------------------------

namespace toolkit
{
    using namespace client;

    inline void add_class(HTMLElement*) {}

    template <typename ... Args> inline void add_class(HTMLElement* aElement, String* aClass, Args&& ... args)
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

    template <typename ... Args> inline void add_class(HTMLElement* aElement, const char* aClass, Args&& ... args)
    {
        add_class(aElement, new String{aClass});
        add_class(aElement, std::forward<Args>(args) ...);
    }

      // ----------------------------------------------------------------------

    inline void remove_class(HTMLElement*) {}
    template <typename S, typename ... Args> inline void remove_class(HTMLElement* aElement, S aClass, Args&& ... args)
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
            template <typename S> inline string_field(S aString) : mString{to_String(aString)} {}
            inline operator String*() const { return mString; }
            inline operator String&() const { return *mString; }

         private:
            String* mString;
        }; // class string_field

    } // namespace internal

    class text : public internal::string_field { public: using internal::string_field::string_field; };
    class class_ : public internal::string_field { public: using internal::string_field::string_field; };

    class attr
    {
     public:
        template <typename S1, typename S2> inline attr(S1 aName, S2 aValue) : name{to_String(aName)}, value{to_String(aValue)} {}
        String* name;
        String* value;
    };

    namespace internal
    {
        inline void set(HTMLElement* aElement, text&& aText) { aElement->set_textContent(aText); }
        inline void set(HTMLElement* aElement, class_&& aClass) { add_class(aElement, aClass); }
        inline void set(HTMLElement* aElement, attr&& aAttr) { aElement->setAttribute(aAttr.name, aAttr.value); }

        inline void set(HTMLElement*) {}
        template <typename Arg, typename ... Args> inline void set(HTMLElement* aElement, Arg&& first, Args&& ... args)
        {
            set(aElement, std::forward<Arg>(first));
            set(aElement, std::forward<Args>(args) ...);
        }
    } // namespace internal

    template <typename ... Args> inline HTMLElement* append_child(HTMLElement* parent, const char* aType, Args&& ... args)
    {
        auto* element = document.createElement(aType);
        internal::set(element, std::forward<Args>(args) ...);
        parent->appendChild(element);
        return element;
    }

} // namespace toolkit

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
