/**
  *  \file ui/widgets/combobox.cpp
  */

#include "ui/widgets/combobox.hpp"
#include "afl/string/format.hpp"
#include "gfx/context.hpp"
#include "gfx/complex.hpp"
#include "ui/widgets/focusablegroup.hpp"
#include "ui/widgets/button.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "ui/eventloop.hpp"
#include "ui/widgets/menuframe.hpp"

namespace {
    void removeAnnotation(String_t& label)
    {
        String_t::size_type pos = label.find('\t');
        if (pos != String_t::npos) {
            label.erase(pos);
        }
    }
}

ui::widgets::ComboBox::ComboBox(Root& root, afl::base::Observable<int32_t>& value, int32_t min, int32_t max, const util::StringList& list)
    : NumberSelector(value, min, max, 1),
      m_root(root),
      m_list(list),
      m_font("+")
{
    // ex UIComboBox::UIComboBox
}

ui::widgets::ComboBox::~ComboBox()
{ }

void
ui::widgets::ComboBox::popupMenu()
{
    // ex UIComboBox::popupMenu
    StringListbox list(m_root.provider(), m_root.colorScheme());
    util::StringList content(m_list);
    list.swapItems(content);
    list.setCurrentKey(value().get());

    EventLoop loop(m_root);
    bool ok = MenuFrame(ui::layout::HBox::instance0, m_root, loop).doMenu(list, getExtent().getBottomLeft());

    int32_t newValue;
    if (ok && list.getCurrentKey(newValue)) {
        value().set(newValue);
    }
}

void
ui::widgets::ComboBox::draw(gfx::Canvas& can)
{
    // ex UIComboBox::drawContent

    // Determine label
    int32_t key;
    String_t label;
    size_t index;
    if (m_list.find(value().get()).get(index) && m_list.get(index, key, label)) {
        removeAnnotation(label);
    } else {
        label = afl::string::Format("%d", value().get());
    }

    // Draw it
    gfx::Rectangle area(getExtent());
    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
    ctx.useFont(*m_root.provider().getFont(m_font));
    drawSolidBar(ctx, area, Color_Gray);
    if (getFocusState() != NoFocus) {
        ctx.setColor(Color_Blue);
    } else {
        ctx.setColor(Color_Black);
    }
    ctx.setTextAlign(gfx::LeftAlign, gfx::MiddleAlign);
    outTextF(ctx, area, label);
}

void
ui::widgets::ComboBox::handleStateChange(State st, bool /*enable*/)
{
    if (st == FocusedState) {
        requestRedraw();
    }
}

void
ui::widgets::ComboBox::handlePositionChange()
{
    requestRedraw();
}

ui::layout::Info
ui::widgets::ComboBox::getLayoutInfo() const
{
    // ex UIComboBox::getLayoutInfo
    afl::base::Ref<gfx::Font> font = m_root.provider().getFont(m_font);
    int height = font->getLineHeight();
    int width = 0;
    for (size_t i = 0, n = m_list.size(); i < n; ++i) {
        int32_t key;
        String_t label;
        if (m_list.get(i, key, label)) {
            removeAnnotation(label);
            width = std::max(width, font->getTextWidth(label));
        }
    }

    return ui::layout::Info(gfx::Point(width, height),
                            gfx::Point(width, height),
                            ui::layout::Info::GrowHorizontal);
}

bool
ui::widgets::ComboBox::handleKey(util::Key_t key, int prefix)
{
    if (key == ' ' || key == util::Key_Tab) {
        popupMenu();
        return true;
    } else {
        return defaultHandleKey(key, prefix);
    }
}

bool
ui::widgets::ComboBox::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    if (!pressedButtons.empty() && getExtent().contains(pt)) {
        requestFocus();
        return true;
    } else {
        return false;
    }
}

ui::Widget&
ui::widgets::ComboBox::addButtons(afl::base::Deleter& del)
{
    // ex createComboBoxControls
    // Note different invocation than original code!
    FocusableGroup& frame = del.addNew(new FocusableGroup(ui::layout::HBox::instance5, 5));
    Button& btnDec = del.addNew(new Button("-", '-', m_root));
    Button& btnInc = del.addNew(new Button("+", '+', m_root));
    Button& btnTab = del.addNew(new Button("\xEE\x85\x80", util::Key_Tab, m_root));

    frame.add(btnDec);
    frame.add(*this);
    frame.add(btnTab);
    frame.add(btnInc);

    btnInc.dispatchKeyAndFocus(*this);
    btnDec.dispatchKeyAndFocus(*this);
    btnTab.sig_fire.add(this, &ComboBox::popupMenu);
    requestFocus();

    return frame;
}

