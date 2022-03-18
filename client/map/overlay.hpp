/**
  *  \file client/map/overlay.hpp
  *  \brief Interface client::map::Overlay
  */
#ifndef C2NG_CLIENT_MAP_OVERLAY_HPP
#define C2NG_CLIENT_MAP_OVERLAY_HPP

#include "afl/base/deletable.hpp"
#include "gfx/canvas.hpp"
#include "gfx/eventconsumer.hpp"

namespace client { namespace map {

    class Renderer;
    class Callback;

    /** Starchart overlay.
        A starchart view can have multiple starchart overlays that can display additional information
        and provide user interactions. */
    class Overlay : public afl::base::Deletable {
     public:
        typedef gfx::EventConsumer::MouseButtons_t MouseButtons_t;

        /** Constructor. */
        Overlay();

        /** Destructor. */
        ~Overlay();

        /** Draw below chart.
            Called before the chart has been drawn, bottom-most overlay first (top-most draws last).
            Use for background stuff like predictions/trails.
            \param can Canvas to draw on.
            \param ren Renderer */
        // ex WChartMode::drawBefore, WChartWidget::drawPre, WChartMode::drawBelow
        virtual void drawBefore(gfx::Canvas& can, const Renderer& ren) = 0;

        /** Draw above chart.
            Called after the chart has been drawn, bottom-most overlay first (top-most draws last).
            \param can Canvas to draw on.
            \param ren Renderer */
        // ex WChartWidget::drawPost, WChartMode::drawOverlays
        virtual void drawAfter(gfx::Canvas& can, const Renderer& ren) = 0;

        /** Draw cursor.
            Called after the chart has been drawn, top-most overlay first.
            Use for cursors.
            \param can Canvas to draw on.
            \param ren Renderer
            \retval true This is the final cursor, do not draw next overlay's cursor
            \retval false This is not the final cursor, draw next cursor overlay's cursor, too */
        // ex WChartMode::drawCursor
        virtual bool drawCursor(gfx::Canvas& can, const Renderer& ren) = 0;

        /** Handle key event.
            Called starting from top-most overlay.
            \param key Key
            \param prefix Prefix
            \param ren Renderer
            \retval true Key was handled, do not call next overlay
            \retval false Key not handled, try next overlay */
        virtual bool handleKey(util::Key_t key, int prefix, const Renderer& ren) = 0;

        /** Handle mouse event.
            Called starting from top-most overlay.
            \param pt Mouse location or movement
            \param pressedButtons Button state
            \param ren Renderer
            \retval true Event was handled, do not call next overlay
            \retval false Event not handled, try next overlay */
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons, const Renderer& ren) = 0;

        /** Set callback.
            \param p Callback. If the callback is still set when the overlay dies, it calls removeOverlay(). */
        void setCallback(Callback* p);

        /** Get current callback. */
        Callback* getCallback() const;

        /** Request redraw.
            Shortcut for Callback::requestRedraw(). */
        void requestRedraw() const;

     private:
        Callback* m_pCallback;
    };

} }

#endif
