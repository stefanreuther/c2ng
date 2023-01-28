/**
  *  \file ui/widgets/combobox.hpp
  */
#ifndef C2NG_UI_WIDGETS_COMBOBOX_HPP
#define C2NG_UI_WIDGETS_COMBOBOX_HPP

#include "ui/widgets/numberselector.hpp"
#include "util/stringlist.hpp"
#include "ui/root.hpp"

namespace ui { namespace widgets {

    class ComboBox : public NumberSelector {
     public:
        ComboBox(Root& root, afl::base::Observable<int32_t>& value, int32_t min, int32_t max, const util::StringList& list);
        ~ComboBox();

        void popupMenu();

        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        Widget& addButtons(afl::base::Deleter& del);

     private:
        Root& m_root;
        util::StringList m_list;
        gfx::FontRequest m_font;
    };

} }

#endif
