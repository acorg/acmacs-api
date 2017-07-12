#pragma once

#error Not used

// ----------------------------------------------------------------------

inline std::ostream& operator << (std::ostream& out, const bsoncxx::array::view& aArray)
{
    auto first = std::begin(aArray), last = std::end(aArray);
    out << '[';
    for (bool insert_separator = false; first != last; ++first) {
        if (insert_separator)
            out << ", ";
        else
            insert_separator = true;
        out << *first;
    }
    return out << ']';
}

inline std::ostream& operator << (std::ostream& out, const bsoncxx::document::view& aDocument)
{
    auto first = std::begin(aDocument), last = std::end(aDocument);
    out << '{';
    for (bool insert_separator = false; first != last; ++first) {
        if (insert_separator)
            out << ", ";
        else
            insert_separator = true;
        out << *first;
    }
    return out << '}';
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
