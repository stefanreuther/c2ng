/**
  *  \file client/widgets/pluginlist.cpp
  *  \brief Class client::widgets::PluginList
  */

#include "client/widgets/pluginlist.hpp"
#include "afl/base/deleter.hpp"
#include "afl/string/format.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"
#include "util/string.hpp"

client::widgets::PluginList::PluginList(ui::Root& root, afl::string::Translator& tx)
    : AbstractListbox(),
      m_root(root),
      m_translator(tx),
      m_content(),
      m_loading(true)
{ }

client::widgets::PluginList::~PluginList()
{ }

void
client::widgets::PluginList::setContent(const Infos_t& content)
{
    // ex WPluginList::init (sort-of)
    // Try to preserve current plugin
    size_t newPos = getCurrentItem();
    if (const Info_t* p = getCurrentPlugin()) {
        for (size_t i = 0; i < content.size(); ++i) {
            if (content[i].id == p->id) {
                newPos = i;
                break;
            }
        }
    }

    // Exchange content
    m_content = content;
    m_loading = false;
    setCurrentItem(newPos);
    handleModelChange();
}

void
client::widgets::PluginList::setLoading()
{
    m_content.clear();
    m_loading = true;
    handleModelChange();
}

const client::widgets::PluginList::Info_t*
client::widgets::PluginList::getCurrentPlugin() const
{
    // WPluginList::getCurrentPluginId
    size_t index = getCurrentItem();
    if (index < m_content.size()) {
        return &m_content[index];
    } else {
        return 0;
    }
}

// AbstractListbox:
size_t
client::widgets::PluginList::getNumItems() const
{
    if (m_loading) {
        return 0;
    } else {
        if (m_content.empty()) {
            return 1;
        } else {
            return m_content.size();
        }
    }
}

bool
client::widgets::PluginList::isItemAccessible(size_t /*n*/) const
{
    if (m_loading || m_content.empty()) {
        return false;
    } else {
        return true;
    }
}

int
client::widgets::PluginList::getItemHeight(size_t /*n*/) const
{
    return getItemHeight();
}

int
client::widgets::PluginList::getHeaderHeight() const
{
    return 0;
}

int
client::widgets::PluginList::getFooterHeight() const
{
    return 0;
}

void
client::widgets::PluginList::drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::PluginList::drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::PluginList::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    // ex WPluginList::drawPart(GfxCanvas& can, int from, int to)
    afl::base::Deleter del;
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    prepareColorListItem(ctx, area, state, m_root.colorScheme(), del);

    if (item == 0 && m_content.empty() && !m_loading) {
        ctx.setTextAlign(gfx::CenterAlign, gfx::MiddleAlign);
        ctx.useFont(*getNormalFont());
        ctx.setColor(util::SkinColor::Static);
        outTextF(ctx, area, m_translator("No plugins installed."));
    } else if (item < m_content.size()) {
        const Info_t& e = m_content[item];
        area.consumeX(5);
        area.consumeY(2);
        ctx.setColor(util::SkinColor::Static);

        afl::base::Ref<gfx::Font> titleFont = getTitleFont();
        ctx.useFont(*titleFont);
        outTextF(ctx, area.splitY(titleFont->getLineHeight()), e.name);

        afl::base::Ref<gfx::Font> subtitleFont = getSubtitleFont();
        ctx.useFont(*subtitleFont);
        String_t line;
        ctx.setColor(formatSubtitle(line, e, m_translator));
        outTextF(ctx, area.splitY(subtitleFont->getLineHeight()), line);
    } else {
        // out of range
    }
}

void
client::widgets::PluginList::handlePositionChange()
{
    return defaultHandlePositionChange();
}

ui::layout::Info
client::widgets::PluginList::getLayoutInfo() const
{
    gfx::Point size(20 * getNormalFont()->getEmWidth(), 7 * getItemHeight());
    return ui::layout::Info(size, ui::layout::Info::GrowBoth);
}

bool
client::widgets::PluginList::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

afl::base::Ref<gfx::Font>
client::widgets::PluginList::getNormalFont() const
{
    return m_root.provider().getFont(gfx::FontRequest());
}

afl::base::Ref<gfx::Font>
client::widgets::PluginList::getTitleFont() const
{
    return m_root.provider().getFont("b");
}

afl::base::Ref<gfx::Font>
client::widgets::PluginList::getSubtitleFont() const
{
    return m_root.provider().getFont("-");
}

int
client::widgets::PluginList::getItemHeight() const
{
    return getTitleFont()->getLineHeight()
        + getSubtitleFont()->getLineHeight()
        + 4;
}

util::SkinColor::Color
client::widgets::formatSubtitle(String_t& out, const util::plugin::Manager::Info& in, afl::string::Translator& tx)
{
    util::SkinColor::Color result = util::SkinColor::Static;
    String_t line = in.id;
    switch (in.status) {
     case util::plugin::Manager::Loaded:
        util::addListItem(line, ", ", tx("loaded"));
        result = util::SkinColor::Faded;
        break;

     case util::plugin::Manager::NotLoaded:
        util::addListItem(line, ", ", tx("not loaded"));
        result = util::SkinColor::Red;
        break;
    }
    out = afl::string::Format("(%s)", line);
    return result;
}
