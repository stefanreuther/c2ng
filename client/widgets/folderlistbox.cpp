/**
  *  \file client/widgets/folderlistbox.cpp
  */

#include "client/widgets/folderlistbox.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"
#include "gfx/complex.hpp"

client::widgets::FolderListbox::FolderListbox(gfx::Point cells, ui::Root& root)
    : AbstractListbox(),
      m_items(),
      m_cells(cells),
      m_root(root),
      m_font(root.provider().getFont(gfx::FontRequest())),
      m_icons(),
      conn_imageChange(root.provider().sig_imageChange.add(this, &FolderListbox::onImageChange))
{ }

// AbstractListbox virtuals:
size_t
client::widgets::FolderListbox::getNumItems()
{
    return m_items.size();
}

bool
client::widgets::FolderListbox::isItemAccessible(size_t /*n*/)
{
    return true;
}

int
client::widgets::FolderListbox::getItemHeight(size_t /*n*/)
{
    return m_font.get() != 0
        ? m_font->getLineHeight()
        : 1;
}

int
client::widgets::FolderListbox::getHeaderHeight()
{
    return 0;
}

void
client::widgets::FolderListbox::drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::FolderListbox::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    // Request image
    onImageChange();

    const Item* pItem = getItem(item);

    gfx::Context ctx(can);
    ctx.useColorScheme(getColorScheme());
    ctx.useFont(*m_font);
    if (pItem != 0 && pItem->indent != 0) {
        drawBackground(ctx, area.splitX(m_font->getEmWidth() * pItem->indent));
    }
    ui::prepareHighContrastListItem(ctx, area, state);
    if (pItem != 0) {
        gfx::Rectangle imageArea = area.splitX(20);
        if (m_icons.get() != 0) {
            gfx::Point anchor(16 * (pItem->icon % 2), 16 * (pItem->icon / 2));
            can.blit(imageArea.getTopLeft() - anchor, *m_icons, gfx::Rectangle(anchor, gfx::Point(16, 16)));
        }
        outText(ctx, area.getTopLeft(), pItem->name);
    }
}

// Widget:
ui::layout::Info
client::widgets::FolderListbox::getLayoutInfo() const
{
    return m_font.get() != 0
        ? m_cells.scaledBy(m_font->getCellSize())
        : gfx::Point(1, 1);
}

void
client::widgets::FolderListbox::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    // FIXME: do we need to do something here?
    requestRedraw();
}

void
client::widgets::FolderListbox::swapItems(Items_t& items)
{
    m_items.swap(items);
    handleModelChange();
}

const client::widgets::FolderListbox::Item*
client::widgets::FolderListbox::getItem(size_t n)
{
    if (n < m_items.size()) {
        return &m_items[n];
    } else {
        return 0;
    }
}

void
client::widgets::FolderListbox::onImageChange()
{
    if (m_icons.get() == 0) {
        m_icons = m_root.provider().getImage("files");
        if (m_icons.get() != 0) {
            requestRedraw();
        }
    }
}
