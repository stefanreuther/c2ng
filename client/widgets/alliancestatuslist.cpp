/**
  *  \file client/widgets/alliancestatuslist.cpp
  */

#include <algorithm>
#include "client/widgets/alliancestatuslist.hpp"
#include "util/translation.hpp"
#include "afl/base/countof.hpp"
#include "ui/draw.hpp"
#include "ui/colorscheme.hpp"
#include "gfx/complex.hpp"
#include "afl/functional/stringtable.hpp"

using afl::base::Ref;
using gfx::Font;

namespace {
    const char*const STATUS_LABELS[] = {
        N_("none"),
        N_("they offered"),
        N_("we offered"),
        N_("established"),
        N_("enemy"),
    };
}


client::widgets::AllianceStatusList::AllianceStatusList(ui::Root& root, afl::string::Translator& tx)
    : AbstractListbox(),
      sig_selectPlayer(),
      sig_toggleAlliance(),
      m_root(root),
      m_translator(tx),
      m_items()
{
    // ex phost.pas:CAlliancePanel
    sig_itemClickAt.add(this, &AllianceStatusList::onItemClickAt);
    sig_change.add(this, &AllianceStatusList::onChange);
}

void
client::widgets::AllianceStatusList::add(int id, const String_t& name, ItemFlags_t flags)
{
    m_items.push_back(Item(id, name, flags));
}

void
client::widgets::AllianceStatusList::setFlags(int id, ItemFlags_t flags)
{
    for (size_t i = 0, n = m_items.size(); i < n; ++i) {
        if (m_items[i].id == id) {
            if (m_items[i].flags != flags) {
                m_items[i].flags = flags;
                updateItem(i);
                break;
            }
        }
    }
}

int
client::widgets::AllianceStatusList::getCurrentPlayer() const
{
    size_t selection = getCurrentItem();
    if (selection < m_items.size()) {
        return m_items[selection].id;
    } else {
        return 0;
    }
}

// AbstractListbox:
size_t
client::widgets::AllianceStatusList::getNumItems() const
{
    return m_items.size();
}

bool
client::widgets::AllianceStatusList::isItemAccessible(size_t n) const
{
    return n < m_items.size() && !m_items[n].flags.contains(Self);
}

int
client::widgets::AllianceStatusList::getItemHeight(size_t /*n*/) const
{
    return getItemHeight();
}

int
client::widgets::AllianceStatusList::getHeaderHeight() const
{
    return 0;
}

int
client::widgets::AllianceStatusList::getFooterHeight() const
{
    return 0;
}

void
client::widgets::AllianceStatusList::drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::AllianceStatusList::drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::AllianceStatusList::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    // ex WAllyList::drawContent
    // Prepare
    Ref<Font> font = m_root.provider().getFont(gfx::FontRequest());
    int leftWidth, rightWidth;
    computeWidth(leftWidth, rightWidth, getExtent().getWidth());

    // Make two contexts. Left half will be drawn in skin colors, right half in UI colors
    gfx::Context<util::SkinColor::Color> leftContext(can, getColorScheme());
    afl::base::Deleter del;
    leftContext.useFont(*font);

    gfx::Context<uint8_t> rightContext(can, m_root.colorScheme());
    rightContext.useFont(*font);

    // Does this field exist?
    if (item < m_items.size()) {
        // Yes, it exists.
        const Item& it = m_items[item];

        // Left part: race name
        gfx::Rectangle leftArea(area.splitX(leftWidth));
        prepareColorListItem(leftContext, leftArea, state, m_root.colorScheme(), del);
        leftArea.consumeX(5);
        outTextF(leftContext, leftArea, it.name);

        // Figure out status
        uint8_t leftColor, rightColor, textColor;
        String_t text;
        bool enemy = it.flags.contains(Enemy);
        if (it.flags.contains(Self)) {
            leftColor = rightColor = textColor = ui::Color_Shield+4;
        } else if (it.flags.contains(WeOffer)) {
            if (it.flags.contains(TheyOffer)) {
                leftColor = ui::Color_GreenScale + 9;
                textColor = ui::Color_White;
                text      = m_translator(STATUS_LABELS[3]);
            } else {
                leftColor = ui::Color_Yellow;
                textColor = ui::Color_Black;
                text      = m_translator(STATUS_LABELS[2]);
            }
            rightColor = enemy ? uint8_t(ui::Color_Red) : leftColor;
        } else {
            if (it.flags.contains(TheyOffer)) {
                leftColor  = ui::Color_Yellow;
                rightColor = enemy ? uint8_t(ui::Color_Red) : leftColor;
                textColor  = ui::Color_Black;
                text       = m_translator(STATUS_LABELS[1]);
            } else {
                leftColor = rightColor = enemy ? ui::Color_Red : ui::Color_Fire+5;
                textColor = ui::Color_Yellow;
                text      = enemy ? m_translator(STATUS_LABELS[4]) : m_translator(STATUS_LABELS[0]);
            }
        }

        // Right part: status
        if (leftColor == rightColor) {
            gfx::drawSolidBar(rightContext, area, leftColor);
        } else {
            int split = (area.getWidth() + area.getHeight())/2;
            int x = area.getLeftX();
            int y = area.getTopY();
            for (int dy = 0; dy < area.getHeight(); ++dy) {
                rightContext.setColor(leftColor);
                drawHLine(rightContext, x,         y, x + split-1);
                rightContext.setColor(rightColor);
                drawHLine(rightContext, x + split, y, x + area.getWidth()-1);
                --split;
                ++y;
            }
        }

        rightContext.setColor(textColor);
        rightContext.setTextAlign(gfx::CenterAlign, gfx::TopAlign);
        outTextF(rightContext, area, text);
        ui::drawFrameUp(rightContext, area);
    } else {
        // Does not exist. Still fill the area.
        prepareColorListItem(leftContext, area, state, m_root.colorScheme(), del);
    }
}

