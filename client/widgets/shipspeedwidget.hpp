/**
  *  \file client/widgets/shipspeedwidget.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_SHIPSPEEDWIDGET_HPP
#define C2NG_CLIENT_WIDGETS_SHIPSPEEDWIDGET_HPP

#include "ui/widgets/numberselector.hpp"
#include "ui/root.hpp"

namespace client { namespace widgets {

    class ShipSpeedWidget : public ui::widgets::NumberSelector {
     public:
        ShipSpeedWidget(afl::base::Observable<int32_t>& value, int32_t limit, int32_t hyp, int32_t opt, ui::Root& root);

        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        int32_t m_hyp;
        int32_t m_optimum;
        ui::Root& m_root;
    };

} }

#endif
