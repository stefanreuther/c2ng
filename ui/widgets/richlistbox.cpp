/**
  *  \file ui/widgets/richlistbox.cpp
  */

#include <algorithm>
#include <climits>
#include "ui/widgets/richlistbox.hpp"
#include "gfx/dimcolorscheme.hpp"
#include "ui/draw.hpp"
#include "ui/icons/image.hpp"
#include "ui/skincolorscheme.hpp"

namespace {
    using util::SkinColor;

    /* Color scheme for color inversion.
       This is used when UseBackgroundColorScheme is set.
       It does not (can not!) provide complete color inversion. */
    class InverseColorScheme : public gfx::ColorScheme<SkinColor::Color> {
     public:
        InverseColorScheme(gfx::ColorScheme<SkinColor::Color>& parent)
            : m_parent(parent)
            { }
        virtual gfx::Color_t getColor(SkinColor::Color index)
            {
                switch (index) {
                 case SkinColor::Static:     return m_parent.getColor(SkinColor::InvStatic);
                 case SkinColor::Background: return m_parent.getColor(SkinColor::Static);
                 case SkinColor::InvStatic:  return m_parent.getColor(SkinColor::Static);
                 default:                    return m_parent.getColor(index);
                }
            }
        virtual void drawBackground(gfx::Canvas& can, const gfx::Rectangle& area)
            { can.drawBar(area, getColor(SkinColor::Background), 0, gfx::FillPattern::SOLID, gfx::OPAQUE_ALPHA); }
     private:
        gfx::ColorScheme<SkinColor::Color>& m_parent;
    };
}


ui::widgets::RichListbox::Item::Item(const util::rich::Text text, afl::base::Ptr<gfx::Canvas> image, bool accessible, gfx::ResourceProvider& provider)
    : accessible(accessible),
      text(text),
      doc(provider),
      image(image)
{ }

ui::widgets::RichListbox::RichListbox(gfx::ResourceProvider& provider, ui::ColorScheme& scheme)
    : m_provider(provider),
      m_colorScheme(scheme),
      m_items(),
      m_renderFlags(),
      m_preferredWidth(400),
      m_preferredHeight(0)
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

void
ui::widgets::RichListbox::setItemAccessible(size_t n, bool accessible)
{
    if (n < m_items.size() && accessible != m_items[n]->accessible) {
        m_items[n]->accessible = accessible;
        updateItem(n);
        if (n == getCurrentItem()) {
            setCurrentItem(n);
        }
    }
}

void
ui::widgets::RichListbox::setPreferredWidth(int width)
{
    m_preferredWidth = width;
}

void
ui::widgets::RichListbox::setPreferredHeight(int height)
{
    m_preferredHeight = height;
}

void
ui::widgets::RichListbox::setRenderFlag(RenderFlag flag, bool value)
{
    if (value != hasRenderFlag(flag)) {
        if (value) {
            m_renderFlags += flag;
        } else {
            m_renderFlags -= flag;
        }
        if (flag == DisableWrap) {
            render(0, m_items.size());
        }
        requestRedraw();
    }
}

bool
ui::widgets::RichListbox::hasRenderFlag(RenderFlag flag) const
{
    return m_renderFlags.contains(flag);
}

// AbstractListbox:
size_t
ui::widgets::RichListbox::getNumItems() const
{
    return m_items.size();
}

bool
ui::widgets::RichListbox::isItemAccessible(size_t n) const
{
    return (n < m_items.size()
            && m_items[n]->accessible);
}

int
ui::widgets::RichListbox::getItemHeight(size_t n) const
{
    return (n < m_items.size()
            ? m_items[n]->doc.getDocumentHeight() + 4
            : 0);
}

int
ui::widgets::RichListbox::getHeaderHeight() const
{
    return 0;
}

int
ui::widgets::RichListbox::getFooterHeight() const
{
    return 0;
}

void
ui::widgets::RichListbox::drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
ui::widgets::RichListbox::drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
ui::widgets::RichListbox::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    SkinColorScheme main(BLACK_COLOR_SET, m_colorScheme);
    SkinColorScheme inv(GRAY_COLOR_SET, m_colorScheme);
    InverseColorScheme inv2(getColorScheme());
    gfx::ColorScheme<util::SkinColor::Color>& cs = hasRenderFlag(UseBackgroundColorScheme) ? getColorScheme() : main;
    gfx::DimColorScheme shade(cs, can);

    gfx::Context<util::SkinColor::Color> ctx(can, main);
    ctx.useColorScheme(cs);
    prepareHighContrastListItem(ctx, area, state);
    if (item < m_items.size()) {
        if (!hasRenderFlag(NoShade) && (hasState(DisabledState) || !m_items[item]->accessible)) {
            ctx.useColorScheme(shade);
        } else if (state == FocusedItem) {
            if (hasRenderFlag(UseBackgroundColorScheme)) {
                ctx.useColorScheme(inv2);
            } else {
                ctx.useColorScheme(inv);
            }
        }
        area.grow(-2, -2);
        m_items[item]->doc.draw(ctx, area, 0);
    }
}

void
ui::widgets::RichListbox::handlePositionChange()
{
    render(0, m_items.size());
    defaultHandlePositionChange();
}

// Widget:
ui::layout::Info
ui::widgets::RichListbox::getLayoutInfo() const
{
    int totalHeight = 0;
    for (size_t i = 0, n = m_items.size(); i < n; ++i) {
        const Item& it = *m_items[i];
        ui::rich::Document doc(m_provider);
        doc.setPageWidth(hasRenderFlag(DisableWrap) ? INT_MAX : m_preferredWidth);
        if (it.image.get() != 0) {
            doc.addFloatObject(doc.deleter().addNew(new ui::icons::Image(*it.image)), true);
        }
        doc.add(it.text);
        doc.finish();
        totalHeight += doc.getDocumentHeight();
        totalHeight += 4;
    }

    int minHeight = (m_preferredHeight > 0 && totalHeight > m_preferredHeight ? m_preferredHeight : totalHeight);
    return ui::layout::Info(gfx::Point(m_preferredWidth, minHeight),
                            gfx::Point(m_preferredWidth, totalHeight),
                            ui::layout::Info::GrowBoth);
}

bool
ui::widgets::RichListbox::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

void
ui::widgets::RichListbox::render(size_t pos, size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        Item& it = *m_items[pos++];
        it.doc.clear();
        it.doc.setPageWidth(hasRenderFlag(DisableWrap) ? INT_MAX : std::max(10, getExtent().getWidth() - 4));
        if (it.image.get() != 0) {
            it.doc.addFloatObject(it.doc.deleter().addNew(new ui::icons::Image(*it.image)), true);
        }
        it.doc.add(it.text);
        it.doc.finish();
    }
    handleModelChange();
}
