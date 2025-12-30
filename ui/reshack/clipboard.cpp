/**
  *  \file ui/reshack/clipboard.cpp
  *  \brief Class ui::reshack::Clipboard
  */

#include "ui/reshack/clipboard.hpp"
#include "gfx/complex.hpp"
#include "ui/reshack/palette.hpp"
#include "ui/reshack/rectangletool.hpp"

/*
 *  CopyTool
 */

ui::reshack::Clipboard::CopyTool::CopyTool(Clipboard& parent, afl::string::Translator& tx)
    : Tool(true, tx("Copy")),
      m_parent(parent),
      m_prev(),
      m_backgroundColor()
{ }

void
ui::reshack::Clipboard::CopyTool::click(gfx::BaseContext& c, gfx::Point pt, gfx::Color_t bg)
{
    // RHCopyTool::click(GfxContext& c, GfxPoint pt, uint32_t bg)
    m_prev = pt;
    drawPixel(c, pt);
    m_backgroundColor = bg;
}

void
ui::reshack::Clipboard::CopyTool::drag(gfx::BaseContext& c, gfx::Point pt)
{
    drawRectangle(c, RectangleTool::makeRectangle(pt, m_prev));
}

void
ui::reshack::Clipboard::CopyTool::release(gfx::BaseContext& c, gfx::Point pt)
{
    gfx::Rectangle area = RectangleTool::makeRectangle(pt, m_prev);
    area.intersect(gfx::Rectangle(gfx::Point(), c.canvas().getSize()));
    if (!area.exists()) {
        return;
    }

    afl::base::Ref<gfx::PalettizedPixmap> clipboard = gfx::PalettizedPixmap::create(area.getWidth(), area.getHeight());
    Palette::copyPalette(*clipboard, c.canvas());

    afl::base::GrowableMemory<gfx::Color_t> colors;
    colors.resize(area.getWidth());

    afl::base::Bytes_t bytes(clipboard->pixels());
    for (int y = 0; y < area.getHeight(); ++y) {
        c.canvas().getPixels(gfx::Point(area.getLeftX(), area.getTopY() + y), colors);
        for (int x = 0; x < area.getWidth(); ++x) {
            if (uint8_t* p = bytes.eat()) {
                *p = static_cast<uint8_t>(*colors.at(x));
            }
        }
    }
    m_parent.set(clipboard.asPtr(), m_backgroundColor);
}

bool
ui::reshack::Clipboard::CopyTool::isUsable()
{
    return true;
}

/*
 *  PasteTool
 */

ui::reshack::Clipboard::PasteTool::PasteTool(Clipboard& parent, bool single, afl::string::Translator& tx)
    : Tool(single, single ? tx("Paste") : tx("Stamp")),
      m_parent(parent)
{ }

void
ui::reshack::Clipboard::PasteTool::click(gfx::BaseContext& c, gfx::Point pt, gfx::Color_t /*bg*/)
{
    afl::base::Ptr<gfx::PalettizedPixmap> clipboard = m_parent.getPixmap();
    if (clipboard.get() != 0) {
        gfx::Color_t colorKey = m_parent.getColorKey();
        afl::base::Bytes_t bytes(clipboard->pixels());
        gfx::Point size = clipboard->getSize();
        for (int y = 0; y < size.getY(); ++y) {
            for (int x = 0; x < size.getX(); ++x) {
                if (const uint8_t* p = bytes.eat()) {
                    gfx::Color_t color = *p;
                    if (color != colorKey) {
                        c.canvas().drawPixels(gfx::Point(pt.getX() + x, pt.getY() + y), afl::base::Memory<const gfx::Color_t>::fromSingleObject(color), gfx::OPAQUE_ALPHA);
                    }
                }
            }
        }
    }
}

void
ui::reshack::Clipboard::PasteTool::drag(gfx::BaseContext& c, gfx::Point pt)
{
    PasteTool::click(c, pt, 0);
}

void
ui::reshack::Clipboard::PasteTool::release(gfx::BaseContext& c, gfx::Point pt)
{
    PasteTool::click(c, pt, 0);
}

bool
ui::reshack::Clipboard::PasteTool::isUsable()
{
    return m_parent.hasContent();
}

/*
 *  Clipboard
 */

ui::reshack::Clipboard::Clipboard()
    : m_content(), m_colorKey()
{ }

bool
ui::reshack::Clipboard::hasContent() const
{
    return m_content.get() != 0;
}

afl::base::Ptr<gfx::PalettizedPixmap>
ui::reshack::Clipboard::getPixmap() const
{
    // ex getClipboardImage()
    return m_content;
}

gfx::Color_t
ui::reshack::Clipboard::getColorKey() const
{
    return m_colorKey;
}

void
ui::reshack::Clipboard::set(afl::base::Ptr<gfx::PalettizedPixmap> content, gfx::Color_t colorKey)
{
    m_content = content;
    m_colorKey = colorKey;
    sig_change.raise();
}
