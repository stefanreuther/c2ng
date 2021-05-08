/**
  *  \file ui/widgets/abstractcheckbox.cpp
  *  \brief Class ui::widgets::AbstractCheckbox
  */

#include "ui/widgets/abstractcheckbox.hpp"
#include "gfx/context.hpp"
#include "gfx/complex.hpp"

namespace {
    const int IMAGE_PAD = 2;
    const int TEXT_LEFT_PAD = 5;
}

ui::widgets::AbstractCheckbox::AbstractCheckbox(ui::Root& root, util::Key_t key, String_t text, gfx::Point imageSize)
    : BaseButton(root, key),
      m_image(imageSize),
      m_focus(m_image),
      m_text(text, root),
      m_hbox(),
      m_imageName(),
      conn_imageChange()
{
    // Populate HBox
    m_hbox.add(m_focus);
    if (!text.empty()) {
        m_hbox.add(m_text);
    }
    m_hbox.setPad(TEXT_LEFT_PAD);
    m_focus.setPad(IMAGE_PAD);

    // Clicking the widget focuses it
    setIsFocusable(true);

    // Redraw when possible images change
    conn_imageChange = root.provider().sig_imageChange.add(this, &AbstractCheckbox::updateImage);

    // Make visible
    setIcon(m_hbox);
}

ui::widgets::AbstractCheckbox::~AbstractCheckbox()
{ }

void
ui::widgets::AbstractCheckbox::setFont(gfx::FontRequest font)
{
    if (m_text.setFont(font)) {
        requestRedraw();
    }
}

void
ui::widgets::AbstractCheckbox::setImage(String_t imageName)
{
    m_imageName = imageName;
    updateImage();
}

void
ui::widgets::AbstractCheckbox::updateImage()
{
    if (m_image.setImage(root().provider().getImage(m_imageName))) {
        requestRedraw();
    }
}
