/**
  *  \file ui/reshack/characternamelistbox.cpp
  *  \brief Class ui::reshack::CharacterNameListbox
  */

#include "ui/reshack/characternamelistbox.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"

ui::reshack::CharacterNameListbox::CharacterNameListbox(Root& root, util::CharacterNameList& names)
    : m_root(root), m_names(names), m_characters()
{ }

void
ui::reshack::CharacterNameListbox::setCharacters(const util::CharacterNameList::CharacterList_t& list)
{
    // Commented out the "preserve current character" code.
    // Using the widget "feels" better without it.

    // afl::charset::Unichar_t ch = getCurrentCharacter();
    m_characters = list;
    handleModelChange();
    // if (ch != 0) {
    //     setCurrentCharacter(ch);
    // }
}

afl::charset::Unichar_t
ui::reshack::CharacterNameListbox::getCurrentCharacter() const
{
    size_t i = getCurrentItem();
    return (i < m_characters.size() ? m_characters[i] : 0);
}

void
ui::reshack::CharacterNameListbox::setCurrentCharacter(afl::charset::Unichar_t ch)
{
    for (size_t i = 0; i < m_characters.size(); ++i) {
        if (ch == m_characters[i]) {
            setCurrentItem(i);
            break;
        }
    }
}

size_t
ui::reshack::CharacterNameListbox::getNumItems() const
{
    return m_characters.size();
}

bool
ui::reshack::CharacterNameListbox::isItemAccessible(size_t /*n*/) const
{
    return true;
}

int
ui::reshack::CharacterNameListbox::getItemHeight(size_t /*n*/) const
{
    return m_root.provider().getFont(gfx::FontRequest())->getLineHeight();
}

void
ui::reshack::CharacterNameListbox::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    // RHCharacterNameList::drawPart(GfxCanvas& can, int from, int to)
    afl::base::Deleter del;
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    ctx.useFont(*m_root.provider().getFont(gfx::FontRequest()));

    prepareColorListItem(ctx, area, state, m_root.colorScheme(), del);
    area.consumeX(5);

    if (item < m_characters.size()) {
        outTextF(ctx, area, m_names.getCharacterName(m_characters[item]));
    }
}

void
ui::reshack::CharacterNameListbox::handlePositionChange()
{
    defaultHandlePositionChange();
}

ui::layout::Info
ui::reshack::CharacterNameListbox::getLayoutInfo() const
{
    return ui::layout::Info(gfx::Point(15 * getItemHeight(0), 200));
}

bool
ui::reshack::CharacterNameListbox::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}
