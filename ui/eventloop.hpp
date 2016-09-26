/**
  *  \file ui/eventloop.hpp
  */
#ifndef C2NG_UI_EVENTLOOP_HPP
#define C2NG_UI_EVENTLOOP_HPP

#include "afl/base/closure.hpp"

namespace ui {

    class Root;

    class EventLoop {
     public:
        EventLoop(Root& root);

        int run();

        void stop(int n);

        afl::base::Closure<void(int)>* makeStop(int n);

     private:
        Root& m_root;
        bool m_stopped;
        int m_result;
    };

}

#endif
