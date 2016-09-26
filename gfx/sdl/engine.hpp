/**
  *  \file gfx/sdl/engine.hpp
  */
#ifndef C2NG_GFX_SDL_ENGINE_HPP
#define C2NG_GFX_SDL_ENGINE_HPP

#include <stdexcept>
#include <SDL_events.h>
#include "gfx/engine.hpp"
#include "gfx/sdl/surface.hpp"
#include "afl/base/types.hpp"
#include "afl/sys/loglistener.hpp"
#include "afl/sys/mutex.hpp"
#include "afl/container/ptrvector.hpp"
#include "gfx/timerqueue.hpp"

namespace gfx { namespace sdl {

    class Engine : public gfx::Engine,
                   private util::RequestDispatcher
    {
     public:
        explicit Engine(afl::sys::LogListener& log);
        ~Engine();

        virtual afl::base::Ptr<Canvas> createWindow(int width, int height, int bpp, WindowFlags_t flags);
        virtual afl::base::Ptr<Canvas> loadImage(afl::io::Stream& file);
        virtual void handleEvent(EventConsumer& consumer, bool relativeMouseMovement);
        virtual util::RequestDispatcher& dispatcher();
        virtual afl::base::Ptr<Timer> createTimer();

     private:
        // Integration
        afl::sys::LogListener& m_log;

        // Current window
        afl::base::Ptr<Surface> m_window;

        // Mouse grab
        bool m_disableGrab;                ///< If set, mouse grab is disabled.
        bool m_grabEnabled;                ///< Current state of mouse grab. true: enabled, reporting relative movement. false: disabled.
        uint32_t m_grabDelay;              ///< Mouse grab delay, ms. Suppress mouse movement after enabling grab.
        uint32_t m_grabEnableTime;         ///< Time of last grab enable.

        // Double click
        uint32_t m_lastClickTime;          ///< Time of last click.
        Point m_lastClickPosition;         ///< Position of last click.
        bool m_buttonPressed;              ///< true if a button was pressed last time.
        uint32_t m_doubleClickDelay;       ///< Double click delay, ms.

        // Timer
        TimerQueue m_timerQueue;

        // External dispatch
        virtual void postNewRunnable(afl::base::Runnable* p);
        void processTaskQueue();
        afl::sys::Mutex m_taskMutex;
        afl::container::PtrVector<afl::base::Runnable> m_taskQueue;

        // Event utilities
        void setMouseGrab(bool enable);
        bool convertEvent(const SDL_Event& se, EventConsumer& consumer, bool infinite);
        bool handleMouse(EventConsumer& consumer, const SDL_MouseButtonEvent& be, bool infinite, uint32_t addButton);

        // Logging
        void logException(std::exception* e, const char* where);
    };

} }

#endif
