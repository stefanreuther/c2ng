/**
  *  \file game/proxy/referenceproxy.cpp
  *  \brief Class game::proxy::ReferenceProxy
  */

#include "game/proxy/referenceproxy.hpp"
#include "game/game.hpp"
#include "game/map/universe.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/turn.hpp"
#include "util/request.hpp"

game::proxy::ReferenceProxy::ReferenceProxy(util::RequestSender<Session> gameSender)
    : m_gameSender(gameSender)
{ }

afl::base::Optional<String_t>
game::proxy::ReferenceProxy::getReferenceName(WaitIndicator& ind, Reference ref, ObjectName which)
{
    class Task : public util::Request<Session> {
     public:
        Task(Reference ref, ObjectName which, afl::base::Optional<String_t>& result)
            : m_ref(ref), m_which(which), m_result(result)
            { }

        void handle(Session& session)
            { m_result = session.getReferenceName(m_ref, m_which); }
     private:
        const Reference m_ref;
        const ObjectName m_which;
        afl::base::Optional<String_t>& m_result;
    };

    afl::base::Optional<String_t> result;
    Task t(ref, which, result);
    ind.call(m_gameSender, t);
    return result;
}

afl::base::Optional<game::map::Point>
game::proxy::ReferenceProxy::getReferencePosition(WaitIndicator& ind, Reference ref)
{
    class Task : public util::Request<Session> {
     public:
        Task(Reference ref)
            : m_ref(ref), m_result()
            { }

        void handle(Session& session)
            {
                if (Game* g = session.getGame().get()) {
                    if (game::map::Object* obj = g->viewpointTurn().universe().getObject(m_ref)) {
                        m_result = obj->getPosition();
                    }
                }
            }

        const afl::base::Optional<game::map::Point>& getResult()
            { return m_result; }

     private:
        const Reference m_ref;
        afl::base::Optional<game::map::Point> m_result;
    };
    Task t(ref);
    ind.call(m_gameSender, t);
    return t.getResult();
}
