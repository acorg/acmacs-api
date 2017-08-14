#include "surface-canvas.hh"
#include "string-client.hh"

// ----------------------------------------------------------------------

class context
{
 public:
    context(SurfaceCanvas& aSurface)
        : mSurface{aSurface}, mScale{aSurface.scale()}, mConext{static_cast<client::CanvasRenderingContext2D*>(aSurface.context())}
        {
              // std::cerr << "origin_offset: " << aSurface.origin_offset() << "  scale: " << mScale << std::endl;
            log("context scale", mScale);
            mConext->save();
            translate(aSurface.origin_offset());
            scale(mScale);
            translate(- aSurface.viewport().origin);
            if (aSurface.clip()) {
                new_path();
                move_to(aSurface.viewport().origin);
                line_to(aSurface.viewport().top_right());
                line_to(aSurface.viewport().bottom_right());
                line_to(aSurface.viewport().bottom_left());
                close_path();
                clip();
            }
        }
    ~context()
        {
            log("~context");
            mConext->restore();
        }

    template <typename S> inline context& set_line_width(S aWidth) { mConext->set_lineWidth(convert(aWidth)); return *this; }
      // inline context& set_source_rgba(Color aColor) { cairo_set_source_rgba(cairo_context(), aColor.red(), aColor.green(), aColor.blue(), aColor.alpha()); return *this; }
    inline context& set_stroke_style(Color aColor) { mConext->set_strokeStyle(aColor.to_hex_String()); return *this; }

    inline context& set_line_cap(Surface::LineCap aLineCap) { mConext->set_lineCap(canvas_line_cap(aLineCap)); return *this; }
    inline context& set_line_join(Surface::LineJoin aLineJoin) { mConext->set_lineJoin(canvas_line_join(aLineJoin)); return *this; }
    inline context& set_line_dash(Surface::Dash aLineDash)
        {
            double dash_size;
            switch (aLineDash) {
              case Surface::Dash::NoDash:
                  mConext->setLineDash(new client::Array{});
                  break;
              case Surface::Dash::Dash1:
                  dash_size = convert(Pixels{1});
                  mConext->setLineDash(new client::Array{dash_size, dash_size});
                  break;
              case Surface::Dash::Dash2:
                  dash_size = convert(Pixels{5});
                  mConext->setLineDash(new client::Array{dash_size, dash_size});
                  break;
            }
            return *this;
        }

    inline context& move_to() { mConext->moveTo(0.0, 0.0); return *this; }
    inline context& move_to(const Location& a) { mConext->moveTo(a.x, a.y); return *this; }
    template <typename S> inline context& move_to(S x, S y) { mConext->moveTo(convert(x), convert(y)); return *this; }
    inline context& line_to(const Location& a) { mConext->lineTo(a.x, a.y); return *this; }
    template <typename S> inline context& line_to(S x, S y) { mConext->lineTo(convert(x), convert(y)); return *this; }
    inline context& lines_to(std::vector<Location>::const_iterator first, std::vector<Location>::const_iterator last) { for ( ; first != last; ++first) { line_to(*first); } return *this; }
    inline context& rectangle(const Location& a, const Size& s) { mConext->rect(a.x, a.y, s.width, s.height); return *this; }
    template <typename S> inline context& rectangle(S x1, S y1, S x2, S y2) { mConext->rect(convert(x1), convert(y1), convert(x2) - convert(x1), convert(y2) - convert(y1)); return *this; }
      // inline context& arc(const Location& a, double radius, double angle1, double angle2) { mConext->arc(a.x, a.y, radius, angle1, angle2); return *this; }
    template <typename S> inline context& circle(S radius) { mConext->arc(0.0, 0.0, convert(radius), 0.0, 2.0 * M_PI); return *this; }
    template <typename S> inline context& arc(S radius, Rotation start, Rotation end) { mConext->arc(0.0, 0.0, convert(radius), start.value(), end.value()); return *this; }
    inline context& circle(const Location& a, double radius) { mConext->arc(a.x, a.y, radius, 0.0, 2.0 * M_PI); return *this; }
    inline context& stroke() { mConext->stroke(); return *this; }
      // inline context& stroke_preserve() { mConext->stroke_preserve(); return *this; }
    inline context& fill() { mConext->fill(); return *this; }
      // inline context& fill_preserve() { mConext->fill_preserve(); return *this; }
    inline context& translate(const Size& a) { mConext->translate(a.width, a.height); return *this; }
    inline context& translate(const Location& a) { mConext->translate(a.x, a.y); return *this; }
    inline context& rotate(Rotation aAngle) { mConext->rotate(aAngle.value()); return *this; }
    inline context& scale(double x, double y) { mConext->scale(x, y); return *this; }
    inline context& scale(double x) { mConext->scale(x, x); return *this; }
    inline context& aspect(Aspect x) { mConext->scale(x.value(), 1.0); return *this; }
    inline context& clip() { mConext->clip(); return *this; }
    inline context& new_path() { mConext->beginPath(); return *this; }
    inline context& close_path() { mConext->closePath(); return *this; }
    inline context& close_path_if(bool aClose) { if (aClose) close_path(); return *this; }
    // inline context& append_path(CairoPath& aPath) { mConext->append_path(aPath); return *this; }
    // inline CairoPath copy_path() { return std::move(mConext->copy_path()); }

