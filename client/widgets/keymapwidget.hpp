/**
  *  \file client/widgets/keymapwidget.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_KEYMAPWIDGET_HPP
#define C2NG_CLIENT_WIDGETS_KEYMAPWIDGET_HPP

#include "ui/invisiblewidget.hpp"
#include "client/si/control.hpp"
#include "util/requestsender.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestdispatcher.hpp"
#include "game/session.hpp"
#include "util/keymap.hpp"
#include "util/slaveobject.hpp"
#include "util/slaverequestsender.hpp"
#include "afl/base/signalconnection.hpp"
#include "afl/base/closure.hpp"
#include "ui/widgets/abstractbutton.hpp"

namespace client { namespace widgets {

    /** Keymap widget.
        This widget implements script-connected keymaps for the user interface.

        <b>Principle of operation:<b>

        Keymaps live in the script world.
        To avoid having to go to scripts for <em>every</em> key, we fetch a set of bound keys from the keymap.
        Only for those that match, we go to the script world.

        Because keymaps may change in the script world, we must set up a listener using a trampoline.

        \todo Feature: user keymaps ("UseKeymap xxx")

        \todo If the game thread hangs, this will make the UI perceived-hang.
        We should set up an emergency keymap in this case. */
    class KeymapWidget : public ui::InvisibleWidget {
     public:
        /** Constructor.
            \param gameSender Sender to game session
            \param self Dispatcher for UI thread
            \param ctl Script controller instance */
        KeymapWidget(util::RequestSender<game::Session> gameSender,
                     util::RequestDispatcher& self,
                     client::si::Control& ctl);

        /** Destructor. */
        ~KeymapWidget();

        /** Set name of keymap.
            \param keymap Name of keymap (upper-case!) */
        void setKeymapName(String_t keymap);

        // InvisibleWidget/Widget/EventConsumer:
        virtual bool handleKey(util::Key_t key, int prefix);

        /** Make a key handler.
            This can be attached to a button's sig_fireKey to make this button fire a keymap action.
            Use as:
            <pre>btn.sig_fireKey.addNewClosure(kw.makeKey());</pre>
            \return newly-allocated closure */
        afl::base::Closure<void(int, util::Key_t)>* makeKey();

        /** Add a button.
            This makes the button invoke keymap keys.
            \todo set up other required magic such as "if button invokes a menu, anchor it at the button"
            \param btn Button */
        void addButton(ui::widgets::AbstractButton& btn);

     private:
        /** RequestReceiver to allow us to receive replies from script side. */
        util::RequestReceiver<KeymapWidget> m_reply;

        /** Script controller. */
        client::si::Control& m_control;

        /** Set of bound keys. */
        util::KeySet_t m_keys;

        /** Current keymap name. */
        String_t m_keymapName;

        class Trampoline : public util::SlaveObject<game::Session> {
         public:
            Trampoline(util::RequestSender<KeymapWidget> reply)
                : conn_keymapChange(),
                  m_reply(reply),
                  m_keymapName()
                { }
            void init(game::Session&);
            void done(game::Session&);
            void setKeymapName(game::Session&, String_t keymapName);
            void update(game::Session& s);
         private:
            afl::base::SignalConnection conn_keymapChange;
            util::RequestSender<KeymapWidget> m_reply;
            String_t m_keymapName;
        };
        util::SlaveRequestSender<game::Session, Trampoline> m_slave;
    };

} }

#endif
