/**
  *  \file client/widgets/cargotransferline.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_CARGOTRANSFERLINE_HPP
#define C2NG_CLIENT_WIDGETS_CARGOTRANSFERLINE_HPP

#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "ui/root.hpp"
#include "ui/widget.hpp"
#include "ui/widgets/button.hpp"
#include "util/numberformatter.hpp"

namespace client { namespace widgets {

    class CargoTransferLine : public ui::Widget {
     public:
        CargoTransferLine(ui::Root& root, afl::string::Translator& tx, String_t name, int id, util::NumberFormatter fmt);

        void setAmounts(bool right, int32_t available, int32_t remaining);

        // Widget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void requestChildRedraw(Widget& child, const gfx::Rectangle& area);
        virtual void handleChildAdded(Widget& child);
        virtual void handleChildRemove(Widget& child);
        virtual void handlePositionChange();
        virtual void handleChildPositionChange(Widget& child, const gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        // id,target,amount
        afl::base::Signal<void(int,bool,int)> sig_move;

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        String_t m_name;
        int m_id;
        util::NumberFormatter m_numberFormatter;
        int32_t m_available[2];
        int32_t m_remaining[2];
        ui::widgets::Button m_moveLeft;
        ui::widgets::Button m_moveRight;

        void drawAmounts(gfx::Canvas& can, bool right, gfx::Rectangle area);
    };

} }

#endif
