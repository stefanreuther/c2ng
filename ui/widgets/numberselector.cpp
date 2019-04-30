/**
  *  \file ui/widgets/numberselector.cpp
  */

#include <algorithm>
#include "ui/widgets/numberselector.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/widgets/button.hpp"

// /** Constructor.
//     \param id Widget Id
//     \param low Lower limit
//     \param high Upper limit
//     \param set Default step size */
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

// /** Set value. Checks ranges and clips the value into range. */
void
ui::widgets::NumberSelector::setValue(int32_t value)
{
    // ex UINumberSelector::setValue
    m_value.set(std::max(m_min, std::min(m_max, value)));
}

int32_t
ui::widgets::NumberSelector::getValue() const
{
    return m_value.get();
}

int32_t
ui::widgets::NumberSelector::getMin() const
{
    return m_min;
}

int32_t
ui::widgets::NumberSelector::getMax() const
{
    return m_max;
}

int32_t
ui::widgets::NumberSelector::getStep() const
{
    return m_step;
}

// /** Set range. Clips value into range if it is outside. */
void
ui::widgets::NumberSelector::setRange(int32_t min, int32_t max)
{
    // ex UINumberSelector::setRange
    m_min = min;
    m_max = max;
    setValue(getValue());
}

void
ui::widgets::NumberSelector::increment(int32_t n)
{
    // ex UINumberSelector::increment
    // FIXME: deal with overflow
    if (n == 0) {
        n = 1;
    }
    requestActive();
    setValue(getValue() + n);
}

void
ui::widgets::NumberSelector::decrement(int32_t n)
{
    // ex UINumberSelector::decrement
    // FIXME: deal with overflow
    if (n == 0) {
        n = 1;
    }
    requestActive();
    setValue(getValue() - n);
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

afl::base::Observable<int32_t>&
ui::widgets::NumberSelector::value()
{
    return m_value;
}

ui::Widget&
ui::widgets::NumberSelector::addButtons(afl::base::Deleter& del, Root& root)
{
    // ex UINumberSelector::doStandardDialog (part)
    Group& g = del.addNew(new Group(ui::layout::HBox::instance5));
    Button& btnMinus = del.addNew(new Button("-", '-', root));
    Button& btnPlus  = del.addNew(new Button("+", '+', root));
    btnMinus.dispatchKeyTo(*this);
    btnPlus.dispatchKeyTo(*this);
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
