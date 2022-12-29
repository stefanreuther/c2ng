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

bool
game::proxy::ReferenceProxy::getReferenceName(WaitIndicator& ind, Reference ref, ObjectName which, String_t& result)
{
    class Task : public util::Request<Session> {
     public:
        Task(Reference ref, ObjectName which)
            : m_ref(ref), m_which(which), m_result(), m_ok(false)
            { }

        void handle(Session& session)
            { m_ok = session.getReferenceName(m_ref, m_which, m_result); }

        bool isOK() const
            { return m_ok; }

        const String_t& getResult() const
            { return m_result; }
     private:
        const Reference m_ref;
        const ObjectName m_which;
        String_t m_result;
        bool m_ok;
    };
    Task t(ref, which);
    ind.call(m_gameSender, t);
    if (t.isOK()) {
        result = t.getResult();
        return true;
    } else {
        return false;
    }
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
                    if (Turn* t = g->getViewpointTurn().get()) {
                        if (game::map::Object* obj = t->universe().getObject(m_ref)) {
                            m_result = obj->getPosition();
                        }
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
