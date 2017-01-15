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
    : AbstractButton(root, key),
      m_text(text),
      m_imageSize(imageSize),
      m_imageName(),
      m_font(),
      conn_imageChange()
{
    // Make sure that clicking the widget focuses it
    sig_fire.add(this, &Widget::requestFocus);

    // Redraw when possible images change
    conn_imageChange = root.provider().sig_imageChange.add(this, static_cast<void (Widget::*)()>(&Widget::requestRedraw));
}

ui::widgets::AbstractCheckbox::~AbstractCheckbox()
{ }

void
ui::widgets::AbstractCheckbox::draw(gfx::Canvas& can)
{
    // ex UIAbstractCheckbox::drawContent
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());

    // Draw background
    gfx::Rectangle area = getExtent();
    drawBackground(ctx, area);

    // Compute dimensions for image, leaving area with the area for the text
    const int desiredImageWidth = m_imageSize.getX() + 2*IMAGE_PAD;
    const int desiredImageHeight = m_imageSize.getY() + 2*IMAGE_PAD;
    gfx::Rectangle imageArea = area.splitX(desiredImageWidth);
    if (imageArea.getHeight() > desiredImageHeight) {
        imageArea.consumeY((imageArea.getHeight() - desiredImageHeight) / 2);
    }

    // Draw focus rectangle
    if (getFocusState() != NoFocus) {
        ctx.setColor(SkinColor::Static);
        drawRectangle(ctx, imageArea);
    }

    // Draw image
    if (imageArea.getWidth() > 0 && imageArea.getHeight() > 0) {
        afl::base::Ptr<gfx::Canvas> pix = root().provider().getImage(m_imageName);
        if (pix.get() != 0) {
            gfx::Point size = pix->getSize();
            if (size.getX() < imageArea.getWidth()) {
                imageArea.consumeX((imageArea.getWidth() - size.getX()) / 2);
                imageArea.setWidth(size.getX());
            }
            if (size.getY() < imageArea.getHeight()) {
                imageArea.consumeY((imageArea.getHeight() - size.getY()) / 2);
                imageArea.setHeight(size.getY());
            }
            blitSized(ctx, imageArea, *pix);
        }
    }

    // Draw text
    if (!m_text.empty()) {
        area.consumeX(TEXT_LEFT_PAD);
        ctx.setColor(hasState(DisabledState) ? SkinColor::Faded : SkinColor::Static);
        ctx.useFont(*root().provider().getFont(m_font));
        ctx.setTextAlign(0, 1);
        outTextF(ctx, area, m_text);
    }
}

void
ui::widgets::AbstractCheckbox::handleStateChange(State st, bool enable)
{
    defaultHandleStateChange(st, enable);
}

void
ui::widgets::AbstractCheckbox::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{ }

ui::layout::Info
ui::widgets::AbstractCheckbox::getLayoutInfo() const
{
    // ex UIAbstractCheckbox::getLayoutInfo (sort-of)
    // Size of image
    int width  = m_imageSize.getX() + 2*IMAGE_PAD;
    int height = m_imageSize.getY() + 2*IMAGE_PAD;

    // Size of text
    if (!m_text.empty()) {
        afl::base::Ref<gfx::Font> font = root().provider().getFont(m_font);
        width += TEXT_LEFT_PAD + font->getTextWidth(m_text);
        height = std::max(height, font->getTextHeight(m_text));
    }

    return ui::layout::Info(gfx::Point(width, height), gfx::Point(width, height), ui::layout::Info::GrowHorizontal);
}

bool
ui::widgets::AbstractCheckbox::handleKey(util::Key_t key, int prefix)
{
    // ex UIAbstractCheckbox::handleEvent (sort-of)
    if (!hasState(DisabledState) && hasState(FocusedState) && key == ' ') {
        requestActive();
        sig_fire.raise(prefix);
        return true;
    } else {
        return defaultHandleKey(key, prefix);
    }
}

bool
ui::widgets::AbstractCheckbox::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    // Request focus if mouse pressed in widget
    if (!(pressedButtons - DoubleClick).empty() && getExtent().contains(pt) && !hasState(DisabledState)) {
        requestActive();
        requestFocus();
    }

    // Otherwise, let the parent handle it
    return defaultHandleMouse(pt, pressedButtons);
}

void
ui::widgets::AbstractCheckbox::setFont(gfx::FontRequest font)
{
    m_font = font;
    requestRedraw();
}

void
ui::widgets::AbstractCheckbox::setImage(String_t imageName)
{
    if (m_imageName != imageName) {
        m_imageName = imageName;
        requestRedraw();
    }
}
