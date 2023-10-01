/**
  *  \file ui/widgets/imagebutton.cpp
  *  \brief Class ui::widgets::ImageButton
  *
  *  \change This is the successor-in-spirit to UIImageWidget and WPixmapButton.
  *  It does NOT automatically add a frame, use FrameGroup for that.
  *  It does however accept user interaction like a normal button;
  *  we get that for free by deriving from BaseButton and it does not hurt.
  */

#include "ui/widgets/imagebutton.hpp"
#include "gfx/context.hpp"
#include "gfx/complex.hpp"

inline
ui::widgets::ImageButton::Icon::Icon(String_t imageName, Root& root, gfx::Point size)
    : m_imageName(imageName), m_root(root), m_size(size), m_font(), m_backgroundColor(-1)
{
    m_font.addSize(-1);
}

gfx::Point
ui::widgets::ImageButton::Icon::getSize() const
{
    return m_size;
}

void
ui::widgets::ImageButton::Icon::draw(gfx::Context<SkinColor::Color>& ctx, gfx::Rectangle area, ButtonFlags_t /*flags*/) const
{
    gfx::Context<uint8_t> ctx2(ctx.canvas(), m_root.colorScheme());

    // Draw background. The image may have transparency, so we must produce a solid color.
    if (m_backgroundColor < 0) {
        drawBackground(ctx, area);
    } else {
        drawSolidBar(ctx2, area, static_cast<uint8_t>(m_backgroundColor));
    }

    // Draw the image.
    afl::base::Ptr<gfx::Canvas> image = m_root.provider().getImage(m_imageName);
    if (image.get() != 0) {
        blitSized(ctx, area, *image);
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
        afl::base::Ref<gfx::Font> font = m_root.provider().getFont(m_font);
        ctx2.useFont(*font);
        ctx2.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
        int x = area.getLeftX();
        int y = area.getBottomY() - font->getTextHeight(m_text);
        int w = area.getWidth();
        for (size_t i = 0; i < N; ++i) {
            ctx2.setColor(spec[i][2]);
            ctx2.setAlpha(spec[i][3]);
            outTextF(ctx2, gfx::Point(x + spec[i][0], y + spec[i][1]), w, m_text);
        }
    }
}



ui::widgets::ImageButton::ImageButton(String_t image, util::Key_t key, ui::Root& root, gfx::Point size)
    : BaseButton(root, key),
      m_icon(image, root, size),
      conn_imageChange(root.provider().sig_imageChange.add(this, &ImageButton::onImageChange))
{
    setIcon(m_icon);
}

ui::widgets::ImageButton::~ImageButton()
{ }

void
ui::widgets::ImageButton::setImage(String_t image)
{
    if (image != m_icon.m_imageName) {
        m_icon.m_imageName = image;
        requestRedraw();
    }
}

void
ui::widgets::ImageButton::setText(String_t text)
{
    if (text != m_icon.m_text) {
        m_icon.m_text = text;
        requestRedraw();
    }
}

void
ui::widgets::ImageButton::setBackgroundColor(uint8_t color)
{
    if (color != m_icon.m_backgroundColor) {
        m_icon.m_backgroundColor = color;
        requestRedraw();
    }
}

void
ui::widgets::ImageButton::onImageChange()
{
    requestRedraw();
}
