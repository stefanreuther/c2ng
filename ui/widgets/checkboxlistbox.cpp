/**
  *  \file ui/widgets/checkboxlistbox.cpp
  *  \brief Class ui::widgets::CheckboxListbox
  */

#include "ui/widgets/checkboxlistbox.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"
#include "util/key.hpp"
#include "util/updater.hpp"

using util::Updater;

namespace {
    const int PAD = 5;
    const int ICON_WIDTH = 16;
    const int ICON_HEIGHT = 16;
    const int ICON_LPAD = 2;
    const int ICON_RPAD = 10;
    const int ICON_VPAD = 2;
}

/*
 *  Internal Item Structure
 */

struct ui::widgets::CheckboxListbox::Item {
    size_t index;
    int id;
    util::Key_t key;
    bool accessible;
    String_t imageName;
    String_t label;
    String_t info;

    Item(size_t index, int id, util::Key_t key, bool accessible, String_t imageName, String_t label, String_t info)
        : index(index), id(id), key(key), accessible(accessible), imageName(imageName), label(label), info(info)
        { }
};


ui::widgets::CheckboxListbox::CheckboxListbox(Root& root, Layout layout)
    : m_root(root), m_items(), m_layout(layout), m_labelWidth(0), m_infoWidth(0), m_preferredHeight(0), m_labelMode(Unknown), m_infoMode(Unknown),
      conn_imageChange(root.provider().sig_imageChange.add(this, (void (Widget::*)()) &Widget::requestRedraw))
{
    sig_itemDoubleClick.add(this, &CheckboxListbox::onItemDoubleClick);
    sig_itemClickAt.add(this, &CheckboxListbox::onItemClickAt);
}

ui::widgets::CheckboxListbox::~CheckboxListbox()
{ }

void
ui::widgets::CheckboxListbox::setLabelWidth(int width)
{
    m_labelWidth = width;
    m_labelMode = (width < 0 ? Unknown : Fixed);
}

void
ui::widgets::CheckboxListbox::setInfoWidth(int width)
{
    m_infoWidth = width;
    m_infoMode = (width < 0 ? Unknown : Fixed);
}

void
ui::widgets::CheckboxListbox::setPreferredHeight(int height)
{
    m_preferredHeight = height;
}

ui::widgets::CheckboxListbox::Item*
ui::widgets::CheckboxListbox::addItem(int id, String_t label)
{
    size_t index = m_items.size();
    Item* result = m_items.pushBackNew(new Item(index, id, 0, true, String_t(), label, String_t()));
    invalidate(m_labelMode);
    invalidate(m_infoMode);
    handleModelChange();
    return result;
}

ui::widgets::CheckboxListbox::Item*
ui::widgets::CheckboxListbox::findItem(int id) const
{
    for (size_t i = 0, n = m_items.size(); i < n; ++i) {
        if (Item* p = m_items[i]) {
            if (p->id == id) {
                return p;
            }
        }
    }
    return 0;
}

ui::widgets::CheckboxListbox::Item*
ui::widgets::CheckboxListbox::getItemByIndex(size_t index) const
{
    return (index < m_items.size()
            ? m_items[index]
            : 0);

}

ui::widgets::CheckboxListbox::Item*
ui::widgets::CheckboxListbox::setItemAccessible(Item* p, bool accessible)
{
    if (p != 0) {
        if (Updater().set(p->accessible, accessible)) {
            updateItem(p->index);
        }
    }
    return p;
}

ui::widgets::CheckboxListbox::Item*
ui::widgets::CheckboxListbox::setItemInfo(Item* p, String_t info)
{
    // ex WPlayerSetSelector::setInfo (sort-of)
    if (p != 0) {
        if (Updater().set(p->info, info)) {
            invalidate(m_infoMode);
            updateItem(p->index);
        }
    }
    return p;
}

