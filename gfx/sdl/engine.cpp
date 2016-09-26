/**
  *  \file gfx/sdl/engine.cpp
  */

#include "config.h"
#ifdef HAVE_SDL
# include <cstdlib>
# include <SDL.h>
# include "afl/string/format.hpp"
# include "afl/sys/time.hpp"
# include "gfx/eventconsumer.hpp"
# include "gfx/graphicsexception.hpp"
# include "gfx/sdl/engine.hpp"
# include "gfx/sdl/surface.hpp"
# include "util/translation.hpp"
# include "gfx/sdl/streaminterface.hpp"
# include "afl/except/fileformatexception.hpp"
# include "util/translation.hpp"
# include "afl/sys/mutexguard.hpp"
# ifdef HAVE_SDL_IMAGE
#  include <SDL_image.h>
# endif

namespace {
    const char LOG_NAME[] = "gfx.sdl";

    const uint8_t SDL_WAKE_EVENT = SDL_USEREVENT;

    /** Convert SDL key modifier to internal key modifier.
        \param m SDL key modifier set
        \return util::Key key modifier set */
    util::Key_t convertModifier(uint32_t m)
    {
        // ex UIEvent::simplMod, sort-of
        uint32_t result = 0;
        if ((m & (KMOD_LSHIFT | KMOD_RSHIFT)) != 0) {
            result |= util::KeyMod_Shift;
        }
        if ((m & (KMOD_LCTRL | KMOD_RCTRL)) != 0) {
            result |= util::KeyMod_Ctrl;
        }
        if ((m & (KMOD_LALT | KMOD_RALT)) != 0) {
            result |= util::KeyMod_Alt;
        }
        if ((m & (KMOD_LMETA | KMOD_RMETA)) != 0) {
            result |= util::KeyMod_Meta;
        }
        return result;
    }


    /** Convert mouse buttons.
        Merges mouse and key modifiers.
        \param mouse SDL mouse button mask
        \param key modifier set
        \return mouse button set */
    gfx::EventConsumer::MouseButtons_t convertMouseButtons(uint32_t mouse, util::Key_t key)
    {
        // ex UIEvent::mergeMod

        // Convert mouse button presses
        gfx::EventConsumer::MouseButtons_t result;
        if ((mouse & SDL_BUTTON_LMASK) != 0) {
            result += gfx::EventConsumer::LeftButton;
        }
        if ((mouse & SDL_BUTTON_RMASK) != 0) {
            result += gfx::EventConsumer::RightButton;
        }
        if ((mouse & SDL_BUTTON_MMASK) != 0) {
            result += gfx::EventConsumer::MiddleButton;
        }

        if (!result.empty()) {
            // Convert key presses.
            // Do not add modifiers if this is a release, so an empty MouseButtons_t() still means mouse release even if a modifier is held.
            if ((key & util::KeyMod_Shift) != 0) {
                result += gfx::EventConsumer::ShiftKey;
            }
            if ((key & util::KeyMod_Ctrl) != 0) {
                result += gfx::EventConsumer::CtrlKey;
            }
            if ((key & util::KeyMod_Alt) != 0) {
                result += gfx::EventConsumer::AltKey;
            }
            if ((key & util::KeyMod_Meta) != 0) {
                result += gfx::EventConsumer::MetaKey;
            }
        }

        return result;
    }

