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
client::widgets::FolderListbox::getNumItems() const
{
    return m_items.size();
}

bool
client::widgets::FolderListbox::isItemAccessible(size_t /*n*/) const
{
    return true;
}

int
client::widgets::FolderListbox::getItemHeight(size_t /*n*/) const
{
    return m_font->getLineHeight();
}

int
client::widgets::FolderListbox::getHeaderHeight() const
{
    return 0;
}

int
client::widgets::FolderListbox::getFooterHeight() const
{
    return 0;
}

void
client::widgets::FolderListbox::drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::FolderListbox::drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::FolderListbox::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    // Request image
    onImageChange();

    const Item* pItem = getItem(item);

    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    ctx.useFont(*m_font);
    if (pItem != 0 && pItem->indent != 0) {
        drawBackground(ctx, area.splitX(m_font->getEmWidth() * pItem->indent));
    }
    ui::prepareHighContrastListItem(ctx, area, state);
    if (pItem != 0) {
        gfx::Rectangle imageArea = area.splitX(20);
        if (pItem->icon != iNone && m_icons.get() != 0) {
            // The "files" image has rows of 2 icons of 16x16 each
            int iconIndex = pItem->icon - 1;
            int iconSize = 16;
            gfx::Point anchor(iconSize * (iconIndex % 2), iconSize * (iconIndex / 2));
            can.blit(imageArea.getTopLeft() - anchor, *m_icons, gfx::Rectangle(anchor, gfx::Point(iconSize, iconSize)));
        }
        outText(ctx, area.getTopLeft(), pItem->name);
    }
}

// Widget:
ui::layout::Info
client::widgets::FolderListbox::getLayoutInfo() const
{
    return m_cells.scaledBy(m_font->getCellSize());
}

bool
client::widgets::FolderListbox::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

void
client::widgets::FolderListbox::handlePositionChange(gfx::Rectangle& oldPosition)
{
    defaultHandlePositionChange(oldPosition);
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
