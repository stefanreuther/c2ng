/**
  *  \file gfx/sdl2/engine.cpp
  */

#include "config.h"
#ifdef HAVE_SDL2
# include <memory>
# include <SDL.h>
# include <SDL_render.h>
# ifdef HAVE_SDL2_IMAGE
#  include <SDL_image.h>
# endif
# include "afl/charset/utf8reader.hpp"
# include "afl/except/fileformatexception.hpp"
# include "afl/string/format.hpp"
# include "afl/sys/mutexguard.hpp"
# include "gfx/eventconsumer.hpp"
# include "gfx/graphicsexception.hpp"
# include "gfx/sdl2/engine.hpp"
# include "gfx/sdl2/streaminterface.hpp"
# include "gfx/sdl2/surface.hpp"
# include "gfx/windowparameters.hpp"
# include "util/key.hpp"
# include "util/translation.hpp"

namespace {
    const char LOG_NAME[] = "gfx.sdl2";

    const uint32_t SDL_WAKE_EVENT = SDL_USEREVENT;

    bool fetchNextEventIfType(SDL_Event& ae, uint32_t eventType)
    {
        return (SDL_PeepEvents(&ae, 1, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT) > 0 && ae.type == eventType
                && SDL_PeepEvents(&ae, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT) > 0 && ae.type == eventType);
    }

    /** Simplify modifier for text input.
        (a) Shift normally affects the case of the character typed, ignore it.
        (b) AltGr (RALT) affacts the character typed and often comes with CTRL. */
    uint32_t simplifyModifier(uint32_t m)
    {
        m &= ~(KMOD_LSHIFT | KMOD_RSHIFT);
        if ((m & KMOD_RALT) != 0) {
            m &= ~(KMOD_RALT | KMOD_CTRL);
        }
        return m;
    }

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
        if ((m & (KMOD_LGUI | KMOD_RGUI)) != 0) {
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
        // FIXME: needed?
        bool shifted = (mod & KMOD_NUM) != 0;

        switch (sym) {
         case SDLK_KP_ENTER:  return util::Key_Return;
         case SDLK_KP_0:      return shifted ? '0' : util::Key_Insert;
         case SDLK_KP_1:      return shifted ? '1' : util::Key_End;
         case SDLK_KP_2:      return shifted ? '2' : util::Key_Down;
         case SDLK_KP_3:      return shifted ? '3' : util::Key_PgDn;
         case SDLK_KP_4:      return shifted ? '4' : util::Key_Left;
         case SDLK_KP_5:      return shifted ? '5' : util::Key_Num5;
         case SDLK_KP_6:      return shifted ? '6' : util::Key_Right;
         case SDLK_KP_7:      return shifted ? '7' : util::Key_Home;
         case SDLK_KP_8:      return shifted ? '8' : util::Key_Up;
         case SDLK_KP_9:      return shifted ? '9' : util::Key_PgUp;
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
         case SDLK_PRINTSCREEN: return util::Key_Print;
         case SDLK_PAUSE:     return util::Key_Pause;
         case SDLK_MENU:      return util::Key_Menu;
         default:
            if (sym >= ' ' && sym < 127) {
                return sym;
            } else {
                return 0;
            }
        }
    }


    bool isKnownIgnorableKey(uint32_t sym)
    {
        return // sym == SDLK_NUMLOCK
            // ||
            sym == SDLK_CAPSLOCK
            // || sym == SDLK_SCROLLOCK
            || sym == SDLK_RSHIFT
            || sym == SDLK_LSHIFT
            || sym == SDLK_RCTRL
            || sym == SDLK_LCTRL
            || sym == SDLK_RALT
            || sym == SDLK_LALT
            || sym == SDLK_RGUI
            || sym == SDLK_LGUI
            // || sym == SDLK_LSUPER
            // || sym == SDLK_RSUPER
            || sym == SDLK_MODE
            // || sym == SDLK_COMPOSE
            || sym == 0 /* SDL does not know to map it either */;
    }

    // /** QUIT handler.
    //     When SDL itself sees a QUIT event, it will think we handle it, and close our window.
    //     This does not fit our model.
    //     We want a QUIT event to generate an event but not disrupt the normal event stream.
    //     Hence, we post the event manually and lie to SDL. */
    // extern "C" int quitHandler(const SDL_Event* event)
    // {
    //     if (event->type == SDL_QUIT) {
    //         /* SDL_PushEvent is SDL_PeepEvents in disguise, but doesn't modify *event */
    //         SDL_PushEvent(const_cast<SDL_Event*>(event));
    //         return 0;
    //     }
    //     return 1;
    // }

}


