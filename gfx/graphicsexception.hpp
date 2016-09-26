/**
  *  \file gfx/graphicsexception.hpp
  */
#ifndef C2NG_GFX_GRAPHICSEXCEPTION_HPP
#define C2NG_GFX_GRAPHICSEXCEPTION_HPP

#include <stdexcept>
#include "afl/string/string.hpp"

namespace gfx {

    /** Graphics operation failed.
        Those are usually fatal.
        The message text usually is SDL_GetError(). */
    class GraphicsException : public std::runtime_error {
     public:
        GraphicsException(const String_t& what);
        GraphicsException(const char* what);
        GraphicsException(const GraphicsException& e);
        ~GraphicsException() throw();
    };

}

#endif
