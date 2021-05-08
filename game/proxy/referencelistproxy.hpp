/**
  *  \file game/proxy/referencelistproxy.hpp
  *  \brief Class game::proxy::ReferenceListProxy
  */
#ifndef C2NG_GAME_PROXY_REFERENCELISTPROXY_HPP
#define C2NG_GAME_PROXY_REFERENCELISTPROXY_HPP

#include <memory>
#include "afl/base/signal.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/ref/configuration.hpp"
#include "game/ref/listobserver.hpp"
#include "game/ref/userlist.hpp"
#include "game/session.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    /** Reference list proxy.

        Maintains a game::ref::UserList, provided by an Initializer_t, and reports updates to the underlying units.
        The UserList can be sorted according to a user configuration.

        Bidirectional, asynchronous:
        - choose configuration (setConfigurationSelection) and modify it (setConfig)
        - set content (setContentNew)
        - updates reported on sig_listChange

        Bidirectional, synchronous:
        - retrieve current configuration (getConfig)
        - finish update processing (waitIdle)

        \see game::ref::ListObserver */
    class ReferenceListProxy {
     public:
        /** Initializer.
            \param session Session
            \param observer New content must be produced here */
        typedef afl::base::Closure<void(Session&, game::ref::ListObserver&)> Initializer_t;


        /** Constructor.
            \param gameSender Game sender
            \param disp RequestDispatcher to receive replies in this thread */
        ReferenceListProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& disp);

        /** Set configuration selection.
            This chooses the slot in UserConfiguration (pcc2.ini) where the configuration is stored.
            This will update the UserList and report an update on sig_listChange.
            \param sel Configuration selection
            \see game::ref::ListObserver::setConfigurationSelection */
        void setConfigurationSelection(const game::ref::ConfigurationSelection& sel);

        /** Set content.
            This invokes the initializer in the game thread to update the UserList,
            and report an update on sig_listChange.
            \param pInit Initializer */
        void setContentNew(std::auto_ptr<Initializer_t> pInit);

        /** Check for idle.
            \retval true No unanswered request; an update represents a change on the game side we did not initiate
            \retval false There still is an active request; an update may follow, a sig_finish will follow */
        bool isIdle() const;

        /** Wait until idle.
            Waits until isIdle() returns true.
            \param link WaitIndicator */
        void waitIdle(WaitIndicator& link);

        /** Get configuration.
            \param link WaitIndicator
            \return effective configuration as determined by setConfigurationSelection */
        game::ref::Configuration getConfig(WaitIndicator& link);

        /** Set configuration.
            Updates the configuration in UserConfiguration and updates the list.
            \param config Configuration */
        void setConfig(const game::ref::Configuration& config);

        /** Signal: new content.
            \param list Content */
        afl::base::Signal<void(const game::ref::UserList& list)> sig_listChange;

        /** Signal: update finished.
            When multiple changes are made to the proxy (i.e. setConfigurationSelection, setContentNew), multiple updates on sig_listChange will be produced.
            sig_finish is produced after the last update (i.e. when isIdle() is true again).
            Further updates are caused by game-side changes. */
        afl::base::Signal<void()> sig_finish;

     private:
        class Observer;
        class ObserverFromSession;
        class Updater;
        class Confirmer;

        util::RequestSender<Session> m_gameSender;
        util::RequestReceiver<ReferenceListProxy> m_receiver;
        util::RequestSender<Observer> m_observerSender;

        int m_pendingRequests;

        void onListChange(const game::ref::UserList& list);
        void confirmRequest();
    };

} }

#endif
