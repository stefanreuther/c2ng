/**
  *  \file client/widgets/componentlist.cpp
  *  \brief Class client::widgets::ComponentList
  */

#include "client/widgets/componentlist.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"
#include "util/unicodechars.hpp"

client::widgets::ComponentList::ComponentList(ui::Root& root, int numLines, int widthInEms)
    : AbstractListbox(),
      m_root(root),
      m_numLines(numLines),
      m_widthInEms(widthInEms),
      m_content()
{
    // ex WHullListbox, WEngineListbox, WBeamListbox, WLauncherListbox
}

client::widgets::ComponentList::~ComponentList()
{ }

void
client::widgets::ComponentList::setContent(const Parts_t& parts)
{
    // ex WHullListbox::swapContents
    int id = getCurrentId();
    m_content = parts;
    handleModelChange();
    setCurrentId(id);
}

void
client::widgets::ComponentList::setCurrentId(int id)
{
    // ex WHullListbox::scrollToHull
    if (id != getCurrentId()) {
        for (size_t i = 0, n = m_content.size(); i < n; ++i) {
            if (m_content[i].id == id) {
                setCurrentItem(i);
                break;
            }
        }
    }
}

int
client::widgets::ComponentList::getCurrentId() const
{
    // ex WHullListbox::getCurrentHull
    size_t pos = getCurrentItem();
    if (pos < m_content.size()) {
        return m_content[pos].id;
    } else {
        return 0;
    }
}

int
client::widgets::ComponentList::getCurrentAmount() const
{
    size_t pos = getCurrentItem();
    if (pos < m_content.size()) {
        return m_content[pos].numParts;
    } else {
        return 0;
    }
}

// AbstractListbox virtuals:
size_t
client::widgets::ComponentList::getNumItems() const
{
    return m_content.size();
}

bool
client::widgets::ComponentList::isItemAccessible(size_t n) const
{
    return n < m_content.size()
        && m_content[n].isAccessible;
}

int
client::widgets::ComponentList::getItemHeight(size_t /*n*/) const
{
    return getFont()->getLineHeight();
}

int
client::widgets::ComponentList::getHeaderHeight() const
{
    return 0;
}

int
client::widgets::ComponentList::getFooterHeight() const
{
    return 0;
}

void
client::widgets::ComponentList::drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::ComponentList::drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::ComponentList::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    // ex WComponentListbox::drawComponent
    if (item < m_content.size()) {
        // Color scheme
        afl::base::Deleter del;
        gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
        ui::prepareColorListItem(ctx, area, state, m_root.colorScheme(), del);
        ctx.useFont(*getFont());

        // Content
        const Part& part = m_content[item];
        if (!part.isAccessible) {
            ctx.setColor(util::SkinColor::Faded);
        } else {
            switch (part.techStatus) {
             case game::AvailableTech:
                ctx.setColor(util::SkinColor::Static);
                break;
             case game::BuyableTech:
                ctx.setColor(util::SkinColor::Blue);
                break;
             case game::LockedTech:
                ctx.setColor(util::SkinColor::Faded);
                break;
            }
        }

        const char* tick = part.numParts != 0 ? UTF_BULLET : " ";
        outTextF(ctx, area, tick + part.name);
    }
}

// Widget virtuals:
void
client::widgets::ComponentList::handlePositionChange()
{
    return defaultHandlePositionChange();
}

ui::layout::Info
client::widgets::ComponentList::getLayoutInfo() const
{
    gfx::Point size = getFont()->getCellSize().scaledBy(m_widthInEms, m_numLines);
    return ui::layout::Info(size, ui::layout::Info::GrowBoth);
}

bool
client::widgets::ComponentList::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

afl::base::Ref<gfx::Font>
client::widgets::ComponentList::getFont() const
{
    return m_root.provider().getFont(gfx::FontRequest());
}
