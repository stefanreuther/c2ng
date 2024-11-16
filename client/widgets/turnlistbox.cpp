/**
  *  \file client/widgets/turnlistbox.cpp
  */

#include "client/widgets/turnlistbox.hpp"
#include "afl/base/optional.hpp"
#include "afl/string/format.hpp"
#include "gfx/complex.hpp"
#include "ui/draw.hpp"

namespace {
    const int OUTLINE_SIZE = 3;
}

client::widgets::TurnListbox::TurnListbox(gfx::Point cells, ui::Root& root, afl::string::Translator& tx)
    : AbstractListbox(),
      m_items(),
      m_cells(cells),
      m_root(root),
      m_translator(tx),
      m_bigFont(root.provider().getFont(gfx::FontRequest().addSize(+1))),
      m_smallFont(root.provider().getFont(gfx::FontRequest().addSize(-1))),
      m_activeTurnNumber()
{ }

// AbstractListbox virtuals:
size_t
client::widgets::TurnListbox::getNumItems() const
{
    return m_items.size();
}

bool
client::widgets::TurnListbox::isItemAccessible(size_t /*n*/) const
{
    return true;
}

int
client::widgets::TurnListbox::getItemHeight(size_t /*n*/) const
{
    return m_bigFont->getLineHeight()
        + m_smallFont->getLineHeight()
        + 2*OUTLINE_SIZE;
}

int
client::widgets::TurnListbox::getHeaderHeight() const
{
    return 0;
}

int
client::widgets::TurnListbox::getFooterHeight() const
{
    return 0;
}

void
client::widgets::TurnListbox::drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::TurnListbox::drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::TurnListbox::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    using ui::SkinColor;
    afl::base::Deleter deleter;

    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    prepareColorListItem(ctx, area, state, m_root.colorScheme(), deleter);

    const Item* pItem = getItem(item);
    if (pItem != 0) {
        SkinColor::Color textColor  = SkinColor::Static;
        SkinColor::Color subColor   = SkinColor::Faded;

        afl::base::Optional<uint8_t> boxColor;
        afl::base::Optional<uint8_t> stateColor;
        String_t stateText;
        String_t turnText = afl::string::Format(m_translator("Turn %d").c_str(), pItem->turnNumber);
        switch (pItem->status) {
         case Unknown:
            boxColor = ui::Color_Grayscale + 2;
            break;
         case Unavailable:
            textColor = SkinColor::Faded;
            boxColor = ui::Color_Grayscale + 2;
            stateColor = ui::Color_White;
            stateText = m_translator("not available");
            break;
         case StronglyAvailable:
            boxColor = ui::Color_GreenScale + 4;
            stateColor = ui::Color_GreenScale + 15;
            stateText = m_translator("available");
            break;
         case WeaklyAvailable:
            boxColor = ui::Color_GreenScale + 4;
            stateColor = ui::Color_GreenScale + 15;
            stateText = m_translator("available?");
            break;
         case Loaded:
            boxColor = ui::Color_GreenScale + 6;
            stateColor = ui::Color_GreenScale + 15;
            stateText = m_translator("loaded");
            break;
         case Failed:
            textColor = SkinColor::Faded;
            boxColor = ui::Color_Fire + 2;
            stateColor = ui::Color_Fire + 20;
            stateText = m_translator("error");
            break;
         case Current:
            boxColor = ui::Color_GreenScale + 6;
            stateColor = ui::Color_GreenScale + 15;
            stateText = m_translator("loaded");
            turnText = m_translator("Current");
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

        uint8_t c = 0;
        if (boxColor.get(c)) {
            gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
            drawSolidBar(ctx, area, c);
            ctx.setTextAlign(gfx::CenterAlign, gfx::MiddleAlign);

            if (stateColor.get(c)) {
                ctx.useFont(*m_smallFont);
                ctx.setColor(c);
                outTextF(ctx, area, stateText);
            }

            if (pItem->turnNumber == m_activeTurnNumber) {
                drawSolidBar(ctx, area.splitRightX(5), ui::Color_GreenScale + 15);
            }
        }
    }
}

// Widget:
ui::layout::Info
client::widgets::TurnListbox::getLayoutInfo() const
{
    return m_cells.scaledBy(m_bigFont->getCellSize());
}

bool
client::widgets::TurnListbox::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix)
        || ((key == util::Key_Up + util::KeyMod_Alt || key == util::Key_Down + util::KeyMod_Alt)
            && defaultHandleKey(key & ~util::KeyMod_Alt, prefix));
}

void
client::widgets::TurnListbox::handlePositionChange()
{
    defaultHandlePositionChange();
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

afl::base::Optional<size_t>
client::widgets::TurnListbox::findTurn(int turnNumber)
{
    // Guess in O(1) assuming regular content
    if (!m_items.empty()) {
        size_t guess = turnNumber - m_items[0].turnNumber;
        if (guess < m_items.size()) {
            return guess;
        }
    }

    // Irregular case, O(n)
    for (size_t i = 0, n = m_items.size(); i < n; ++i) {
        if (m_items[i].turnNumber == turnNumber) {
            return i;
        }
    }
    return afl::base::Nothing;
}

void
client::widgets::TurnListbox::setItem(const Item& content)
{
    afl::base::Optional<size_t> index = findTurn(content.turnNumber);
    if (const size_t* p = index.get()) {
        setItem(*p, content);
    }
}

void
client::widgets::TurnListbox::setCurrentTurnNumber(int turnNumber)
{
    afl::base::Optional<size_t> index = findTurn(turnNumber);
    if (const size_t* p = index.get()) {
        const size_t index = *p;
        setCurrentItem(index);
    } else if (!m_items.empty()) {
        // Try to adjust when going across boundaries
        if (turnNumber <= m_items.front().turnNumber) {
            // "Previous" invoked from turn 1
            setCurrentItem(0);
        } else if (turnNumber >= m_items.back().turnNumber) {
            // "Next" invoked from last turn
            setCurrentItem(m_items.size()-1);
        } else {
            // Cannot solve.
        }
    }
}

void
client::widgets::TurnListbox::setActiveTurnNumber(int turnNumber)
{
    if (m_activeTurnNumber != turnNumber) {
        m_activeTurnNumber = turnNumber;
        requestRedraw();
    }
}
