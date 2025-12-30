/**
  *  \file ui/reshack/application.hpp
  *  \brief Class ui::reshack::Application
  */
#ifndef C2NG_UI_RESHACK_APPLICATION_HPP
#define C2NG_UI_RESHACK_APPLICATION_HPP

#include "gfx/application.hpp"
#include "afl/sys/dialog.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/environment.hpp"
#include "afl/io/filesystem.hpp"

namespace ui { namespace reshack {

    class Session;

    /** Main Entry Point for ResHack application. */
    class Application : public gfx::Application {
     public:
        /** Constructor.
            @param dialog Dialog instance (for help messages)
            @param tx     Translator instance
            @param env    Environment instance
            @param fs     File System instance */
        Application(afl::sys::Dialog& dialog,
                    afl::string::Translator& tx,
                    afl::sys::Environment& env,
                    afl::io::FileSystem& fs);

        /** Main entry point of graphical application.
            @param engine Graphics engine */
        virtual void appMain(gfx::Engine& engine);

     private:
        struct Parameters;
        class EditorList;
        class NewPictureDialog;
        class MainDialog;

        afl::sys::Environment& m_environment;
        afl::io::FileSystem& m_fileSystem;

        void parseParameters(Parameters& p);
        void loadCharacterNames(Session& s, Parameters& p);
    };

} }

#endif