    /** Convert key symbol.
        We translate the keypad keys into their "gray" equivalent. We need not care for the status of NumLock,
        when that is on, SDL will translate the keys into their ASCII / Unicode value for us which is handled elsewhere.
        \param sym SDL keysym
        \param mod SDL key modifiers
        \return key */
    util::Key_t convertKey(uint32_t sym, uint32_t mod)
    {
        // ex ui/event.cc:simplSym

        /* SDL-1.2.2 on Windows does not assign Unicode numbers to numpad keys, so we'll
           do it here instead. We do not want Shift to be a temporary Numlock replacement,
           though, to accept Shift-Arrows as Shift-Arrows, not digits. */
        bool shifted = (mod & KMOD_NUM) != 0;

        switch (sym) {
         case SDLK_KP_ENTER:  return util::Key_Return;
         case SDLK_KP0:       return shifted ? '0' : util::Key_Insert;
         case SDLK_KP1:       return shifted ? '1' : util::Key_End;
         case SDLK_KP2:       return shifted ? '2' : util::Key_Down;
         case SDLK_KP3:       return shifted ? '3' : util::Key_PgDn;
         case SDLK_KP4:       return shifted ? '4' : util::Key_Left;
         case SDLK_KP5:       return shifted ? '5' : util::Key_Num5;
         case SDLK_KP6:       return shifted ? '6' : util::Key_Right;
         case SDLK_KP7:       return shifted ? '7' : util::Key_Home;
         case SDLK_KP8:       return shifted ? '8' : util::Key_Up;
         case SDLK_KP9:       return shifted ? '9' : util::Key_PgUp;
         case SDLK_KP_PERIOD: return shifted ? '.' : util::Key_Delete;
         case SDLK_F1:        return util::Key_F1;
         case SDLK_F2:        return util::Key_F2;
         case SDLK_F3:        return util::Key_F3;
         case SDLK_F4:        return util::Key_F4;
         case SDLK_F5:        return util::Key_F5;
         case SDLK_F6:        return util::Key_F6;
         case SDLK_F7:        return util::Key_F7;
         case SDLK_F8:        return util::Key_F8;
         case SDLK_F9:        return util::Key_F9;
         case SDLK_F10:       return util::Key_F10;
         case SDLK_F11:       return util::Key_F11;
         case SDLK_F12:       return util::Key_F12;
         case SDLK_F13:       return util::Key_F13;
         case SDLK_F14:       return util::Key_F14;
         case SDLK_F15:       return util::Key_F15;
         case SDLK_UP:        return util::Key_Up;
         case SDLK_DOWN:      return util::Key_Down;
         case SDLK_LEFT:      return util::Key_Left;
         case SDLK_RIGHT:     return util::Key_Right;
         case SDLK_HOME:      return util::Key_Home;
         case SDLK_END:       return util::Key_End;
         case SDLK_PAGEUP:    return util::Key_PgUp;
         case SDLK_PAGEDOWN:  return util::Key_PgDn;
         case SDLK_TAB:       return util::Key_Tab;
         case SDLK_BACKSPACE: return util::Key_Backspace;
         case SDLK_DELETE:    return util::Key_Delete;
         case SDLK_INSERT:    return util::Key_Insert;
         case SDLK_RETURN:    return util::Key_Return;
         case SDLK_ESCAPE:    return util::Key_Escape;
         case SDLK_PRINT:     return util::Key_Print;
         case SDLK_PAUSE:     return util::Key_Pause;
         case SDLK_MENU:      return util::Key_Menu;
         default:             return sym >= ' ' && sym < 127 ? sym : 0;
        }
    }


    bool isKnownIgnorableKey(uint32_t sym)
    {
        return sym == SDLK_NUMLOCK
            || sym == SDLK_CAPSLOCK
            || sym == SDLK_SCROLLOCK
            || sym == SDLK_RSHIFT
            || sym == SDLK_LSHIFT
            || sym == SDLK_RCTRL
            || sym == SDLK_LCTRL
            || sym == SDLK_RALT
            || sym == SDLK_LALT
            || sym == SDLK_RMETA
            || sym == SDLK_LMETA
            || sym == SDLK_LSUPER
            || sym == SDLK_RSUPER
            || sym == SDLK_MODE
            || sym == SDLK_COMPOSE
            || sym == 0 /* SDL does not know to map it either */;
    }

