/**
  *  \file ui/res/provider.hpp
  */
#ifndef C2NG_UI_RES_PROVIDER_HPP
#define C2NG_UI_RES_PROVIDER_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/ptr.hpp"
#include "gfx/canvas.hpp"
#include "afl/string/string.hpp"
#include "afl/io/stream.hpp"
#include "afl/io/directory.hpp"

namespace ui { namespace res {

    class Manager;

    class Provider : public afl::base::Deletable {
     public:
        virtual afl::base::Ptr<gfx::Canvas> loadImage(String_t name, Manager& mgr) = 0;

        /*
         *  Utility functions
         */
        static afl::base::Ptr<afl::io::Stream> openResourceFile(afl::io::Directory& dir, String_t fileName, afl::base::Memory<const char*const> suffixes);

        static afl::base::Memory<const char*const> graphicsSuffixes();
    };

} }

#endif
