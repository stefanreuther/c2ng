/**
  *  \file client/widgets/simulationlist.cpp
  */

#include "client/widgets/simulationlist.hpp"
#include "afl/string/format.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"
#include "util/string.hpp"

namespace {
    const int X_PAD = 5;
    const int Y_PAD = 2;
}

client::widgets::SimulationList::SimulationList(ui::Root& root, afl::string::Translator& tx)
    : AbstractListbox(),
      m_root(root),
      m_translator(tx),
      m_content(),
      m_numLines(15)
{ }

client::widgets::SimulationList::~SimulationList()
{ }

void
client::widgets::SimulationList::setContent(const ListItems_t& items)
{
    m_content = items;
    handleModelChange();
}

const client::widgets::SimulationList::ListItem_t*
client::widgets::SimulationList::getItem(size_t index) const
{
    if (index < m_content.size()) {
        return &m_content[index];
    } else {
        return 0;
    }
}

size_t
client::widgets::SimulationList::getNumItems() const
{
    return m_content.size();
}

void
client::widgets::SimulationList::setPreferredHeight(int numLines)
{
    m_numLines = numLines;
}

// AbstractListbox:
size_t
client::widgets::SimulationList::getNumItems()
{
    return m_content.size();
}

bool
client::widgets::SimulationList::isItemAccessible(size_t /*n*/)
{
    return true;
}

int
client::widgets::SimulationList::getItemHeight(size_t /*n*/)
{
    return getLineHeight();
}

int
client::widgets::SimulationList::getHeaderHeight() const
{
    return 0;
}

int
client::widgets::SimulationList::getFooterHeight() const
{
    return 0;
}

void
client::widgets::SimulationList::drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::SimulationList::drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::SimulationList::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    afl::base::Deleter del;
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    ui::prepareColorListItem(ctx, area, state, m_root.colorScheme(), del);

    if (item < m_content.size()) {
        afl::base::Ref<gfx::Font> normalFont = m_root.provider().getFont(gfx::FontRequest());
        afl::base::Ref<gfx::Font> smallFont = m_root.provider().getFont(gfx::FontRequest("-"));

        const ListItem_t& e = m_content[item];
        // FIXME: own/enemy color-keying?
        ctx.setColor(e.disabled ? util::SkinColor::Faded : util::SkinColor::Static);

        area.consumeY(Y_PAD);
        area.consumeX(X_PAD);

        ctx.useFont(*normalFont);
        outTextF(ctx, area.splitY(normalFont->getLineHeight()), e.name);

        String_t subtitle = afl::string::Format("#%d", e.id);
        util::addListItem(subtitle, ", ", e.info);
        if (e.disabled) {
            util::addListItem(subtitle, ", ", m_translator("disabled"));
        }
        ctx.useFont(*smallFont);
        outTextF(ctx, area, subtitle);
    }
}

// Widget:
void
client::widgets::SimulationList::handlePositionChange(gfx::Rectangle& oldPosition)
{
    return defaultHandlePositionChange(oldPosition);
}

ui::layout::Info
client::widgets::SimulationList::getLayoutInfo() const
{
    gfx::Point size(m_root.provider().getFont(gfx::FontRequest())->getEmWidth() * 20, getLineHeight() * m_numLines);
    return ui::layout::Info(size, size, ui::layout::Info::GrowBoth);
}

bool
client::widgets::SimulationList::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

int
client::widgets::SimulationList::getLineHeight() const
{
    return m_root.provider().getFont(gfx::FontRequest())->getLineHeight()
        + m_root.provider().getFont(gfx::FontRequest("-"))->getLineHeight()
        + 2*Y_PAD;
}
