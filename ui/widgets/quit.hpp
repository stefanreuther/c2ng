/**
  *  \file ui/widgets/quit.hpp
  */
#ifndef C2NG_UI_WIDGETS_QUIT_HPP
#define C2NG_UI_WIDGETS_QUIT_HPP

#include "ui/invisiblewidget.hpp"
#include "ui/root.hpp"
#include "ui/eventloop.hpp"

namespace ui { namespace widgets {

    class Quit : public InvisibleWidget {
     public:
        Quit(Root& root, EventLoop& loop);

        virtual bool handleKey(util::Key_t key, int prefix);

     private:
        Root& m_root;
        EventLoop& m_loop;
    };

} }

#endif
