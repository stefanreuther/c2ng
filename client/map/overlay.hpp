/**
  *  \file client/map/overlay.hpp
  */
#ifndef C2NG_CLIENT_MAP_OVERLAY_HPP
#define C2NG_CLIENT_MAP_OVERLAY_HPP

#include "gfx/eventconsumer.hpp"
#include "gfx/canvas.hpp"
#include "afl/base/deletable.hpp"

namespace client { namespace map {

    class Renderer;
    class Callback;

    class Overlay : public afl::base::Deletable {
     public:
        typedef gfx::EventConsumer::MouseButtons_t MouseButtons_t;

        Overlay();
        ~Overlay();

        /** Draw below chart.
            Called before the chart has been drawn, bottom-most overlay first (top-most draws last).
            Use for background stuff like predictions/trails.
            \param Canvas to draw on.
            \param ren Renderer */
        // ex WChartMode::drawBefore, WChartWidget::drawPre
        virtual void drawBefore(gfx::Canvas& can, const Renderer& ren) = 0;

        /** Draw above chart.
            Called after the chart has been drawn, bottom-most overlay first (top-most draws last).
            \param Canvas to draw on.
            \param ren Renderer */
        // ex WChartWidget::drawPost
        virtual void drawAfter(gfx::Canvas& can, const Renderer& ren) = 0;

        /** Draw cursor.
            Called after the chart has been drawn, top-most overlay first.
            Use for cursors.
            \param Canvas to draw on.
            \param ren Renderer
            \retval true Draw next cursor overlay's cursor, too
            \retval false Do not draw next overlay's cursor */
        // ex WChartMode::drawCursor
        virtual bool drawCursor(gfx::Canvas& can, const Renderer& ren) = 0;

        // EventConsumer:
        virtual bool handleKey(util::Key_t key, int prefix, const Renderer& ren) = 0;
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons, const Renderer& ren) = 0;

        void setCallback(Callback* p);

        Callback* getCallback();

     private:
        Callback* m_pCallback;
    };

} }

#endif
