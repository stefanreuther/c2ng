/**
  *  \file game/proxy/keymapproxy.hpp
  *  \class game::proxy::KeymapProxy
  */
#ifndef C2NG_GAME_PROXY_KEYMAPPROXY_HPP
#define C2NG_GAME_PROXY_KEYMAPPROXY_HPP

#include "afl/base/signalconnection.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/session.hpp"
#include "util/keymap.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    /** Bidirectional proxy for keymap information.

        Provides asynchronous access to keymap population information,
        and synchronous access to other metadata.

        Keymap population information (which keys are bound) is required to implement keymaps on the UI side.
        UI side needs to know which keys are bound to not have to call into the script side for each key.
        Population information can change. */
    class KeymapProxy {
     public:
        /** Listener for asynchronous keymap population updates. */
        class Listener {
         public:
            /** Virtual destructor. */
            virtual ~Listener() { }

            /** Update key list.
                \param keys Keys. You can loot this object. */
            virtual void updateKeyList(util::KeySet_t& keys) = 0;
        };

        /** Result class of a getKey() query. */
        enum Result {
            Unassigned,         ///< Key not bound at all.
            Cancelled,          ///< Binding explicitly cancelled (bound to 0).
            Internal,           ///< Internal binding (numeric).
            Normal              ///< Normal binding (atom).
        };

        /** Result of a getKey() query. */
        struct Info {
            Result result;                    ///< Result class.
            String_t keymapName;              ///< Name of keymap of binding. Set if result is not Unassigned.
            String_t command;                 ///< Command. Set if result is Normal, otherwise empty.
            String_t alternateKeymapName;     ///< If the key triggers an alternate keymap, name of the keymap. Otherwise empty.
            String_t origin;                  ///< Origin (typically, name of a plugin) of the command providing this binding.
        };


        /** Constructor.
            \param gameSender RequestSender to send to the game
            \param reply RequestDispatcher to receive replies. */
        KeymapProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply);

        /** Destructor. */
        ~KeymapProxy();

        /** Set listener for asynchronous keymap population updates.
            If desired, call this after constructing the KeymapProxy.
            Only one listener can be set.
            \param listener Listener */
        void setListener(Listener& listener);

        /** Set keymap name.
            This triggers a listener callback and sets the keymap for further synchronous calls.
            \param keymap Name of keymap */
        void setKeymapName(String_t keymap);

        /** Get description of the current keymap.
            \param link WaitIndicator
            \param out [out] Result will be produced here. Empty if keymap is not set or undefined. */
        void getDescription(WaitIndicator& link, util::KeymapInformation& out);

        /** Get description of a key.
            \param link WaitIndicator
            \param key Key
            \param result [out] Result will be produced here. Unassigned if keymap is not set or undefined. */
        void getKey(WaitIndicator& link, util::Key_t key, Info& result);

     private:
        util::RequestReceiver<KeymapProxy> m_reply;

        /*
         *  Trampoline:
         *  We need a persistent trampoline to manage asynchronous updates.
         */
        class Trampoline;
        class TrampolineFromSession;
        util::RequestSender<Trampoline> m_sender;

        Listener* m_pListener;
    };

} }

#endif
