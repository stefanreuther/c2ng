/**
  *  \file server/host/cronimpl.hpp
  *  \brief Class server::host::CronImpl
  */
#ifndef C2NG_SERVER_HOST_CRONIMPL_HPP
#define C2NG_SERVER_HOST_CRONIMPL_HPP

#include <list>
#include "afl/base/stoppable.hpp"
#include "afl/base/uncopyable.hpp"
#include "afl/sys/mutex.hpp"
#include "afl/sys/semaphore.hpp"
#include "afl/sys/thread.hpp"
#include "server/host/cron.hpp"
#include "util/processrunner.hpp"

namespace server { namespace host {

    class Root;

    /** Implementation of Cron.
        This implements the actual scheduler.

        The scheduler has three queues:
        - the main scheduler queue (m_futureEvents) containing future events.
        - a queue of overdue events (m_dueEvents). All games in this queue are locked.
        - a queue of games notified by other components (m_changedGames).

        To support examinability, this code maintains the invariant that every game which is subject to scheduling
        actually appears in one of the three lists.
        Therefore, for example, we don't take a game out of the m_dueEvents list, run host, and place it in the m_futureEvents list;
        we keep it in the m_dueEvents list for the whole host run so people see that something happens there.

        An important property of CronImpl is that it exports the game (under exclusive access),
        runs host, and then re-imports the game (under exclusive access).
        During the host run, the game is locked using GameArbiter (e.g. preventing modifications),
        but otherwise, the database can be accessed by other users. */
    class CronImpl : public Cron,
                     private afl::base::Uncopyable,
                     private afl::base::Stoppable
    {
     public:
        /** Constructor.
            This will start a separate thread to process scheduler events.
            \param root Service root
            \param runner Runner to use. Should be distinct from Root's runner. */
        CronImpl(Root& root, util::ProcessRunner& runner);

        /** Destructor.
            This will stop the separate thread. */
        ~CronImpl();

        // Cron:
        virtual Event_t getGameEvent(int32_t gameId);
        virtual void listGameEvents(std::vector<Event_t>& result);
        virtual void handleGameChange(int32_t gameId);

     private:
        Root& m_root;
        util::ProcessRunner& m_runner;
        afl::sys::Thread m_thread;

        // FIXME: we can probably get rid of this mutex and rely on Root's, to avoid nested-mutex trouble.
        // FIXME: must review the general mutex/reconnect behaviour!!!!
        afl::sys::Mutex m_mutex;       // ex mutex
        afl::sys::Semaphore m_wake;    // ex new_command_sem
        bool m_stopFlag;               // protected by mutex

        std::list<int32_t> m_changedGames;  // protected by mutex

        // Schedule
        // Note: Event_t is ex ScheduleItem
        std::list<Event_t> m_futureEvents;       ///< Future actions.
        std::list<Event_t> m_dueEvents;          ///< Due actions. Games in this list are locked if they are MasterAction or HostAction.

        virtual void run();
        virtual void stop();

        void schedulerMain();
        bool isStopRequested();

        // Utilities
        void logAction(const char* what, const Event_t& item);
        void generateInitialSchedule();
        void processRequests();
        void moveDueItems();
        void runDueItem(int32_t gameId, std::list<Event_t>& newSchedule);
    };

    /** Compute actions for a game.
        This method is published for testability.

        \param now    [in] Current time
        \param root   [in] Service root
        \param gameId [in] Game to work on
        \param sch    [out] Event(s) will be produced here. This method appends zero or one event. */
    void computeGameTimes(const int32_t now, Root& root, const int32_t gameId, std::list<server::interface::HostCron::Event>& sch);

} }

#endif
