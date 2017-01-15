/**
  *  \file ui/widgets/radiobutton.cpp
  *  \brief Class ui::widgets::RadioButton
  */

#include "ui/widgets/radiobutton.hpp"

namespace {
    const char RES_ON[] = "ui.radio1";
    const char RES_OFF[] = "ui.radio0";
}

ui::widgets::RadioButton::RadioButton(ui::Root& root, util::Key_t key, String_t text, afl::base::Observable<int>& value, int myValue)
    : AbstractCheckbox(root, key, text, gfx::Point(20, 20)),
      m_value(value),
      conn_change(value.sig_change.add(this, &RadioButton::onChange)),
      m_myValue(myValue)
{
    sig_fire.add(this, &RadioButton::onClick);
    updateImage();
}

afl::base::Observable<int>&
ui::widgets::RadioButton::value()
{
    return m_value;
}

bool
ui::widgets::RadioButton::isChecked() const
{
    return m_value.get() == m_myValue;
}

void
ui::widgets::RadioButton::onClick()
{
    // ex UIRadioButton::click
    m_value.set(m_myValue);
}

void
ui::widgets::RadioButton::onChange()
{
    updateImage();
}

void
ui::widgets::RadioButton::updateImage()
{
    // ex UIRadioButton::getPixmap
    if (isChecked()) {
        setImage(RES_ON);
    } else {
        setImage(RES_OFF);
    }
}
