/**
  *  \file client/widgets/messageactionpanel.hpp
  *  \brief Class client::widgets::MessageActionPanel
  */
#ifndef C2NG_CLIENT_WIDGETS_MESSAGEACTIONPANEL_HPP
#define C2NG_CLIENT_WIDGETS_MESSAGEACTIONPANEL_HPP

#include "afl/base/signal.hpp"
#include "afl/base/signalconnection.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/string/translator.hpp"
#include "ui/root.hpp"
#include "ui/widget.hpp"
#include "ui/widgets/button.hpp"

namespace client { namespace widgets {

    /** Panel with all sorts of message-related actions.
        This widget displays a panel and buttons, and converts keystrokes into commands.
        A user will configure it using setPosition(), enableAction(), disableAction(),
        and hook sig_action to receive action requests. */
    class MessageActionPanel : public ui::Widget {
     public:
        /*
         *  Layout:
         *    [<] 10/10 [>]
         *    [G] go to obj         optional
         *    [X] go to X/Y         optional
         *    [R] reply to          optional
         *    [C] confirm           optional
         *    [A] accept            optional
         *    ...
         *    [E] Edit              optional
         *    [T] To...             optional
         *    [Del] Delete          optional
         *    [F] Forward
         *    [S] Search
         *    [W] Write to file
         */

        /** Actions.
            Each action can optionally receive
            - modifiers (util::KeyMod_Shift, util::KeyMod_Ctrl)
            - prefix argument
            Whether it makes sense to decode either of these parameters depends on the action. */
        enum Action {
            GoTo1,              ///< "G" (go to object mentioned in message).
            GoTo2,              ///< "X" (go to coordinate mentioned in message).
            Reply,              ///< "R" (reply to message).
            Confirm,            ///< "C" (confirm notification).
            Accept,             ///< "A" (accept transfer).
            Edit,               ///< "E" (edit).
            Redirect,           ///< "T" (edit receivers).
            Delete,             ///< "Del" (delete message).
            Forward,            ///< "F" (forward message).
            Search,             ///< "S", F7 (search message).
            Write,              ///< "W" (save to file).
            // Additional actions that do not correspond to a toggle-able button
            BrowsePrevious,     ///< Up/PgUp,"-",WheelUp (previous message). With argument: that many.
            BrowsePreviousAll,  ///< Shift-Up/PgUp (previous message, including filtered). With argument: that many.
            BrowseNext,         ///< Dn/PgDn,"+",WheelDn (next message). With argument: that many.
            BrowseNextAll,      ///< Shift-Dn/PgDn (next message, including filtered). With argument: that many.
            BrowseFirst,        ///< Home (first message).
            BrowseFirstAll,     ///< Shift-Home (first message, including filtered).
            BrowseLast,         ///< End (last message).
            BrowseLastAll,      ///< Shift-End (last message, including filtered).
            BrowseNth,          ///< "=" (n-th message). Argument is message number.
            SearchNext,         ///< "N", Shift-F7 (search next message).
            WriteAll,           ///< Ctrl-W (save all).
            ReplyAll,           ///< Ctrl-R (reply all).
            BrowseSubjects      ///< Tab.
        };

        /** Constructor.
            @param root    UI root
            @param tx      Translator */
        MessageActionPanel(ui::Root& root, afl::string::Translator& tx);

        /** Destructor. */
        ~MessageActionPanel();

        /** Enable an action.
            This shows the action, together with an explanatory note.
            @param a     Action
            @param note  Note; brief human-readable text */
        void enableAction(Action a, const String_t& note);

        /** Disable an action.
            This hides the action.
            Note that this does not prevent the action from being generated on sig_action.
            @param a     Action */
        void disableAction(Action a);

        /** Set position in mailbox.
            Sets the position indicator ("10/30").
            @param label  Position indicator
            @param dim    true to show in dim color (current message is filtered) */
        void setPosition(String_t label, bool dim);

        /** Set priority of Reply action.
            With this flag set to false, Reply is the preferred default option if it is available.
            With this flag set to true, Reply is not selected if other options are available.
            Set this to true if default reply addressee is the Host.

            @param flag Flag */
        void setAvoidReply(bool flag);

        // Widget:
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void requestChildRedraw(Widget& child, const gfx::Rectangle& area);
        virtual void handleChildAdded(Widget& child);
        virtual void handleChildRemove(Widget& child);
        virtual void handlePositionChange();
        virtual void handleChildPositionChange(Widget& child, const gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;

        /** Signal: Action.
            This action is emitted whenever an action is selected.
            Note that MessageActionPanel can emit actions even if they are disabled.
            @param action   Action
            @param prefix   Prefix argument */
        afl::base::Signal<void(Action, int)> sig_action;

     private:
        struct LabeledButton {
            ui::widgets::Button button;
            String_t label;
            String_t note;

            LabeledButton(ui::Root& root, util::Key_t key, String_t buttonLabel, String_t label);
        };

        ui::Root& m_root;
        ui::widgets::Button m_prevButton;
        ui::widgets::Button m_nextButton;
        String_t m_positionLabel;
        bool m_positionDimmed;
        bool m_avoidReply;
        afl::container::PtrVector<LabeledButton> m_actions;

        afl::base::SignalConnection conn_imageChange;

        void init(ui::Root& root, afl::string::Translator& tx);
        void updatePositions();

        void onKey(int arg, util::Key_t key);
        bool handleBuiltinKey(util::Key_t key, int arg);
        void doAction(Action a, int arg);
        bool hasAction(Action a) const;
    };

} }

#endif
