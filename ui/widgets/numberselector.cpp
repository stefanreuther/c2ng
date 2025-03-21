/**
  *  \file ui/widgets/numberselector.cpp
  *  \brief Class ui::widgets::NumberSelector
  */

#include <algorithm>
#include "ui/widgets/numberselector.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/widgets/button.hpp"

ui::widgets::NumberSelector::NumberSelector(afl::base::Observable<int32_t>& value, int32_t min, int32_t max, int32_t step)
    : SimpleWidget(),
      m_value(value),
      m_min(min),
      m_max(max),
      m_step(step),
      conn_change(value.sig_change.add(this, &NumberSelector::onChange))
{
    // ex UINumberSelector::UINumberSelector
}

ui::widgets::NumberSelector::~NumberSelector()
{ }

void
ui::widgets::NumberSelector::setValue(int32_t value)
{
    // ex UINumberSelector::setValue
    m_value.set(std::max(m_min, std::min(m_max, value)));
}

void
ui::widgets::NumberSelector::increment(int32_t n)
{
    // ex UINumberSelector::increment
    if (n <= 0) {
        n = 1;
    }
    requestActive();
    setValue(getValue() + std::min(m_max - getValue(), n));
}

void
ui::widgets::NumberSelector::decrement(int32_t n)
{
    // ex UINumberSelector::decrement
    if (n <= 0) {
        n = 1;
    }
    requestActive();
    setValue(getValue() - std::min(getValue() - m_min, n));
}

bool
ui::widgets::NumberSelector::defaultHandleKey(util::Key_t key, int prefix)
{
    // ex UINumberSelector::handleEvent
    if (hasState(FocusedState)) {
        switch (key) {
         case '+':
         case util::Key_Right:
            increment(prefix != 0 ? prefix : m_step);
            return true;

         case '-':
         case util::Key_Left:
            decrement(prefix != 0 ? prefix : m_step);
            return true;

         case util::KeyMod_Ctrl + '+':
         case util::KeyMod_Ctrl + util::Key_Right:
            increment(100);
            return true;

         case util::KeyMod_Ctrl + '-':
         case util::KeyMod_Ctrl + util::Key_Left:
            decrement(100);
            return true;

         case util::KeyMod_Shift + '+':
         case util::KeyMod_Shift + util::Key_Right:
            increment(1);
            return true;

         case util::KeyMod_Shift + '-':
         case util::KeyMod_Shift + util::Key_Left:
            decrement(1);
            return true;

         case util::KeyMod_Alt + '+':
         case util::KeyMod_Alt + util::Key_Right:
            requestActive();
            setValue(m_max);
            return true;

         case util::KeyMod_Alt + '-':
         case util::KeyMod_Alt + util::Key_Left:
            requestActive();
            setValue(m_min);
            return true;
        }
    }
    return false;
}

ui::Widget&
ui::widgets::NumberSelector::addButtons(afl::base::Deleter& del, Root& root)
{
    // ex UINumberSelector::doStandardDialog (part)
    Group& g = del.addNew(new Group(ui::layout::HBox::instance5));
    Button& btnMinus = del.addNew(new Button("-", '-', root));
    Button& btnPlus  = del.addNew(new Button("+", '+', root));
    btnMinus.dispatchKeyAndFocus(*this);
    btnPlus.dispatchKeyAndFocus(*this);
    g.add(btnMinus);
    g.add(*this);
    g.add(btnPlus);
    requestFocus();
    return g;
}

void
ui::widgets::NumberSelector::onChange()
{
    requestRedraw();
}
