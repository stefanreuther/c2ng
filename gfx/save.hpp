/**
  *  \file gfx/save.hpp
  *  \brief Function gfx::saveCanvas
  */
#ifndef C2NG_GFX_SAVE_HPP
#define C2NG_GFX_SAVE_HPP

#include "afl/io/stream.hpp"

namespace gfx {

    class Canvas;

    /** Save canvas to file in 24-bit BMP format.
        \param can Canvas to read
        \param stream Stream to write to

        Note that this will always produce a 24-bit BMP file, no matter what format the canvas is in. */
    void saveCanvas(Canvas& can, afl::io::Stream& stream);

}

#endif
