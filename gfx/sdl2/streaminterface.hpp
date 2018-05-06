/**
  *  \file gfx/sdl2/streaminterface.hpp
  */
#ifndef C2NG_GFX_SDL2_STREAMINTERFACE_HPP
#define C2NG_GFX_SDL2_STREAMINTERFACE_HPP

#include <SDL_rwops.h>
#include "afl/io/stream.hpp"

namespace gfx { namespace sdl2 {

    /** Adapter between afl::io::Stream and SDL_RWops.

        This class translates a Stream object into the SDL_RWops interface for SDL routines, such as SDL_LoadBMP().
        Lifetime management is still up to the caller, the close() function exported to SDL is a no-op. */
    class StreamInterface : public SDL_RWops {
     public:
        /** Constructor.
            \param parent Parent stream. Lifetime must exceed that of the StreamInterface. */
        explicit StreamInterface(afl::io::Stream& parent);

        /** Access parent stream.
            \return parent stream */
        afl::io::Stream& parent();

     private:
        afl::io::Stream& m_parent;
    };

} }

#endif
