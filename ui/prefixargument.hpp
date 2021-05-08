/**
  *  \file ui/prefixargument.hpp
  *  \brief Class ui::PrefixArgument
  */
#ifndef C2NG_UI_PREFIXARGUMENT_HPP
#define C2NG_UI_PREFIXARGUMENT_HPP

#include "ui/invisiblewidget.hpp"

namespace ui {

    class Root;

    /** Prefix argument trigger.
        This widget, when added to a group, will allow the user to specify a prefix argument.
        The widget itself is invisble, but will open a popup when the user starts typing.

        If the prefix argument is confirmed with a keypress, the key is re-posted on the Root.
        If the prefix argument is confirmed with a mouse click, the prefix argument is posted using Root::setMousePrefixArgument(). */
    class PrefixArgument : public InvisibleWidget {
     public:
        /** Constructor.
            \param root UI root */
        explicit PrefixArgument(Root& root);

        virtual bool handleKey(util::Key_t key, int prefix);

        /** Show popup.
            Published for convenience to start the prefix argument manually.
            Returns when a prefix argument has been confirmed or canceled.
            \param initialValue Initial value */
        void showPopup(int initialValue);

     private:
        Root& m_root;
    };

}

#endif