ui::widgets::CheckboxListbox::Item*
ui::widgets::CheckboxListbox::setItemLabel(Item* p, String_t label)
{
    if (p != 0) {
        if (Updater().set(p->label, label)) {
            invalidate(m_labelMode);
            updateItem(p->index);
        }
    }
    return p;
}

ui::widgets::CheckboxListbox::Item*
ui::widgets::CheckboxListbox::setItemImageName(Item* p, String_t imageName)
{
    if (p != 0) {
        if (Updater().set(p->imageName, imageName)) {
            updateItem(p->index);
        }
    }
    return p;
}

ui::widgets::CheckboxListbox::Item*
ui::widgets::CheckboxListbox::setItemKey(Item* p, util::Key_t key)
{
    if (p != 0) {
        p->key = key;
    }
    return p;
}

int
ui::widgets::CheckboxListbox::getItemId(Item* p) const
{
    return (p != 0 ? p->id : 0);
}

// AbstractListbox virtuals:
size_t
ui::widgets::CheckboxListbox::getNumItems() const
{
    return m_items.size();
}

bool
ui::widgets::CheckboxListbox::isItemAccessible(size_t n) const
{
    Item* p = getItemByIndex(n);
    return p != 0 && p->accessible;
}

int
ui::widgets::CheckboxListbox::getItemHeight(size_t /*n*/) const
{
    return getItemHeight();
}

int
ui::widgets::CheckboxListbox::getHeaderHeight() const
{
    return 0;
}

int
ui::widgets::CheckboxListbox::getFooterHeight() const
{
    return 0;
}

void
ui::widgets::CheckboxListbox::drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
ui::widgets::CheckboxListbox::drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
ui::widgets::CheckboxListbox::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    // ex WPlayerSetSelector::drawPart (sort-of), AttachmentList::drawPart
    afl::base::Deleter del;
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    prepareColorListItem(ctx, area, state, m_root.colorScheme(), del);
    updateWidths();

    if (const Item* p = getItemByIndex(item)) {
        // Image
        area.consumeX(ICON_LPAD);
        gfx::Rectangle imageArea = area.splitX(ICON_WIDTH);
        afl::base::Ptr<gfx::Canvas> image = m_root.provider().getImage(p->imageName);
        if (image.get() != 0) {
            gfx::Rectangle a(gfx::Point(), image->getSize());
            a.centerWithin(imageArea);
            blitSized(ctx, a, *image);
        }
        area.consumeX(ICON_RPAD);

        // Align the text
        afl::base::Ref<gfx::Font> labelFont = getLabelFont();
        afl::base::Ref<gfx::Font> infoFont = getInfoFont();
        int textHeight = 1;
        switch (m_layout) {
         case SingleLine: textHeight = std::max(labelFont->getLineHeight(), infoFont->getLineHeight()); break;
         case MultiLine:  textHeight = labelFont->getLineHeight() + infoFont->getLineHeight();          break;
        }
        if (textHeight < area.getHeight()) {
            area.consumeY((area.getHeight() - textHeight) / 2);
        }

        // Draw the text
        ctx.useFont(*labelFont);
        switch (m_layout) {
         case SingleLine: outTextF(ctx, area.splitX(m_labelWidth),               p->label); break;
         case MultiLine:  outTextF(ctx, area.splitY(labelFont->getLineHeight()), p->label); break;
        }
        ctx.useFont(*infoFont);
        outTextF(ctx, area, p->info);
    }
}

// Widget virtuals:
void
ui::widgets::CheckboxListbox::handlePositionChange()
{
    defaultHandlePositionChange();
}

