/**
  *  \file ui/widgets/keyforwarder.hpp
  *  \brief Class ui::widgets::KeyForwarder
  */
#ifndef C2NG_UI_WIDGETS_KEYFORWARDER_HPP
#define C2NG_UI_WIDGETS_KEYFORWARDER_HPP

#include "gfx/keyeventconsumer.hpp"
#include "ui/invisiblewidget.hpp"

namespace ui { namespace widgets {

    class KeyForwarder : public InvisibleWidget {
     public:
        explicit KeyForwarder(gfx::KeyEventConsumer& consumer);
        virtual bool handleKey(util::Key_t key, int prefix);

     private:
        gfx::KeyEventConsumer& m_consumer;
    };

} }

#endif
