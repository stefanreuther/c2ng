/**
  *  \file game/proxy/visibilityrangeproxy.cpp
  *  \brief Class game::proxy::VisibilityRangeProxy
  */

#include "game/proxy/visibilityrangeproxy.hpp"
#include "game/game.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"

game::proxy::VisibilityRangeProxy::VisibilityRangeProxy(util::RequestSender<Session> gameSender)
    : m_gameSender(gameSender)
{ }

game::map::VisConfig
game::proxy::VisibilityRangeProxy::loadVisibilityConfiguration(WaitIndicator& ind)
{
    class Task : public util::Request<Session> {
     public:
        Task(game::map::VisConfig& result)
            : m_result(result)
            { }

        virtual void handle(Session& session)
            {
                if (Root* r = session.getRoot().get()) {
                    m_result = game::map::loadVisibilityConfiguration(r->userConfiguration());
                }
            }

     private:
        game::map::VisConfig& m_result;
    };

    game::map::VisConfig result;
    Task t(result);
    ind.call(m_gameSender, t);
    return result;
}

game::map::VisSettings_t
game::proxy::VisibilityRangeProxy::getVisibilityRangeSettings(WaitIndicator& ind)
{
    class Task : public util::Request<Session> {
     public:
        Task(game::map::VisSettings_t& result)
            : m_result(result)
            { }

        virtual void handle(Session& session)
            {
                if (Game* g = session.getGame().get()) {
                    if (Root* r = session.getRoot().get()) {
                        m_result = game::map::getVisibilityRangeSettings(r->hostConfiguration(), g->getViewpointPlayer(), session.translator());
                    }
                }
            }

     private:
        game::map::VisSettings_t& m_result;
    };

    game::map::VisSettings_t result;
    Task t(result);
    ind.call(m_gameSender, t);
    return result;
}

std::auto_ptr<game::map::RangeSet>
game::proxy::VisibilityRangeProxy::buildVisibilityRange(WaitIndicator& ind, const game::map::VisConfig& vc)
{
    class Task : public util::Request<Session> {
     public:
        Task(game::map::RangeSet& result, const game::map::VisConfig& vc)
            : m_result(result), m_visConfig(vc)
            { }

        virtual void handle(Session& session)
            {
                if (Game* g = session.getGame().get()) {
                    game::map::buildVisibilityRange(m_result, g->viewpointTurn().universe(), m_visConfig, g->teamSettings());
                }
                if (Root* r = session.getRoot().get()) {
                    game::map::saveVisibilityConfiguration(r->userConfiguration(), m_visConfig);
                }
            }

     private:
        game::map::RangeSet& m_result;
        const game::map::VisConfig& m_visConfig;
    };

    // This must be a newly-allocated object.
    // It cannot be an in-place operation with the user's object because user might need their copy
    // to plot the previous result while the operation is ongoing, causing parallel access from both threads.
    std::auto_ptr<game::map::RangeSet> result(new game::map::RangeSet());
    Task t(*result, vc);
    ind.call(m_gameSender, t);
    return result;
}
