/**
  *  \file ui/widgets/checkbox.cpp
  *  \brief Class ui::widgets::Checkbox
  */

#include "ui/widgets/checkbox.hpp"

ui::widgets::Checkbox::Checkbox(ui::Root& root, util::Key_t key, String_t text, afl::base::Observable<int>& value)
    : AbstractCheckbox(root, key, text, gfx::Point(20, 20)),
      m_imageMap(),
      m_value(value),
      conn_change(m_value.sig_change.add(this, &Checkbox::onChange))
{
    sig_fire.add(this, &Checkbox::onClick);
    updateImage();
}

void
ui::widgets::Checkbox::addImage(int id, String_t name)
{
    // Add to image map
    m_imageMap.add(id, name);

    // If it should currently be shown, request it
    if (id == m_value.get()) {
        setImage(name);
    }

    // Preload
    root().provider().getImage(name);
}

void
ui::widgets::Checkbox::addDefaultImages()
{
    addImage(0, "ui.cb0");
    addImage(1, "ui.cb1");
}

afl::base::Observable<int>&
ui::widgets::Checkbox::value()
{
    return m_value;
}

void
ui::widgets::Checkbox::onClick()
{
    // ex UICheckbox::click
    // Find next index
    size_t index;
    if (m_imageMap.find(m_value.get()).get(index) && index+1 < m_imageMap.size()) {
        ++index;
    } else {
        index = 0;
    }

    // Find matching value
    int32_t value;
    String_t tmp;
    if (m_imageMap.get(index, value, tmp)) {
        m_value.set(value);        // triggers updateImage() etc.
    }
}

void
ui::widgets::Checkbox::onChange()
{
    // ex UICheckbox::setValue
    updateImage();
}

void
ui::widgets::Checkbox::updateImage()
{
    size_t index;
    int32_t value;
    String_t image;
    if (m_imageMap.find(m_value.get()).get(index) && m_imageMap.get(index, value, image)) {
        setImage(image);
    }
}
