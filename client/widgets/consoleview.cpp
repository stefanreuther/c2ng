/**
  *  \file client/widgets/consoleview.cpp
  */

#include "client/widgets/consoleview.hpp"
#include "gfx/context.hpp"
#include "afl/string/format.hpp"
#include "gfx/complex.hpp"

client::widgets::ConsoleView::ConsoleView(gfx::ResourceProvider& provider, gfx::Point sizeCells)
    : m_provider(provider),
      m_sizeCells(sizeCells),
      m_lines(),
      m_scrollback(0)
{ }

client::widgets::ConsoleView::~ConsoleView()
{ }

void
client::widgets::ConsoleView::draw(gfx::Canvas& can)
{
    // ex WConsoleView::drawContent
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());

    gfx::Rectangle area = getExtent();
    for (int i = 0; i < m_sizeCells.getY(); ++i) {
        // Fetch content
        gfx::FontRequest fontReq;
        String_t text;
        int align = 0;
        if (size_t(i) < m_lines.size() && m_lines[i] != 0) {
            fontReq.addWeight(m_lines[i]->bold);
            text = m_lines[i]->text;
            align = m_lines[i]->align;
            ctx.setColor(m_lines[i]->color);
        } else {
            ctx.setColor(util::SkinColor::Static);
        }

        // Draw
        afl::base::Ref<gfx::Font> font = m_provider.getFont(fontReq);

        // Clear line
        gfx::Rectangle lineArea = area.splitY(font->getLineHeight());
        drawBackground(ctx, lineArea);

        // Display message with correct alignment
        size_t pos;
        ctx.useFont(*font);
        if (align == 1 && (pos = text.find('\t')) != String_t::npos) {
            // Centered, and it has a tab. Center at the tab.
            ctx.setTextAlign(2, 0);
            outTextF(ctx, lineArea.splitX(lineArea.getWidth() / 2), text.substr(0, pos));
            ctx.setTextAlign(0, 0);
            outTextF(ctx, lineArea,                                 text.substr(pos+1));
        } else {
            // Normal
            ctx.setTextAlign(align, 0);
            outTextF(ctx, lineArea, text);
        }
    }

    // Show scrollback indicator
    if (m_scrollback != 0) {
        afl::base::Ref<gfx::Font> font = m_provider.getFont(gfx::FontRequest());
        ctx.useFont(*font);
        String_t text = afl::string::Format("[-%d]", m_scrollback);
        int width = font->getTextWidth(text);
        int height = font->getTextHeight(text);

        gfx::Rectangle area(getExtent().getRightX() - width, getExtent().getTopY(), width, height);
        drawSolidBar(ctx, area, util::SkinColor::Red);
        ctx.setColor(util::SkinColor::White);
        ctx.setTextAlign(0, 0);
        outTextF(ctx, area, text);
    }
}

void
client::widgets::ConsoleView::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::widgets::ConsoleView::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    requestRedraw();
}

ui::layout::Info
client::widgets::ConsoleView::getLayoutInfo() const
{
    gfx::Point result = m_sizeCells;
    afl::base::Ref<gfx::Font> p = m_provider.getFont(gfx::FontRequest());
    result = result.scaledBy(p->getCellSize());
    return result;
}

bool
client::widgets::ConsoleView::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
client::widgets::ConsoleView::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}

void
client::widgets::ConsoleView::addLine(int nr, String_t text, int align, int bold, util::SkinColor::Color color)
{
    if (nr >= 0 && nr < m_sizeCells.getY()) {
        size_t requiredSize = nr + 1;
        if (requiredSize > m_lines.size()) {
            m_lines.resize(requiredSize);
        }

        Line* p = m_lines.replaceElementNew(nr, new Line());
        p->text = text;
        p->align = align;
        p->bold = bold;
        p->color = color;
        requestRedraw();
    }
}

void
client::widgets::ConsoleView::clear()
{
    if (!m_lines.empty()) {
        m_lines.clear();
        requestRedraw();
    }
}

void
client::widgets::ConsoleView::setScrollbackIndicator(int n)
{
    if (m_scrollback != n) {
        m_scrollback = n;
        requestRedraw();
    }
}
