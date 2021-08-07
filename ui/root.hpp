/**
  *  \file ui/root.hpp
  *  \brief Class ui::Root
  */
#ifndef C2NG_UI_ROOT_HPP
#define C2NG_UI_ROOT_HPP

#include <memory>
#include "afl/base/closure.hpp"
#include "afl/base/signal.hpp"
#include "afl/container/ptrqueue.hpp"
#include "gfx/engine.hpp"
#include "gfx/multiclipfilter.hpp"
#include "gfx/resourceprovider.hpp"
#include "gfx/windowparameters.hpp"
#include "ui/colorscheme.hpp"
#include "ui/widget.hpp"

namespace ui {

    /** UI root.
        This is the root container for widgets.
        It provides drawing for child widgets.
        It does not have a parent.

        Root also provides basic engine-independant event handling:
        - mouse state tracking (postMouseEvent(), setMousePrefixArgument())
        - synthetic keyboard events (postKeyEvent(), ungetKeyEvent())

        <b>Contained elements:</b>
        - Root provides a ui::ColorScheme to descendants
        - Root keeps a reference to a gfx::Engine to obtain events and manage a graphics window
        - Root keeps a reference to a gfx::ResourceProvider.
          It does not need this itself, but it comes in very handy at times.

        <b>Invocation sequences:</b>
        - Constructing a Root will create a gfx::Engine window.
        - Call handleEvent() in a loop.
          This will schedule redraw and dispatch events.
          The thread calling handleEvent() should not block.

        <b>Multithreading:</b> All Root methods can only be called from the UI thread.
        There is no internal interlocking. */
    class Root : public Widget {
     public:
        typedef afl::base::Closure<void(gfx::EventConsumer&)> EventTask_t;

        /** Constructor.
            \param engine Graphics engine
            \param provider Resource provider
            \param param Window parameters
            \see gfx::Engine::createWindow. */
        Root(gfx::Engine& engine, gfx::ResourceProvider& provider, const gfx::WindowParameters& param);

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

        /** Handle an event, relative-mouse version.
            This waits for and dispatches a single event.
            It returns after the event has been dispatched.

            Note that event processing may have accumulated redraw requests.
            Those will be performed on the next handleEvent() call.

            This function produces relative mouse events and dispatches them to the EventConsumer.
            It does NOT call the regular widget hierarchy's event handling functions.

            Use this function instead of calling Engine::handleEvent() directly to keep
            the local event queue (postKeyEvent() etc.) and deferred redraws working.

            \param consumer EventConsumer target */
        void handleEventRelative(EventConsumer& consumer);

        /** Post a mouse event.
            Makes sure that a future handleEvent() call will eventually process a mouse event even if the mouse state didn't change. */
        void postMouseEvent();

        /** Post a key event.
            Makes sure that a future handleEvent() call will eventually process the given keyboard event.
            All synthetic key events will be processed before new user input is received.
            This is a queue, not a stack, thus key events will be processed in the same order as posted.

            FIXME: maybe we can do without this method

            \param key Key
            \param prefix Prefix argument */
        void postKeyEvent(util::Key_t key, int prefix);

        /** Unget a key event.
            Makes sure that a future handleEvent() call will eventually process the given keyboard event.
            This event will be processed before all other keyboard events (postKeyEvent, ungetKeyEvent and real ones).
            This is a stack, the latest ungetKeyEvent() event will be processed first.

            \param key Key
            \param prefix Prefix argument */
        void ungetKeyEvent(util::Key_t key, int prefix);

        /** Set prefix argument for next mouse command.
            Because it's up to a widget to determine when a mouse-click is treated as such, we cannot associate a prefix argument with mouse events.
            Instead, ui::PrefixArgument will post a prefix argument using this method,
            and the consuming widget will consume is using consumeMousePrefixArgument().
            Root contains logic to clear an unused prefix argument when a mouse click is definitely over.

            For keyboard events, the prefix argument is provided directly in the event callback.

            \param prefix New prefix argument
            \see ui::PrefixArgument */
        void setMousePrefixArgument(int prefix);

        /** Consume prefix argument.
            A consuming widget should call this method when it determined a successful mouse click.
            For keyboard events, the prefix argument is provided directly in the event callback and this callback is pointless.

            \return prefix argument, 0 if none
            \see ui::PrefixArgument  */
        int consumeMousePrefixArgument();

        /** Get color scheme.
            The same color scheme can also be obtained as getColorScheme(), but this method has a more specific prototype.
            \return color scheme */
        ColorScheme& colorScheme();

        /** Get resource provider.
            \return resource provider */
        gfx::ResourceProvider& provider();

        /** Get engine.
            \todo do we want to export the engine or just the methods we need?
            \return engine */
        gfx::Engine& engine();

        /** Add widget.
            The widget (a window, usually) is placed on top of the widget stack (i.e. frontmost).
            \param child Widget */
        void add(Widget& child);

        /** Remove widget.
            \param child Widget */
        void remove(Widget& child);

        /** Center widget on screen.
            \param widget Widget to move */
        void centerWidget(Widget& widget);

        /** Move widget to screen edge.
            \param widget Widget to move
            \param xPos   Relative X position
            \param yPos   Relative Y position
            \param offset Distance to edge. When anchored at an edge, leave that many pixels from that edge. */
        void moveWidgetToEdge(Widget& widget, gfx::HorizontalAlignment xPos, gfx::VerticalAlignment yPos, int offset);

        /** Save a screenshot.
            Saves the current canvas by invoking sig_screenshot with the right parameters. */
        void saveScreenshot();

        afl::base::Signal<void(gfx::Canvas&)> sig_screenshot;

     private:
        gfx::Engine& m_engine;                                             ///< Reference to underlying engine.
        gfx::WindowParameters m_engineWindowParameters;                    ///< Configured window parameters.

        afl::base::Ptr<gfx::Canvas> m_window;                              ///< Current engine window.
        std::auto_ptr<gfx::MultiClipFilter> m_filter;                      ///< List of dirty areas. Never null.

        afl::container::PtrQueue<EventTask_t> m_localTaskQueue;            ///< Local task queue. Contains local events.

        ColorScheme m_colorScheme;                                         ///< Color scheme (palette -> pixel mapping).
        gfx::ResourceProvider& m_provider;                                 ///< Resource provider.

        // Mouse state
        bool m_mouseEventKnown;                                            ///< Mouse state: true if m_mousePosition, m_mouseButtons valid.
        bool m_mouseEventRequested;                                        ///< Mouse state: true if postMouseEvent() was called but not yet confirmed.
        gfx::Point m_mousePosition;                                        ///< Mouse state: last position.
        MouseButtons_t m_mouseButtons;                                     ///< Mouse state: last button state.
        int m_mousePrefix;                                                 ///< Mouse state: prefix for next mouse click.
        bool m_mousePrefixPosted;                                          ///< Mouse state: recognition of unused prefix. See handleMouse()

        void initWindow();
        void performDeferredRedraws();
        void drawFrames(gfx::Canvas& can, Widget& widget);
    };

}

#endif
