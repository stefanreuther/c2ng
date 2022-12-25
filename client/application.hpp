/**
  *  \file client/application.hpp
  *  \brief Class client::Application
  */
#ifndef C2NG_CLIENT_APPLICATION_HPP
#define C2NG_CLIENT_APPLICATION_HPP

#include "afl/io/filesystem.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/dialog.hpp"
#include "afl/sys/environment.hpp"
#include "gfx/application.hpp"

namespace client {

    /** Graphical Client Application, Main Entry Point. */
    class Application : public gfx::Application {
     public:
        /** Constructor.
            @param dialog Dialog instance (for help messages)
            @param tx     Translator instance
            @param env    Environment instance
            @param fs     File System instance
            @param net    Network Stack instance*/
        Application(afl::sys::Dialog& dialog,
                    afl::string::Translator& tx,
                    afl::sys::Environment& env,
                    afl::io::FileSystem& fs,
                    afl::net::NetworkStack& net);

        /** Main entry point of graphical application.
            @param engine Graphics engine */
        virtual void appMain(gfx::Engine& engine);

     private:
        afl::sys::Environment& m_environment;
        afl::io::FileSystem& m_fileSystem;
        afl::net::NetworkStack& m_networkStack;
    };

}

#endif
