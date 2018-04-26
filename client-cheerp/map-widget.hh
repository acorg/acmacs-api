#pragma once

#include "widget.hh"
#include "handler.hh"

// ----------------------------------------------------------------------

class MapWidget;
class ChartAce;
class ChartDraw;
class SurfaceCanvas;

class MapHandler : public Handler
{
 public:
    inline MapHandler(Application* aApp, String* aChartId)
        : Handler{aApp}, mChartId{aChartId}, mWidget{nullptr}
        {
            log("MapHandler");
        }
      //inline MapHandler(const MapHandler&) = default;
    inline ~MapHandler() override { log("~MapHandler"); }

    void reset() override;
    void run();
    void on_message(client::RawMessage* aMessage) override;
    void on_error(String* aMessage) override;

 private:
    friend class MapWidget;

    String* mChartId;
    MapWidget* mWidget;

    void show_widget(ChartAce* aChartAce);
    void hide_widget();

}; // class MapHandler

// ----------------------------------------------------------------------

class MapWidget : public Widget
{
 public:
    MapWidget(MapHandler* aHandler, ChartAce* aChartAce);
    ~MapWidget();

    void show() override;
    void hide() override;
    void reset() override;

 private:
    MapHandler* mHandler;
    ChartAce* mChartAce;
    ChartDraw* mDraw;
    SurfaceCanvas* mSurface;

    HTMLElement* div;
    HTMLElement* title;
    HTMLCanvasElement* canvas;

    void create();
    void attach();
    void sample_drawings();

}; // class MapWidget

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
