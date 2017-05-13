/**
  *  \file gfx/filter.hpp
  *  \brief Class gfx::Filter
  */
#ifndef C2NG_GFX_FILTER_HPP
#define C2NG_GFX_FILTER_HPP

#include "gfx/canvas.hpp"

namespace gfx {

    /** Filtering canvas.
        A filter is a canvas that somehow modifies the content drawn on it, e.g. by modifying coordinates or colors.
        This class implements the methods that are typically passed through to the underlying canvas.
        Canvases that need this feature can derive from Filter.
        Otherwise nothing should require filters being derived from Filter. */
    class Filter : public Canvas {
     public:
        /** Constructor.
            \param parent Underlying canvas */
        explicit Filter(Canvas& parent);
        ~Filter();

        // Canvas:
        virtual void getPixels(Point pt, afl::base::Memory<Color_t> colors);
        virtual Point getSize();
        virtual int getBitsPerPixel();
        virtual void setPalette(Color_t start, afl::base::Memory<const ColorQuad_t> colorDefinitions, afl::base::Memory<Color_t> colorHandles);
        virtual void decodeColors(afl::base::Memory<const Color_t> colorHandles, afl::base::Memory<ColorQuad_t> colorDefinitions);
        virtual void encodeColors(afl::base::Memory<const ColorQuad_t> colorDefinitions, afl::base::Memory<Color_t> colorHandles);
        virtual afl::base::Ref<Canvas> convertCanvas(afl::base::Ref<Canvas> orig);

        /** Access parent canvas.
            \return canvas */
        Canvas& parent();

     private:
        Canvas& m_parent;
    };

}

#endif