    /** QUIT handler.
        When SDL itself sees a QUIT event, it will think we handle it, and close our window.
        This does not fit our model.
        We want a QUIT event to generate an event but not disrupt the normal event stream.
        Hence, we post the event manually and lie to SDL. */
    extern "C" int quitHandler(const SDL_Event* event)
    {
        if (event->type == SDL_QUIT) {
            /* SDL_PushEvent is SDL_PeepEvents in disguise, but doesn't modify *event */
            SDL_PushEvent(const_cast<SDL_Event*>(event));
            return 0;
        }
        return 1;
    }

}

gfx::sdl::Engine::Engine(afl::sys::LogListener& log)
    : m_log(log),
      m_window(),
      m_disableGrab(false),
      m_grabEnabled(false),
      m_grabDelay(1000 / 10),
      m_grabEnableTime(0),
      m_lastClickTime(0),
      m_lastClickPosition(),
      m_buttonPressed(false),
      m_doubleClickDelay(1000 / 3),
      m_timerQueue(),
      m_taskMutex(),
      m_taskQueue()
{
    // ex gfx/init.cc:initGraphics, ui/event.cc:initEvents
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        throw GraphicsException(afl::string::Format(_("Error initializing SDL: %s").c_str(), SDL_GetError()));
    }

    // FIXME: needed?
    // global_timer_id = SDL_AddTimer(TIMER_MS, timerFunction, 0);
    // if (!global_timer_id)
    //     throw InitException(string_t(_("Unable to start timer: ")) + SDL_GetError());

    SDL_SetEventFilter(quitHandler);
    SDL_EnableUNICODE(1);
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    // FIXME: needed?
    // SDL_EventState(SDL_ENABLE, ev_Tick);
    SDL_EventState(SDL_ENABLE, SDL_KEYDOWN);
    SDL_EventState(SDL_ENABLE, SDL_MOUSEBUTTONUP);
    SDL_EventState(SDL_ENABLE, SDL_MOUSEBUTTONDOWN);
    SDL_EventState(SDL_ENABLE, SDL_MOUSEMOTION);

    m_grabEnabled = false;

    std::atexit(SDL_Quit);
}

gfx::sdl::Engine::~Engine()
{
    // ex ui/event.cc:doneEvents
    // FIXME: needed?
    // if (global_timer_id) {
    //     SDL_RemoveTimer(global_timer_id);
    //     global_timer_id = 0;
    // }
    SDL_SetEventFilter(0);
    SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_TIMER);
}

afl::base::Ptr<gfx::Canvas>
gfx::sdl::Engine::createWindow(int width, int height, int bpp, WindowFlags_t flags)
{
    int sdlFlags = 0;
    if (flags.contains(FullscreenWindow)) {
        sdlFlags |= SDL_FULLSCREEN;
    }
    if (flags.contains(ResizableWindow)) {
        sdlFlags |= SDL_RESIZABLE;
    }

    // FIXME: icon, title

    SDL_Surface* sfc = SDL_SetVideoMode(width, height, bpp, sdlFlags);
    if (!sfc) {
        throw GraphicsException(afl::string::Format(_("Error setting video mode: %s").c_str(), SDL_GetError()));
    }
    m_window = new Surface(sfc, false);

    // Log it
    char driverName[100];
    if (SDL_VideoDriverName(driverName, sizeof driverName)) {
        if (const SDL_VideoInfo* vi = SDL_GetVideoInfo()) {
            String_t flags;
            if (vi->wm_available) {  // do not translate the flag strings
                flags += ", wm";
            }
            if (vi->hw_available) {
                flags += ", hw";
            }
            if (vi->blit_hw) {
                flags += ", hw->hw";
            }
            if (vi->blit_sw) {
                flags += ", sw->hw";
            }
            if (vi->blit_fill) {
                flags += ", fill";
            }
            m_log.write(m_log.Info, LOG_NAME, afl::string::Format(_("Video driver: %s (%dk%s)").c_str(), driverName, vi->video_mem, flags));
        }
    }

    return m_window;
}

