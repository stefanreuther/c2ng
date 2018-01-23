/**
  *  \file server/host/hostschedule.hpp
  *  \brief Class server::host::HostSchedule
  */
#ifndef C2NG_SERVER_HOST_HOSTSCHEDULE_HPP
#define C2NG_SERVER_HOST_HOSTSCHEDULE_HPP

#include "server/interface/hostschedule.hpp"

namespace server { namespace host {

    class Root;
    class Session;

    /** Implementation of HostSchedule interface.
        This interface implements SCHEDULE commands. */
    class HostSchedule : public server::interface::HostSchedule {
     public:
        /** Constructor.
            \param session Session
            \param root    Service root */
        HostSchedule(Session& session, Root& root);

        // HostSchedule virtuals:
        virtual void add(int32_t gameId, const Schedule& sched);
        virtual void replace(int32_t gameId, const Schedule& sched);
        virtual void modify(int32_t gameId, const Schedule& sched);
        virtual void getAll(int32_t gameId, std::vector<Schedule>& result);
        virtual void drop(int32_t gameId);
        virtual void preview(int32_t gameId,
                             afl::base::Optional<Time_t> timeLimit,
                             afl::base::Optional<int32_t> turnLimit,
                             afl::data::IntegerList_t& result);

     private:
        Session& m_session;
        Root& m_root;

        void doAddReplace(int32_t gameId, const Schedule& sched, bool add);
    };


} }

#endif
