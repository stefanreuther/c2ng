/**
  *  \file gfx/application.cpp
  *  \brief Class gfx::Application
  */

#include <stdexcept>
#include "config.h"
#include "gfx/application.hpp"
#include "afl/except/commandlineexception.hpp"
#ifdef HAVE_SDL
# include "gfx/sdl/engine.hpp"
typedef gfx::sdl::Engine Engine_t;
#elif defined(HAVE_SDL2)
# include "gfx/sdl2/engine.hpp"
typedef gfx::sdl2::Engine Engine_t;
#else
# define NO_ENGINE
#endif

// Constructor.
gfx::Application::Application(afl::sys::Dialog& dialog, afl::string::Translator& tx, const String_t& title)
    : m_dialog(dialog),
      m_translator(tx),
      m_title(title),
      m_log()
{ }

#ifndef NO_ENGINE
/* If we don't have an engine, let linking fail.
   This way, code that does not need an engine still builds even if it is contained in guilib. */
int
gfx::Application::run()
{
    try {
        Engine_t engine(m_log);
        appMain(engine);
    }
    catch (afl::except::CommandLineException& cx) {
        m_dialog.showError(cx.what(), m_title);
        return 1;
    }
    catch (std::exception& e) {
        m_dialog.showError(m_log.formatException(m_translator.translateString("Uncaught exception"), e) + "\n\n" + m_translator.translateString("Program exits abnormally (crash)"), m_title);
        return 1;
    }
    catch (...) {
        m_dialog.showError(m_translator.translateString("Uncaught exception") + "\n\n" + m_translator.translateString("Program exits abnormally (crash)"), m_title);
        return 1;
    }
    return 0;
}
#endif
