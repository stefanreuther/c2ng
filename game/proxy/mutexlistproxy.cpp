/**
  *  \file game/proxy/mutexlistproxy.cpp
  *  \brief Class game::proxy::MutexListProxy
  */

#include "game/proxy/mutexlistproxy.hpp"

// Constructor.
game::proxy::MutexListProxy::MutexListProxy(util::RequestSender<Session> gameSender)
    : m_gameSender(gameSender)
{ }    

// Enumerate mutexes owned by a process.
void
game::proxy::MutexListProxy::enumMutexes(WaitIndicator& link, Infos_t& result, uint32_t processId)
{
    class Task : public util::Request<Session> {
     public:
        Task(Infos_t& result, uint32_t processId)
            : m_result(result), m_processId(processId)
            { }
        virtual void handle(Session& session)
            {
                if (interpreter::Process* p = session.processList().getProcessById(m_processId)) {
                    buildList(m_result, session, p);
                }
            }
     private:
        Infos_t& m_result;
        uint32_t m_processId;
    };
    Task t(result, processId);
    link.call(m_gameSender, t);
}

// Enumerate all active mutexes.
void
game::proxy::MutexListProxy::enumMutexes(WaitIndicator& link, Infos_t& result)
{
    class Task : public util::Request<Session> {
     public:
        Task(Infos_t& result)
            : m_result(result)
            { }
        virtual void handle(Session& session)
            { buildList(m_result, session, 0); }
     private:
        Infos_t& m_result;
    };
    Task t(result);
    link.call(m_gameSender, t);
}

void
game::proxy::MutexListProxy::buildList(Infos_t& result, Session& session, interpreter::Process* p)
{
    // Get list of mutexes
    std::vector<interpreter::MutexList::Mutex*> data;
    session.world().mutexList().enumMutexes(data, p);

    // Produce result
    for (size_t i = 0, n = data.size(); i < n; ++i) {
        if (const interpreter::MutexList::Mutex* mtx = data[i]) {
            uint32_t processId = 0;
            if (const interpreter::Process* owner = mtx->getOwner()) {
                processId = owner->getProcessId();
            }
            result.push_back(Info(mtx->getName(), processId));
        }
    }
}
