#include "surface-canvas.hh"
#include "string-client.hh"

// ----------------------------------------------------------------------

class context
{
 public:
    context(SurfaceCanvas& aSurface)
        : mSurface{aSurface}, mScale{aSurface.scale()}, mContext{static_cast<client::CanvasRenderingContext2D*>(aSurface.context())}
        {
              // std::cerr << "origin_offset: " << aSurface.origin_offset() << "  scale: " << mScale << std::endl;
            mContext->save();
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
            // log("~context");
            mContext->restore();
        }

    template <typename S> inline context& set_line_width(S aWidth) { mContext->set_lineWidth(convert(aWidth)); return *this; }
    inline context& set_stroke_style(Color aColor) { mContext->set_strokeStyle(aColor.to_hex_String()); return *this; }
    inline context& set_fill_style(Color aColor) { mContext->set_fillStyle(aColor.to_hex_String()); return *this; }

    inline context& set_line_cap(Surface::LineCap aLineCap) { mContext->set_lineCap(canvas_line_cap(aLineCap)); return *this; }
    inline context& set_line_join(Surface::LineJoin aLineJoin) { mContext->set_lineJoin(canvas_line_join(aLineJoin)); return *this; }
    inline context& set_line_dash(Surface::Dash aLineDash)
        {
            double dash_size;
            switch (aLineDash) {
              case Surface::Dash::NoDash:
                  mContext->setLineDash(new client::Array{});
                  break;
              case Surface::Dash::Dash1:
                  dash_size = convert(Pixels{1});
                  mContext->setLineDash(new client::Array{dash_size, dash_size});
                  break;
              case Surface::Dash::Dash2:
                  dash_size = convert(Pixels{5});
                  mContext->setLineDash(new client::Array{dash_size, dash_size});
                  break;
            }
            return *this;
        }

    inline context& move_to() { mContext->moveTo(0.0, 0.0); return *this; }
    inline context& move_to(const Location& a) { mContext->moveTo(a.x, a.y); return *this; }
    template <typename S> inline context& move_to(S x, S y) { mContext->moveTo(convert(x), convert(y)); return *this; }
    inline context& line_to(const Location& a) { mContext->lineTo(a.x, a.y); return *this; }
    template <typename S> inline context& line_to(S x, S y) { mContext->lineTo(convert(x), convert(y)); return *this; }
    inline context& lines_to(std::vector<Location>::const_iterator first, std::vector<Location>::const_iterator last) { for ( ; first != last; ++first) { line_to(*first); } return *this; }
    inline context& rectangle(const Location& a, const Size& s) { mContext->rect(a.x, a.y, s.width, s.height); return *this; }
    template <typename S> inline context& rectangle(S x1, S y1, S x2, S y2) { mContext->rect(convert(x1), convert(y1), convert(x2) - convert(x1), convert(y2) - convert(y1)); return *this; }
      // inline context& arc(const Location& a, double radius, double angle1, double angle2) { mContext->arc(a.x, a.y, radius, angle1, angle2); return *this; }
    template <typename S> inline context& circle(S radius) { mContext->arc(0.0, 0.0, convert(radius), 0.0, 2.0 * M_PI); return *this; }
    template <typename S> inline context& arc(S radius, Rotation start, Rotation end) { mContext->arc(0.0, 0.0, convert(radius), start.value(), end.value()); return *this; }
    inline context& circle(const Location& a, double radius) { mContext->arc(a.x, a.y, radius, 0.0, 2.0 * M_PI); return *this; }
    inline context& stroke() { mContext->stroke(); return *this; }
    inline context& fill() { mContext->fill(); return *this; }
    inline context& translate(const Size& a) { mContext->translate(a.width, a.height); return *this; }
    inline context& translate(const Location& a) { mContext->translate(a.x, a.y); return *this; }
    inline context& rotate(Rotation aAngle) { mContext->rotate(aAngle.value()); return *this; }
    inline context& scale(double x, double y) { mContext->scale(x, y); return *this; }
    inline context& scale(double x) { mContext->scale(x, x); return *this; }
    inline context& aspect(Aspect x) { mContext->scale(x.value(), 1.0); return *this; }
    inline context& clip() { mContext->clip(); return *this; }
    inline context& new_path() { mContext->beginPath(); return *this; }
    inline context& close_path() { mContext->closePath(); return *this; }
    inline context& close_path_if(bool aClose) { if (aClose) close_path(); return *this; }
    // inline context& append_path(CairoPath& aPath) { mContext->append_path(aPath); return *this; }
    // inline CairoPath copy_path() { return std::move(mContext->copy_path()); }

