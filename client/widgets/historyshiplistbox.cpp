/**
  *  \file client/widgets/historyshiplistbox.cpp
  *  \brief Class client::widgets::HistoryShipListbox
  */

#include "client/widgets/historyshiplistbox.hpp"
#include "client/widgets/referencelistbox.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"
#include "util/string.hpp"

using game::ref::UserList;

namespace {
    const int TotalWidth = 30;   // ems
    const int AgeWidth = 7;      // ems
    const int PadWidth = 5;      // px
}


client::widgets::HistoryShipListbox::HistoryShipListbox(ui::Root& root, afl::string::Translator& tx)
    : m_root(root),
      m_translator(tx),
      m_content(),
      m_pendingReference(),
      m_numLines(15),
      m_width(root.provider().getFont(gfx::FontRequest())->getCellSize().getX() * TotalWidth)
{ }

client::widgets::HistoryShipListbox::~HistoryShipListbox()
{ }

void
client::widgets::HistoryShipListbox::setNumLines(int n)
{
    m_numLines = n;
}

void
client::widgets::HistoryShipListbox::setWidth(int width)
{
    m_width = width;
}

void
client::widgets::HistoryShipListbox::setContent(const game::ref::HistoryShipList& list)
{
    // ex WHistoryShipList::updateList, WObjectList::handleListChange (sort-of)
    // FIXME: optimize for common case of just one item changed (=selection toggle)?
    // Find new position
    size_t newPos = 0;
    if (!list.empty()) {
        if (m_pendingReference.isSet()) {
            newPos = list.find(m_pendingReference).orElse(0);
        } else {
            if (const Item_t* pItem = getItem(getCurrentItem())) {
                newPos = list.find(pItem->reference).orElse(0);
            }
        }
        m_pendingReference = game::Reference();
    }

    // Update
    m_content = list;
    setCurrentItem(newPos);
    handleModelChange();
}

void
client::widgets::HistoryShipListbox::setCurrentReference(game::Reference ref)
{
    if (m_content.empty()) {
        m_pendingReference = ref;
    } else {
        size_t pos = 0;
        if (m_content.find(ref).get(pos)) {
            setCurrentItem(pos);
        }
    }
}

game::Reference
client::widgets::HistoryShipListbox::getCurrentReference() const
{
    if (const Item_t* p = getItem(getCurrentItem())) {
        return p->reference;
    } else {
        return m_pendingReference;
    }
}

size_t
client::widgets::HistoryShipListbox::getNumItems() const
{
    return m_content.size();
}

bool
client::widgets::HistoryShipListbox::isItemAccessible(size_t n) const
{
    // ex WObjectList::isAccessible
    bool ok = false;
    if (const Item_t* it = getItem(n)) {
        switch (it->type) {
         case UserList::OtherItem:
         case UserList::ReferenceItem:
            ok = true;
            break;

         case UserList::DividerItem:
         case UserList::SubdividerItem:
            ok = false;
            break;
        }
    }
    return ok;
}

int
client::widgets::HistoryShipListbox::getItemHeight(size_t /*n*/) const
{
    return m_root.provider().getFont(gfx::FontRequest())->getCellSize().getY();
}

int
client::widgets::HistoryShipListbox::getHeaderHeight() const
{
    return 0;
}

int
client::widgets::HistoryShipListbox::getFooterHeight() const
{
    return 0;
}

void
client::widgets::HistoryShipListbox::drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::HistoryShipListbox::drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::HistoryShipListbox::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    // ex WObjectList::drawPart, WHistoryShipList::getInfoForObject
    // Prepare
    afl::base::Deleter d;
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    ui::prepareColorListItem(ctx, area, state, m_root.colorScheme(), d);

    // Draw
    if (const Item_t* it = getItem(item)) {
        if (it->type == UserList::ReferenceItem) {
            // Item is shown with age
            ctx.setColor(it->color);
            ctx.useFont(*m_root.provider().getFont(gfx::FontRequest()));
            ctx.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
            gfx::Rectangle ageArea = area.splitRightX(ctx.getFont()->getEmWidth() * AgeWidth);
            if (it->turnNumber == 0) {
                outTextF(ctx, ageArea, m_translator("unknown"));
            } else {
                outTextF(ctx, ageArea, util::formatAge(m_content.getReferenceTurn(), it->turnNumber, m_translator));
            }
            area.consumeRightX(PadWidth);

            ReferenceListbox::drawItem(ctx, area, *it, m_root.provider());
        } else {
            // Just draw it normally
            ReferenceListbox::drawItem(ctx, area, *it, m_root.provider());
        }
    }
}

void
client::widgets::HistoryShipListbox::handlePositionChange()
{
    defaultHandlePositionChange();
}

ui::layout::Info
client::widgets::HistoryShipListbox::getLayoutInfo() const
{
    gfx::Point size(m_width, m_root.provider().getFont(gfx::FontRequest())->getCellSize().getY() * m_numLines);
    return ui::layout::Info(size, size, ui::layout::Info::GrowBoth);
}

bool
client::widgets::HistoryShipListbox::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

const client::widgets::HistoryShipListbox::Item_t*
client::widgets::HistoryShipListbox::getItem(size_t index) const
{
    return m_content.get(index);
}
