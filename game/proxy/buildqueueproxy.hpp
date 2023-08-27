/**
  *  \file game/proxy/buildqueueproxy.hpp
  *  \brief Class game::proxy::BuildQueueProxy
  */
#ifndef C2NG_GAME_PROXY_BUILDQUEUEPROXY_HPP
#define C2NG_GAME_PROXY_BUILDQUEUEPROXY_HPP

#include "game/actions/changebuildqueue.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/session.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    /** Build queue proxy.

        Bidirectional, synchronous:
        - retrieve initial status (getStatus())

        Bidirectional, asynchronous:
        - modifications and commit (everything else)

        Status is maintained as a vector of all build orders.
        This status can be obtained using init(), or as a callback after modifications.
        Items are identified by an index into the status vector.

        \see game::actions::ChangeBuildQueue */
    class BuildQueueProxy {
     public:
        typedef game::actions::ChangeBuildQueue::Infos_t Infos_t;
        typedef game::actions::ChangeBuildQueue::Info Info_t;

        struct GlobalInfo {
            int numBases;       ///< Number of bases owned by player.
            int totalBases;     ///< Total number of bases in game. 0 if not known.
            GlobalInfo()
                : numBases(), totalBases()
                { }
        };

        /** Constructor.
            \param gameSender Sender
            \param reply RequestDispatcher to send replies back */
        BuildQueueProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply);

        /** Get current status.
            Call this to synchronously obtain the current status.
            \param link [in] WaitIndicator
            \param data [out] Current status
            \param global [out] Global status */
        void getStatus(WaitIndicator& link, Infos_t& data, GlobalInfo& global);

        /** Set priority of a build order.
            Will produce a sig_update callback with new status.
            \param slot Slot number
            \param pri New priority (1-9 = PBx, 0 = no priority order)
            \see game::actions::ChangeBuildQueue::setPriority */
        void setPriority(size_t slot, int pri);

        /** Increase a slot's priority (build earlier).
            Will produce a sig_update callback with new status.
            \param slot Slot number
            \see game::actions::ChangeBuildQueue::increasePriority */
        void increasePriority(size_t slot);

        /** Decrease a slot's priority (build later).
            Will produce a sig_update callback with new status.
            \param slot Slot number
            \see game::actions::ChangeBuildQueue::decreasePriority */
        void decreasePriority(size_t slot);

        /** Write all changes back to universe. */
        void commit();

        /** Callback: status change.
            \param infos New status */
        afl::base::Signal<void(const Infos_t& infos)> sig_update;

     private:
        class Trampoline;
        class TrampolineFromSession;
        util::RequestReceiver<BuildQueueProxy> m_reply;
        util::RequestSender<Trampoline> m_request;
    };

} }


#endif