void
client::widgets::AllianceStatusList::handlePositionChange(gfx::Rectangle& oldPosition)
{
    defaultHandlePositionChange(oldPosition);
}

ui::layout::Info
client::widgets::AllianceStatusList::getLayoutInfo() const
{
    // Compute widths. Pass 0 as availableWidth so we'll be returned the minimum possible values.
    int leftWidth, rightWidth;
    computeWidth(leftWidth, rightWidth, 0);

    // Layout constraints:
    // - we will need at minimum 11 lines (=standard VGAP), but will take as many as will fit on the screen if the game has more
    // - we prefer to have an extra margin of 50 pixels to make it look nice, but don't insist on it
    int itemHeight = getItemHeight();
    int numItems = static_cast<int>(m_items.size());
    return ui::layout::Info(gfx::Point(leftWidth + rightWidth, itemHeight * std::min(numItems, 11)),
                            gfx::Point(leftWidth + rightWidth + 50, itemHeight * numItems),
                            ui::layout::Info::Fixed);
}

bool
client::widgets::AllianceStatusList::handleKey(util::Key_t key, int prefix)
{
    // ex WAllyList::handleEvent (part)
    if (key == ' ') {
        if (int player = getCurrentPlayer()) {
            sig_toggleAlliance.raise(player);
        }
        return true;
    } else {
        return defaultHandleKey(key, prefix);
    }
}

int
client::widgets::AllianceStatusList::getItemHeight() const
{
    return m_root.provider().getFont(gfx::FontRequest())->getCellSize().getY();
}

void
client::widgets::AllianceStatusList::computeWidth(int& leftWidth, int& rightWidth, int availableWidth) const
{
    Ref<Font> font = m_root.provider().getFont(gfx::FontRequest());

    // Right width
    rightWidth = font->getMaxTextWidth(afl::functional::createStringTable(STATUS_LABELS).map(m_translator));
    rightWidth += 10;

    // Left width
    leftWidth = 0;
    for (size_t i = 0; i < m_items.size(); ++i) {
        leftWidth = std::max(leftWidth, font->getTextWidth(m_items[i].name));
    }
    leftWidth += 10;

    // If we have more room than we actually need, extend the left side
    if (availableWidth > leftWidth + rightWidth) {
        leftWidth = availableWidth - rightWidth;
    }
}

void
client::widgets::AllianceStatusList::onItemClickAt(size_t /*item*/, gfx::Point relativePosition)
{
    int player = getCurrentPlayer();
    int leftWidth, rightWidth;
    computeWidth(leftWidth, rightWidth, getExtent().getWidth());
    if (relativePosition.getX() >= leftWidth && player != 0) {
        sig_toggleAlliance.raise(player);
    }
}

void
client::widgets::AllianceStatusList::onChange()
{
    sig_selectPlayer.raise(getCurrentPlayer());
}
