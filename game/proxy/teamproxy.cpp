/**
  *  \file game/proxy/teamproxy.cpp
  *  \brief Class game::proxy::TeamProxy
  */

#include "game/proxy/teamproxy.hpp"
#include "game/game.hpp"

game::proxy::TeamProxy::TeamProxy(util::RequestSender<Session> gameSender)
    : m_gameSender(gameSender)
{ }

void
game::proxy::TeamProxy::init(WaitIndicator& link, TeamSettings& out)
{
    class Task : public util::Request<Session> {
     public:
        Task(TeamSettings& out)
            : m_out(out)
            { }

        virtual void handle(Session& s)
            {
                if (Game* g = s.getGame().get()) {
                    m_out.copyFrom(g->teamSettings());
                }
            }

     private:
        TeamSettings& m_out;
    };

    Task t(out);
    link.call(m_gameSender, t);
}

void
game::proxy::TeamProxy::commit(const TeamSettings& in)
{
    class Task : public util::Request<Session> {
     public:
        Task(const TeamSettings& in)
            : m_in()
            { m_in.copyFrom(in); }

        virtual void handle(Session& s)
            {
                if (Game* g = s.getGame().get()) {
                    g->teamSettings().copyFrom(m_in);
                }
            }

     private:
        TeamSettings m_in;
    };

    m_gameSender.postNewRequest(new Task(in));
}
