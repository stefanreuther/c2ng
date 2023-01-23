/**
  *  \file game/proxy/fleetcostproxy.cpp
  *  \brief Class game::proxy::FleetCostProxy
  *
  *  FIXME 20201219: The idea of this class is that it provides the summary for the setup edited by a SimulationSetupProxy.
  *  Should the SimulationSetupProxy be able to access a setup other than the game::sim::Session, we'd need some handover.
  *  Currently, infrastructure to do that is missing, so we just take the game::Session from the SimulationSetupProxy,
  *  and find the game::sim::Session internally.
  */

#include "game/proxy/fleetcostproxy.hpp"
#include "game/sim/sessionextra.hpp"
#include "game/proxy/simulationsetupproxy.hpp"
#include "game/game.hpp"
#include "game/root.hpp"

using game::sim::FleetCostOptions;
using game::sim::getSimulatorSession;

namespace game { namespace proxy { namespace {

    void computeFleetCostsImpl(Session& s, const FleetCostOptions& opts, PlayerSet_t players, bool isTeam, game::spec::CostSummary& out)
    {
        // Map teams; fail if we don't have teams
        if (isTeam) {
            Game* g = s.getGame().get();
            if (g == 0) {
                return;
            }

            PlayerSet_t mappedPlayers;
            for (int i = 1; i <= MAX_PLAYERS; ++i) {
                if (players.contains(g->teamSettings().getPlayerTeam(i))) {
                    mappedPlayers += i;
                }
            }
            players = mappedPlayers;
        }

        // Check preconditions; fail if we don't have them
        Root* r = s.getRoot().get();
        game::spec::ShipList* sl = s.getShipList().get();
        if (r == 0 || sl == 0) {
            return;
        }

        // Operate
        afl::base::Ref<game::sim::Session> simSession = getSimulatorSession(s);
        game::sim::computeFleetCosts(out, simSession->setup(), simSession->configuration(), opts, *sl, r->hostConfiguration(), r->playerList(), players, s.translator());

    }

} } }



game::proxy::FleetCostProxy::FleetCostProxy(SimulationSetupProxy& setup)
    : m_gameSender(setup.gameSender()),
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
    class Task : public util::Request<Session> {
     public:
        Task(const FleetCostOptions& opts, PlayerSet_t players, bool isTeam, game::spec::CostSummary& out)
            : m_options(opts), m_players(players), m_isTeam(isTeam), m_out(out)
            { }
        virtual void handle(Session& s)
            { computeFleetCostsImpl(s, m_options, m_players, m_isTeam, m_out); }
     private:
        FleetCostOptions m_options;
        PlayerSet_t m_players;
        bool m_isTeam;
        game::spec::CostSummary& m_out;
    };

    Task t(m_options, players, isTeam, out);
    ind.call(m_gameSender, t);
}

game::PlayerSet_t
game::proxy::FleetCostProxy::getInvolvedPlayers(WaitIndicator& ind)
{
    class Task : public util::Request<Session> {
     public:
        Task()
            : m_result()
            { }
        virtual void handle(Session& s)
            { m_result = getSimulatorSession(s)->setup().getInvolvedPlayers(); }
        PlayerSet_t getResult() const
            { return m_result; }
     private:
        PlayerSet_t m_result;
    };

    Task t;
    ind.call(m_gameSender, t);
    return t.getResult();
}

game::PlayerSet_t
game::proxy::FleetCostProxy::getInvolvedTeams(WaitIndicator& ind)
{
    class Task : public util::Request<Session> {
     public:
        Task()
            : m_result()
            { }
        virtual void handle(Session& s)
            {
                Game* g = s.getGame().get();
                if (g != 0) {
                    m_result = getSimulatorSession(s)->setup().getInvolvedTeams(g->teamSettings());
                }
            }
        PlayerSet_t getResult() const
            { return m_result; }
     private:
        PlayerSet_t m_result;
    };

    Task t;
    ind.call(m_gameSender, t);
    return t.getResult();
}
