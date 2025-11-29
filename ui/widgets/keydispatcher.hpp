/**
  *  \file ui/widgets/keydispatcher.hpp
  *  \brief Class ui::widgets::KeyDispatcher
  */
#ifndef C2NG_UI_WIDGETS_KEYDISPATCHER_HPP
#define C2NG_UI_WIDGETS_KEYDISPATCHER_HPP

#include "ui/invisiblewidget.hpp"
#include "afl/container/ptrmap.hpp"
#include "afl/base/closure.hpp"

namespace ui { namespace widgets {

    /** Simple key handler.
        This is an invisible widget that accepts keystrokes and dispatches these to closures. */
    class KeyDispatcher : public InvisibleWidget {
     public:
        typedef afl::base::Closure<void(int)> Closure_t;
        typedef util::Key_t Key_t;

        /** Constructor.
            Makes an empty KeyDispatcher that does not do anything. */
        KeyDispatcher();

        /** Destructor. */
        ~KeyDispatcher();

        /** Add static key handler function.
            @tparam Fun function type
            @param key Key to handle
            @param fun Function to call */
        template<typename Fun>
        void add(Key_t key, Fun fun);

        /** Add key handler member function.
            @tparam Obj Object pointer type
            @tparem Fun Member function type
            @param key Key to handle
            @param obj Object pointer ("T*")
            @param fun Member function ("void (T::*)(int)") */
        template<typename Obj, typename Fun>
        void add(Key_t key, Obj obj, Fun fun);

        /** Add generic closure.
            @param key Key
            @param closure Newly-allocated closure. KeyDispatcher will take ownership. */
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