afl::base::Ptr<gfx::Canvas>
gfx::sdl::Engine::loadImage(afl::io::Stream& file)
{
    StreamInterface iface(file);

    SDL_Surface* sfc;
#if HAVE_SDL_IMAGE
    sfc = IMG_Load_RW(&iface, 0);
#else
    sfc = SDL_LoadBMP_RW(&iface, 0);
#endif
    if (!sfc) {
        throw afl::except::FileFormatException(file, SDL_GetError());
    }
    return new Surface(sfc, true);
}

// /** Get Event from Queue. Fetches one event and returns it in
//     \c event. If \c idle is false (default), waits for an event.
//     If \c idle is true, returns an event with type ev_Idle
//     if there is no event in the queue. If \c infinite is false,
//     returns normal mouse events; otherwise returns relative
//     mouse movement. */
void
gfx::sdl::Engine::handleEvent(EventConsumer& consumer, bool relativeMouseMovement)
{
    // ex ui/event.cc:getEvent, sort-of (no queue, no idle)

    // Flush output
    if (Surface* sfc = m_window.get()) {
        sfc->ensureUnlocked();
    }

    // Update mouse grab
    setMouseGrab(relativeMouseMovement);

    // Wait for event to arrive
    SDL_Event ev;
    while (1) {
        afl::sys::Timeout_t t = m_timerQueue.getNextTimeout();
        if (t == afl::sys::INFINITE_TIMEOUT) {
            // Easy case: wait for event
            SDL_WaitEvent(&ev);
            if (convertEvent(ev, consumer, relativeMouseMovement)) {
                break;
            }
        } else {
            // Not-so-easy case: wait until we have an event.
            // SDL-1.2 has no SDL_WaitEventTimeout, so we do a delay loop.
            // This is less crappy than it sounds because SDL ultimately does the same in its SDL_WaitEvent.
            uint32_t start = afl::sys::Time::getTickCounter();
            uint32_t elapsed = 0;
            bool eventStatus = false;
            while (!eventStatus && elapsed < t) {
                eventStatus = (SDL_PollEvent(&ev) == 1);
                if (!eventStatus) {
                    SDL_Delay(10);
                }
                elapsed = afl::sys::Time::getTickCounter() - start;
            }

            // Evaluate result
            bool timerResult = m_timerQueue.handleElapsedTime(elapsed);
            bool eventResult = eventStatus && convertEvent(ev, consumer, relativeMouseMovement);
            if (timerResult || eventResult) {
                break;
            }
        }
    }
}

util::RequestDispatcher&
gfx::sdl::Engine::dispatcher()
{
    return *this;
}

afl::base::Ptr<gfx::Timer>
gfx::sdl::Engine::createTimer()
{
    return m_timerQueue.createTimer();
}


// /** Set mouse mode.
//     \param infinite \c false = normal mode. Mouse events report normal
//     x/y coordinates. Pointer is visible. \c true = infinite movement
//     mode. Mouse events report relative x/y coordinates. Pointer is
//     invisible. */
void
gfx::sdl::Engine::setMouseGrab(bool enable)
{
    // ex ui/event.cc:setMouseMode
    if (!m_disableGrab && (enable != m_grabEnabled)) {
        m_grabEnabled = enable;
        if (enable) {
            SDL_ShowCursor(0);
            SDL_WM_GrabInput(SDL_GRAB_ON);
            m_grabEnableTime = afl::sys::Time::getTickCounter();
        } else {
            SDL_ShowCursor(1);
            SDL_WM_GrabInput(SDL_GRAB_OFF);
        }
    }
}

/** Convert SDL event into our format.
    \param se [input] the SDL event to convert
    \param ue [output] the event we generate
    \param infinite true iff we are in "infinite movement" mode and want
    relative mouse movement to be reported.
    \returns true iff successful, false if event doesn't map to our scheme. */
