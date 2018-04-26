#pragma once

// ----------------------------------------------------------------------

// Not clear what weak-vtables warning means in the context of generating js
// warning: '<class>' has no out-of-line virtual method definitions; its vtable will be emitted in every translation unit [-Wweak-vtables]

#ifdef __clang__
#pragma GCC diagnostic ignored "-Wweak-vtables"
#endif

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
