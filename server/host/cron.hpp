/**
  *  \file server/host/cron.hpp
  *  \brief Interface server::host::Cron
  */
#ifndef C2NG_SERVER_HOST_CRON_HPP
#define C2NG_SERVER_HOST_CRON_HPP

#include "server/interface/hostcron.hpp"
#include "afl/base/deletable.hpp"
#include "server/types.hpp"

namespace server { namespace host {

    /** Interface for scheduler.
        The scheduler runs asynchronously to the main command handler.
        The host server can run with the scheduler disabled (mainly for testing).
        This interface codifies the comminucation between the command handler and the scheduler. */
    class Cron : public afl::base::Deletable {
     public:
        /** Shortcut for a scheduler event. */
        typedef server::interface::HostCron::Event Event_t;

        /** Shortcut for a scheduler action. */
        typedef server::interface::HostCron::Action Action_t;


        /** Get next event for a game.
            \param gameId Game Id
            \return event (time, action) */
        virtual Event_t getGameEvent(int32_t gameId) = 0;

        /** List all events (times, actions).
            \param result [out] Events */
        virtual void listGameEvents(std::vector<Event_t>& result) = 0;

        /** Reconsider a game.
            Called when the game changed in a way that may need recomputation of the next action.
            \param gameId Game Id */
        virtual void handleGameChange(int32_t gameId) = 0;

        /** Suspend scheduler.
            \param absTime Absolute time */
        virtual void suspendScheduler(Time_t absTime) = 0;
    };

} }

#endif