bool
gfx::sdl::Engine::convertEvent(const SDL_Event& se, gfx::EventConsumer& consumer, bool infinite)
{
    // ex ui/event.cc:convertEvent
    switch(se.type) {
     case SDL_KEYDOWN: {
        uint32_t u = se.key.keysym.unicode;
        uint32_t sdlMod = se.key.keysym.mod;

        // Windows reports AltGr keys with RALT and LCTRL.
        // Thus, if it's printable, remove the Alt/Ctrl.
        if (u != 0 && (sdlMod & KMOD_RALT) != 0) {
            sdlMod &= ~KMOD_RALT;
            sdlMod &= ~KMOD_LCTRL;
        }
        util::Key_t key = convertModifier(sdlMod);
        if (u >= 32 && u != 127 && u < util::Key_FirstSpecial) {
            // It is a printable character. Use as-is.
            key |= u;

            // Discount shift on these keys unless they're from the number pad.
            // These are the only ones that can generate shifted printable.
            if (se.key.keysym.sym < SDLK_KP0 || se.key.keysym.sym > SDLK_KP_EQUALS) {
                key &= ~util::KeyMod_Shift;
            }
        } else {
            // It is a special key.
            key |= convertKey(se.key.keysym.sym, se.key.keysym.mod);
        }

        if ((key & util::Key_Mask) != 0) {
            consumer.handleKey(key, 0);
            return true;
        } else {
            if (!isKnownIgnorableKey(se.key.keysym.sym)) {
                m_log.write(m_log.Trace, LOG_NAME, afl::string::Format(_("Key not mapped: 0x%x").c_str(), int(se.key.keysym.sym)));
            }
            return false;
        }
     }

     case SDL_MOUSEMOTION:
        /* In infinite mode, refuse movement events
           - when mouse grab is disabled (reported coordinates are wrong)
           - mouse grab has been active for too little time, i.e. the reported
           coordinates may be an unexpected jump */
        if (infinite) {
            if (m_disableGrab || (afl::sys::Time::getTickCounter() - m_grabEnableTime) < m_grabDelay) {
                return false;
            } else {
                Point pt(se.motion.xrel, se.motion.yrel);
                /* If there are more motion events, consume those as well, and merge
                   them with the current event. This avoids that events pile up and motion
                   "lags" when we cannot process events fast enough. This happens with the
                   starcharts at high resolution. */
                SDL_Event ae;
                while (SDL_PeepEvents(&ae, 1, SDL_PEEKEVENT, ~0U) > 0 && ae.type == SDL_MOUSEMOTION
                       && SDL_PeepEvents(&ae, 1, SDL_GETEVENT, ~0U) > 0 && ae.type == SDL_MOUSEMOTION)
                {
                    pt += Point(ae.motion.xrel, ae.motion.yrel);
                }
                consumer.handleMouse(pt, convertMouseButtons(se.motion.state, convertModifier(SDL_GetModState())));
                return true;
            }
        } else {
            Point pt(se.motion.x, se.motion.y);

            // If there are more motion events, consume those as well.
            SDL_Event ae;
            while (SDL_PeepEvents(&ae, 1, SDL_PEEKEVENT, ~0U) > 0 && ae.type == SDL_MOUSEMOTION
                   && SDL_PeepEvents(&ae, 1, SDL_GETEVENT, ~0U) > 0 && ae.type == SDL_MOUSEMOTION)
            {
                pt = Point(ae.motion.x, ae.motion.y);
            }
            consumer.handleMouse(pt, convertMouseButtons(se.motion.state, convertModifier(SDL_GetModState())));
            return true;
        }

     case SDL_MOUSEBUTTONDOWN:
            /* SDL 1.2.1 has no mouse wheel API, but the Windows backend
               reports a mouse wheel as buttons 4 and 5 (the fbcon backend
               detects an imaginary Z axis, but that doesn't yet fit into the
               SDL_MouseMotionEvent. */
        if (se.button.button == 4) {
            consumer.handleKey(util::Key_WheelUp | convertModifier(SDL_GetModState()), 0);
            return true;
        } else if (se.button.button == 5) {
            consumer.handleKey(util::Key_WheelDown | convertModifier(SDL_GetModState()), 0);
            return true;
        } else {
            /* Reportedly, touchpads generate very quick sequences of
               button-down and button-up, so we never see a button pressed
               in SDL_GetMouseState(). However, since this is a press, we
               know that the respective button must be down, so force it
               to be included. */
            return handleMouse(consumer, se.button, infinite, 1 << (se.button.button-1));
        }

     case SDL_MOUSEBUTTONUP:
        return handleMouse(consumer, se.button, infinite, 0);

        // FIXME: needed?
        //  case ev_Tick:
        //  {
        //      long now = global_timer;
        //      ue.type = ev_Tick;
        //      if (last_tick_event >= 0)
        //          ue.tick.elapsed = (now - last_tick_event);
        //      else
        //          ue.tick.elapsed = 1;
        //      last_tick_event = now;
        //  }
        //  return true;
        //  case SDL_NOEVENT:
        //     ue.type = ev_Idle;
        //     return true;

     case SDL_WAKE_EVENT:
        processTaskQueue();
        return true;

     case SDL_QUIT:
        consumer.handleKey(util::Key_Quit, 0);
        return true;

     default:
        return false;
    }
}

