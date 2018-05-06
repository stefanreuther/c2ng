/**
  *  \file gfx/scan.hpp
  */
#ifndef C2NG_GFX_SCAN_HPP
#define C2NG_GFX_SCAN_HPP

namespace gfx {

    class Canvas;

    bool scanCanvas(Canvas& canvas, int& y, int& minX, int& maxX);

}

#endif
