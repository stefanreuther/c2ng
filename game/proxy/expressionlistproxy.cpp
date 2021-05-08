/**
  *  \file game/proxy/expressionlistproxy.cpp
  *  \brief Class game::proxy::ExpressionListProxy
  */

#include "game/proxy/expressionlistproxy.hpp"
#include "game/config/expressionlists.hpp"
#include "game/game.hpp"

using game::config::ExpressionLists;
using util::ExpressionList;


game::proxy::ExpressionListProxy::ExpressionListProxy(util::RequestSender<Session> gameSender, game::config::ExpressionLists::Area area)
    : m_gameSender(gameSender),
      m_area(area)
{ }

void
game::proxy::ExpressionListProxy::getList(WaitIndicator& ind, game::config::ExpressionLists::Items_t& out)
{
    class Task : public util::Request<Session> {
     public:
        Task(ExpressionLists::Area area, ExpressionLists::Items_t& out)
            : m_area(area), m_out(out)
            { }

        virtual void handle(Session& s)
            {
                if (Game* g = s.getGame().get()) {
                    g->expressionLists().pack(m_out, m_area, s.translator());
                }
            }

     private:
        ExpressionLists::Area m_area;
        ExpressionLists::Items_t& m_out;
    };
    Task t(m_area, out);
    ind.call(m_gameSender, t);
}

void
game::proxy::ExpressionListProxy::pushRecent(String_t flags, String_t expr)
{
    class Task : public util::Request<Session> {
     public:
        Task(ExpressionLists::Area area, String_t flags, String_t expr)
            : m_area(area), m_flags(flags), m_expression(expr)
            { }

        virtual void handle(Session& s)
            {
                if (Game* g = s.getGame().get()) {
                    g->expressionLists().pushRecent(m_area, m_flags, m_expression);
                }
            }

     private:
        ExpressionLists::Area m_area;
        String_t m_flags;
        String_t m_expression;
    };
    m_gameSender.postNewRequest(new Task(m_area, flags, expr));
}
