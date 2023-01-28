/**
  *  \file ui/widgets/stringlistbox.cpp
  */

#include "ui/widgets/stringlistbox.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"
#include "afl/charset/unicode.hpp"
#include "afl/charset/utf8.hpp"

ui::widgets::StringListbox::StringListbox(gfx::ResourceProvider& provider, ui::ColorScheme& scheme)
    : AbstractListbox(),
      m_content(),
      m_provider(provider),
      m_colorScheme(scheme),
      m_preferredWidth(0),
      m_preferredHeight(0),
      m_preferredWidthInPixels(0)
{ }

ui::widgets::StringListbox::~StringListbox()
{ }

// AbstractListbox:
size_t
ui::widgets::StringListbox::getNumItems() const
{
    return m_content.size();
}

bool
ui::widgets::StringListbox::isItemAccessible(size_t /*n*/) const
{
    return true;
}
int
ui::widgets::StringListbox::getItemHeight(size_t /*n*/) const
{
    return m_provider.getFont(gfx::FontRequest())->getLineHeight();
}

int
ui::widgets::StringListbox::getHeaderHeight() const
{
    return 0;
}

int
ui::widgets::StringListbox::getFooterHeight() const
{
    return 0;
}

void
ui::widgets::StringListbox::drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
ui::widgets::StringListbox::drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
ui::widgets::StringListbox::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    // ex UIStandardListbox::drawPart

    afl::base::Deleter del;
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    ctx.useFont(*m_provider.getFont(gfx::FontRequest()));

    ui::prepareColorListItem(ctx, area, state, m_colorScheme, del);
    area.consumeX(5);

    int32_t key;
    String_t text;
    if (m_content.get(item, key, text)) {
        outTextF(ctx, area, text);
    }
}

// Widget:
void
ui::widgets::StringListbox::handlePositionChange()
{
    defaultHandlePositionChange();
}

ui::layout::Info
ui::widgets::StringListbox::getLayoutInfo() const
{
    const afl::base::Ref<gfx::Font> font = m_provider.getFont(gfx::FontRequest());
    const gfx::Point cellSize = font->getCellSize();

    // Find width
    int width = m_preferredWidth;
    if (!m_preferredWidthInPixels) {
        width *= cellSize.getX();
    }
    if (width == 0) {
        for (size_t i = 0, n = m_content.size(); i < n; ++i) {
            String_t s;
            int32_t k;
            if (m_content.get(i, k, s)) {
                width = std::max(width, font->getTextWidth(s));
            }
        }
        width += 10;
    }

    // Find height
    int height = m_preferredHeight;
    if (height == 0) {
        height = std::max(3, std::min(20, int(m_content.size())));
    }
    height *= cellSize.getY();

    // Result
    return ui::layout::Info(gfx::Point(width, height),
                            gfx::Point(width, height),
                            ui::layout::Info::GrowBoth);
}

bool
ui::widgets::StringListbox::handleKey(util::Key_t key, int prefix)
{
    // ex UIStandardListbox::handleEvent
    if (hasState(FocusedState)
        && !hasState(DisabledState)
        && key < util::Key_FirstSpecial)
    {
        // Printable key
        const afl::charset::Unichar_t lcKey = afl::charset::getLowerCase(key);

        size_t pos = getCurrentItem();
        bool wrapped = false;
        afl::charset::Utf8 u8;
        while (1) {
            // Advance position
            ++pos;
            if (pos >= m_content.size()) {
                if (wrapped) {
                    break;
                }
                pos = 0;
                wrapped = true;
            }

            // Match current item
            String_t title;
            int32_t tmp;
            if (m_content.get(pos, tmp, title)) {
                if (lcKey == afl::charset::getLowerCase(u8.charAt(title, 0))) {
                    setCurrentItem(pos);
                    return true;
                }
            }
        }
    }
    return defaultHandleKey(key, prefix);
}

// StringListbox:
void
ui::widgets::StringListbox::addItem(int32_t key, const String_t& s)
{
    m_content.add(key, s);
    handleModelChange();
}

void
ui::widgets::StringListbox::addItems(const afl::functional::StringTable_t& tab)
{
    int32_t i;
    for (bool v = tab.getFirstKey(i); v; v = tab.getNextKey(i)) {
        m_content.add(i, tab(i));
    }
    handleModelChange();
}

void
ui::widgets::StringListbox::sortItemsAlphabetically()
{
    // FIXME: preserve current key
    m_content.sortAlphabetically();
    handleModelChange();
}

void
ui::widgets::StringListbox::swapItems(util::StringList& other)
{
    // FIXME: preserve current key
    m_content.swap(other);
    handleModelChange();
}

void
ui::widgets::StringListbox::setItems(const util::StringList& other)
{
    // FIXME: preserve current key
    m_content = other;
    handleModelChange();
}

const util::StringList&
ui::widgets::StringListbox::getStringList() const
{
    return m_content;
}

bool
ui::widgets::StringListbox::getCurrentKey(int32_t& key) const
{
    String_t tmp;
    return m_content.get(getCurrentItem(), key, tmp);
}

void
ui::widgets::StringListbox::setCurrentKey(int32_t key)
{
    // ex UIStandardListbox::scrollToKey
    // FIXME: PCC2 has a way to mark items inaccessible
    size_t pos;
    if (m_content.find(key, pos)) {
        setCurrentItem(pos);
    }
}

void
ui::widgets::StringListbox::setPreferredWidth(int n, bool pixels)
{
    m_preferredWidth = n;
    m_preferredWidthInPixels = pixels;
}

void
ui::widgets::StringListbox::setPreferredHeight(int n)
{
    m_preferredHeight = n;
}
