/**
  *  \file client/widgets/historypositionlistbox.cpp
  *  \brief Class client::widgets::HistoryPositionListbox
  */

#include "client/widgets/historypositionlistbox.hpp"
#include "gfx/context.hpp"
#include "afl/base/deleter.hpp"
#include "ui/draw.hpp"
#include "afl/string/format.hpp"
#include "client/tiles/historyadaptor.hpp"

/*
 *  We want to display the content, bottom-aligned, in reverse order (current turn at bottom).
 *  Therefore,
 *  - item #0 always is a spacer.
 *    If the list has fewer items than fit in the allocated size, it consumes all the remaining space.
 *    Otherwise, it has size 0.
 *  - items #1..#x are the content items, in reverse order.
 *
 *  Try to minimize the knowledge about the item-index/array-index mapping:
 *  as of 20220612, only getItemHeight() and getItem() know about that mapping;
 *  everything else is interpreted in terms of those.
 */

client::widgets::HistoryPositionListbox::HistoryPositionListbox(ui::Root& root, afl::string::Translator& tx)
    : m_root(root),
      m_translator(tx),
      m_content(),
      m_numLines(5),
      m_width(100)
{ }

client::widgets::HistoryPositionListbox::~HistoryPositionListbox()
{ }

void
client::widgets::HistoryPositionListbox::setNumLines(int n)
{
    m_numLines = n;
}

void
client::widgets::HistoryPositionListbox::setWidth(int width)
{
    m_width = width;
}

void
client::widgets::HistoryPositionListbox::setContent(const Infos_t& content)
{
    m_content = content;
    handleModelChange();
}

void
client::widgets::HistoryPositionListbox::setCurrentTurnNumber(int turnNumber)
{
    // ex WHistoryShipPositionList::setTurn
    for (size_t i = 0, n = getNumItems(); i < n; ++i) {
        if (const Info_t* p = getItem(i)) {
            if (p->turnNumber == turnNumber) {
                setCurrentItem(i);
                break;
            }
        }
    }
}

int
client::widgets::HistoryPositionListbox::getCurrentTurnNumber() const
{
    // ex WHistoryShipPositionList::getTurn
    if (const Info_t* p = getItem(getCurrentItem())) {
        return p->turnNumber;
    } else {
        return 0;
    }
}

// AbstractListbox / Widget:
size_t
client::widgets::HistoryPositionListbox::getNumItems() const
{
    return m_content.size() + 1;
}

bool
client::widgets::HistoryPositionListbox::isItemAccessible(size_t n) const
{
    // ex WHistoryShipPositionList::isAccessible, CHistoryTurnList.Accessible
    const Info_t* p = getItem(n);
    return p != 0 && p->position.isValid();
}

int
client::widgets::HistoryPositionListbox::getItemHeight(size_t n) const
{
    const int lineHeight = m_root.provider().getFont(gfx::FontRequest())->getCellSize().getY();
    if (n == 0) {
        const int contentHeight = int(m_content.size()) * lineHeight;
        const int widgetHeight = getExtent().getHeight();
        return std::max(0, widgetHeight - contentHeight);
    } else {
        return lineHeight;
    }
}

int
client::widgets::HistoryPositionListbox::getHeaderHeight() const
{
    return 0;
}

int
client::widgets::HistoryPositionListbox::getFooterHeight() const
{
    return 0;
}

void
client::widgets::HistoryPositionListbox::drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::HistoryPositionListbox::drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::HistoryPositionListbox::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    // ex WHistoryShipPositionList::drawPart, CHistoryTurnList.DrawPart
    // Prepare
    afl::base::Deleter d;
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    ui::prepareColorListItem(ctx, area, state, m_root.colorScheme(), d);

    // Draw
    if (const Info_t* p = getItem(item)) {
        // Allocate space: 3 em for turn, 5 em for mass, remainder for name
        afl::base::Ref<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest());
        ctx.useFont(*font);
        const int emWidth = font->getEmWidth();
        const int turnWidth = 3*emWidth;
        const int massWidth = 5*emWidth;
        if (!p->position.isValid()) {
            ctx.setColor(util::SkinColor::Faded);
        }

        // Turn number
        ctx.setTextAlign(gfx::RightAlign, gfx::TopAlign);
        outTextF(ctx, area.splitX(turnWidth), afl::string::Format("%d: ", p->turnNumber));

        // Mass
        gfx::Rectangle massArea = area.splitRightX(massWidth);
        int mass;
        if (p->mass.get(mass)) {
            ctx.setTextAlign(gfx::RightAlign, gfx::TopAlign);
            // FIXME: formatNumber
            massArea.consumeRightX(5);
            outTextF(ctx, massArea, afl::string::Format(m_translator("%d kt"), /*numToString*/(mass)));
        }

        // Position
        ctx.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
        outTextF(ctx, area, p->positionName.empty() ? m_translator("unknown") : p->positionName);
    }
}

void
client::widgets::HistoryPositionListbox::handlePositionChange(gfx::Rectangle& oldPosition)
{
    defaultHandlePositionChange(oldPosition);
}

ui::layout::Info
client::widgets::HistoryPositionListbox::getLayoutInfo() const
{
    gfx::Point size(m_width, m_root.provider().getFont(gfx::FontRequest())->getCellSize().getY() * m_numLines);
    return ui::layout::Info(size, size, ui::layout::Info::GrowBoth);
}

bool
client::widgets::HistoryPositionListbox::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

const client::widgets::HistoryPositionListbox::Info_t*
client::widgets::HistoryPositionListbox::getItem(size_t index) const
{
    size_t effIndex = m_content.size() - index;
    if (effIndex < m_content.size()) {
        return &m_content[effIndex];
    } else {
        return 0;
    }
}
