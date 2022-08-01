/**
  *  \file client/widgets/exportfieldlist.cpp
  *  \brief Class client::widgets::ExportFieldList
  */

#include "client/widgets/exportfieldlist.hpp"
#include "afl/base/deleter.hpp"
#include "afl/string/format.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"
#include "util/string.hpp"
#include "util/unicodechars.hpp"

using afl::string::Format;

client::widgets::ExportFieldList::ExportFieldList(ui::Root& root, afl::string::Translator& tx)
    : m_root(root), m_translator(tx), m_content()
{ }

client::widgets::ExportFieldList::~ExportFieldList()
{ }

void
client::widgets::ExportFieldList::setContent(const interpreter::exporter::FieldList& newContent)
{
    m_content = newContent;
    handleModelChange();
}

// AbstractListbox:
size_t
client::widgets::ExportFieldList::getNumItems() const
{
    // ex WExportFieldList::update (sort-of)
    /* "+1" to allow users to scroll past the end and insert there */
    return m_content.size() + 1;
}

bool
client::widgets::ExportFieldList::isItemAccessible(size_t /*n*/) const
{
    return true;
}

int
client::widgets::ExportFieldList::getItemHeight(size_t /*n*/) const
{
    return getFont()->getLineHeight();
}

int
client::widgets::ExportFieldList::getHeaderHeight() const
{
    return 0;
}

int
client::widgets::ExportFieldList::getFooterHeight() const
{
    return 0;
}

void
client::widgets::ExportFieldList::drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::ExportFieldList::drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::ExportFieldList::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    // ex WExportFieldList::drawPart, CUsedFieldList.DrawPart
    const afl::base::Ref<gfx::Font> font = getFont();
    const int widthWidth = std::min(4 * font->getEmWidth(), area.getWidth() / 2);

    afl::base::Deleter del;
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    ctx.useFont(*font);
    ui::prepareColorListItem(ctx, area, state, m_root.colorScheme(), del);

    area.consumeX(5);
    area.consumeRightX(5);
    if (item < m_content.size()) {
        // Regular item
        const int wi = m_content.getFieldWidth(item);
        if (wi != 0) {
            ctx.setTextAlign(gfx::RightAlign, gfx::TopAlign);
            if (wi < 0) {
                outTextF(ctx, area.splitRightX(widthWidth), Format("%d " UTF_LEFT_ARROW, -wi));
            } else {
                outTextF(ctx, area.splitRightX(widthWidth), Format(UTF_RIGHT_ARROW " %d", wi));
            }
            ctx.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
        }
        outTextF(ctx, area, util::formatName(m_content.getFieldName(item)));
    } else {
        // Placeholder item at end
        ctx.setColor(util::SkinColor::Faded);
        outTextF(ctx, area, m_translator("(more...)"));
    }
}

// Widget:
void
client::widgets::ExportFieldList::handlePositionChange(gfx::Rectangle& oldPosition)
{
    return defaultHandlePositionChange(oldPosition);
}

ui::layout::Info
client::widgets::ExportFieldList::getLayoutInfo() const
{
    const gfx::Point cellSize = getFont()->getCellSize();
    return ui::layout::Info(cellSize.scaledBy(10, 15),
                            cellSize.scaledBy(15, 20),
                            ui::layout::Info::GrowBoth);
}

bool
client::widgets::ExportFieldList::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

afl::base::Ref<gfx::Font>
client::widgets::ExportFieldList::getFont() const
{
    return m_root.provider().getFont(gfx::FontRequest());
}
