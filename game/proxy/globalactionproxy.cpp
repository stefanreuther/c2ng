/**
  *  \file game/proxy/globalactionproxy.cpp
  *  \brief Class game::proxy::GlobalActionProxy
  */

#include "game/proxy/globalactionproxy.hpp"
#include "game/interface/globalactionextra.hpp"
#include "game/proxy/waitindicator.hpp"

game::proxy::GlobalActionProxy::GlobalActionProxy(util::RequestSender<Session> gameSender)
    : m_gameSender(gameSender)
{ }

void
game::proxy::GlobalActionProxy::getActions(WaitIndicator& ind, util::TreeList& result)
{
    class Task : public util::Request<Session> {
     public:
        Task(util::TreeList& result)
            : m_result(result)
            { }
        virtual void handle(Session& session)
            {
                if (game::interface::GlobalActionExtra* p = game::interface::GlobalActionExtra::get(session)) {
                    m_result = p->actionNames();
                }
            }

     private:
        util::TreeList& m_result;
    };

    Task t(result);
    result.clear();
    ind.call(m_gameSender, t);
}
