#include "asm.hh"

// ----------------------------------------------------------------------

void make_asm_definitions()
{
    __asm__("window.object_keys = Object.keys;");
      // https://stackoverflow.com/questions/4059147/check-if-a-variable-is-a-string
    __asm__("window.is_string = function(obj) { return Object.prototype.toString.call(obj) === '[object String]'; };");
    __asm__("window.typeof = function(obj) { return typeof(obj); };");
    __asm__("window.debug = function() { debugger; };");
    __asm__("window.make_undefined = function() { return undefined; };");
    __asm__("window.is_undefined = function(obj) { return obj === undefined; };");
    __asm__("window.is_undefined_or_null = function(obj) { return obj === undefined || obj === null; };");
    __asm__("window.is_not_null = function(obj) { return obj !== undefined && obj !== null; };");
    __asm__("window.make_cnonce = function() { return Math.floor(Math.random() * 0xFFFFFFFF).toString(16); };");
    __asm__("window.make_number = function(num) { return new Number(num); };");
    __asm__("window.console_log = console.log;");
    __asm__("window.console_error = console.error;");

      // https://stackoverflow.com/questions/4810841/how-can-i-pretty-print-json-using-javascript
    __asm__(R"(window.json_syntax_highlight = function(data) {
    data = data.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
    return data.replace(/("(\\u[a-zA-Z0-9]{4}|\\[^u]|[^\\"])*"(\s*:)?|\b(true|false|null)\b|-?\d+(?:\.\d*)?(?:[eE][+\-]?\d+)?)/g, function (match) {
        var cls = 'number';
        if (/^"/.test(match)) {
            if (/:$/.test(match)) {
                cls = 'key';
            } else {
                cls = 'string';
            }
        } else if (/true|false/.test(match)) {
            cls = 'boolean';
        } else if (/null/.test(match)) {
            cls = 'null';
        }
        return '<span class="' + cls + '">' + match + '</span>';
    });
};)");

} // make_asm_definitions

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
