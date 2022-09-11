/**
  *  \file game/proxy/globalactionproxy.cpp
  *  \brief Class game::proxy::GlobalActionProxy
  */

#include "game/proxy/globalactionproxy.hpp"
#include "game/interface/globalactioncontext.hpp"
#include "game/proxy/waitindicator.hpp"

game::proxy::GlobalActionProxy::GlobalActionProxy(util::RequestSender<Session> gameSender)
    : m_gameSender(gameSender)
{ }

void
game::proxy::GlobalActionProxy::getActions(WaitIndicator& ind, util::TreeList& result, interpreter::VariableReference ref)
{
    class Task : public util::Request<Session> {
     public:
        Task(util::TreeList& result, const interpreter::VariableReference& ref)
            : m_result(result), m_ref(ref)
            { }
        virtual void handle(Session& session)
            {
                if (game::interface::GlobalActionContext* ctx = dynamic_cast<game::interface::GlobalActionContext*>(m_ref.get(session.processList()))) {
                    m_result = ctx->data()->actionNames;
                }
            }

     private:
        util::TreeList& m_result;
        interpreter::VariableReference m_ref;
    };

    Task t(result, ref);
    result.clear();
    ind.call(m_gameSender, t);
}
