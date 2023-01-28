/**
  *  \file client/widgets/techbar.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_TECHBAR_HPP
#define C2NG_CLIENT_WIDGETS_TECHBAR_HPP

#include "ui/widgets/numberselector.hpp"
#include "ui/root.hpp"

namespace client { namespace widgets {

    class TechBar : public ui::widgets::NumberSelector {
     public:
        TechBar(ui::Root& root, afl::base::Observable<int32_t>& value, int32_t low, int32_t high, String_t name);
        ~TechBar();

        // SimpleWidget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        ui::Root& m_root;
        String_t m_name;

        gfx::Rectangle getBarPosition() const;
    };

} }

#endif
