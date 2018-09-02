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
        list.find(pItem->reference, newPos);
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
    if (m_content.find(ref, pos)) {
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
client::widgets::ReferenceListbox::getNumItems()
{
    return m_content.size();
}

bool
client::widgets::ReferenceListbox::isItemAccessible(size_t n)
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
client::widgets::ReferenceListbox::getItemHeight(size_t /*n*/)
{
    return m_root.provider().getFont(gfx::FontRequest())->getCellSize().getY();
}

int
client::widgets::ReferenceListbox::getHeaderHeight()
{
    return 0;
}

void
client::widgets::ReferenceListbox::drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
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
        switch (it->type) {
         case UserList::OtherItem:
         case UserList::ReferenceItem:
            ctx.useFont(*m_root.provider().getFont(gfx::FontRequest()));
            if (it->marked) {
                ctx.setColor(util::SkinColor::Selection);
                drawSelection(ctx, area.splitX(15).getCenter(), 1, 2);
            } else {
                area.consumeX(5);
            }
            ctx.setColor(it->color);
            ctx.setTextAlign(0, 0);
            // FIXME: PCC2 allows to draw an info text
            outTextF(ctx, area, it->name);
            break;

         case UserList::DividerItem:
         case UserList::SubdividerItem: {
            // FIXME: this is incomplete
            ctx.useFont(*m_root.provider().getFont(gfx::FontRequest().addWeight(1)));
            ctx.setColor(util::SkinColor::Faded);

            int y = area.getTopY() + ctx.getFont()->getCellSize().getY() / 2 - 1;
            int max = std::max(0, std::min(ctx.getFont()->getTextWidth(it->name), area.getWidth() - 30));

            drawHLine(ctx, area.getLeftX() + 2,        y, area.getLeftX() + 28);
            drawHLine(ctx, area.getLeftX() + 32 + max, y, area.getLeftX() + area.getWidth() - 2);
            if (it->type == UserList::DividerItem) {
                drawHLine(ctx, area.getLeftX() + 2,        y - 2, area.getLeftX() + 28);
                drawHLine(ctx, area.getLeftX() + 32 + max, y - 2, area.getLeftX() + area.getWidth() - 2);
                drawHLine(ctx, area.getLeftX() + 2,        y + 2, area.getLeftX() + 28);
                drawHLine(ctx, area.getLeftX() + 32 + max, y + 2, area.getLeftX() + area.getWidth() - 2);
            }

            outTextF(ctx, gfx::Point(area.getLeftX() + 30, area.getTopY()), max, it->name);
            break;
         }
        }
    }
}

void
client::widgets::ReferenceListbox::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    handleModelChange();
}

ui::layout::Info
client::widgets::ReferenceListbox::getLayoutInfo() const
{
    // FIXME: different in UIListbox::getLayoutInfo
    gfx::Point size(m_width, m_root.provider().getFont(gfx::FontRequest())->getCellSize().getY() * m_numLines);
    return ui::layout::Info(size, size, ui::layout::Info::GrowBoth);
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
