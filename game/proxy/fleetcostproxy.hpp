/**
  *  \file game/proxy/fleetcostproxy.hpp
  *  \brief Class game::proxy::FleetCostProxy
  */
#ifndef C2NG_GAME_PROXY_FLEETCOSTPROXY_HPP
#define C2NG_GAME_PROXY_FLEETCOSTPROXY_HPP

#include "game/proxy/simulationadaptor.hpp"
#include "game/sim/fleetcost.hpp"
#include "game/spec/costsummary.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    class WaitIndicator;

    /** Fleet cost summary proxy.

        This is a bidirectional, synchronous proxy to access the fleet cost summary for a battle simulation.
        It implements a simple call/return scheme with no asynchronous notifications */
    class FleetCostProxy {
     public:
        /** Constructor.
            \param adaptorSender Access to SimulationAdaptor */
        explicit FleetCostProxy(util::RequestSender<SimulationAdaptor> adaptorSender);
        ~FleetCostProxy();

        /** Set options.
            New options will apply for the next computeFleetCosts call.
            \param opts New options */
        void setOptions(const game::sim::FleetCostOptions& opts);

        /** Get current options.
            \param [in]  ind  WaitIndicator for UI synchronisation
            \param [out] opts Options */
        void getOptions(WaitIndicator& ind, game::sim::FleetCostOptions& opts);

        /** Compute fleet cost.
            \param [in]  ind     WaitIndicator for UI synchronisation
            \param [in]  players Players to retrieve information for
            \param [in]  isTeam  true if players is actually a set of team numbers
            \param [out] out     Result
            \see game::sim::computeFleetCosts */
        void computeFleetCosts(WaitIndicator& ind, PlayerSet_t players, bool isTeam, game::spec::CostSummary& out);

        /** Get set of players involved in setup.
            \param ind WaitIndicator for UI synchronisation
            \return player set
            \see game::sim::Setup::getInvolvedPlayers */
        PlayerSet_t getInvolvedPlayers(WaitIndicator& ind);

        /** Get set of teams involved in setup.
            \param ind WaitIndicator for UI synchronisation
            \return team set
            \see game::sim::Setup::getInvolvedTeams */
        PlayerSet_t getInvolvedTeams(WaitIndicator& ind);

     private:
        util::RequestSender<SimulationAdaptor> m_adaptorSender;
        game::sim::FleetCostOptions m_options;
    };

} }

#endif
