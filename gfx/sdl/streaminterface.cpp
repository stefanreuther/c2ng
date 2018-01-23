/**
  *  \file gfx/sdl/streaminterface.cpp
  *  \brief Class gfx::sdl::StreamInterface
  */

#include <stdexcept>
#include "config.h"
#if HAVE_SDL
# include "gfx/sdl/streaminterface.hpp"

namespace {
    using gfx::sdl::StreamInterface;

    extern "C" int SDLIF_seek(SDL_RWops* context, int offset, int whence)
    {
        afl::io::Stream& s = static_cast<StreamInterface*>(context)->parent();
        if (s.getCapabilities() & s.CanSeek) {
            try {
                switch(whence) {
                 case SEEK_SET:
                    s.setPos(offset);
                    break;
                 case SEEK_CUR:
                    s.setPos(s.getPos() + offset);
                    break;
                 case SEEK_END:
                    s.setPos(s.getSize() + offset);
                    break;
                }
            }
            catch (std::exception& e) {
                SDL_SetError("%s", e.what());
                return -1;
            }
            return static_cast<int>(s.getPos());
        } else {
            SDL_SetError("Seek error");
            return -1;
        }
    }

    extern "C" int SDLIF_read(SDL_RWops* context, void* ptr, int size, int maxnum)
    {
        afl::io::Stream& s = static_cast<StreamInterface*>(context)->parent();
        try {
            if (size <= 0 || maxnum <= 0) {
                return maxnum;
            } else {
                size_t r = s.read(afl::base::Bytes_t::unsafeCreate(static_cast<uint8_t*>(ptr), size_t(size) * maxnum));
                if (!r) {
                    /* according to SDL_rwops.c, it is an error if only 0 items could be read */
                    SDL_SetError("Read error (end of file)");
                    return 0;
                }
                return static_cast<int>(r/size);
            }
        }
        catch (std::exception& e) {
            SDL_SetError(e.what());
            return 0;
        }
    }

    extern "C" int SDLIF_write(SDL_RWops* context, const void* ptr, int size, int num)
    {
        afl::io::Stream& s = static_cast<StreamInterface*>(context)->parent();
        try {
            if (size <= 0 || num <= 0) {
                return num;
            }
            size_t w = s.write(afl::base::ConstBytes_t::unsafeCreate(static_cast<const uint8_t*>(ptr), size_t(size) * num));
            if (!w) {
                /* according to SDL_rwops.c, it is an error if only 0 items could be written */
                SDL_SetError("Write error (disk full)");
                return 0;
            }
            return static_cast<int>(w/size);
        }
        catch (std::exception& e) {
            SDL_SetError(e.what());
            return 0;
        }
    }

    extern "C" int SDLIF_close(SDL_RWops*)
    {
        // No delete code. Object is deleted automatically.
        return 0;
    }
}

// Constructor.
gfx::sdl::StreamInterface::StreamInterface(afl::io::Stream& parent)
    : m_parent(parent)
{
    SDL_RWops::read  = &SDLIF_read;
    SDL_RWops::write = &SDLIF_write;
    SDL_RWops::seek  = &SDLIF_seek;
    SDL_RWops::close = &SDLIF_close;
}

afl::io::Stream&
gfx::sdl::StreamInterface::parent()
{
    return m_parent;
}

#else
int g_dummyToMakeGfxSdlStreamInterfaceNotEmpty;
#endif
