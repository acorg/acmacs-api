#include "acmacs-map-draw/draw.hh"
#include "map-widget.hh"
#include "chart-ace.hh"

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
    : mHandler{aHandler}, mChartAce{aChartAce}
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
    canvas = (HTMLCanvasElement*)append_child(div, "canvas");

} // MapWidget::create

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
