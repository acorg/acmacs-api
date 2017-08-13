#pragma once

#include <cheerp/clientlib.h>

#include "acmacs-draw/surface.hh"

// ----------------------------------------------------------------------

class SurfaceCanvas : public Surface
{
 public:
    inline SurfaceCanvas(client::HTMLCanvasElement* aCanvas) : mCanvas(aCanvas) {}

    inline void move(const Location& aOriginInParent) override {}
    inline void move_resize(const Location& aOriginInParent, double aWidthInParent) override {}

    inline void line(const Location& a, const Location& b, Color aColor, Pixels aWidth, LineCap aLineCap = LineCap::Butt) override {}
    inline void line(const Location& a, const Location& b, Color aColor, Scaled aWidth, LineCap aLineCap = LineCap::Butt) override {}
    inline void rectangle(const Location& a, const Size& s, Color aColor, Pixels aWidth, LineCap aLineCap = LineCap::Butt) override {}
    inline void rectangle_filled(const Location& a, const Size& s, Color aOutlineColor, Pixels aWidth, Color aFillColor, LineCap aLineCap = LineCap::Butt) override {}

    inline void circle(const Location& aCenter, Pixels aDiameter, Aspect aAspect, Rotation aAngle, Color aOutlineColor, Pixels aOutlineWidth) override {}
    inline void circle(const Location& aCenter, Scaled aDiameter, Aspect aAspect, Rotation aAngle, Color aOutlineColor, Pixels aOutlineWidth) override {}
    inline void circle_filled(const Location& aCenter, Pixels aDiameter, Aspect aAspect, Rotation aAngle, Color aOutlineColor, Pixels aOutlineWidth, Color aFillColor) override {}
    inline void circle_filled(const Location& aCenter, Scaled aDiameter, Aspect aAspect, Rotation aAngle, Color aOutlineColor, Pixels aOutlineWidth, Color aFillColor) override {}
    inline void sector_filled(const Location& aCenter, Scaled aDiameter, Rotation aStart, Rotation aEnd, Color aOutlineColor, Pixels aOutlineWidth, Color aRadiusColor, Pixels aRadiusWidth, Dash aRadiusDash, Color aFillColor) override {}
    inline void square_filled(const Location& aCenter, Pixels aSide, Aspect aAspect, Rotation aAngle, Color aOutlineColor, Pixels aOutlineWidth, Color aFillColor, LineCap aLineCap = LineCap::Butt) override {}
    inline void square_filled(const Location& aCenter, Scaled aSide, Aspect aAspect, Rotation aAngle, Color aOutlineColor, Pixels aOutlineWidth, Color aFillColor, LineCap aLineCap = LineCap::Butt) override {}
    inline void triangle_filled(const Location& aCenter, Pixels aSide, Aspect aAspect, Rotation aAngle, Color aOutlineColor, Pixels aOutlineWidth, Color aFillColor, LineCap aLineCap = LineCap::Butt) override {}
    inline void triangle_filled(const Location& aCenter, Scaled aSide, Aspect aAspect, Rotation aAngle, Color aOutlineColor, Pixels aOutlineWidth, Color aFillColor, LineCap aLineCap = LineCap::Butt) override {}
    inline void triangle_filled(const Location& aCorner1, const Location& aCorner2, const Location& aCorner3, Color aOutlineColor, Pixels aOutlineWidth, Color aFillColor, LineCap aLineCap = LineCap::Butt) override {}

    inline void path_outline(std::vector<Location>::const_iterator first, std::vector<Location>::const_iterator last, Color aOutlineColor, Pixels aOutlineWidth, bool aClose = false, LineCap aLineCap = LineCap::Butt) override {}
    inline void path_outline(const double* first, const double* last, Color aOutlineColor, Pixels aOutlineWidth, bool aClose = false, LineCap aLineCap = LineCap::Butt) override {}
    inline void path_fill(std::vector<Location>::const_iterator first, std::vector<Location>::const_iterator last, Color aFillColor) override {}
    inline void path_fill(const double* first, const double* last, Color aFillColor) override {}

