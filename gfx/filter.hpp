/**
  *  \file gfx/filter.hpp
  */
#ifndef C2NG_GFX_FILTER_HPP
#define C2NG_GFX_FILTER_HPP

#include "gfx/canvas.hpp"

namespace gfx {

    class Filter : public Canvas {
     public:
        Filter(Canvas& parent);
        ~Filter();

        virtual Color_t getPixel(Point pt);
        virtual void getPixels(Point pt, afl::base::Memory<Color_t> colors);
        virtual Point getSize();
        virtual int getBitsPerPixel();
        virtual void setPalette(Color_t start, afl::base::Memory<const ColorQuad_t> colorDefinitions, afl::base::Memory<Color_t> colorHandles);
        virtual void decodeColors(afl::base::Memory<const Color_t> colorHandles, afl::base::Memory<ColorQuad_t> colorDefinitions);
        virtual void encodeColors(afl::base::Memory<const ColorQuad_t> colorDefinitions, afl::base::Memory<Color_t> colorHandles);
        virtual afl::base::Ptr<Canvas> convertCanvas(afl::base::Ptr<Canvas> orig);

        Canvas& parent();

     private:
        Canvas& m_parent;
    };

}

#endif