ui::layout::Info
ui::widgets::CheckboxListbox::getLayoutInfo() const
{
    const_cast<CheckboxListbox&>(*this).updateWidths();

    int numLines = m_preferredHeight > 0 ? m_preferredHeight : int(m_items.size());
    int lineHeight = getItemHeight();
    int width = 1;
    switch (m_layout) {
     case SingleLine:
        width = ICON_WIDTH + ICON_LPAD + ICON_RPAD + m_labelWidth + m_infoWidth;
        break;
     case MultiLine:
        width = ICON_WIDTH + ICON_LPAD + ICON_RPAD + std::max(m_labelWidth, m_infoWidth);
        break;
    }

    gfx::Point size(width, lineHeight * numLines);
    return ui::layout::Info(size, ui::layout::Info::GrowBoth);
}

bool
ui::widgets::CheckboxListbox::handleKey(util::Key_t key, int prefix)
{
    if (hasState(FocusedState)) {
        for (size_t i = 0, n = m_items.size(); i < n; ++i) {
            if (const Item* p = m_items[i]) {
                if (p->key == key) {
                    requestActive();
                    setCurrentItem(i);
                    sig_checkboxClick.raise(p->id);
                    return true;
                }
            }
        }
        if (key == ' ') {
            if (const Item* p = getItemByIndex(getCurrentItem())) {
                requestActive();
                sig_checkboxClick.raise(p->id);
                return true;
            }
        }
    }
    return defaultHandleKey(key, prefix);
}

afl::base::Ref<gfx::Font>
ui::widgets::CheckboxListbox::getLabelFont() const
{
    return m_root.provider().getFont(gfx::FontRequest());
}

afl::base::Ref<gfx::Font>
ui::widgets::CheckboxListbox::getInfoFont() const
{
    return m_root.provider().getFont(m_layout == SingleLine ? gfx::FontRequest() : gfx::FontRequest("-"));
}

int
ui::widgets::CheckboxListbox::getItemHeight() const
{
    const int labelHeight = getLabelFont()->getLineHeight();
    const int infoHeight = getInfoFont()->getLineHeight();
    switch (m_layout) {
     case SingleLine: return std::max(ICON_HEIGHT + 2*ICON_VPAD, std::max(labelHeight, infoHeight));
     case MultiLine:  return std::max(ICON_HEIGHT + 2*ICON_VPAD, labelHeight + infoHeight);
    }
    return 1;
}

void
ui::widgets::CheckboxListbox::invalidate(WidthMode& m)
{
    if (m != Fixed) {
        m = Unknown;
    }
}

void
ui::widgets::CheckboxListbox::updateWidths()
{
    if (m_labelMode == Unknown || m_infoMode == Unknown) {
        const afl::base::Ref<gfx::Font> labelFont = getLabelFont();
        const afl::base::Ref<gfx::Font> infoFont = getInfoFont();
        if (m_labelMode == Unknown) {
            m_labelWidth = 0;
        }
        if (m_infoMode == Unknown) {
            m_infoWidth = 0;
        }
        for (size_t i = 0, n = m_items.size(); i < n; ++i) {
            if (Item* p = m_items[i]) {
                if (m_labelMode == Unknown) {
                    m_labelWidth = std::max(m_labelWidth, labelFont->getTextWidth(p->label) + PAD);
                }
                if (m_infoMode == Unknown) {
                    m_infoWidth = std::max(m_infoWidth, infoFont->getTextWidth(p->info) + PAD);
                }
            }
        }
        if (m_labelMode == Unknown) {
            m_labelMode = Known;
        }
        if (m_infoMode == Unknown) {
            m_infoMode = Known;
        }
    }
}

void
ui::widgets::CheckboxListbox::onItemDoubleClick(size_t index)
{
    if (const Item* p = getItemByIndex(index)) {
        sig_checkboxClick.raise(p->id);
    }
}

void
ui::widgets::CheckboxListbox::onItemClickAt(size_t index, gfx::Point pos)
{
    // ex WPlayerSetSelector::onItemClick (sort-of)
    if (pos.getX() < ICON_LPAD + ICON_RPAD + ICON_WIDTH) {
        if (const Item* p = getItemByIndex(index)) {
            sig_checkboxClick.raise(p->id);
        }
    }
}
