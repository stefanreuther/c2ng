/**
  *  \file gfx/basecontext.cpp
  *  \brief Class gfx::BaseContext
  */

#include "gfx/basecontext.hpp"

// Constructor.
gfx::BaseContext::BaseContext(Canvas& canvas)
    : m_rawColor(0),
      m_lineThickness(1),
      m_linePattern(SOLID_LINE),
      m_transparentBackground(true),
      m_fillPattern(FillPattern::SOLID),
      m_alpha(OPAQUE_ALPHA),
      m_cursor(),
      m_textAlign(),
      m_font(0),
      m_canvas(&canvas)
{
    // ex GfxContext::GfxContext
}
