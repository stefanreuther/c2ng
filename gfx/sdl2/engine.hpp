/**
  *  \file gfx/sdl2/engine.hpp
  */
#ifndef C2NG_GFX_SDL2_ENGINE_HPP
#define C2NG_GFX_SDL2_ENGINE_HPP

#include <SDL.h>
#include "afl/container/ptrqueue.hpp"
#include "afl/sys/loglistener.hpp"
#include "afl/sys/mutex.hpp"
#include "afl/sys/semaphore.hpp"
#include "gfx/engine.hpp"
#include "gfx/sdl2/presenter.hpp"
#include "gfx/sdl2/surface.hpp"
#include "gfx/timerqueue.hpp"
#include "util/key.hpp"
#include "util/requestdispatcher.hpp"

namespace gfx { namespace sdl2 {

    /** gfx::Engine implementation for SDL 2. */
    class Engine : public gfx::Engine,
                   private util::RequestDispatcher
    {
     public:
        /** Constructor.
            This will initialize SDL.
            \param log Logger */
        explicit Engine(afl::sys::LogListener& log);

        /** Destructor.
            This will shut down SDL. */
        ~Engine();

        // Engine methods:
        virtual afl::base::Ref<Canvas> createWindow(const WindowParameters& param);
        virtual afl::base::Ref<Canvas> loadImage(afl::io::Stream& file);
        virtual void handleEvent(EventConsumer& consumer, bool relativeMouseMovement);
        virtual util::RequestDispatcher& dispatcher();
        virtual afl::base::Ref<Timer> createTimer();

     private:
        // Integration
        afl::sys::LogListener& m_log;

        // Current window
        std::auto_ptr<Presenter> m_presenter;
        afl::base::Ptr<Surface> m_window;

        // Mouse grab
        bool m_disableGrab;                ///< If set, mouse grab is disabled. FIXME: make this configurable
        bool m_grabEnabled;                ///< Current state of mouse grab. true: enabled, reporting relative movement. false: disabled.
        uint32_t m_grabDelay;              ///< Mouse grab delay, ms. Suppress mouse movement after enabling grab.
        uint32_t m_grabEnableTime;         ///< Time of last grab enable.

        // Double click
        uint32_t m_lastClickTime;          ///< Time of last click.
        Point m_lastClickPosition;         ///< Position of last click.
        bool m_buttonPressed;              ///< true if a button was pressed last time.
        uint32_t m_doubleClickDelay;       ///< Double click delay, ms. FIXME: make this configurable

        // Command performance hack, see handleEvent()
        afl::sys::Semaphore m_runnableSemaphore;
        bool m_lastWasRunnable;

        // Timer
        TimerQueue m_timerQueue;

        // External dispatch
        virtual void postNewRunnable(afl::base::Runnable* p);
        void processTaskQueue();
        afl::sys::Mutex m_taskMutex;
        afl::container::PtrQueue<afl::base::Runnable> m_taskQueue;

        // // Event utilities
        // void setMouseGrab(bool enable);
        bool convertEvent(const SDL_Event& se, EventConsumer& consumer, bool infinite);
        bool handleMouse(EventConsumer& consumer, const SDL_MouseButtonEvent& be, bool infinite, uint32_t addButton);
        void handleTextInput(EventConsumer& consumer, const char* text, util::Key_t mod);
    };

} }

#endif
