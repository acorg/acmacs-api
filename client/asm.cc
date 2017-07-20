#include "asm.hh"

// ----------------------------------------------------------------------

void make_asm_definitions()
{
    __asm__("window.object_keys = Object.keys;");
      // https://stackoverflow.com/questions/4059147/check-if-a-variable-is-a-string
    __asm__("window.is_string = function(obj) { return Object.prototype.toString.call(obj) === '[object String]'; };");
    __asm__("window.make_undefined = function() { return undefined; };");
    __asm__("window.is_undefined = function(obj) { return obj === undefined; };");
    __asm__("window.make_cnonce = function() { return Math.floor(Math.random() * 0xFFFFFFFF).toString(16); };");
    __asm__("window.console_log = console.log;");

} // make_asm_definitions

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
