/**
  *  \file client/widgets/referencelistbox.cpp
  *  \brief Class client::widgets::ReferenceListbox
  */

#include "client/widgets/referencelistbox.hpp"
#include "client/marker.hpp"
#include "gfx/complex.hpp"
#include "ui/draw.hpp"

using game::ref::UserList;

client::widgets::ReferenceListbox::ReferenceListbox(ui::Root& root)
    : m_root(root),
      m_content(),
      m_numLines(5),
      m_width(root.provider().getFont(gfx::FontRequest())->getCellSize().getX() * 20)
{
    // ex WObjectList::WObjectList
}

client::widgets::ReferenceListbox::~ReferenceListbox()
{ }

void
client::widgets::ReferenceListbox::setNumLines(int n)
{
    m_numLines = n;
}

void
client::widgets::ReferenceListbox::setWidth(int width)
{
    m_width = width;
}

void
client::widgets::ReferenceListbox::setContent(const game::ref::UserList& list)
{
    // ex WObjectList::handleListChange (sort-of)
    // FIXME: optimize for common case of just one item changed (=selection toggle)?
    // Find new position
    size_t newPos = 0;
    if (const Item_t* pItem = getItem(getCurrentItem())) {
        newPos = list.find(pItem->reference).orElse(0);
    }

    // Update
    m_content = list;
    setCurrentItem(newPos);
    handleModelChange();
}

void
client::widgets::ReferenceListbox::setCurrentReference(game::Reference ref)
{
    size_t pos = 0;
    if (m_content.find(ref).get(pos)) {
        setCurrentItem(pos);
    }
}

game::Reference
client::widgets::ReferenceListbox::getCurrentReference() const
{
    if (const Item_t* p = getItem(getCurrentItem())) {
        return p->reference;
    } else {
        return game::Reference();
    }
}

size_t
client::widgets::ReferenceListbox::getNumItems() const
{
    return m_content.size();
}

bool
client::widgets::ReferenceListbox::isItemAccessible(size_t n) const
{
    // ex WObjectList::isAccessible
    bool ok = false;
    if (const Item_t* it = getItem(n)) {
        switch (it->type) {
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
client::widgets::ReferenceListbox::getItemHeight(size_t /*n*/) const
{
    return m_root.provider().getFont(gfx::FontRequest())->getCellSize().getY();
}

int
client::widgets::ReferenceListbox::getHeaderHeight() const
{
    return 0;
}

int
client::widgets::ReferenceListbox::getFooterHeight() const
{
    return 0;
}

void
client::widgets::ReferenceListbox::drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::ReferenceListbox::drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::ReferenceListbox::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    // ex WObjectList::drawPart
    // Prepare
    afl::base::Deleter d;
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    ui::prepareColorListItem(ctx, area, state, m_root.colorScheme(), d);

    // Draw
    if (const Item_t* it = getItem(item)) {
        drawItem(ctx, area, *it, m_root.provider());
    }
}

void
client::widgets::ReferenceListbox::handlePositionChange()
{
    defaultHandlePositionChange();
}

ui::layout::Info
client::widgets::ReferenceListbox::getLayoutInfo() const
{
    // FIXME: different in UIListbox::getLayoutInfo
    gfx::Point size(m_width, m_root.provider().getFont(gfx::FontRequest())->getCellSize().getY() * m_numLines);
    return ui::layout::Info(size, ui::layout::Info::GrowBoth);
}

bool
client::widgets::ReferenceListbox::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

const client::widgets::ReferenceListbox::Item_t*
client::widgets::ReferenceListbox::getItem(size_t index) const
{
    return m_content.get(index);
}

void
client::widgets::ReferenceListbox::drawItem(gfx::Context<util::SkinColor::Color>& ctx,
                                            gfx::Rectangle area,
                                            const Item_t& item,
                                            gfx::ResourceProvider& provider)
{
    ctx.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
    switch (item.type) {
     case UserList::OtherItem:
     case UserList::ReferenceItem:
        ctx.useFont(*provider.getFont(gfx::FontRequest()));
        if (item.marked) {
            ctx.setColor(util::SkinColor::Selection);
            drawSelection(ctx, area.splitX(15).getCenter(), 1, 2);
        } else {
            area.consumeX(5);
        }
        ctx.setColor(item.color);
        // FIXME: PCC2 allows to draw an info text
        outTextF(ctx, area, item.name);
        break;

     case UserList::DividerItem:
     case UserList::SubdividerItem:
        // FIXME: this is incomplete
        ctx.useFont(*provider.getFont(gfx::FontRequest().addWeight(1)));
        ctx.setColor(util::SkinColor::Faded);
        ui::drawDivider(ctx, area, item.name, item.type == UserList::DividerItem);
        break;
    }
}
