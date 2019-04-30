/**
  *  \file server/interface/hostcron.hpp
  *  \brief Interface server::interface::HostCron
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTCRON_HPP
#define C2NG_SERVER_INTERFACE_HOSTCRON_HPP

#include <vector>
#include <map>
#include "afl/base/deletable.hpp"
#include "afl/base/optional.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    /** Host Cron interface.
        Accesses the host service's scheduler. */
    class HostCron : public afl::base::Deletable {
     public:
        /** Scheduler action. */
        enum Action {
            UnknownAction,             ///< Not known.
            NoAction,                  ///< No action (for this game).
            HostAction,                ///< Run host.
            ScheduleChangeAction,      ///< Change schedule (and determine next action from new schedule).
            MasterAction               ///< Run master.
        };

        /** Scheduler event. */
        struct Event {
            int32_t gameId;            ///< Affected game.
            Action action;             ///< Action to perform.
            Time_t time;               ///< Time.

            Event(int32_t gameId, Action action, Time_t time)
                : gameId(gameId), action(action), time(time)
                { }
            Event()
                : gameId(0), action(UnknownAction), time(0)
                { }
        };

        /** Map of broken games.
            Keys are game Ids, values are crash messages. */
        typedef std::map<int32_t, String_t> BrokenMap_t;



        /** Get next scheduler action for a game (CRONGET).
            \param gameId Game Id
            \return event */
        virtual Event getGameEvent(int32_t gameId) = 0;

        /** Get next scheduler actions (CRONLIST).
            \param [in] limit Maximum number of elements to return
            \param [out] result Result */
        virtual void listGameEvents(afl::base::Optional<int32_t> limit, std::vector<Event>& result) = 0;

        /** Restart scheduler for a game (CRONKICK).
            \param gameId Game Id
            \return true if game was restarted, false if it was not broken */
        virtual bool kickstartGame(int32_t gameId) = 0;

        /** Suspend scheduler for the given relative time (CRONSUSPEND).
            \param relativeTime Time (0 to cancel suspension) */
        virtual void suspendScheduler(int32_t relativeTime) = 0;

        /** List broken games and reasons of breakage (CRONLSBROKEN).
            \param [out] result BrokenMap_t Result */
        virtual void getBrokenGames(BrokenMap_t& result) = 0;
    };

} }

#endif
