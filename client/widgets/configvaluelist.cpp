/**
  *  \file client/widgets/configvaluelist.cpp
  *  \brief Class client::widgets::ConfigValueList
  */

#include "client/widgets/configvaluelist.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"

client::widgets::ConfigValueList::ConfigValueList(ui::Root& root)
    : m_root(root),
      m_content(),
      m_highlightedSource(game::config::ConfigurationEditor::Game),
      m_nameColumnWidth(20),
      m_valueColumnWidth(15),
      m_preferredHeight(20)
{
    // WConfigView::WConfigView
}

void
client::widgets::ConfigValueList::setHighlightedSource(game::config::ConfigurationEditor::Source source)
{
    // WConfigView::setHighlightSource
    m_highlightedSource = source;
    requestRedraw();
}

void
client::widgets::ConfigValueList::setNameColumnWidth(int widthInEms)
{
    // WConfigView::setNameColumnWidth
    m_nameColumnWidth = widthInEms;
    requestRedraw();
}

void
client::widgets::ConfigValueList::setValueColumnWidth(int widthInEms)
{
    m_valueColumnWidth = widthInEms;
}

void
client::widgets::ConfigValueList::setPreferredHeight(int numLines)
{
    m_preferredHeight = numLines;
}

void
client::widgets::ConfigValueList::setContent(const Infos_t& infos)
{
    // WConfigView::setContent
    m_content = infos;
    handleModelChange();
}

void
client::widgets::ConfigValueList::setItemContent(size_t index, const game::config::ConfigurationEditor::Info& info)
{
    if (index < m_content.size()) {
        m_content[index] = info;
        updateItem(index);
    }
}

const game::config::ConfigurationEditor::Info*
client::widgets::ConfigValueList::getCurrentOption() const
{
    // WConfigView::getCurrentOption()
    size_t i = getCurrentItem();
    if (i < m_content.size()) {
        return &m_content[i];
    } else {
        return 0;
    }
}

size_t
client::widgets::ConfigValueList::getNumItems() const
{
    return m_content.size();
}

bool
client::widgets::ConfigValueList::isItemAccessible(size_t /*n*/) const
{
    return true;
}

int
client::widgets::ConfigValueList::getItemHeight(size_t /*n*/) const
{
    return m_root.provider().getFont(gfx::FontRequest())->getLineHeight();
}

int
client::widgets::ConfigValueList::getHeaderHeight() const
{
    return 0;
}
int
client::widgets::ConfigValueList::getFooterHeight() const
{
    return 0;
}

void
client::widgets::ConfigValueList::drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::ConfigValueList::drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::ConfigValueList::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    // WConfigView::drawPart
    afl::base::Deleter del;
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    prepareColorListItem(ctx, area, state, m_root.colorScheme(), del);
    if (item < m_content.size()) {
        const game::config::ConfigurationEditor::Info& opt = m_content[item];
        afl::base::Ref<gfx::Font> normalFont = m_root.provider().getFont("");
        afl::base::Ref<gfx::Font> thisFont = (opt.source > m_highlightedSource ? m_root.provider().getFont("b") : normalFont);
        ctx.useFont(*thisFont);
        area.consumeX(5);
        outTextF(ctx, area.splitX(m_nameColumnWidth * normalFont->getEmWidth()), opt.name);
        area.consumeX(5);
        outTextF(ctx, area, opt.value);
    }
}

void
client::widgets::ConfigValueList::handlePositionChange()
{
    defaultHandlePositionChange();
}

ui::layout::Info
client::widgets::ConfigValueList::getLayoutInfo() const
{
    afl::base::Ref<gfx::Font> normalFont = m_root.provider().getFont("");
    gfx::Point pt(normalFont->getCellSize().scaledBy(m_nameColumnWidth + m_valueColumnWidth, m_preferredHeight));
    pt.addX(10);
    return ui::layout::Info(pt, ui::layout::Info::GrowBoth);
}

bool
client::widgets::ConfigValueList::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}
