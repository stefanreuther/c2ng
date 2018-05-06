/**
  *  \file gfx/gen/colorrange.cpp
  *  \brief Class gfx::gen::ColorRange
  */

#include "gfx/gen/colorrange.hpp"

namespace {
    uint8_t mix(uint8_t start, uint8_t end, int index, int numSteps)
    {
        // Let A=(start-end)/(numSteps-1), B=256/numSteps (divide color range and selector range into equal parts).
        // Result is (result-start)/A = index/B,
        // hence result = index/B*A + start,
        //              = index/(256/numSteps) * (end-start)/(numSteps-1) + start
        //              = index*(numSteps/256) * (end-start)/(numSteps-1) + start
        // Mathematically, the two "numSteps" cancel each other out, but the first one produces the intended rounding we want.
        return (numSteps < gfx::gen::ColorRange::MIN_STEPS
                ? start
                : static_cast<uint8_t>((index * numSteps / 256) * (end - start) / (numSteps - 1) + start));
    }
}


gfx::gen::ColorRange::ColorRange()
    : m_start(), m_end(), m_steps(MAX_STEPS)
{ }

gfx::gen::ColorRange::ColorRange(ColorQuad_t color)
    : m_start(color), m_end(color), m_steps(MAX_STEPS)
{ }

gfx::gen::ColorRange::ColorRange(ColorQuad_t start, ColorQuad_t end, int steps)
    : m_start(start), m_end(end), m_steps(steps)
{ }

gfx::ColorQuad_t
gfx::gen::ColorRange::get(int index) const
{
    return COLORQUAD_FROM_RGBA(mix(RED_FROM_COLORQUAD(m_start),   RED_FROM_COLORQUAD(m_end),   index, m_steps),
                               mix(GREEN_FROM_COLORQUAD(m_start), GREEN_FROM_COLORQUAD(m_end), index, m_steps),
                               mix(BLUE_FROM_COLORQUAD(m_start),  BLUE_FROM_COLORQUAD(m_end),  index, m_steps),
                               mix(ALPHA_FROM_COLORQUAD(m_start), ALPHA_FROM_COLORQUAD(m_end), index, m_steps));
}

gfx::ColorQuad_t
gfx::gen::ColorRange::getStartColor() const
{
    return m_start;
}

gfx::ColorQuad_t
gfx::gen::ColorRange::getEndColor() const
{
    return m_end;
}

int
gfx::gen::ColorRange::getNumSteps() const
{
    return m_steps;
}

bool
gfx::gen::ColorRange::parse(util::StringParser& p)
{
    ColorQuad_t start, end;
    if (!parseColor(p, start)) {
        return false;
    }
    if (p.parseCharacter('-')) {
        if (!parseColor(p, end)) {
            return false;
        }
    } else {
        end = start;
    }

    int n;
    if (p.parseCharacter('/')) {
        if (!p.parseInt(n) || n < MIN_STEPS || n > MAX_STEPS) {
            return false;
        }
    } else {
        n = MAX_STEPS;
    }

    // Produce result
    m_start = start;
    m_end = end;
    m_steps = n;
    return true;
}
