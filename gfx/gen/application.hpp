/**
  *  \file gfx/gen/application.hpp
  *  \brief Class gfx::gen::Application
  */
#ifndef C2NG_GFX_GEN_APPLICATION_HPP
#define C2NG_GFX_GEN_APPLICATION_HPP

#include "afl/io/filesystem.hpp"
#include "afl/sys/environment.hpp"
#include "util/application.hpp"
#include "afl/sys/commandlineparser.hpp"

namespace gfx { namespace gen {

    /** Graphics Generator Application (c2gfxgen).
        This is a standalone application to control the builtin procedural generation algorithms.
        It can be used to generate images for use by the web application, for example. */
    class Application : public util::Application {
     public:
        /** Constructor.
            \param env Environment
            \param fs File system */
        Application(afl::sys::Environment& env, afl::io::FileSystem& fs);

        virtual void appMain();

     private:
        struct CommonOptions;

        void showHelp();
        void doSpace(afl::base::Ref<afl::sys::Environment::CommandLine_t> cmdl);
        void doPlanet(afl::base::Ref<afl::sys::Environment::CommandLine_t> cmdl);
        void doOrbit(afl::base::Ref<afl::sys::Environment::CommandLine_t> cmdl);
        void doExplosion(afl::base::Ref<afl::sys::Environment::CommandLine_t> cmdl);
        void doShield(afl::base::Ref<afl::sys::Environment::CommandLine_t> cmdl);
        void doTexture(afl::base::Ref<afl::sys::Environment::CommandLine_t> cmdl);

        bool handleCommonOption(CommonOptions& opt, const String_t& text, afl::sys::CommandLineParser& parser);
    };

} }

#endif
