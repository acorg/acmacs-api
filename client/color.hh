#pragma once

#include "asm.hh"
#include "string-client.hh"

// ----------------------------------------------------------------------

class Color
{
 public:
    using value_type = uint32_t;
    constexpr static const value_type NoChange = 0xFFFFFFFE;

    inline Color() : mSColor("pink"_S), mIColor{NoChange} {}
    template <typename Uint, typename std::enable_if<std::is_integral<Uint>::value>::type* = nullptr> constexpr inline Color(Uint aColor) : mSColor{nullptr}, mIColor(static_cast<uint32_t>(aColor)) {}
    inline Color(const char* aColor) : mSColor{::to_String(aColor)}, mIColor{NoChange} {}
    inline Color(String* aColor) : mSColor{aColor}, mIColor{NoChange} {}
    template <typename Uint, typename std::enable_if<std::is_integral<Uint>::value>::type* = nullptr> inline Color& operator=(Uint aColor) { mSColor = nullptr; mIColor = static_cast<uint32_t>(aColor); return *this; }
    template <typename Uint, typename std::enable_if<std::is_integral<Uint>::value>::type* = nullptr> inline Color& operator=(String* aColor) { mSColor = aColor; mIColor = NoChange; return *this; }
    template <typename Uint, typename std::enable_if<std::is_integral<Uint>::value>::type* = nullptr> inline Color& operator=(const char* aColor) { mSColor = ::to_String(aColor); mIColor = NoChange; return *this; }

    // inline bool operator == (const Color& aColor) const { return mSColor ? mSColor == aColor.mColor; }
    // inline bool operator != (const Color& aColor) const { return ! operator==(aColor); }
    // inline bool operator < (const Color& aColor) const { return mColor < aColor.mColor; }


    // inline size_t alphaI() const { return static_cast<size_t>((mColor >> 24) & 0xFF); }
    // inline void alphaI(value_type v) { mColor = (mColor & 0xFFFFFF) | ((v & 0xFF) << 24); }
    // inline size_t rgbI() const { return static_cast<size_t>(mColor & 0xFFFFFF); }

    inline bool empty() const { return client::is_empty(mSColor) && mIColor == NoChange; }

    // void light(double value);

    // inline void set_transparency(double aTransparency) { mColor = (mColor & 0x00FFFFFF) | ((int(aTransparency * 255.0) & 0xFF) << 24); }

    // void from_string(std::string aColor);
    inline client::String* to_String() const
        {
            if (mSColor) {
                return mSColor;
            }
            else {
                return concat("rgba(", ::to_String(red()), ",", ::to_String(green()), ",", ::to_String(blue()), ",", ::to_String(alpha()), ")");
            }
        }

    inline std::string to_string() const { return static_cast<std::string>(*to_String()); }
    inline client::String* to_hex_String() const { return concat("#", client::to_hex_string(mIColor, 6)); }
    inline std::string to_hex_string() const { return static_cast<std::string>(*to_hex_String()); }

    // static const value_type DistinctColors[];
    // static std::vector<std::string> distinct_colors();

 private:
    String* mSColor;
    value_type mIColor; // 4 bytes, most->least significant: transparency-red-green-blue, 0x00FF0000 - opaque red, 0xFF000000 - fully transparent

    inline double alpha() const { return double(0xFF - ((mIColor >> 24) & 0xFF)) / 255.0; }
    inline double red() const { return double((mIColor >> 16) & 0xFF); }
    inline double green() const { return double((mIColor >> 8) & 0xFF); }
    inline double blue() const { return double(mIColor & 0xFF); }

}; // class Color

const Color ColorNoChange{Color::NoChange};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
