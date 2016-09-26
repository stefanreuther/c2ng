/**
  *  \file client/widgets/turnlistbox.cpp
  */

#include "client/widgets/turnlistbox.hpp"
#include "ui/draw.hpp"
#include "util/translation.hpp"
#include "afl/string/format.hpp"
#include "gfx/complex.hpp"

namespace {
    const int OUTLINE_SIZE = 3;
}

client::widgets::TurnListbox::TurnListbox(gfx::Point cells, ui::Root& root)
    : AbstractListbox(),
      m_items(),
      m_cells(cells),
      m_root(root),
      m_bigFont(root.provider().getFont(gfx::FontRequest().addSize(+1))),
      m_smallFont(root.provider().getFont(gfx::FontRequest().addSize(-1)))
{ }

// AbstractListbox virtuals:
size_t
client::widgets::TurnListbox::getNumItems()
{
    return m_items.size();
}

bool
client::widgets::TurnListbox::isItemAccessible(size_t /*n*/)
{
    return true;
}

int
client::widgets::TurnListbox::getItemHeight(size_t /*n*/)
{
    if (m_bigFont.get() != 0 && m_smallFont.get() != 0) {
        return m_bigFont->getLineHeight()
            + m_smallFont->getLineHeight()
            + 2*OUTLINE_SIZE;
    } else {
        return 1;
    }
}

int
client::widgets::TurnListbox::getHeaderHeight()
{
    return 0;
}

void
client::widgets::TurnListbox::drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::TurnListbox::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    using ui::SkinColor;
    afl::base::Deleter deleter;

    gfx::Context ctx(can);
    prepareColorListItem(ctx, area, state, m_root.colorScheme(), deleter);

    const Item* pItem = getItem(item);
    if (pItem != 0 && m_bigFont.get() != 0 && m_smallFont.get() != 0) {
        SkinColor::Color textColor  = SkinColor::Static;
        SkinColor::Color subColor   = SkinColor::Faded;

        int boxColor = -1;
        int stateColor = -1;
        String_t stateText;
        String_t turnText = afl::string::Format(_("Turn %d").c_str(), pItem->turnNumber);
        switch (pItem->status) {
         case Unknown:
            boxColor = ui::Color_Grayscale + 2;
            break;
         case Unavailable:
            textColor = SkinColor::Faded;
            boxColor = ui::Color_Grayscale + 2;
            stateColor = ui::Color_White;
            stateText = _("not available");
            break;
         case StronglyAvailable:
            boxColor = ui::Color_GreenScale + 4;
            stateColor = ui::Color_GreenScale + 15;
            stateText = _("available");
            break;
         case WeaklyAvailable:
            boxColor = ui::Color_GreenScale + 4;
            stateColor = ui::Color_GreenScale + 15;
            stateText = _("available?");
            break;
         case Loaded:
            boxColor = ui::Color_GreenScale + 6;
            stateColor = ui::Color_GreenScale + 15;
            stateText = _("loaded");
            break;
         case Failed:
            textColor = SkinColor::Faded;
            boxColor = ui::Color_Fire + 2;
            stateColor = ui::Color_Fire + 20;
            stateText = _("error");
            break;
         case Current:
            boxColor = ui::Color_GreenScale + 6;
            stateColor = ui::Color_GreenScale + 15;
            stateText = _("loaded");
            turnText = _("Current");
            break;
         case Active:
            boxColor = ui::Color_GreenScale + 6;
            stateColor = ui::Color_GreenScale + 15;
            stateText = _("active");
            break;
        }

        area.grow(-OUTLINE_SIZE, -OUTLINE_SIZE);

        gfx::Rectangle textArea = area.splitX(m_bigFont->getEmWidth() * 6);
        ctx.useFont(*m_bigFont);
        ctx.setColor(textColor);
        outTextF(ctx, textArea.splitY(m_bigFont->getLineHeight()), turnText);
        ctx.useFont(*m_smallFont);
        ctx.setColor(subColor);
        outTextF(ctx, textArea, pItem->time);

        if (boxColor >= 0) {
            ctx.useColorScheme(m_root.colorScheme());
            drawSolidBar(ctx, area, boxColor);
            ctx.setTextAlign(1, 1);
            if (stateColor >= 0) {
                ctx.setColor(stateColor);
                outTextF(ctx, area, stateText);
            }
        }
    }
}

// Widget:
ui::layout::Info
client::widgets::TurnListbox::getLayoutInfo() const
{
    return m_bigFont.get() != 0
        ? m_cells.scaledBy(m_bigFont->getCellSize())
        : gfx::Point(1, 1);
}

void
client::widgets::TurnListbox::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    // FIXME: do we need to do something here?
    requestRedraw();
}

void
client::widgets::TurnListbox::swapItems(Items_t& items)
{
    m_items.swap(items);
    handleModelChange();
}

void
client::widgets::TurnListbox::setItem(size_t index, const Item& content)
{
    if (index < m_items.size()) {
        m_items[index] = content;
        updateItem(index);
    }
}

const client::widgets::TurnListbox::Item*
client::widgets::TurnListbox::getItem(size_t n)
{
    if (n < m_items.size()) {
        return &m_items[n];
    } else {
        return 0;
    }
}
