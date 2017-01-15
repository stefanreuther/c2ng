/**
  *  \file ui/rich/draw.cpp
  *  \brief Simple Rich-Text Drawing Functions
  */

#include "ui/rich/draw.hpp"
#include "ui/rich/splitter.hpp"
#include "gfx/complex.hpp"
#include "gfx/colorscheme.hpp"

// Get width of text.
int
ui::rich::getTextWidth(const util::rich::Text& text, gfx::ResourceProvider& provider)
{
    return getTextSize(text, provider).getX();
}

// Get height of text.
int
ui::rich::getTextHeight(const util::rich::Text& text, gfx::ResourceProvider& provider)
{
    return getTextSize(text, provider).getY();
}

// Get size of text.
gfx::Point
ui::rich::getTextSize(const util::rich::Text& text, gfx::ResourceProvider& provider)
{
    class Sizer : public Splitter {
     public:
        Sizer(gfx::ResourceProvider& provider)
            : m_width(0),
              m_height(0),
              m_provider(provider)
            { }
        virtual void handlePart(const String_t& text, gfx::FontRequest font, bool /*isUnderlined*/, bool /*isKey*/, util::SkinColor::Color /*color*/)
            {
                if (!text.empty()) {
                    afl::base::Ref<gfx::Font> f = m_provider.getFont(font);
                    m_width += f->getTextWidth(text);
                    m_height = std::max(m_height, f->getTextHeight(text));
                }
            }
        virtual void handleOtherAttribute(const util::rich::Attribute& /*att*/, bool /*start*/)
            { }

        gfx::Point getResult() const
            { return gfx::Point(m_width, m_height); }

     private:
        int m_width;
        int m_height;
        gfx::ResourceProvider& m_provider;
    };

    Sizer sz(provider);
    sz.visit(text);
    return sz.getResult();
}

// Write line of rich text.
void
ui::rich::outText(gfx::Context<util::SkinColor::Color>& ctx, gfx::Point pt, const util::rich::Text& text, gfx::ResourceProvider& provider)
{
    class Writer : public Splitter {
     public:
        Writer(gfx::Context<util::SkinColor::Color>& ctx, gfx::Point pt, int yAlign, gfx::ResourceProvider& provider)
            : m_context(ctx),
              m_pos(pt),
              m_yAlign(yAlign),
              m_provider(provider)
            { }
        virtual void handlePart(const String_t& text, gfx::FontRequest font, bool isUnderlined, bool /*isKey*/, util::SkinColor::Color color)
            {
                if (!text.empty()) {
                    m_context.setColor(color);
                    afl::base::Ref<gfx::Font> f = m_provider.getFont(font);

                    // Find dimensions height (avoid call if not needed)
                    int height = (m_yAlign != 0 || isUnderlined) ? f->getTextHeight(text) : 0;
                    int width  = f->getTextWidth(text);

                    // Write aligned
                    gfx::Point pos = m_pos;
                    pos.addY(-m_yAlign * f->getTextHeight(text) / 2);
                    f->outText(m_context, pos, text);

                    // Underline
                    if (isUnderlined && width != 0) {
                        drawHLine(m_context, pos.getX(), pos.getY() + height*17/20, pos.getX() + width - 1);
                    }

                    // Advance X
                    m_pos.addX(f->getTextWidth(text));
                }
            }
        virtual void handleOtherAttribute(const util::rich::Attribute& /*att*/, bool /*start*/)
            { }

     private:
        gfx::Context<util::SkinColor::Color>& m_context;
        gfx::Point m_pos;
        const int m_yAlign;
        gfx::ResourceProvider& m_provider;
    };

    if (int xAlign = ctx.getTextAlign().getX()) {
        pt.addX(-xAlign * getTextWidth(text, provider) / 2);
    }

    Writer wr(ctx, pt, ctx.getTextAlign().getY(), provider);
    wr.visit(text);
}
