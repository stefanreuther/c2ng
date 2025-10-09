/**
  *  \file game/proxy/fleetcostproxy.cpp
  *  \brief Class game::proxy::FleetCostProxy
  */

#include "game/proxy/fleetcostproxy.hpp"
#include "game/proxy/simulationsetupproxy.hpp"
#include "game/game.hpp"
#include "game/root.hpp"

using game::sim::FleetCostOptions;

namespace game { namespace proxy { namespace {

    void computeFleetCostsImpl(SimulationAdaptor& s, const FleetCostOptions& opts, PlayerSet_t players, bool isTeam, game::spec::CostSummary& out)
    {
        // Map teams; fail if we don't have teams
        if (isTeam) {
            const TeamSettings* t = s.getTeamSettings();
            if (t == 0) {
                return;
            }

            PlayerSet_t mappedPlayers;
            for (int i = 1; i <= MAX_PLAYERS; ++i) {
                if (players.contains(t->getPlayerTeam(i))) {
                    mappedPlayers += i;
                }
            }
            players = mappedPlayers;
        }

        // Check preconditions; fail if we don't have them
        const Root* r = s.getRoot().get();
        const game::spec::ShipList* sl = s.getShipList().get();
        if (r == 0 || sl == 0) {
            return;
        }

        // Operate
        game::sim::Session& simSession = s.simSession();
        game::sim::computeFleetCosts(out, simSession.setup(), simSession.configuration(), opts, *sl, r->hostConfiguration(), r->playerList(), players, s.translator());
    }

} } }



game::proxy::FleetCostProxy::FleetCostProxy(util::RequestSender<SimulationAdaptor> adaptorSender)
    : m_adaptorSender(adaptorSender),
      m_options()
{ }

game::proxy::FleetCostProxy::~FleetCostProxy()
{ }

void
game::proxy::FleetCostProxy::setOptions(const game::sim::FleetCostOptions& opts)
{
    m_options = opts;
}

void
game::proxy::FleetCostProxy::getOptions(WaitIndicator& ind, game::sim::FleetCostOptions& opts)
{
    // FIXME: should persist the configuration somehow
    (void) ind;
    opts = m_options;
}

void
game::proxy::FleetCostProxy::computeFleetCosts(WaitIndicator& ind, PlayerSet_t players, bool isTeam, game::spec::CostSummary& out)
{
    class Task : public util::Request<SimulationAdaptor> {
     public:
        Task(const FleetCostOptions& opts, PlayerSet_t players, bool isTeam, game::spec::CostSummary& out)
            : m_options(opts), m_players(players), m_isTeam(isTeam), m_out(out)
            { }
        virtual void handle(SimulationAdaptor& s)
            { computeFleetCostsImpl(s, m_options, m_players, m_isTeam, m_out); }
     private:
        FleetCostOptions m_options;
        PlayerSet_t m_players;
        bool m_isTeam;
        game::spec::CostSummary& m_out;
    };

    Task t(m_options, players, isTeam, out);
    ind.call(m_adaptorSender, t);
}

game::PlayerSet_t
game::proxy::FleetCostProxy::getInvolvedPlayers(WaitIndicator& ind)
{
    class Task : public util::Request<SimulationAdaptor> {
     public:
        Task()
            : m_result()
            { }
        virtual void handle(SimulationAdaptor& s)
            { m_result = s.simSession().setup().getInvolvedPlayers(); }
        PlayerSet_t getResult() const
            { return m_result; }
     private:
        PlayerSet_t m_result;
    };

    Task t;
    ind.call(m_adaptorSender, t);
    return t.getResult();
}

game::PlayerSet_t
game::proxy::FleetCostProxy::getInvolvedTeams(WaitIndicator& ind)
{
    class Task : public util::Request<SimulationAdaptor> {
     public:
        Task()
            : m_result()
            { }
        virtual void handle(SimulationAdaptor& s)
            {
                const TeamSettings* t = s.getTeamSettings();
                if (t != 0) {
                    m_result = s.simSession().setup().getInvolvedTeams(*t);
                }
            }
        PlayerSet_t getResult() const
            { return m_result; }
     private:
        PlayerSet_t m_result;
    };

    Task t;
    ind.call(m_adaptorSender, t);
    return t.getResult();
}
