/**
  *  \file test/gfx/sdl/enginetest.cpp
  *  \brief Test for gfx::sdl::Engine
  */

#include "config.h"
#if HAVE_SDL
# include "gfx/sdl/engine.hpp"
#endif

#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST_NOARG("gfx.sdl.Engine:instantiation")
{
    /* We want to test that instantiating the engine works, mainly,
       that it does not have unimplemented methods and links
       correctly. We will not perform any functional tests. However,
       instantiating it will fail in some testing environments even
       though linking worked (like Cygwin or servers, which have SDL
       and X libraries but no display X), or do funny things on
       others (like Debian with DISPLAY unset).

       Thus, we make the totally unfounded assumption that
       instantiation will work if DISPLAY is set. */
#if HAVE_SDL
    if (std::getenv("DISPLAY") != 0) {
        afl::sys::Log log;
        afl::string::NullTranslator tx;
        gfx::sdl::Engine testee(log, tx);
    }
#endif
}
