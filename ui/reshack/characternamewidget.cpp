/**
  *  \file ui/reshack/characternamewidget.cpp
  *  \brief Class ui::reshack::CharacterNameWidget
  */

#include "ui/reshack/characternamewidget.hpp"
#include "gfx/context.hpp"

namespace {
    const afl::charset::Unichar_t NIL = afl::charset::Unichar_t(-1);
}

ui::reshack::CharacterNameWidget::CharacterNameWidget(Root& root, util::CharacterNameList& list)
    : m_list(list),
      m_root(root),
      m_character(NIL)
{
    // RHCharacterName::RHCharacterName()
}

void
ui::reshack::CharacterNameWidget::setCharacter(afl::charset::Unichar_t ch)
{
    // RHCharacterName::setCharacter(uint32_t character)
    if (ch != m_character) {
        m_character = ch;
        requestRedraw();
    }
}

void
ui::reshack::CharacterNameWidget::draw(gfx::Canvas& can)
{
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    ctx.useFont(*m_root.provider().getFont(""));
    ctx.setTextAlign(gfx::LeftAlign, gfx::MiddleAlign);
    ctx.setSolidBackground();
    ctx.setColor(util::SkinColor::Static);
    outTextF(ctx, getExtent(), m_character == NIL ? String_t() : m_list.getCharacterName(m_character));
}

void
ui::reshack::CharacterNameWidget::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
ui::reshack::CharacterNameWidget::handlePositionChange()
{
    requestRedraw();
}

ui::layout::Info
ui::reshack::CharacterNameWidget::getLayoutInfo() const
{
    // RHCharacterName::getLayoutInfo(LayoutInfo& info)
    return ui::layout::Info(m_root.provider().getFont("")->getCellSize().scaledBy(20, 1),
                            ui::layout::Info::GrowHorizontal);
}

bool
ui::reshack::CharacterNameWidget::handleKey(util::Key_t /*key*/, int /*prefix*/)
{
    return false;
}

bool
ui::reshack::CharacterNameWidget::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/)
{
    return false;
}
