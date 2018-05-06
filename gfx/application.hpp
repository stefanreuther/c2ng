/**
  *  \file gfx/application.hpp
  */
#ifndef C2NG_GFX_APPLICATION_HPP
#define C2NG_GFX_APPLICATION_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/dialog.hpp"
#include "afl/sys/log.hpp"
#include "gfx/engine.hpp"

namespace gfx {

    /** Graphical application base.
        This class provides a means to obtain a gfx::Engine instance and basic exception handling boilerplate. */
    class Application : public afl::base::Deletable {
     public:
        /** Constructor.
            \param dialog Dialog instance (used for error reporting)
            \param tx Translator instance
            \param title Application title */
        Application(afl::sys::Dialog& dialog, afl::string::Translator& tx, const String_t& title);

        /** Entry point.
            Call from your main() function.
            \return exit code to return from main() */
        int run();

        /** Access loggeer.
            \return logger */
        afl::sys::Log& log();

        /** Access translator.
            \return translator */
        afl::string::Translator& translator();

        /** Access dialog instance.
            \return dialog instance */
        afl::sys::Dialog& dialog();

        /** Application entry point.
            Implement your application here.
            \param engine Engine instance */
        virtual void appMain(Engine& engine) = 0;

     private:
        afl::sys::Dialog& m_dialog;
        afl::string::Translator& m_translator;
        String_t m_title;
        afl::sys::Log m_log;
    };

}


inline afl::sys::Log&
gfx::Application::log()
{
    return m_log;
}

inline afl::string::Translator&
gfx::Application::translator()
{
    return m_translator;
}

inline afl::sys::Dialog&
gfx::Application::dialog()
{
    return m_dialog;
}

#endif
