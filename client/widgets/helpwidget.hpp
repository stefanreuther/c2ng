/**
  *  \file client/widgets/helpwidget.hpp
  *  \brief Class client::widgets::HelpWidget
  */
#ifndef C2NG_CLIENT_WIDGETS_HELPWIDGET_HPP
#define C2NG_CLIENT_WIDGETS_HELPWIDGET_HPP

#include "afl/bits/smallset.hpp"
#include "game/session.hpp"
#include "ui/invisiblewidget.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace widgets {

    /** Help invoker widget.
        This is an invisible widget that handles help invocation.
        When it detects a help key (Alt-H, H, F1), it will open a help page.

        If you have a help button, call dispatchKeyTo(helpWidget).

        By default, this widget consumes H and F1.
        If these keys shall have another meaning and are not consumed by a widget that has keyboard focus, use setFlag to disable them.
        (If the other widget has keyboard focus, it will be guaranteed to process the keys before us.) */
    class HelpWidget : public ui::InvisibleWidget {
     public:
        enum Flag {
            AcceptH,
            AcceptF1
        };
        typedef afl::bits::SmallSet<Flag> Flags_t;

        /** Constructor.
            \param root       Root (required by doHelpDialog)
            \param gameSender Game sender (required by doHelpDialog)
            \param pageName   Help page name */
        HelpWidget(ui::Root& root, util::RequestSender<game::Session> gameSender, String_t pageName);

        /** Destructor. */
        ~HelpWidget();

        /** Set flag.
            \param flag  Flag to modify
            \param value Whether to set or reset the flag
            \return *this */
        HelpWidget& setFlag(Flag flag, bool value);

        // InvisibleWidget/Widget:
        virtual bool handleKey(util::Key_t key, int prefix);

     private:
        ui::Root& m_root;
        util::RequestSender<game::Session> m_gameSender;
        String_t m_pageName;
        Flags_t m_flags;
    };

} }

#endif
