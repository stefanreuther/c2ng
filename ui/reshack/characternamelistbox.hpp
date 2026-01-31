/**
  *  \file ui/reshack/characternamelistbox.hpp
  *  \brief Class ui::reshack::CharacterNameListbox
  */
#ifndef C2NG_UI_RESHACK_CHARACTERNAMELISTBOX_HPP
#define C2NG_UI_RESHACK_CHARACTERNAMELISTBOX_HPP

#include "ui/root.hpp"
#include "ui/widgets/abstractlistbox.hpp"
#include "util/characternamelist.hpp"

namespace ui { namespace reshack {

    /** Listbox displaying a list of character names.
        Use setCharacters() to set the list. */
    class CharacterNameListbox : public ui::widgets::AbstractListbox {
     public:
        /** Constructor.
            @param root   UI root
            @param names  Character names */
        CharacterNameListbox(Root& root, util::CharacterNameList& names);

        /** Set list of characters to display.
            @param list List; will be copied */
        void setCharacters(const util::CharacterNameList::CharacterList_t& list);

        /** Get current character.
            @return character code; 0 if none (list is empty) */
        afl::charset::Unichar_t getCurrentCharacter() const;

        /** Set current character.
            If this character is in the list being displayed, places cursor on it.
            @param ch Character code */
        void setCurrentCharacter(afl::charset::Unichar_t ch);

        // AbstractListbox:
        virtual size_t getNumItems() const;
        virtual bool isItemAccessible(size_t n) const;
        virtual int getItemHeight(size_t n) const;
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);

     private:
        Root& m_root;
        util::CharacterNameList& m_names;
        util::CharacterNameList::CharacterList_t m_characters;
    };

} }

#endif
