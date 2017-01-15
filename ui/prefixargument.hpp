/**
  *  \file ui/prefixargument.hpp
  */
#ifndef C2NG_UI_PREFIXARGUMENT_HPP
#define C2NG_UI_PREFIXARGUMENT_HPP

#include "ui/invisiblewidget.hpp"

namespace ui {

    class Root;

    // /** Prefix argument trigger. This widget, when added to a group, will
    //     allow the user to specify a prefix argument. It is invisible, but
    //     will call startPrefixArg() to display the input popup when the
    //     user types a digit key. In the first event handling pass, it only
    //     accepts Alt+digit, in the second pass, it also accepts regular
    //     digits; this allows coexistence with regular input widgets. */
    class PrefixArgument : public InvisibleWidget {
     public:
        PrefixArgument(Root& root);

        virtual bool handleKey(util::Key_t key, int prefix);

        void showPopup(int initialValue);

     private:
        Root& m_root;
    };

}

#endif
