/**
  *  \file client/widgets/fleetmemberlistbox.cpp
  *  \brief Class client::widgets::FleetMemberListbox
  */

#include "client/widgets/fleetmemberlistbox.hpp"
#include "gfx/context.hpp"
#include "util/unicodechars.hpp"
#include "afl/string/format.hpp"
#include "ui/draw.hpp"
#include "client/marker.hpp"

using game::ref::FleetMemberList;
using game::Reference;
using game::ref::UserList;

client::widgets::FleetMemberListbox::FleetMemberListbox(ui::Root& root, int prefLines, int prefWidth)
    : AbstractListbox(),
      m_root(root),
      m_content(),
      m_preferredNumLines(prefLines),
      m_preferredWidth(prefWidth)
{ }

client::widgets::FleetMemberListbox::~FleetMemberListbox()
{ }

void
client::widgets::FleetMemberListbox::setContent(const game::ref::FleetMemberList& content)
{
    // ex WFleetMemberList::setFleet (sort-of)
    if (m_content != content) {
        m_content = content;
        handleModelChange();
    }
}

void
client::widgets::FleetMemberListbox::setCurrentFleetMember(game::Id_t shipId)
{
    size_t pos;
    if (m_content.find(Reference(Reference::Ship, shipId), pos)) {
        setCurrentItem(pos);
    }
}

game::Id_t
client::widgets::FleetMemberListbox::getCurrentFleetMember() const
{
    // ex WFleetMemberList::getCurrentMemberId()
    const FleetMemberList::Item* p = m_content.get(getCurrentItem());
    if (p != 0 && p->type == UserList::ReferenceItem && p->reference.getType() == Reference::Ship) {
        return p->reference.getId();
    } else {
        return 0;
    }
}

size_t
client::widgets::FleetMemberListbox::getNumItems() const
{
    return m_content.size();
}

bool
client::widgets::FleetMemberListbox::isItemAccessible(size_t n) const
{
    bool ok = false;
    if (const FleetMemberList::Item* p = m_content.get(n)) {
        switch (p->type) {
         case UserList::OtherItem:
         case UserList::ReferenceItem:
            ok = true;
            break;

         case UserList::DividerItem:
         case UserList::SubdividerItem:
            ok = false;
            break;
        }
    }
    return ok;
}

int
client::widgets::FleetMemberListbox::getItemHeight(size_t /*n*/) const
{
    return getFont()->getLineHeight();
}

int
client::widgets::FleetMemberListbox::getHeaderHeight() const
{
    return 0;
}

int
client::widgets::FleetMemberListbox::getFooterHeight() const
{
    return 0;
}

void
client::widgets::FleetMemberListbox::drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::FleetMemberListbox::drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::FleetMemberListbox::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    // ex WFleetMemberList::drawPart(GfxCanvas& can, int from, int to)
    afl::base::Deleter del;
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    prepareColorListItem(ctx, area, state, m_root.colorScheme(), del);

    afl::base::Ref<gfx::Font> font = getFont();
    ctx.useFont(*font);

    if (const FleetMemberList::Item* p = m_content.get(item)) {
        switch (p->type) {
         case UserList::OtherItem:
         case UserList::ReferenceItem: {
            // Show a member
            // Decide icon: bullet for leader, 't' for tower, dot for away, dim color for towee
            const char* icon;
            if (p->flags.contains(FleetMemberList::Leader)) {
                icon = UTF_SQUARE_BULLET;
            } else if (p->flags.contains(FleetMemberList::Towing)) {
                icon = "t";
            } else if (p->flags.contains(FleetMemberList::Away)) {
                icon = UTF_MIDDLE_DOT;
            } else {
                icon = "";
            }
            if (p->flags.contains(FleetMemberList::Towed)) {
                ctx.setColor(util::SkinColor::Blue);
            }

            // Allocate space
            gfx::Rectangle iconArea = area.splitX(font->getEmWidth());
            gfx::Rectangle fcodeArea = area.splitRightX(3 * font->getEmWidth());

            // Draw it
            ctx.setTextAlign(gfx::CenterAlign, gfx::TopAlign);
            outTextF(ctx, iconArea, icon);
            ctx.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
            outTextF(ctx, area, afl::string::Format("%d: %s", p->reference.getId(), p->name));
            outTextF(ctx, fcodeArea, p->friendlyCode);

            // Selection icon
            if (p->marked) {
                ctx.setColor(util::SkinColor::Selection);
                drawSelection(ctx, iconArea.getCenter(), 1, 2);
            }
            break;
         }

         case UserList::DividerItem:
         case UserList::SubdividerItem:
            ctx.useFont(*m_root.provider().getFont(gfx::FontRequest().addWeight(1)));
            ctx.setColor(util::SkinColor::Faded);
            ui::drawDivider(ctx, area, p->name, p->type == UserList::DividerItem);
            break;
        }
    }
}

void
client::widgets::FleetMemberListbox::handlePositionChange()
{
    defaultHandlePositionChange();
}

ui::layout::Info
client::widgets::FleetMemberListbox::getLayoutInfo() const
{
    int lineHeight = getFont()->getLineHeight();
    return ui::layout::Info(gfx::Point(m_preferredWidth, 3*lineHeight),
                            gfx::Point(m_preferredWidth, m_preferredNumLines*lineHeight),
                            ui::layout::Info::GrowBoth);
}

bool
client::widgets::FleetMemberListbox::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

afl::base::Ref<gfx::Font>
client::widgets::FleetMemberListbox::getFont() const
{
    return m_root.provider().getFont(gfx::FontRequest());
}