gfx::sdl2::Engine::Engine(afl::sys::LogListener& log)
    : m_log(log),
      m_window(0),
      m_sdlWindow(0),
      m_sdlTexture(0),
      m_sdlRenderer(0),
      m_disableGrab(false),
      m_grabEnabled(false),
      m_grabDelay(1000 / 10),
      m_grabEnableTime(0),
      m_lastClickTime(0),
      m_lastClickPosition(),
      m_buttonPressed(false),
      m_doubleClickDelay(1000 / 3),
      m_runnableSemaphore(0),
      m_lastWasRunnable(false),
      m_timerQueue(),
      m_taskMutex(),
      m_taskQueue()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        throw GraphicsException(afl::string::Format(_("Error initializing SDL: %s").c_str(), SDL_GetError()));
    }

    // SDL_SetEventFilter(quitHandler);
    // SDL_EnableUNICODE(1);
    // SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    SDL_EventState(SDL_ENABLE, SDL_KEYDOWN);
    SDL_EventState(SDL_ENABLE, SDL_MOUSEBUTTONUP);
    SDL_EventState(SDL_ENABLE, SDL_MOUSEBUTTONDOWN);
    SDL_EventState(SDL_ENABLE, SDL_MOUSEMOTION);
    SDL_EventState(SDL_ENABLE, SDL_MOUSEWHEEL);
    SDL_EventState(SDL_DISABLE, SDL_TEXTINPUT);

    // m_grabEnabled = false;

    // std::atexit(SDL_Quit);
}

gfx::sdl2::Engine::~Engine()
{
    // ex ui/event.cc:doneEvents
    clearWindowStuff();
    // SDL_SetEventFilter(0);
    SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_TIMER);
}