    inline void double_arrow(const Location& a, const Location& b, Color aColor, Pixels aLineWidth, Pixels aArrowWidth) override {}
    inline void grid(Scaled aStep, Color aLineColor, Pixels aLineWidth) override {}
    inline void border(Color aLineColor, Pixels aLineWidth) override {}
    inline void background(Color aColor) override {}

    inline void text(const Location& a, std::string aText, Color aColor, Pixels aSize, const TextStyle& aTextStyle = TextStyle(), Rotation aRotation = Rotation{0}) override {}
    inline void text(const Location& a, std::string aText, Color aColor, Scaled aSize, const TextStyle& aTextStyle = TextStyle(), Rotation aRotation = Rotation{0}) override {}
    inline void text_right_aligned(const Location& aEnd, std::string aText, Color aColor, Pixels aSize, const TextStyle& aTextStyle = TextStyle(), Rotation aRotation = Rotation{0}) override {}
    inline void text_right_aligned(const Location& aEnd, std::string aText, Color aColor, Scaled aSize, const TextStyle& aTextStyle = TextStyle(), Rotation aRotation = Rotation{0}) override {}
    inline Size text_size(std::string aText, Pixels aSize, const TextStyle& aTextStyle = TextStyle(), double* x_bearing = nullptr) override {}
    inline Size text_size(std::string aText, Scaled aSize, const TextStyle& aTextStyle = TextStyle(), double* x_bearing = nullptr) override {}

    inline void new_page() override { log_warning("new_page is not supported in SurfaceCanvas"); }

 protected:
    inline Location arrow_head(const Location& a, double angle, double sign, Color aColor, Pixels aArrowWidth) override {}

    Surface* make_child(const Location& aOriginInParent, Scaled aWidthInParent, const Viewport& aViewport, bool aClip) override;

 private:
    client::HTMLCanvasElement* mCanvas;

}; // class SurfaceCanvas

// ----------------------------------------------------------------------

class SurfaceCanvasChild : public SurfaceCanvas
{
 public:
    inline Surface& root() override { return mParent.root(); }
    inline const Surface& root() const override { return mParent.root(); }

    inline void move(const Location& aOriginInParent) override { change_origin(aOriginInParent); }
    inline void move_resize(const Location& aOriginInParent, double aWidthInParent) override { change_origin(aOriginInParent); change_width_in_parent(aWidthInParent); }

    inline double scale() const override { return mParent.scale() * (width_in_parent() / viewport().size.width); }
    inline Location origin_offset() const override { return mParent.origin_offset() + origin_in_parent() * mParent.scale(); }

 protected:
    inline bool clip() const override { return mClip; }

 private:
    SurfaceCanvas& mParent;
    bool mClip;                 // force surface area clipping

    inline SurfaceCanvasChild(SurfaceCanvas& aParent, const Location& aOriginInParent, Scaled aWidthInParent, const Viewport& aViewport, bool aClip)
        : SurfaceCanvas{aOriginInParent, aWidthInParent, aViewport}, mParent{aParent}, mClip{aClip} {}
    // inline SurfaceCanvasChild(SurfaceCanvas& aParent, const Size& aOffset, const Size& aSize, double aScale, bool aClip)
    //     : mParent(aParent), mOffset(aOffset), mSize(aSize), mScale(aScale), mClip(aClip) {}

    friend class SurfaceCanvas;

}; // class SurfaceCanvasChild

// ----------------------------------------------------------------------

Surface* SurfaceCanvas::make_child(const Location& aOriginInParent, Scaled aWidthInParent, const Viewport& aViewport, bool aClip)
{
    return new SurfaceCanvasChild(*this, aOriginInParent, aWidthInParent, aViewport, aClip);
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
