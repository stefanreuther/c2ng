/**
  *  \file client/proxy/referencelistproxy.hpp
  */
#ifndef C2NG_CLIENT_PROXY_REFERENCELISTPROXY_HPP
#define C2NG_CLIENT_PROXY_REFERENCELISTPROXY_HPP

#include <memory>
#include "afl/base/signal.hpp"
#include "afl/string/translator.hpp"
#include "game/ref/configuration.hpp"
#include "game/ref/listobserver.hpp"
#include "game/ref/userlist.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"
#include "util/slaverequestsender.hpp"

namespace client { namespace proxy {

    class ReferenceListProxy {
     public:
        typedef afl::base::Closure<void(game::Session&, game::ref::ListObserver&)> Initializer_t;
        
        ReferenceListProxy(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx);

        void setConfigurationSelection(const game::ref::ConfigurationSelection& sel);

        void setContentNew(std::auto_ptr<Initializer_t> pInit);

        bool isIdle() const;

        void onMenu(gfx::Point pt);

        afl::base::Signal<void(const game::ref::UserList& list)> sig_listChange;

        afl::base::Signal<void()> sig_finish;

     private:
        class Observer;
        class Updater;
        class Confirmer;

        ui::Root& m_root;
        util::RequestSender<game::Session> m_gameSender;
        afl::string::Translator& m_translator;
        util::RequestReceiver<ReferenceListProxy> m_receiver;
        util::SlaveRequestSender<game::Session, Observer> m_observerSender;

        int m_pendingRequests;

        void onListChange(const game::ref::UserList& list);
        void confirmRequest();

        game::ref::Configuration getConfig();
        void setConfig(const game::ref::Configuration& config);
    };

} }

#endif
