/**
  *  \file client/proxy/buildqueueproxy.hpp
  */
#ifndef C2NG_CLIENT_PROXY_BUILDQUEUEPROXY_HPP
#define C2NG_CLIENT_PROXY_BUILDQUEUEPROXY_HPP

#include "client/downlink.hpp"
#include "game/actions/changebuildqueue.hpp"
#include "game/session.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestsender.hpp"
#include "util/slaverequestsender.hpp"

namespace client { namespace proxy {

    /** Build queue proxy.

        Bidirectional, synchronous:
        - set up (init())

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

        /** Constructor.
            \param gameSender Sender
            \param reply RequestDispatcher to send replies back */
        BuildQueueProxy(util::RequestSender<game::Session> gameSender, util::RequestDispatcher& reply);

        /** Synchronous initalisation.
            Call this to synchronously obtain the current status.
            \param link [in] Downlink
            \param data [out] Current status */
        void init(client::Downlink& link, Infos_t& data);

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
        util::RequestReceiver<BuildQueueProxy> m_reply;
        util::SlaveRequestSender<game::Session, Trampoline> m_request;
    };


} }

#endif
