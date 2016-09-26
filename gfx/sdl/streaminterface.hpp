/**
  *  \file gfx/sdl/streaminterface.hpp
  */
#ifndef C2NG_GFX_SDL_STREAMINTERFACE_HPP
#define C2NG_GFX_SDL_STREAMINTERFACE_HPP

#include <SDL_rwops.h>
#include "afl/io/stream.hpp"

namespace gfx { namespace sdl {

    /*! \class StreamSDLInterface
        \brief Stream Interface to SDL_RWops

        This class translates a Stream object into the SDL_RWops interface
        for SDL routines, such as SDL_LoadBMP(). Lifetime management is still
        up to the caller, the close() function exported to SDL is a no-op.

        (This used to be part of the Stream class itself, requiring all
        programs including textual ones to be linked to SDL.) */
    class StreamInterface : public SDL_RWops {
     public:
        StreamInterface(afl::io::Stream& parent);

        afl::io::Stream& parent();

     private:
        afl::io::Stream& m_parent;
    };

} }

#endif
