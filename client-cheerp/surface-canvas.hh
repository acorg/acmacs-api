#pragma once

#include <cheerp/clientlib.h>

#include "acmacs-draw/surface.hh"

// ----------------------------------------------------------------------

class SurfaceCanvas : public Surface
{
 public:
    SurfaceCanvas(client::HTMLCanvasElement* aCanvas, const Viewport& aViewport);

    void line(const Location& a, const Location& b, Color aColor, Pixels aWidth, LineCap aLineCap = LineCap::Butt) override;
    void line(const Location& a, const Location& b, Color aColor, Scaled aWidth, LineCap aLineCap = LineCap::Butt) override;
    inline void rectangle(const Location& a, const Size& s, Color aColor, Pixels aWidth, LineCap aLineCap = LineCap::Butt) override {}
    inline void rectangle_filled(const Location& a, const Size& s, Color aOutlineColor, Pixels aWidth, Color aFillColor, LineCap aLineCap = LineCap::Butt) override {}

    void circle(const Location& aCenter, Pixels aDiameter, Aspect aAspect, Rotation aAngle, Color aOutlineColor, Pixels aOutlineWidth) override;
    void circle(const Location& aCenter, Scaled aDiameter, Aspect aAspect, Rotation aAngle, Color aOutlineColor, Pixels aOutlineWidth) override;
    void circle_filled(const Location& aCenter, Pixels aDiameter, Aspect aAspect, Rotation aAngle, Color aOutlineColor, Pixels aOutlineWidth, Color aFillColor) override;
    void circle_filled(const Location& aCenter, Scaled aDiameter, Aspect aAspect, Rotation aAngle, Color aOutlineColor, Pixels aOutlineWidth, Color aFillColor) override;
    void sector_filled(const Location& aCenter, Scaled aDiameter, Rotation aStart, Rotation aEnd, Color aOutlineColor, Pixels aOutlineWidth, Color aRadiusColor, Pixels aRadiusWidth, Dash aRadiusDash, Color aFillColor) override;
    void square_filled(const Location& aCenter, Pixels aSide, Aspect aAspect, Rotation aAngle, Color aOutlineColor, Pixels aOutlineWidth, Color aFillColor, LineCap aLineCap = LineCap::Butt) override;
    void square_filled(const Location& aCenter, Scaled aSide, Aspect aAspect, Rotation aAngle, Color aOutlineColor, Pixels aOutlineWidth, Color aFillColor, LineCap aLineCap = LineCap::Butt) override;
    void triangle_filled(const Location& aCenter, Pixels aSide, Aspect aAspect, Rotation aAngle, Color aOutlineColor, Pixels aOutlineWidth, Color aFillColor, LineCap aLineCap = LineCap::Butt) override;
    void triangle_filled(const Location& aCenter, Scaled aSide, Aspect aAspect, Rotation aAngle, Color aOutlineColor, Pixels aOutlineWidth, Color aFillColor, LineCap aLineCap = LineCap::Butt) override;
    void triangle_filled(const Location& aCorner1, const Location& aCorner2, const Location& aCorner3, Color aOutlineColor, Pixels aOutlineWidth, Color aFillColor, LineCap aLineCap = LineCap::Butt) override;

    void path_outline(std::vector<Location>::const_iterator first, std::vector<Location>::const_iterator last, Color aOutlineColor, Pixels aOutlineWidth, bool aClose = false, LineCap aLineCap = LineCap::Butt) override;
    void path_outline(const double* first, const double* last, Color aOutlineColor, Pixels aOutlineWidth, bool aClose = false, LineCap aLineCap = LineCap::Butt) override;
    void path_fill(std::vector<Location>::const_iterator first, std::vector<Location>::const_iterator last, Color aFillColor) override;
    void path_fill(const double* first, const double* last, Color aFillColor) override;

    void text(const Location& a, std::string aText, Color aColor, Pixels aSize, const TextStyle& aTextStyle = TextStyle(), Rotation aRotation = NoRotation) override;
    void text(const Location& a, std::string aText, Color aColor, Scaled aSize, const TextStyle& aTextStyle = TextStyle(), Rotation aRotation = NoRotation) override;
    void text_right_aligned(const Location& aEnd, std::string aText, Color aColor, Pixels aSize, const TextStyle& aTextStyle = TextStyle(), Rotation aRotation = NoRotation) override;
    void text_right_aligned(const Location& aEnd, std::string aText, Color aColor, Scaled aSize, const TextStyle& aTextStyle = TextStyle(), Rotation aRotation = NoRotation) override;
    Size text_size(std::string aText, Pixels aSize, const TextStyle& aTextStyle = TextStyle(), double* x_bearing = nullptr) override;
    Size text_size(std::string aText, Scaled aSize, const TextStyle& aTextStyle = TextStyle(), double* x_bearing = nullptr) override;

    inline void new_page() override { log_warning("new_page is not supported in SurfaceCanvas"); }

    inline client::RenderingContext* get_context_2d() { return canvas()->getContext("2d"); }

 protected:
    inline SurfaceCanvas(const Location& aOriginInParent, Scaled aWidthInParent, const Viewport& aViewport)
        : Surface{aOriginInParent, aWidthInParent, aViewport}, mCanvas{nullptr} {}

    Surface* make_child(const Location& aOriginInParent, Scaled aWidthInParent, const Viewport& aViewport, bool aClip) override;
    virtual client::HTMLCanvasElement* canvas() { return mCanvas; }

 private:
    client::HTMLCanvasElement* mCanvas;

    friend class SurfaceCanvasChild;

}; // class SurfaceCanvas

// ----------------------------------------------------------------------

class SurfaceCanvasChild : public SurfaceChild<SurfaceCanvas>
{
 public:
    inline SurfaceCanvas& parent() override { return mParent; }
    inline const SurfaceCanvas& parent() const override { return mParent; }

 protected:
    inline SurfaceCanvasChild(SurfaceCanvas& aParent, const Location& aOriginInParent, Scaled aWidthInParent, const Viewport& aViewport, bool aClip)
        : SurfaceChild{aOriginInParent, aWidthInParent, aViewport, aClip}, mParent{aParent} {}
    client::HTMLCanvasElement* canvas() override { return mParent.canvas(); }

 private:
    SurfaceCanvas& mParent;

    friend class SurfaceCanvas;

}; // class SurfaceCanvasChild

// ----------------------------------------------------------------------

inline Surface* SurfaceCanvas::make_child(const Location& aOriginInParent, Scaled aWidthInParent, const Viewport& aViewport, bool aClip)
{
    return new SurfaceCanvasChild(*this, aOriginInParent, aWidthInParent, aViewport, aClip);
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
