/**
  *  \file client/widgets/friendlycodelist.cpp
  */

#include "client/widgets/friendlycodelist.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"

client::widgets::FriendlyCodeList::FriendlyCodeList(ui::Root& root, const game::data::FriendlyCodeList_t& list)
    : AbstractListbox(),
      m_root(root),
      m_list(list)
{ }

client::widgets::FriendlyCodeList::~FriendlyCodeList()
{ }

void
client::widgets::FriendlyCodeList::setFriendlyCode(const String_t& code)
{
    // ex WFCodeListbox::setFCode
    // Query current code first. If it already matches, don't change anything.
    // If users define multiple identical codes, they wouldn't be able to scroll across the duplicates,
    // without this special handling, because scrolling would always reset it to the first one.
    if (code != getFriendlyCode()) {
        for (size_t i = 0, n = m_list.size(); i < n; ++i) {
            if (m_list[i].getCode() == code) {
                setCurrentItem(i);
                break;
            }
        }
    }
}

String_t
client::widgets::FriendlyCodeList::getFriendlyCode() const
{
    // ex WFCodeListbox::getFCode
    size_t i = getCurrentItem();
    if (i < m_list.size()) {
        return m_list[i].getCode();
    } else {
        return String_t();
    }
}

// AbstractListbox virtuals:
size_t
client::widgets::FriendlyCodeList::getNumItems()
{
    return m_list.size();
}

bool
client::widgets::FriendlyCodeList::isItemAccessible(size_t /*n*/)
{
    return true;
}

int
client::widgets::FriendlyCodeList::getItemHeight(size_t /*n*/)
{
    return m_root.provider().getFont(gfx::FontRequest())->getLineHeight();
}

int
client::widgets::FriendlyCodeList::getHeaderHeight()
{
    return 0;
}

void
client::widgets::FriendlyCodeList::drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::FriendlyCodeList::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    // ex WFCodeListbox::drawPart
    afl::base::Deleter del;
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    ctx.useFont(*m_root.provider().getFont(gfx::FontRequest()));

    ui::prepareColorListItem(ctx, area, state, m_root.colorScheme(), del);
    if (item < m_list.size()) {
        area.consumeX(5);
        outTextF(ctx, area.splitX(ctx.getFont()->getEmWidth() * 3), m_list[item].getCode());
        outTextF(ctx, area, m_list[item].getDescription());
    }
}

// Widget virtuals:
void
client::widgets::FriendlyCodeList::handlePositionChange(gfx::Rectangle& oldPosition)
{
    defaultHandlePositionChange(oldPosition);
}

ui::layout::Info
client::widgets::FriendlyCodeList::getLayoutInfo() const
{
    return m_root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(30, 10);
}

bool
client::widgets::FriendlyCodeList::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}
