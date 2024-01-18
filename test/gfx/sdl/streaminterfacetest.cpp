/**
  *  \file test/gfx/sdl/streaminterfacetest.cpp
  *  \brief Test for gfx::sdl::StreamInterface
  */

#include "gfx/sdl/streaminterface.hpp"

#include "afl/base/memory.hpp"
#include "afl/io/memorystream.hpp"
#include "afl/test/testrunner.hpp"
#include "config.h"

#if HAVE_SDL
# include "gfx/sdl/streaminterface.hpp"
#endif


/** Simple test. */
AFL_TEST("gfx.sdl.StreamInterface", a)
{
#if HAVE_SDL
    // Some data buffer
    uint8_t data[20];
    afl::base::Bytes_t(data).fill(0);

    // Testee
    afl::io::MemoryStream ms(data);
    gfx::sdl::StreamInterface testee(ms);

    SDL_RWops* ctx = &testee;

    // Test using SDL operations
    // - write 4 bytes, "hiho................"
    a.checkEqual("01. SDL_RWtell",  SDL_RWtell(ctx), 0);
    a.checkEqual("02. SDL_RWwrite", SDL_RWwrite(ctx, "hi", 2, 1), 1);
    a.checkEqual("03. SDL_RWwrite", SDL_RWwrite(ctx, "ho", 1, 2), 2);
    a.checkEqual("04. SDL_RWtell",  SDL_RWtell(ctx), 4);

    // - seek backward and read 6 bytes
    a.checkEqual("11. SDL_RWseek",  SDL_RWseek(ctx, -3, SEEK_CUR), 1);
    a.checkEqual("12. SDL_RWtell",  SDL_RWtell(ctx), 1);

    uint8_t out[6];
    static const uint8_t expect[] = {'i','h','o',0,0,0};
    a.checkEqual("21. SDL_RWread",  SDL_RWread(ctx, out, 2, 3), 3);
    a.checkEqualContent<uint8_t>("22. data", out, expect);
    a.checkEqual("23. SDL_RWtell",  SDL_RWtell(ctx), 7);

    // - seek to end of file
    a.checkEqual("31. SDL_RWseek",  SDL_RWseek(ctx, 0, SEEK_END), 20);
    a.checkEqual("32. SDL_RWread",  SDL_RWread(ctx, out, 2, 3), 0);
    a.checkEqual("33. SDL_RWwrite", SDL_RWwrite(ctx, out, 2, 3), 0);

    // - close (no-op)
    a.checkEqual("41. SDL_RWclose", SDL_RWclose(ctx), 0);
#else
    (void) a;
#endif
}
