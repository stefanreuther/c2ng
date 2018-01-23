/**
  *  \file ui/widgets/imagebutton.cpp
  *  \brief Class ui::widgets::ImageButton
  *
  *  \change This is the successor-in-spirit to UIImageWidget and WPixmapButton.
  *  It does NOT automatically add a frame, use FrameGroup for that.
  *  It does however accept user interaction like a normal button;
  *  we get that for free by deriving from AbstractButton and it does not hurt.
  */

#include "ui/widgets/imagebutton.hpp"
#include "gfx/context.hpp"
#include "gfx/complex.hpp"

ui::widgets::ImageButton::ImageButton(String_t image, util::Key_t key, ui::Root& root, gfx::Point size)
    : AbstractButton(root, key),
      m_image(image),
      m_size(size),
      conn_imageChange(root.provider().sig_imageChange.add(this, &ImageButton::onImageChange)),
      m_text(),
      m_font()
{
    m_font.addSize(-1);
}

ui::widgets::ImageButton::~ImageButton()
{ }

void
ui::widgets::ImageButton::draw(gfx::Canvas& can)
{
    // ex UIImageWidget::drawContent (sort-of), WPixmapButton::drawContent

    // Color scheme/context
    {
        gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());

        // Draw background. The image may have transparency, so we must produce a solid color.
        drawBackground(ctx, getExtent());

        // Draw the image.
        afl::base::Ptr<gfx::Canvas> image = root().provider().getImage(m_image);
        if (image.get() != 0) {
            // FIXME: 20180101: blitSized() assumes that the image is fully opaque.
            blitSized(ctx, getExtent(), *image);
        }
    }

    // Draw the text, if any.
    if (!m_text.empty()) {
        // Config:
        const size_t N = 9;
        static const uint8_t spec[N][4] = {
            { 0, 0, Color_Black, 128 },
            { 0, 2, Color_Black, 128 },
            { 2, 0, Color_Black, 128 },
            { 2, 2, Color_Black, 128 },
            { 0, 1, Color_Black, gfx::OPAQUE_ALPHA },
            { 1, 0, Color_Black, gfx::OPAQUE_ALPHA },
            { 2, 1, Color_Black, gfx::OPAQUE_ALPHA },
            { 1, 2, Color_Black, gfx::OPAQUE_ALPHA },
            { 1, 1, Color_White, gfx::OPAQUE_ALPHA }
        };

        // Draw
        afl::base::Ref<gfx::Font> font = root().provider().getFont(m_font);
        gfx::Context<uint8_t> ctx(can, root().colorScheme());
        ctx.useFont(*font);
        ctx.setTextAlign(0, 0);
        int x = getExtent().getLeftX();
        int y = getExtent().getBottomY() - font->getTextHeight(m_text);
        int w = getExtent().getWidth();
        for (size_t i = 0; i < N; ++i) {
            ctx.setColor(spec[i][2]);
            ctx.setAlpha(spec[i][3]);
            outTextF(ctx, gfx::Point(x + spec[i][0], y + spec[i][1]), w, m_text);
        }
    }
}

void
ui::widgets::ImageButton::handleStateChange(State st, bool enable)
{
    defaultHandleStateChange(st, enable);
}

void
ui::widgets::ImageButton::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{ }

ui::layout::Info
ui::widgets::ImageButton::getLayoutInfo() const
{
    return m_size;
}

bool
ui::widgets::ImageButton::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
ui::widgets::ImageButton::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}

void
ui::widgets::ImageButton::setImage(String_t image)
{
    if (image != m_image) {
        m_image = image;
        requestRedraw();
    }
}

void
ui::widgets::ImageButton::setText(String_t text)
{
    if (text != m_text) {
        m_text = text;
        requestRedraw();
    }
}

void
ui::widgets::ImageButton::onImageChange()
{
    requestRedraw();
}

// FIXME: this method is missing. Allow the caller to supply a ready-made pixmap object.
// This would override the default pixmap-by-name.
//
// void
// UIImageWidget::setImage(Ptr<GfxPixmap> pix)
// {
//     this->pix = pix;
//     drawWidget(false);
// }