// Engine methods:
afl::base::Ref<gfx::Canvas>
gfx::sdl2::Engine::createWindow(const WindowParameters& param)
{
    // FIXME: we currently ignore the bpp and always produce a ARGB surface
    int sdlFlags = 0;
    if (param.fullScreen) {
        sdlFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    const int width = param.size.getX();
    const int height = param.size.getY();

    SDL_Window* window = SDL_CreateWindow(param.title.empty() ? "gfx::sdl2::Engine Window" : param.title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, sdlFlags);
    if (window == 0) {
        throw GraphicsException(afl::string::Format(_("Error setting video mode (%s): %s").c_str(), "SDL_CreateWindow", SDL_GetError()));
    }

    if (param.icon.get() != 0) {
        // Convert the icon to something manageable.
        // It must be a SDL surface, but SDL can handle pretty much any format and converts that to OS limits.
        Point iconSize = param.icon->getSize();
        SDL_Surface* iconSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, iconSize.getX(), iconSize.getY(), 32,
                                                        0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
        if (iconSurface) {
            Surface iconCopy(iconSurface, true);
            iconCopy.blit(Point(0, 0), *param.icon, Rectangle(Point(0, 0), iconSize));
            iconCopy.ensureUnlocked();
            SDL_SetWindowIcon(window, iconSurface);
        }
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == 0) {
        throw GraphicsException(afl::string::Format(_("Error setting video mode (%s): %s").c_str(), "SDL_CreateRenderer", SDL_GetError()));
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.
    SDL_RenderSetLogicalSize(renderer, width, height);
    SDL_RenderClear(renderer);

    SDL_Texture* tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (tex == 0) {
        throw GraphicsException(afl::string::Format(_("Error setting video mode (%s): %s").c_str(), "SDL_CreateTexture", SDL_GetError()));
    }

    // Log it
    SDL_RendererInfo info;
    if (SDL_GetRendererInfo(renderer, &info) == 0) {
        String_t flags;
        if (info.flags & SDL_RENDERER_SOFTWARE) {
            flags += ", sw";
        }
        if (info.flags & SDL_RENDERER_ACCELERATED) {
            flags += ", hw";
        }
        if (info.flags & SDL_RENDERER_TARGETTEXTURE) {
            flags += ", target texture";
        }
        m_log.write(m_log.Info, LOG_NAME, afl::string::Format(_("Video driver: %s%s").c_str(), info.name, flags));
    }

    SDL_Surface* sfc = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
    if (sfc == 0) {
        throw GraphicsException(afl::string::Format(_("Error setting video mode (%s): %s").c_str(), "SDL_CreateRGBSurface", SDL_GetError()));
    }

    setWindowStuff(window, tex, renderer);
    m_window = new Surface(sfc, true);

    return *m_window;
}

afl::base::Ref<gfx::Canvas>
gfx::sdl2::Engine::loadImage(afl::io::Stream& file)
{
    StreamInterface iface(file);

    SDL_Surface* sfc;
#if HAVE_SDL2_IMAGE
    sfc = IMG_Load_RW(&iface, 0);
#else
    sfc = SDL_LoadBMP_RW(&iface, 0);
#endif
    if (!sfc) {
        throw afl::except::FileFormatException(file, SDL_GetError());
    }

    return *new Surface(sfc, true);
}

void
gfx::sdl2::Engine::handleEvent(EventConsumer& consumer, bool relativeMouseMovement)
{
    // Performance hack.
    // SDL runs at 100 Hz. This means that a task that posts Runnables in lock-step mode
    // (i.e. a new Runnable is posted after the previous one confirmed)
    // will get a throughput of at best 100 operations per second.
    // Building a scripted dialog will need several dozen actions, leading to noticeable delays.
    // If we know the last event was a Runnable, we therefore anticipate that the next one will be one, too.
    // Sleeping a little will give the other side time to prepare their event.
    // Posting a Runnable will post the semaphore, interrupting the wait.
    // This improves throughput good enough such that constructing a dialog such as
    // "CCUI.Ship.SetExtendedMission" no longer leads to a noticeable delay.
    bool hasRunnable = false;
    if (m_lastWasRunnable) {
        hasRunnable = m_runnableSemaphore.wait(5);
        m_lastWasRunnable = false;
    }

    if (!hasRunnable) {
        // Flush output
        if (Surface* sfc = m_window.get()) {
            if (m_sdlTexture != 0 && m_sdlRenderer != 0) {
                sfc->presentUpdate(m_sdlTexture, m_sdlRenderer);
            }
        }
    }

    // // Update mouse grab
    // setMouseGrab(relativeMouseMovement);

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
            // FIXME: SDL2 has SDL_WaitEventTimeout
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

// Get request dispatcher.
util::RequestDispatcher&
gfx::sdl2::Engine::dispatcher()
{
    return *this;
}

// Create a user-interface timer.
afl::base::Ref<gfx::Timer>
gfx::sdl2::Engine::createTimer()
{
    return m_timerQueue.createTimer();
}

// /*
//  *  Privates
//  */

void
gfx::sdl2::Engine::setWindowStuff(SDL_Window* win, SDL_Texture* tex, SDL_Renderer* renderer)
{
    clearWindowStuff();
    m_sdlWindow = win;
    m_sdlTexture = tex;
    m_sdlRenderer = renderer;
}

void
gfx::sdl2::Engine::clearWindowStuff()
{
    // FIXME: I think SDL_DestroyRenderer is needed, but if I add it, valgrind sees a bad memory access:
    // SDL_DestroyRenderer(m_renderer);
    m_sdlRenderer = 0;
    if (m_sdlTexture != 0) {
        SDL_DestroyTexture(m_sdlTexture);
        m_sdlTexture = 0;
    }
    if (m_sdlWindow != 0) {
        SDL_DestroyWindow(m_sdlWindow);
        m_sdlWindow = 0;
    }
}

// /** Set mouse mode.
//     \param enable true: grab mouse pointer and start reporting infinite movement; false: normal mouse behaviour */
// void
// gfx::sdl2::Engine::setMouseGrab(bool enable)
// {
//     // ex ui/event.cc:setMouseMode
//     if (!m_disableGrab && (enable != m_grabEnabled)) {
//         m_grabEnabled = enable;
//         if (enable) {
//             SDL_ShowCursor(0);
//             SDL_WM_GrabInput(SDL_GRAB_ON);
//             m_grabEnableTime = afl::sys::Time::getTickCounter();
//         } else {
//             SDL_ShowCursor(1);
//             SDL_WM_GrabInput(SDL_GRAB_OFF);
//         }
//     }
// }

/** Convert and dispatch SDL event.
    \param se       SDL event
    \param consumer Event consumer to receive the event
    \param infinite true iff we are in "infinite movement" mode and want relative mouse movement to be reported.
    \retval true Event was dispatched
    \retval false Event not understood */
bool
gfx::sdl2::Engine::convertEvent(const SDL_Event& se, gfx::EventConsumer& consumer, bool infinite)
{
    // ex ui/event.cc:convertEvent
    m_lastWasRunnable = false;
    switch(se.type) {
     // For now, ignore SDL_TEXTINPUT. SDL_TEXTINPUT always comes in combination with SDL_KEYDOWN and is processed there.
     // case SDL_TEXTINPUT:
     //    printf("SDL_TEXTINPUT:     text='%s' [mods=%d]\n", se.text.text, SDL_GetModState());
     //    handleTextInput(consumer, se.text.text, convertModifier(simplifyModifier(SDL_GetModState())));
     //    return true;

     case SDL_KEYDOWN: {
        // Key handling in SDL2 seems to be complex.
        // On German keyboard, X11:
        //     key           scancode  sym  mod     textinput
        //     e                  8    101  0        'e'
        //     shift-e            8    101  LSHIFT   'E'
        //     caps e             8    101  CAPS     'E'
        //     altgr-e            8    101  RALT     '\u20AC'
        //     ctrl-e             8    101  LCTRL    -
        //     alt-e              8    101  LALT     'E' (!)
        // Thus, we're going for the following logic:
        //   If a SDL_TEXTINPUT event is in the queue at the time a SDL_KEYDOWN is received, process that.
        //   Otherwise, process the SDL_KEYDOWN as key event.
        // printf("SDL_KEYDOWN:       scan=%d, sym=%d, mod=%d\n", se.key.keysym.scancode, se.key.keysym.sym, se.key.keysym.mod);

        SDL_Event otherEvent;
        if (fetchNextEventIfType(otherEvent, SDL_TEXTINPUT)) {
            // printf("  SDL_TEXTINPUT:   text='%s'\n", otherEvent.text.text);
            handleTextInput(consumer, otherEvent.text.text, convertModifier(simplifyModifier(se.key.keysym.mod)));
            return true;
        } else {
            uint32_t sdlMod = se.key.keysym.mod;
            util::Key_t mod = convertModifier(sdlMod);
            util::Key_t key = convertKey(se.key.keysym.sym, sdlMod);
            if (key != 0) {
                consumer.handleKey(key | mod, 0);
                return true;
            } else {
                if (!isKnownIgnorableKey(se.key.keysym.sym)) {
                    m_log.write(m_log.Trace, LOG_NAME, afl::string::Format(_("Key not mapped: 0x%x").c_str(), int(se.key.keysym.sym)));
                }
                return false;
            }
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
                while (fetchNextEventIfType(ae, SDL_MOUSEMOTION)) {
                    pt += Point(ae.motion.xrel, ae.motion.yrel);
                }
                consumer.handleMouse(pt, convertMouseButtons(se.motion.state, convertModifier(SDL_GetModState())));
                return true;
            }
        } else {
            Point pt(se.motion.x, se.motion.y);

            // If there are more motion events, consume those as well.
            SDL_Event ae;
            while (fetchNextEventIfType(ae, SDL_MOUSEMOTION)) {
                pt = Point(ae.motion.x, ae.motion.y);
            }
            consumer.handleMouse(pt, convertMouseButtons(se.motion.state, convertModifier(SDL_GetModState())));
            return true;
        }

     case SDL_MOUSEBUTTONDOWN:
        /* Reportedly, touchpads generate very quick sequences of
           button-down and button-up, so we never see a button pressed
           in SDL_GetMouseState(). However, since this is a press, we
           know that the respective button must be down, so force it
           to be included. */
        return handleMouse(consumer, se.button, infinite, 1 << (se.button.button-1));

     case SDL_MOUSEWHEEL:
        if (se.wheel.y > 0) {
            consumer.handleKey(util::Key_WheelUp | convertModifier(SDL_GetModState()), 0);
            return true;
        } else if (se.wheel.y < 0) {
            consumer.handleKey(util::Key_WheelDown | convertModifier(SDL_GetModState()), 0);
            return true;
        } else {
            return false;
        }

     case SDL_MOUSEBUTTONUP:
        return handleMouse(consumer, se.button, infinite, 0);

     case SDL_WAKE_EVENT:
        m_lastWasRunnable = true;
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
gfx::sdl2::Engine::handleMouse(EventConsumer& consumer, const SDL_MouseButtonEvent& be, bool infinite, uint32_t addButton)
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
gfx::sdl2::Engine::handleTextInput(EventConsumer& consumer, const char* text, util::Key_t mod)
{
    // FIXME: probably some code breaks if this actually produces multiple events
    afl::charset::Utf8Reader rdr(afl::string::toBytes(text), 0);
    while (rdr.hasMore()) {
        consumer.handleKey(rdr.eat() | mod, 0);
    }
}

void
gfx::sdl2::Engine::postNewRunnable(afl::base::Runnable* p)
{
    // Make sure to post only non-null runnables.
    // Each Runnable will be accompanied by a single SDL_WAKE_EVENT.
    if (p != 0) {
        afl::sys::MutexGuard g(m_taskMutex);
        m_taskQueue.pushBackNew(p);

        SDL_Event event;
        event.type = SDL_WAKE_EVENT;
        event.user.code = 0;
        event.user.data1 = 0;
        event.user.data2 = 0;
        SDL_PushEvent(&event);
        m_runnableSemaphore.post();
    }
}

void
gfx::sdl2::Engine::processTaskQueue()
{
    // Process precisely one Runnable.
    // Processing the Runnable may spawn a new event loop.
    // If we were using the "swap whole list and process everything" optimisation here,
    // events posted before the spawn would be held up.
    m_runnableSemaphore.wait(0);
    std::auto_ptr<afl::base::Runnable> t;
    {
        afl::sys::MutexGuard g(m_taskMutex);
        t.reset(m_taskQueue.extractFront());
    }

    if (t.get() != 0) {
        t->run();
    }
}

#else
int g_dummyToMakeGfxSdl2EngineNotEmpty;
#endif
