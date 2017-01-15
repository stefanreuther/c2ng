/**
  *  \file ui/widgets/keydispatcher.hpp
  */
#ifndef C2NG_UI_WIDGETS_KEYDISPATCHER_HPP
#define C2NG_UI_WIDGETS_KEYDISPATCHER_HPP

#include "ui/invisiblewidget.hpp"
#include "afl/container/ptrmap.hpp"
#include "afl/base/closure.hpp"

namespace ui { namespace widgets {

    class KeyDispatcher : public InvisibleWidget {
     public:
        typedef afl::base::Closure<void(int)> Closure_t;
        typedef util::Key_t Key_t;

        KeyDispatcher();
        ~KeyDispatcher();

        template<typename Fun>
        void add(Key_t key, Fun fun);

        template<typename Obj, typename Fun>
        void add(Key_t key, Obj obj, Fun fun);

        void addNewClosure(Key_t key, Closure_t* closure);

        // Widget:
        virtual bool handleKey(util::Key_t key, int prefix);

     private:
        afl::container::PtrMap<Key_t, Closure_t> m_keys;
    };

} }

template<typename Fun>
inline void
ui::widgets::KeyDispatcher::add(Key_t key, Fun fun)
{
    addNewClosure(key, Closure_t::makeStatic(fun));
}

template<typename Obj, typename Fun>
inline void
ui::widgets::KeyDispatcher::add(Key_t key, Obj obj, Fun fun)
{
    addNewClosure(key, Closure_t::makeBound(obj, fun));
}

#endif
