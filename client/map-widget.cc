#include "acmacs-map-draw/draw.hh"
#include "map-widget.hh"
#include "chart-ace.hh"
#include "surface-canvas.hh"

// ----------------------------------------------------------------------

inline void MapHandler::show_widget(ChartAce* aChartAce)
{
    if (!mWidget)
        mWidget = new MapWidget{this, aChartAce};
    mWidget->show();

} // MapHandler::show_widget

// ----------------------------------------------------------------------

inline void MapHandler::hide_widget()
{
    if (mWidget)
        mWidget->hide();

} // MapHandler::hide_widget

// ----------------------------------------------------------------------

void MapHandler::reset()
{
    if (mWidget)
        mWidget->reset();

} // MapHandler::reset

// ----------------------------------------------------------------------

void MapHandler::run()
{
    send(new client::Command_chart{mChartId});

} // MapHandler::run

// ----------------------------------------------------------------------

void MapHandler::on_message(client::RawMessage* aMessage)
{
    auto* msg = (client::Response_chart*)aMessage;
    show_widget(new ChartAce{(client::ChartAceData*)msg->get_chart_ace()});

} // MapHandler::on_message

// ----------------------------------------------------------------------

void MapHandler::on_error(String* aMessage)
{

} // MapHandler::on_error

// ----------------------------------------------------------------------

MapWidget::MapWidget(MapHandler* aHandler, ChartAce* aChartAce)
    : mHandler{aHandler}, mChartAce{aChartAce}, mSurface(nullptr)
{
    log("ace", mChartAce);
    create();
    attach();

} // MapWidget::MapWidget

// ----------------------------------------------------------------------

MapWidget::~MapWidget()
{
    delete mDraw;
    mDraw = nullptr;
      // dettach();
    delete mChartAce;
    mChartAce = nullptr;
    delete mSurface;
    mSurface = nullptr;

} // MapWidget::~MapWidget

// ----------------------------------------------------------------------

void MapWidget::show()
{
    toolkit::remove_class(div, "hidden");

} // MapWidget::show

// ----------------------------------------------------------------------

void MapWidget::hide()
{
    toolkit::add_class(div, "hidden");

} // MapWidget::hide

// ----------------------------------------------------------------------

void MapWidget::reset()
{

} // MapWidget::reset

// ----------------------------------------------------------------------

void MapWidget::create()
{
    using namespace toolkit;

    div = append_child(document.get_body(), "div", class_{"antigenic-map box-shadow-popup hidden"});
    title = (HTMLCanvasElement*)append_child(div, "div", class_{"title"}, text{concat(mChartAce->name(), " ", mChartAce->number_of_antigens(), ":", mChartAce->number_of_sera())});
      // canvas width and height must be explicitly set here (not in css)
    canvas = (HTMLCanvasElement*)append_child(div, "canvas", attr{"width", 500}, attr{"height", 500}, text{"Antigenic map shown here in the browsers supporting html canvas"});
    ((client::EventTarget&)client::window).set_("CCC", (client::Node*)canvas);

    mSurface = new SurfaceCanvas(canvas, {0, 0, 8.5});

    sample_drawings();

} // MapWidget::create

// ----------------------------------------------------------------------

void MapWidget::sample_drawings()
{
    mSurface->grid(Scaled{1}, Color{0xA0A0A0}, Pixels{1});
    mSurface->line({1.1, 1}, {1.1, 2}, 0xFF0000, Pixels{1});
    mSurface->circle({2, 2}, Scaled{1}, Aspect{1.0}, Rotation{0.0}, Color{0x00FF00}, Pixels{1});
    mSurface->circle_filled({2, 4}, Scaled{1}, Aspect{1.0}, Rotation{0.0}, Color{0x00FFA0}, Pixels{1}, Color{0xA0FFE0});
    mSurface->sector_filled({3.2, 2.2}, Scaled{3}, NoRotation, RotationDegrees(60), Color{0x00FFA0}, Pixels{1}, Color{0x008000}, Pixels{1}, Surface::Dash::Dash2, Color{0xC0FFF0});
    mSurface->square_filled({5.9, 2.2}, Scaled{1}, AspectNormal, NoRotation, Color{0xFFA000}, Pixels{1}, Color{0xFFE0B0});

} // MapWidget::sample_drawings

// ----------------------------------------------------------------------

void MapWidget::attach()
{
    using namespace client;
    using namespace toolkit;

} // MapWidget::attach

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
