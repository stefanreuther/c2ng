/**
  *  \file gfx/gen/colorrange.hpp
  *  \brief Class gfx::gen::ColorRange
  */
#ifndef C2NG_GFX_GEN_COLORRANGE_HPP
#define C2NG_GFX_GEN_COLORRANGE_HPP

#include "gfx/types.hpp"
#include "util/stringparser.hpp"

namespace gfx { namespace gen {

    /** Color range.
        Describes a range (line in RGBA colorspace) of colors and allows obtaining colors on that line.

        Start and end colors are inclusive, that is, a range from #000 to #FFF will be able to produce colors #000 and #FFF.

        The line can be divided into a number of discrete segments (steps), with a minimum of 2.
        With four steps, a color range #000 to #333 will produce colors #000, #111, #222, and #333.

        A color can be obtained from an index which runs from [0,255].
        The [0,255] interval will be divided into segment of approximately equal size according to the number of steps.
        With four steps, index [0,63], [64,127], [128,191], [192,255] will produce the same color, respectively. */
    class ColorRange {
     public:
        /** Minimum number of segments. */
        static const int MIN_STEPS = 2;

        /** Maximum number of segments. */
        static const int MAX_STEPS = 256;

        /** Maximum index. */
        static const int MAX_INDEX = 255;

        /** Default constructor.
            Constructs a ColorRange that produces only color 0. */
        ColorRange();

        /** Single color constructor.
            Constructs a ColorRange that produces only the given color.
            \param color Color */
        ColorRange(ColorQuad_t color);

        /** Constructor.
            \param start First color in interval
            \param end Last color in interval
            \param steps Number of steps (segments), [MIN_STEPS,MAX_STEPS] */
        ColorRange(ColorQuad_t start, ColorQuad_t end, int steps = MAX_STEPS);

        /** Get interpolated color.
            \param index Index [0,MAX_INDEX]
            \return color */
        ColorQuad_t get(int index) const;

        /** Get start color.
            \return color (normally, same as get(0)) */
        ColorQuad_t getStartColor() const;

        /** Get end color.
            \return color (normally, same as get(MAX_INDEX) */
        ColorQuad_t getEndColor() const;

        /** Get number of steps (number of discrete result colors).
            \return number of steps */
        int getNumSteps() const;

        /** Parse from string.
            Accepted syntax is "COLOR[-COLOR][/STEPS]",
            where COLOR are colors according to gfx::parseColor(), and STEPS is a number of steps.
            \param p [in/out] StringParser object
            \retval true Successfully parsed, object has been updated
            \retval false Parse error, object has not been modified. \c p points at error location */
        bool parse(util::StringParser& p);

     private:
        ColorQuad_t m_start;
        ColorQuad_t m_end;
        int m_steps;
    };

} }

#endif
