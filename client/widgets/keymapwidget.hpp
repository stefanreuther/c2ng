/**
  *  \file client/widgets/keymapwidget.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_KEYMAPWIDGET_HPP
#define C2NG_CLIENT_WIDGETS_KEYMAPWIDGET_HPP

#include "client/si/control.hpp"
#include "game/proxy/keymapproxy.hpp"
#include "game/session.hpp"
#include "ui/invisiblewidget.hpp"
#include "util/keymap.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestsender.hpp"

namespace client { namespace widgets {

    /** Keymap widget.
        This widget implements script-connected keymaps for the user interface.

        <b>Principle of operation:<b>

        Keymaps live in the script world.
        To avoid having to go to scripts for <em>every</em> key, we fetch a set of bound keys from the keymap.
        Only for those that match, we go to the script world.

        \todo Feature: user keymaps ("UseKeymap xxx")

        \todo If the game thread hangs, this will make the UI perceived-hang.
        We should set up an emergency keymap in this case. */
    class KeymapWidget : public ui::InvisibleWidget, private game::proxy::KeymapProxy::Listener {
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

     private:
        // KeymapProxy::Listener
        virtual void updateKeyList(util::KeySet_t& keys);

        /** Keymap proxy. Allows us to access the keymap. */
        game::proxy::KeymapProxy m_proxy;

        /** Script controller. */
        client::si::Control& m_control;

        /** Set of bound keys. */
        util::KeySet_t m_keys;

        /** Current keymap name. */
        String_t m_keymapName;
    };

} }

#endif
