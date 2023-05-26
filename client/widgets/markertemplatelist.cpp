/**
  *  \file client/widgets/markertemplatelist.cpp
  *  \brief Class client::widgets::MarkerTemplateList
  */

#include <algorithm>
#include "client/widgets/markertemplatelist.hpp"
#include "afl/base/deleter.hpp"
#include "afl/string/format.hpp"
#include "client/map/renderer.hpp"
#include "client/marker.hpp"
#include "gfx/complex.hpp"
#include "ui/draw.hpp"

client::widgets::MarkerTemplateList::MarkerTemplateList(ui::Root& root, afl::string::Translator& tx)
    : AbstractListbox(),
      m_root(root),
      m_translator(tx),
      m_content()
{ }

client::widgets::MarkerTemplateList::~MarkerTemplateList()
{ }

void
client::widgets::MarkerTemplateList::setContent(const DataVector_t& content)
{
    m_content = content;
    requestRedraw();
}

// AbstractListbox:
size_t
client::widgets::MarkerTemplateList::getNumItems() const
{
    return m_content.size();
}

bool
client::widgets::MarkerTemplateList::isItemAccessible(size_t /*n*/) const
{
    return true;
}

int
client::widgets::MarkerTemplateList::getItemHeight(size_t /*n*/) const
{
    return getLineHeight();
}

int
client::widgets::MarkerTemplateList::getHeaderHeight() const
{
    return 0;
}

int
client::widgets::MarkerTemplateList::getFooterHeight() const
{
    return 0;
}

void
client::widgets::MarkerTemplateList::drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::MarkerTemplateList::drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::MarkerTemplateList::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    // ex WMarkerList::drawPart, CMarkerView.Draw
    afl::base::Deleter del;
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    prepareColorListItem(ctx, area, state, m_root.colorScheme(), del);
    ctx.setTextAlign(gfx::LeftAlign, gfx::MiddleAlign);
    ctx.useFont(*getFont());

    if (item < m_content.size()) {
        // Data
        const Data_t& m = m_content[item];

        // Compute geometry and show text
        gfx::Rectangle box = area.splitX(20);
        area.consumeX(5);

        String_t note = m.note;
        if (note.empty()) {
            if (item == 0) {
                note = m_translator("Default marker");
            } else {
                note = afl::string::Format(m_translator("Marker %d"), item);
            }
            ctx.setColor(util::SkinColor::Faded);
        }
        outTextF(ctx, area, note);

        // Show marker, always on black ground
        box.grow(-1, -1);
        if (item != getCurrentItem()) {
            drawSolidBar(ctx, box, util::SkinColor::Static);
        } else {
            ctx.setColor(util::SkinColor::Faded);
            drawRectangle(ctx, box);
        }

        ctx.setRawColor(m_root.colorScheme().getColor(client::map::getUserColor(m.color)));
        if (const Marker* p = getUserMarker(m.markerKind, true)) {
            drawMarker(ctx, *p, box.getCenter());
        }
    }
}

// Widget:
void
client::widgets::MarkerTemplateList::handlePositionChange()
{
    defaultHandlePositionChange();
}

ui::layout::Info
client::widgets::MarkerTemplateList::getLayoutInfo() const
{
    gfx::Point size(20 * getFont()->getEmWidth(),
                    10 * getLineHeight());
    return ui::layout::Info(size, ui::layout::Info::GrowBoth);
}

bool
client::widgets::MarkerTemplateList::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

int
client::widgets::MarkerTemplateList::getLineHeight() const
{
    return std::max(20, getFont()->getLineHeight());
}

afl::base::Ref<gfx::Font>
client::widgets::MarkerTemplateList::getFont() const
{
    return m_root.provider().getFont(gfx::FontRequest());
}
