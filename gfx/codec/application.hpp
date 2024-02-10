/**
  *  \file gfx/codec/application.hpp
  *  \brief Class gfx::codec::Application
  */
#ifndef C2NG_GFX_CODEC_APPLICATION_HPP
#define C2NG_GFX_CODEC_APPLICATION_HPP

#include "util/application.hpp"
#include "util/stringparser.hpp"

namespace gfx { namespace codec {

    /** Graphics codec application (c2gfxcodec).
        This is a standalone application to use image codecs.
        Its main purpose is to convert to and from our custom codecs. */
    class Application : public util::Application {
     public:
        struct Status;

        /** Constructor.
            @param env Environment
            @param fs  File system */
        Application(afl::sys::Environment& env, afl::io::FileSystem& fs);

     protected:
        virtual void appMain();

        void showHelp();
        void doConvert(afl::base::Ref<afl::sys::Environment::CommandLine_t> cmdl);
        void doCreateResource(afl::base::Ref<afl::sys::Environment::CommandLine_t> cmdl);
        void doGallery(afl::base::Ref<afl::sys::Environment::CommandLine_t> cmdl);

        bool openInput(Status& st, util::StringParser& p);
        bool openOutput(Status& st, util::StringParser& p);
        bool openFile(Status& st, util::StringParser& p, afl::io::FileSystem::OpenMode mode);
    };

} }

#endif
