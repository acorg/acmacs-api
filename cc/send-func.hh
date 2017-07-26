#pragma once

#include <functional>

// ----------------------------------------------------------------------

enum class send_message_type { text, binary };

using SendFunc = std::function<void (std::string, send_message_type)>;

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
