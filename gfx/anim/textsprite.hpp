/**
  *  \file gfx/anim/textsprite.hpp
  */
#ifndef C2NG_GFX_ANIM_TEXTSPRITE_HPP
#define C2NG_GFX_ANIM_TEXTSPRITE_HPP

#include "gfx/resourceprovider.hpp"
#include "gfx/anim/sprite.hpp"

namespace gfx { namespace anim {

    class TextSprite : public Sprite {
     public:
        TextSprite(ResourceProvider& provider);
        ~TextSprite();

        void setFont(FontRequest font);
        void setPosition(Point pt);
        void setTextAlign(HorizontalAlignment x, VerticalAlignment y);
        void setText(const String_t& text);
        void setColor(Color_t color);

        virtual void tick();
        virtual void draw(Canvas& can);

     private:
        ResourceProvider& m_provider;
        FontRequest m_font;
        Point m_position;
        Point m_textAlign;
        String_t m_text;
        Color_t m_color;

        void updatePosition();
    };

} }

#endif