    // template <typename S> inline context& prepare_for_text(S aSize, const TextStyle& aTextStyle) { mConext->select_font_face(aTextStyle.font_family().c_str(), mConext->font_slant(aTextStyle.slant()), mConext->font_weight(aTextStyle.weight())); mConext->set_font_size(convert(aSize)); return *this; }
    //inline context& show_text(std::string aText) { mConext->fillText(aText.c_str()); return *this; }
    //inline context& text_extents(std::string aText, cairo_text_extents_t& extents) { mConext->text_extents(aText.c_str(), &extents); return *this; }

      // if Location::x is negative - move_to, else - path_to. It assumes origin is {0,0}!!!
    inline context& move_to_line_to(std::vector<Location>::const_iterator first, std::vector<Location>::const_iterator last)
        {
            for ( ; first != last; ++first) {
                if (first->x < 0)
                    move_to({std::abs(first->x), std::abs(first->y)});
                else
                    line_to(*first);
            }
            return *this;
        }

      // the same as above but with raw data
    inline context& move_to_line_to(const double* first, const double* last)
        {
            for ( ; first != last; first += 2) {
                if (*first < 0)
                    move_to({std::abs(*first), std::abs(*(first+1))});
                else
                    line_to({*first, *(first+1)});
            }
            return *this;
        }

    inline context& close_move_to_line_to(const double* first, const double* last)
        {
            for ( ; first != last; first += 2) {
                if (*first < 0) {
                    close_path();
                    move_to({std::abs(*first), std::abs(*(first+1))});
                }
                else
                    line_to({*first, *(first+1)});
            }
            return *this;
        }

 private:
    SurfaceCanvas& mSurface;
    double mScale;
    client::CanvasRenderingContext2D* mConext;

    inline double convert(double aValue) { return aValue; }
    inline double convert(Scaled aValue) { return aValue.value(); }
    inline double convert(Pixels aValue) { return aValue.value() / mScale; }

    inline String* canvas_line_cap(Surface::LineCap aLineCap) const
        {
            switch (aLineCap) {
              case Surface::LineCap::Butt:
                  return "butt"_S;
              case Surface::LineCap::Round:
                  return "round"_S;
              case Surface::LineCap::Square:
                  return "square"_S;
            }
            return "butt"_S; // gcc wants return
        }

    inline String* canvas_line_join(Surface::LineJoin aLineJoin) const
        {
            switch (aLineJoin) {
              case Surface::LineJoin::Miter:
                  return "miter"_S;
              case Surface::LineJoin::Round:
                  return "round"_S;
              case Surface::LineJoin::Bevel:
                  return "bevel"_S;
            }
            return "miter"_S; // gcc wants return
        }

    inline String* canvas_font_slant(TextStyle::Slant aSlant) const
        {
            switch (aSlant) {
              case TextStyle::Slant::Normal:
                  return ""_S;
              case TextStyle::Slant::Italic:
                  return "italic"_S;
                    // case TextStyle::Slant::Oblique:
                    //     return "oblique"_S;
            }
            return ""_S; // gcc wants return
        }

    inline String* canvas_font_weight(TextStyle::Weight aWeight) const
        {
            switch (aWeight) {
              case TextStyle::Weight::Normal:
                  return ""_S;
              case TextStyle::Weight::Bold:
                  return "bold"_S;
            }
            return ""_S; // gcc wants return
        }
};

// ----------------------------------------------------------------------

template <typename S> static inline void s_line(SurfaceCanvas& aSurface, const Location& a, const Location& b, Color aColor, S aWidth, Surface::LineCap aLineCap)
{
    context(aSurface)
            .set_line_width(aWidth)
            .set_stroke_style(aColor)
            .set_line_cap(aLineCap)
            .move_to(a)
            .line_to(b)
            .stroke();
}

void SurfaceCanvas::line(const Location& a, const Location& b, Color aColor, Pixels aWidth, LineCap aLineCap)
{
    s_line(*this, a, b, aColor, aWidth, aLineCap);

} // SurfaceCanvas::line

void SurfaceCanvas::line(const Location& a, const Location& b, Color aColor, Scaled aWidth, LineCap aLineCap)
{
    s_line(*this, a, b, aColor, aWidth, aLineCap);

} // SurfaceCanvas::line

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
