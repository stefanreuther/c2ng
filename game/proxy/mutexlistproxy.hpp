/**
  *  \file game/proxy/mutexlistproxy.hpp
  *  \brief Class game::proxy::MutexListProxy
  */
#ifndef C2NG_GAME_PROXY_MUTEXLISTPROXY_HPP
#define C2NG_GAME_PROXY_MUTEXLISTPROXY_HPP

#include <vector>
#include "afl/base/types.hpp"
#include "afl/string/string.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/session.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    /** Bidirectional proxy for mutex list access.
        This proxies a Session's interpreter::MutexList object.

        Bidirectional synchronous: enumerate mutexes. */
    class MutexListProxy {
     public:
        /** Description of a mutex. */
        struct Info {
            /** Mutex name ("S10.WAYPOINT"). */
            String_t name;
            /** Owning process Id. */
            uint32_t processId;

            Info(const String_t& name, uint32_t processId)
                : name(name), processId(processId)
                { }
        };

        /** List of descriptions. */
        typedef std::vector<Info> Infos_t;


        /** Constructor.
            \param gameSender Game sender */
        explicit MutexListProxy(util::RequestSender<Session> gameSender);

        /** Enumerate mutexes owned by a process.
            \param link         WaitIndicator object for UI synchronisation
            \param result [out] Result
            \param processId    List only this process's mutexes
            \see interpreter::MutexList::enumMutexes() */
        void enumMutexes(WaitIndicator& link, Infos_t& result, uint32_t processId);

        /** Enumerate all active mutexes.
            \param link         WaitIndicator object for UI synchronisation
            \param result [out] Result
            \see interpreter::MutexList::enumMutexes()  */
        void enumMutexes(WaitIndicator& link, Infos_t& result);

     private:
        util::RequestSender<Session> m_gameSender;

        static void buildList(Infos_t& result, Session& session, interpreter::Process* p);
    };

} }

#endif