bool
gfx::sdl::Engine::handleMouse(EventConsumer& consumer, const SDL_MouseButtonEvent& be, bool infinite, uint32_t addButton)
{
    // No button events when we're in infinite mode but shouldn't be.
    if (infinite && m_disableGrab) {
        return false;
    } else {
        // Convert event
        Point pt = infinite ? Point() : Point(be.x, be.y);
        EventConsumer::MouseButtons_t btn = convertMouseButtons(SDL_GetMouseState(0, 0) | addButton, convertModifier(SDL_GetModState()));

        if (btn.empty() && m_buttonPressed) {
            // Mouse was released
            uint32_t time = afl::sys::Time::getTickCounter();
            if (m_lastClickTime > 0
                && time - m_lastClickTime < m_doubleClickDelay
                && std::abs(m_lastClickPosition.getX() - pt.getX()) < 5 && std::abs(m_lastClickPosition.getY() - pt.getY()) < 5)
            {
                btn += EventConsumer::DoubleClick;
                m_lastClickTime = 0;
            } else {
                m_lastClickTime = time;
            }
            m_lastClickPosition = pt;
        }
        m_buttonPressed = !(btn - EventConsumer::DoubleClick).empty();
        consumer.handleMouse(pt, btn);
        return true;
    }
}

void
gfx::sdl::Engine::postNewRunnable(afl::base::Runnable* p)
{
    if (p != 0) {
        afl::sys::MutexGuard g(m_taskMutex);
        m_taskQueue.pushBackNew(p);
        if (m_taskQueue.size() == 1) {
            SDL_Event event;
            event.type = SDL_WAKE_EVENT;
            event.user.code = 0;
            event.user.data1 = 0;
            event.user.data2 = 0;
            SDL_PushEvent(&event);
        }
    }
}

void
gfx::sdl::Engine::processTaskQueue()
{
    // FIXME: this probably needs to be changed because we must allow recursion!
    // Swap out old task queue
    afl::container::PtrVector<afl::base::Runnable> q;
    {
        afl::sys::MutexGuard g(m_taskMutex);
        m_taskQueue.swap(q);
    }

    // Process task queue
    for (size_t i = 0, n = q.size(); i < n; ++i) {
        q[i]->run();
    }
}

#else
int g_dummyToMakeGfxSdlEngineNotEmpty;
#endif
