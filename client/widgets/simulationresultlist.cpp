/**
  *  \file client/widgets/simulationresultlist.cpp
  */

#include "client/widgets/simulationresultlist.hpp"
#include "afl/string/format.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"

namespace {
    afl::base::Ref<gfx::Font> getFont(ui::Root& r)
    {
        return r.provider().getFont(gfx::FontRequest());
    }
}


client::widgets::SimulationResultList::SimulationResultList(ui::Root& root)
    : m_root(root),
      m_playerNames(),
      m_playerSet(),
      m_classResults(),
      m_labelWidth(1),
      m_cellWidth(1),
      m_header(*this)
{
    // ex WSimClassResultList::WSimClassResultList
    setHeader(&m_header);
}

client::widgets::SimulationResultList::~SimulationResultList()
{ }

void
client::widgets::SimulationResultList::setPlayerNames(const game::PlayerArray<String_t>& names)
{
    m_playerNames = names;
    requestRedraw();
}

void
client::widgets::SimulationResultList::setPlayers(game::PlayerSet_t set)
{
    m_playerSet = set;
    requestRedraw();
}

void
client::widgets::SimulationResultList::setClassResults(const ClassInfos_t& list)
{
    // FIXME: possibly follow cursor and/or highlight last result
    m_classResults = list;
    requestRedraw();
    handleModelChange();
}

// AbstractListbox:
size_t
client::widgets::SimulationResultList::getNumItems() const
{
    return m_classResults.size();
}

bool
client::widgets::SimulationResultList::isItemAccessible(size_t /*n*/) const
{
    return true;
}

int
client::widgets::SimulationResultList::getItemHeight(size_t /*n*/) const
{
    return getFont(m_root)->getLineHeight();
}

void
client::widgets::SimulationResultList::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    // ex WSimClassResultList::drawPart
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    afl::base::Deleter del;
    ui::prepareColorListItem(ctx, area, state, m_root.colorScheme(), del);

    if (item < m_classResults.size()) {
        const ClassInfo_t& info = m_classResults[item];
        ctx.useFont(*getFont(m_root));
        ctx.setColor(util::SkinColor::Static);
        ctx.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
        outTextF(ctx, area.splitX(m_labelWidth), info.label);

        for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
            if (m_playerSet.contains(i)) {
                gfx::Rectangle cell = area.splitX(m_cellWidth);
                ctx.setTextAlign(gfx::RightAlign, gfx::TopAlign);
                if (int n = info.ownedUnits.get(i)) {
                    ctx.setColor(util::SkinColor::Static);
                    outTextF(ctx, cell, afl::string::Format("%d", n));
                } else {
                    ctx.setColor(util::SkinColor::Faded);
                    outTextF(ctx, cell, "-");
                }
            }
        }
    }
}

// Widget:
void
client::widgets::SimulationResultList::handlePositionChange()
{
    // ex WSimClassResultList::onResize
    afl::base::Ref<gfx::Font> font(getFont(m_root));

    // Label width
    m_labelWidth = font->getTextWidth("999x (100.0%)") + 10;

    // Cell width: divide remainder between players, but never more than 7 em per player
    const int numPlayers = std::max(1, int(m_playerSet.size()));
    const int remainder  = std::max(0, getExtent().getWidth() - m_labelWidth);
    m_cellWidth = std::min(7 * font->getEmWidth(), remainder / numPlayers);

    // Adjust list
    defaultHandlePositionChange();
}

ui::layout::Info
client::widgets::SimulationResultList::getLayoutInfo() const
{
    gfx::Point size = getFont(m_root)->getCellSize().scaledBy(40, 15);
    return ui::layout::Info(size, ui::layout::Info::GrowBoth);
}

bool
client::widgets::SimulationResultList::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

gfx::Point
client::widgets::SimulationResultList::Header::getSize() const
{
    return getFont(m_parent.m_root)->getCellSize();
}

void
client::widgets::SimulationResultList::Header::draw(gfx::Context<util::SkinColor::Color>& ctx, gfx::Rectangle area, ui::ButtonFlags_t /*flags*/) const
{
    // ex WSimClassResultList::drawContent
    ctx.useFont(*getFont(m_parent.m_root));
    ctx.setColor(util::SkinColor::Static);

    area.consumeX(m_parent.m_labelWidth);
    for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
        if (m_parent.m_playerSet.contains(i)) {
            outTextF(ctx, area.splitX(m_parent.m_cellWidth), m_parent.m_playerNames.get(i));
        }
    }
}
