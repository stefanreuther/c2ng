/**
  *  \file gfx/graphicsexception.hpp
  *  \brief Class gfx::GraphicsException
  */
#ifndef C2NG_GFX_GRAPHICSEXCEPTION_HPP
#define C2NG_GFX_GRAPHICSEXCEPTION_HPP

#include <stdexcept>
#include "afl/string/string.hpp"

namespace gfx {

    /** Graphics operation failed.
        Those are usually fatal.
        The message text usually comes form the graphics library, i.e. SDL_GetError(). */
    class GraphicsException : public std::runtime_error {
     public:
        /** Construct from string.
            \param what Error text */
        GraphicsException(const String_t& what);

        /** Construct from literal.
            \param what Error text */
        GraphicsException(const char* what);

        /** Copy constructor.
            \param e Other exception */
        GraphicsException(const GraphicsException& e);

        /** Destructor. */
        ~GraphicsException() throw();
    };

}

#endif
