/**
  *  \file gfx/sdl2/streaminterface.cpp
  */

#include <stdexcept>
#include "config.h"
#if HAVE_SDL2
# include "gfx/sdl2/streaminterface.hpp"

namespace {
    using gfx::sdl2::StreamInterface;

    extern "C" Sint64 SDLIF_size(SDL_RWops* context)
    {
        return static_cast<StreamInterface*>(context)->parent().getSize();
    }

    extern "C" Sint64 SDLIF_seek(SDL_RWops* context, Sint64 offset, int whence)
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
            return s.getPos();
        } else {
            SDL_SetError("Seek error");
            return -1;
        }
    }

    extern "C" size_t SDLIF_read(SDL_RWops* context, void* ptr, size_t size, size_t maxnum)
    {
        afl::io::Stream& s = static_cast<StreamInterface*>(context)->parent();
        try {
            if (size <= 0 || maxnum <= 0) {
                return maxnum;
            } else {
                size_t r = s.read(afl::base::Bytes_t::unsafeCreate(static_cast<uint8_t*>(ptr), size * maxnum));
                if (!r) {
                    /* according to SDL_rwops.c, it is an error if only 0 items could be read */
                    SDL_SetError("Read error (end of file)");
                    return 0;
                }
                return r/size;
            }
        }
        catch (std::exception& e) {
            SDL_SetError(e.what());
            return 0;
        }
    }

    extern "C" size_t SDLIF_write(SDL_RWops* context, const void* ptr, size_t size, size_t num)
    {
        afl::io::Stream& s = static_cast<StreamInterface*>(context)->parent();
        try {
            if (size <= 0 || num <= 0) {
                return num;
            }
            size_t w = s.write(afl::base::ConstBytes_t::unsafeCreate(static_cast<const uint8_t*>(ptr), size * num));
            if (!w) {
                /* according to SDL_rwops.c, it is an error if only 0 items could be written */
                SDL_SetError("Write error (disk full)");
                return 0;
            }
            return w/size;
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
gfx::sdl2::StreamInterface::StreamInterface(afl::io::Stream& parent)
    : m_parent(parent)
{
    SDL_RWops::type = SDL_RWOPS_UNKNOWN;
    SDL_RWops::size  = &SDLIF_size;
    SDL_RWops::read  = &SDLIF_read;
    SDL_RWops::write = &SDLIF_write;
    SDL_RWops::seek  = &SDLIF_seek;
    SDL_RWops::close = &SDLIF_close;
}

afl::io::Stream&
gfx::sdl2::StreamInterface::parent()
{
    return m_parent;
}

#else
int g_dummyToMakeGfxSdlStreamInterfaceNotEmpty;
#endif

