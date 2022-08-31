/**
  *  \file game/proxy/referenceproxy.cpp
  *  \brief Class game::proxy::ReferenceProxy
  */

#include "game/proxy/referenceproxy.hpp"
#include "game/proxy/waitindicator.hpp"
#include "util/request.hpp"

game::proxy::ReferenceProxy::ReferenceProxy(util::RequestSender<Session> gameSender)
    : m_gameSender(gameSender)
{ }

bool
game::proxy::ReferenceProxy::getReferenceName(WaitIndicator& ind, Reference ref, ObjectName which, String_t& result)
{
    class Task : public util::Request<Session> {
     public:
        Task(Reference ref, ObjectName which, String_t& result)
            : m_ref(ref), m_which(which), m_result(result), m_ok(false)
            { }

        void handle(Session& session)
            { m_ok = session.getReferenceName(m_ref, m_which, m_result); }

        bool isOK() const
            { return m_ok; }
     private:
        const Reference m_ref;
        const ObjectName m_which;
        String_t& m_result;
        bool m_ok;
    };
    Task t(ref, which, result);
    ind.call(m_gameSender, t);
    return t.isOK();
}
