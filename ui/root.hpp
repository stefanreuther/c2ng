/**
  *  \file ui/root.hpp
  *  \brief Class ui::Root
  */
#ifndef C2NG_UI_ROOT_HPP
#define C2NG_UI_ROOT_HPP

#include <memory>
#include "ui/widget.hpp"
#include "gfx/engine.hpp"
#include "gfx/multiclipfilter.hpp"
#include "ui/colorscheme.hpp"
#include "gfx/resourceprovider.hpp"

namespace ui {

    /** UI root.
        This is the root container for widgets.
        It provides drawing for child widgets.
        It does not have a parent.

        <b>Contained elements:</b>
        - Root provides a ui::ColorScheme to descendants
        - Root keeps a reference to a gfx::Engine to obtain events and manage a graphics window
        - Root keeps a reference to a gfx::ResourceProvider.
          It does not need this itself, but it comes in very handy at times.

        <b>Invocation sequences:</b>
        - Constructing a Root will create a gfx::Engine window.
        - Call handleEvent() in a loop.
          This will schedule redraw and dispatch events.
          The thread calling handleEvent() should not block. */
    class Root : public Widget {
     public:
        /** Constructor.
            \param engine Graphics engine
            \param provider Resource provider
            \param wi Width (X) of graphics window
            \param he Height (Y) of graphics window
            \param bpp Bits per pixel of graphics window
            \param flags Graphics window flags. \see gfx::Engine::createWindow. */
        Root(gfx::Engine& engine, gfx::ResourceProvider& provider, int wi, int he, int bpp, gfx::Engine::WindowFlags_t flags);

        /** Destructor. */
        ~Root();

        // Widget methods:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void requestChildRedraw(Widget& child, const gfx::Rectangle& area);
        virtual void handleChildAdded(Widget& child);
        virtual void handleChildRemove(Widget& child);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual void handleChildPositionChange(Widget& child, gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        /** Handle an event.
            This waits for and dispatches a single event.
            It returns after the event has been dispatched.

            Note that event processing may have accumulated redraw requests.
            Those will be performed on the next handleEvent() call. */
        void handleEvent();

        /** Post a mouse event. */
        void postMouseEvent();

        /** Get color scheme.
            The same color scheme can also be obtained as getColorScheme(), but this method has a more specific prototype.
            \return color scheme */
        ColorScheme& colorScheme();

        /** Get resource provider.
            \return resource provider */
        gfx::ResourceProvider& provider();

        /** Get engine.
            \todo do we want to export the engine or just the methods we need? */
        gfx::Engine& engine();

        void add(Widget& child);

        void remove(Widget& child);

        /** Center widget on screen.
            \param widget Widget to move */
        void centerWidget(Widget& widget);

        /** Move widget to screen edge.
            \param widget Widget to move
            \param xPos   Relative X position (0: left, 1: center, 2: right)
            \param yPos   Relative Y position (0: top, 1: center, 2: bottom)
            \param offset Distance to edge. When anchored at an edge (0 or 2),
                          leave that many pixels from that edge. */
        void moveWidgetToEdge(Widget& widget, int xPos, int yPos, int offset);

     private:
        gfx::Engine& m_engine;
        gfx::Engine::WindowFlags_t m_engineWindowFlags;
        gfx::Point m_engineWindowSize;
        int m_engineWindowBPP;

        afl::base::Ptr<gfx::Canvas> m_window;
        std::auto_ptr<gfx::MultiClipFilter> m_filter;

        ColorScheme m_colorScheme;
        gfx::ResourceProvider& m_provider;

        // Mouse state
        bool m_mouseEventKnown;
        bool m_mouseEventRequested;
        gfx::Point m_mousePosition;
        MouseButtons_t m_mouseButtons;

        void initWindow();
        void performDeferredRedraws();
        void drawFrames(gfx::Canvas& can, Widget& widget);
    };

}

#endif
