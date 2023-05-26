/**
  *  \file client/widgets/combatunitlist.cpp
  */

#include "client/widgets/combatunitlist.hpp"
#include "gfx/context.hpp"
#include "afl/base/deleter.hpp"
#include "util/unicodechars.hpp"
#include "ui/draw.hpp"

client::widgets::CombatUnitList::CombatUnitList(ui::Root& root)
    : m_root(root), m_items()
{ }

client::widgets::CombatUnitList::~CombatUnitList()
{ }

void
client::widgets::CombatUnitList::clear()
{
    m_items.clear();
    handleModelChange();
}

void
client::widgets::CombatUnitList::addItem(Kind k, size_t slot, String_t label, Flags_t flags, util::SkinColor::Color color)
{
    // ex ulAddItem(me, klass, label, isFleet, id)
    m_items.push_back(Item(k, slot, flags, label, color));
    handleModelChange();
}

bool
client::widgets::CombatUnitList::findItem(Kind k, size_t slot, size_t& index) const
{
    // ex ulFind(me, isFleet, id, index)
    for (size_t i = 0, n = m_items.size(); i < n; ++i) {
        if (m_items[i].kind == k && m_items[i].slot == slot) {
            index = i;
            return true;
        }
    }
    return false;
}

bool
client::widgets::CombatUnitList::getItem(size_t index, Kind& k, size_t& slot) const
{
    if (index < m_items.size()) {
        k = m_items[index].kind;
        slot = m_items[index].slot;
        return true;
    } else {
        return false;
    }
}

void
client::widgets::CombatUnitList::setFlagBySlot(Kind k, size_t slot, Flag flag, bool set)
{
    size_t index;
    if (findItem(k, slot, index)) {
        setFlagByIndex(index, flag, set);
    }
}

void
client::widgets::CombatUnitList::setFlagByIndex(size_t index, Flag flag, bool set)
{
    // ex ulMarkDead(me, isFleet, id, flag), ulMarkFollowed(me, fleetNr)
    if (index < m_items.size()) {
        m_items[index].flags.set(flag, set);
        updateItem(index);
    }
}

bool
client::widgets::CombatUnitList::getCurrentFleet(size_t& slot) const
{
    // ex ulGetCurrentFleet(me)
    size_t i = getCurrentItem();
    while (i > 0 && i < m_items.size() && m_items[i].kind != Fleet) {
        --i;
    }
    if (i < m_items.size() && m_items[i].kind == Fleet) {
        slot = m_items[i].slot;
        return true;
    } else {
        return false;
    }
}

bool
client::widgets::CombatUnitList::getCurrentShip(size_t& slot) const
{
    // ex ulGetCurrentShip(me)
    size_t i = getCurrentItem();
    if (i < m_items.size() && m_items[i].kind == Unit) {
        slot = m_items[i].slot;
        return true;
    } else {
        return false;
    }
}

size_t
client::widgets::CombatUnitList::getNumItems() const
{
    return m_items.size();
}

bool
client::widgets::CombatUnitList::isItemAccessible(size_t n) const
{
    return n < m_items.size() && !m_items[n].flags.contains(Inaccessible);
}

int
client::widgets::CombatUnitList::getItemHeight(size_t /*n*/) const
{
    return getFont()->getLineHeight();
}

int
client::widgets::CombatUnitList::getHeaderHeight() const
{
    return 0;
}

int
client::widgets::CombatUnitList::getFooterHeight() const
{
    return 0;
}

void
client::widgets::CombatUnitList::drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::CombatUnitList::drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
client::widgets::CombatUnitList::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    afl::base::Ref<gfx::Font> font(getFont());
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    afl::base::Deleter del;
    prepareColorListItem(ctx, area, state, m_root.colorScheme(), del);
    ctx.useFont(*font);

    if (item < m_items.size()) {
        const Item& it = m_items[item];
        ctx.setColor(it.flags.contains(Dead) ? util::SkinColor::Faded : it.color);

        String_t text;
        switch (it.kind) {
         case Unit:
            area.consumeX(font->getEmWidth());
            break;
         case Fleet:
            text = " ";
            break;
        }
        if (it.flags.contains(Tagged)) {
            text = UTF_BULLET;
        }
        outTextF(ctx, area, text + m_items[item].label);
    }
}

void
client::widgets::CombatUnitList::handlePositionChange()
{
    return defaultHandlePositionChange();
}

ui::layout::Info
client::widgets::CombatUnitList::getLayoutInfo() const
{
    // ex FlakShipList::getLayoutInfo
    gfx::Point size = getFont()->getCellSize().scaledBy(12, 20);
    return ui::layout::Info(size, ui::layout::Info::GrowBoth);
}

bool
client::widgets::CombatUnitList::handleKey(util::Key_t key, int prefix)
{
    size_t n;
    switch (key) {
     case util::Key_Up + util::KeyMod_Ctrl:
        n = getCurrentItem();
        while (n > 0 && n < m_items.size()) {
            --n;
            if (!m_items[n].flags.contains(Dead) && !m_items[n].flags.contains(Inaccessible)) {
                setCurrentItem(n);
                break;
            }
        }
        return true;

     case util::Key_Down + util::KeyMod_Ctrl:
        n = getCurrentItem();
        while (++n < m_items.size()) {
            if (!m_items[n].flags.contains(Dead) && !m_items[n].flags.contains(Inaccessible)) {
                setCurrentItem(n);
                break;
            }
        }
        return true;

     default:
        return defaultHandleKey(key, prefix);
    }
}

afl::base::Ref<gfx::Font>
client::widgets::CombatUnitList::getFont() const
{
    return m_root.provider().getFont(gfx::FontRequest());
}
