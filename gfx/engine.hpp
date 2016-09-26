/**
  *  \file gfx/engine.hpp
  *  \brief Base class gfx::Engine
  */
#ifndef C2NG_GFX_ENGINE_HPP
#define C2NG_GFX_ENGINE_HPP

#include "afl/base/refcounted.hpp"
#include "afl/base/ptr.hpp"
#include "afl/bits/smallset.hpp"
#include "afl/io/stream.hpp"
#include "util/requestdispatcher.hpp"
#include "gfx/timer.hpp"

namespace gfx {

    class Canvas;
    class EventConsumer;

    /** Base class for a graphics engine.
        This encapsulates all logic for talking to a GUI library.
        A GUI library consists of three parts:
        - a graphics output window; a single, user-visible canvas
        - a graphics file loader; produces invisible canvases, probably optimized for blitting onto the window
        - user input (event) acquisition

        In order to meaningfully work with the event loop, we also implement
        - a util::RequestDispatcher to execute tasks
        - timers */
    class Engine : public afl::base::RefCounted {
     public:
        enum WindowFlag {
            ResizableWindow,    ///< Make the window resizable. FIXME: interface to report these events is still missing
            FullscreenWindow    ///< Make the window full-screen.
        };

        /** Options for the window. */
        typedef afl::bits::SmallSet<WindowFlag> WindowFlags_t;

        virtual ~Engine()
            { }

        /** Create a window.

            Note that only one window can be active.
            Calling createWindow a second time will invalidate previously-created windows.

            <b>Multithreading:</b> Call this method from the GUI thread only.

            \param width Width of window, pixels
            \param height Height of window, pixels
            \param bpp Bits per pixel (8/16/24/32 to pick a specific width; 0 to pick the default)
            \param flags Flags
            \return Canvas for that window; never null
            \throw GraphicsException on error */
        virtual afl::base::Ptr<Canvas> createWindow(int width, int height, int bpp, WindowFlags_t flags) = 0;

        /** Load an image file.

            Uses the GUI library's file loading mechanisms to load an image file.
            The returned canvas is in a format optimized for blitting.

            <b>Multithreading:</b> Call this method from any thread (preferrably not the GUI thread to not block it).

            \param file Opened file
            \return Canvas containing the loaded image; never null
            \throw GraphicsException on error */
        virtual afl::base::Ptr<Canvas> loadImage(afl::io::Stream& file) = 0;

        /** Wait for and handle an event.
            Returns when a user event, dispatcher request, or timer has been processed.
            (Note that this may process multiple events/requests in one go.)

            <b>Multithreading:</b> Call this method from the GUI thread only.

            \param consumer Event consumer
            \param relativeMouseMovement Receive relative mouse movement if enabled (default: absolute) */
        virtual void handleEvent(EventConsumer& consumer, bool relativeMouseMovement) = 0;

        /** Get request dispatcher.
            Actions posted into the request dispatcher will be executed in the GUI thread,
            and cause handleEvent() to return.

            <b>Multithreading:</b> Call this method from any thread.

            \return Event dispatcher */
        virtual util::RequestDispatcher& dispatcher() = 0;

        /** Create a user-interface timer.
            The timer callback will execute from within handleEvent() and cause it to return.

            <b>Multithreading:</b> Call this method from the GUI thread only.

            \return newly-allocated timer; never null */
        virtual afl::base::Ptr<Timer> createTimer() = 0;
    };

}

#endif
