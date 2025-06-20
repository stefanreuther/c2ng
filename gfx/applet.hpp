/**
  *  \file gfx/applet.hpp
  *  \brief Class gfx::Applet
  */
#ifndef C2NG_GFX_APPLET_HPP
#define C2NG_GFX_APPLET_HPP

#include "afl/container/ptrvector.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/environment.hpp"
#include "gfx/application.hpp"
#include "util/application.hpp"

namespace gfx {

    /** Graphics test applet.
        Applets are mainly intended for testing.

        To use,
        - derive classes and implement run()
        - add to an instance of Applet::Runner thatis used as main entry point

        This class is similar (but not identical) to util::Applet for text-based applets. */
    class Applet {
     public:
        virtual ~Applet()
            { }

        /** Applet entry point.
            @param app     Containing Application instance (e.g. to call Application::exit())
            @param engine  Engine instance
            @param env     Environment
            @param fs      FileSystem
            @param cmdl    Command line (applet name has already been removed)
            @return exit code */
        virtual int run(Application& app, Engine& engine, afl::sys::Environment& env, afl::io::FileSystem& fs, afl::sys::Environment::CommandLine_t& cmdl) = 0;

        class Runner;
    };

    /** Graphics test applet runner. */
    class Applet::Runner : private afl::string::NullTranslator, public Application {
     public:
        /** Constructor.
            @param dialog    Dialog instance
            @param env       Environment instance
            @param fs        FileSystem instance
            @param title     Application name */
        Runner(afl::sys::Dialog& dialog, afl::sys::Environment& env, afl::io::FileSystem& fs, const String_t& title);

        /** Destructor. */
        ~Runner();

        /** Add an applet.
            @param name              Applet name (for command line)
            @param untranslatedInfo  Information (for help; will be passed through translator)
            @param p                 Newly-created applet instance, Runner will take ownership; must not be null
            @return this, for chaining */
        Runner& addNew(String_t name, String_t untranslatedInfo, Applet* p);

     protected:
        virtual void appMain(Engine& engine);

        afl::sys::Environment& m_environment;
        afl::io::FileSystem& m_fileSystem;
        String_t m_title;

        struct Info;
        afl::container::PtrVector<Info> m_applets;
        String_t m_untranslatedName;

        const Info* findApplet(const String_t& appletName) const;
        void showHelp();
    };

}

#endif
