/**
  *  \file u/t_gfx_sdl_streaminterface.cpp
  *  \brief Test for gfx::sdl::StreamInterface
  */

#include "config.h"
#if HAVE_SDL
# include "gfx/sdl/streaminterface.hpp"
#endif

#include "t_gfx_sdl.hpp"
#include "afl/io/memorystream.hpp"
#include "afl/base/memory.hpp"

/** Simple test. */
void
TestGfxSdlStreamInterface::testIt()
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
    TS_ASSERT_EQUALS(SDL_RWtell(ctx), 0);
    TS_ASSERT_EQUALS(SDL_RWwrite(ctx, "hi", 2, 1), 1);
    TS_ASSERT_EQUALS(SDL_RWwrite(ctx, "ho", 1, 2), 2);
    TS_ASSERT_EQUALS(SDL_RWtell(ctx), 4);

    // - seek backward and read 6 bytes
    TS_ASSERT_EQUALS(SDL_RWseek(ctx, -3, SEEK_CUR), 1);
    TS_ASSERT_EQUALS(SDL_RWtell(ctx), 1);

    uint8_t out[6];
    TS_ASSERT_EQUALS(SDL_RWread(ctx, out, 2, 3), 3);
    TS_ASSERT_SAME_DATA(out, "iho\0\0\0", 6);
    TS_ASSERT_EQUALS(SDL_RWtell(ctx), 7);

    // - seek to end of file
    TS_ASSERT_EQUALS(SDL_RWseek(ctx, 0, SEEK_END), 20);
    TS_ASSERT_EQUALS(SDL_RWread(ctx, out, 2, 3), 0);
    TS_ASSERT_EQUALS(SDL_RWwrite(ctx, out, 2, 3), 0);

    // - close (no-op)
    TS_ASSERT_EQUALS(SDL_RWclose(ctx), 0);
#endif
}