    // template <typename S> inline context& prepare_for_text(S aSize, const TextStyle& aTextStyle) { mContext->select_font_face(aTextStyle.font_family().c_str(), mContext->font_slant(aTextStyle.slant()), mContext->font_weight(aTextStyle.weight())); mContext->set_font_size(convert(aSize)); return *this; }
    //inline context& show_text(std::string aText) { mContext->fillText(aText.c_str()); return *this; }
    //inline context& text_extents(std::string aText, cairo_text_extents_t& extents) { mContext->text_extents(aText.c_str(), &extents); return *this; }

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
    client::CanvasRenderingContext2D* mContext;

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

SurfaceCanvas::SurfaceCanvas(client::HTMLCanvasElement* aCanvas, const Viewport& aViewport)
    : Surface{{0, 0}, Scaled{aCanvas->get_width()}, aViewport}, mCanvas(aCanvas)
{
    log("SurfaceCanvas", viewport(), scale(), origin_offset());

} // SurfaceCanvas::SurfaceCanvas

// ----------------------------------------------------------------------

template <typename S> static inline void s_line(SurfaceCanvas& aSurface, const Location& a, const Location& b, Color aColor, S aWidth, Surface::LineCap aLineCap)
{
    context(aSurface)
            .new_path()
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

template <typename S> static inline void s_circle(SurfaceCanvas& aSurface, const Location& aCenter, S aDiameter, Aspect aAspect, Rotation aAngle, Color aOutlineColor, Pixels aOutlineWidth)
{
    context(aSurface)
            .new_path()
            .set_line_width(aOutlineWidth)
            .translate(aCenter)
            .rotate(aAngle)
            .aspect(aAspect)
            .circle(aDiameter / 2)
            .set_stroke_style(aOutlineColor)
            .stroke();
}

void SurfaceCanvas::circle(const Location& aCenter, Pixels aDiameter, Aspect aAspect, Rotation aAngle, Color aOutlineColor, Pixels aOutlineWidth)
{
    s_circle(*this, aCenter, aDiameter, aAspect, aAngle, aOutlineColor, aOutlineWidth);

} // SurfaceCanvas::circle

void SurfaceCanvas::circle(const Location& aCenter, Scaled aDiameter, Aspect aAspect, Rotation aAngle, Color aOutlineColor, Pixels aOutlineWidth)
{
    s_circle(*this, aCenter, aDiameter, aAspect, aAngle, aOutlineColor, aOutlineWidth);

} // SurfaceCanvas::circle

// ----------------------------------------------------------------------

template <typename S> static inline void s_circle_filled(SurfaceCanvas& aSurface, const Location& aCenter, S aDiameter, Aspect aAspect, Rotation aAngle, Color aOutlineColor, Pixels aOutlineWidth, Color aFillColor)
{
    context(aSurface)
            .new_path()
            .set_line_width(aOutlineWidth)
            .translate(aCenter)
            .rotate(aAngle)
            .aspect(aAspect)
            .circle(aDiameter / 2)
            .set_fill_style(aFillColor)
            .fill()
            .set_stroke_style(aOutlineColor)
            .stroke();
}

void SurfaceCanvas::circle_filled(const Location& aCenter, Pixels aDiameter, Aspect aAspect, Rotation aAngle, Color aOutlineColor, Pixels aOutlineWidth, Color aFillColor)
{
    s_circle_filled(*this, aCenter, aDiameter, aAspect, aAngle, aOutlineColor, aOutlineWidth, aFillColor);

} // SurfaceCanvas::circle_filled

void SurfaceCanvas::circle_filled(const Location& aCenter, Scaled aDiameter, Aspect aAspect, Rotation aAngle, Color aOutlineColor, Pixels aOutlineWidth, Color aFillColor)
{
    s_circle_filled(*this, aCenter, aDiameter, aAspect, aAngle, aOutlineColor, aOutlineWidth, aFillColor);

} // SurfaceCanvas::circle_filled

// ----------------------------------------------------------------------

void SurfaceCanvas::sector_filled(const Location& aCenter, Scaled aDiameter, Rotation aStart, Rotation aEnd, Color aOutlineColor, Pixels aOutlineWidth, Color aRadiusColor, Pixels aRadiusWidth, Dash aRadiusDash, Color aFillColor)
{
    class context ctx{*this};
    ctx.translate(aCenter);

      // arc
    ctx.new_path()
            .arc(aDiameter / 2, aStart, aEnd)
            .set_line_width(aOutlineWidth)
            .set_stroke_style(aOutlineColor);
    ctx.stroke();

      // radius lines
    ctx.new_path()
            .rotate(aEnd)
            .move_to(aDiameter / 2, Scaled(0))
            .line_to(Scaled(0), Scaled(0))
            .rotate(aStart - aEnd)
            .line_to(aDiameter / 2, Scaled(0))
            .set_line_width(aRadiusWidth)
            .set_stroke_style(aRadiusColor)
            .set_line_dash(aRadiusDash)
              // .set_line_join(Surface::LineJoin::Miter)
            .stroke()
            .rotate(-aStart);

      // fill area
    ctx.arc(aDiameter / 2, aStart, aEnd)
            .set_fill_style(aFillColor)
            .fill();

} // SurfaceCanvas::sector_filled

// ----------------------------------------------------------------------

template <typename S> static inline void s_square_filled(SurfaceCanvas& aSurface, const Location& aCenter, S aSide, Aspect aAspect, Rotation aAngle, Color aOutlineColor, Pixels aOutlineWidth, Color aFillColor, Surface::LineCap aLineCap)
{
    context(aSurface)
            .new_path()
            .set_line_width(aOutlineWidth)
            .set_line_cap(aLineCap)
            .translate(aCenter)
            .rotate(aAngle)
            .rectangle(- aSide / 2 * aAspect.value(), - aSide / 2, aSide / 2 * aAspect.value(), aSide / 2)
            .set_fill_style(aFillColor)
            .fill()
            .set_stroke_style(aOutlineColor)
            .stroke();
}

void SurfaceCanvas::square_filled(const Location& aCenter, Pixels aSide, Aspect aAspect, Rotation aAngle, Color aOutlineColor, Pixels aOutlineWidth, Color aFillColor, LineCap aLineCap)
{
    s_square_filled(*this, aCenter, aSide, aAspect, aAngle, aOutlineColor, aOutlineWidth, aFillColor, aLineCap);

} // SurfaceCanvas::square_filled

void SurfaceCanvas::square_filled(const Location& aCenter, Scaled aSide, Aspect aAspect, Rotation aAngle, Color aOutlineColor, Pixels aOutlineWidth, Color aFillColor, LineCap aLineCap)
{
    s_square_filled(*this, aCenter, aSide, aAspect, aAngle, aOutlineColor, aOutlineWidth, aFillColor, aLineCap);

} // SurfaceCanvas::square_filled

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
