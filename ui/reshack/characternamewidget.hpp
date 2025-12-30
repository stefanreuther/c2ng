/**
  *  \file ui/reshack/characternamewidget.hpp
  *  \brief Class ui::reshack::CharacterNameWidget
  */
#ifndef C2NG_UI_RESHACK_CHARACTERNAMEWIDGET_HPP
#define C2NG_UI_RESHACK_CHARACTERNAMEWIDGET_HPP

#include "ui/simplewidget.hpp"
#include "ui/root.hpp"
#include "util/characternamelist.hpp"

namespace ui { namespace reshack {

    /** Widget displaying a character name. */
    class CharacterNameWidget : public SimpleWidget {
     public:
        /** Constructor.
            @param root   UI root
            @param names  Character names */
        CharacterNameWidget(Root& root, util::CharacterNameList& list);

        /** Set character whose name is to be displayed.
            @param ch Unicode codepoint */
        void setCharacter(afl::charset::Unichar_t ch);

        // SimpleWidget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        util::CharacterNameList& m_list;
        Root& m_root;
        afl::charset::Unichar_t m_character;
    };

} }

#endif
