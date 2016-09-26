/**
  *  \file ui/widgets/richlistbox.cpp
  */

#include <algorithm>
#include "ui/widgets/richlistbox.hpp"
#include "ui/draw.hpp"
#include "ui/rich/imageobject.hpp"
#include "ui/skincolorscheme.hpp"

ui::widgets::RichListbox::Item::Item(const util::rich::Text text, afl::base::Ptr<gfx::Canvas> image, bool accessible, gfx::ResourceProvider& provider)
    : accessible(accessible),
      text(text),
      doc(provider),
      image(image)
{ }

ui::widgets::RichListbox::RichListbox(gfx::ResourceProvider& provider, ui::ColorScheme& scheme)
    : m_provider(provider),
      m_colorScheme(scheme),
      m_items()
{ }

ui::widgets::RichListbox::~RichListbox()
{ }

void
ui::widgets::RichListbox::clear()
{
    m_items.clear();
    handleModelChange();
}

void
ui::widgets::RichListbox::addItem(const util::rich::Text text, afl::base::Ptr<gfx::Canvas> image, bool accessible)
{
    size_t n = m_items.size();
    m_items.pushBackNew(new Item(text, image, accessible, m_provider));
    render(n, 1);
}

// AbstractListbox:
size_t
ui::widgets::RichListbox::getNumItems()
{
    return m_items.size();
}

bool
ui::widgets::RichListbox::isItemAccessible(size_t n)
{
    return (n < m_items.size()
            && m_items[n]->accessible);
}

int
ui::widgets::RichListbox::getItemHeight(size_t n)
{
    return (n < m_items.size()
            ? m_items[n]->doc.getDocumentHeight() + 4
            : 0);
}

int
ui::widgets::RichListbox::getHeaderHeight()
{
    return 0;
}

void
ui::widgets::RichListbox::drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
ui::widgets::RichListbox::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    gfx::Context ctx(can);
    SkinColorScheme main(BLACK_COLOR_SET, m_colorScheme);
    SkinColorScheme inv(GRAY_COLOR_SET, m_colorScheme);
    ctx.useColorScheme(main);
    prepareHighContrastListItem(ctx, area, state);
    if (item < m_items.size()) {
        if (state == FocusedItem) {
            ctx.useColorScheme(inv);
        }
        area.grow(-2, -2);
        m_items[item]->doc.draw(ctx, area, 0);
    }
}

void
ui::widgets::RichListbox::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    render(0, m_items.size());
    requestRedraw();
    // FIXME: was AbstractListbox::handlePositionChange(oldPosition);
}

// Widget:
ui::layout::Info
ui::widgets::RichListbox::getLayoutInfo() const
{
    // FIXME 1.0e+38
    return gfx::Point(400, 400);
}

void
ui::widgets::RichListbox::render(size_t pos, size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        Item& it = *m_items[pos++];
        it.doc.clear();
        it.doc.setPageWidth(std::max(10, getExtent().getWidth() - 4));
        if (it.image.get() != 0) {
            it.doc.addFloatObject(std::auto_ptr<ui::rich::BlockObject>(new ui::rich::ImageObject(it.image)), true);
        }
        it.doc.add(it.text);
        it.doc.finish();
    }
    handleModelChange();
}
