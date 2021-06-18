/**
  *  \file game/proxy/vcroverviewproxy.hpp
  *  \brief Class game::proxy::VcrOverviewProxy
  */
#ifndef C2NG_GAME_PROXY_VCROVERVIEWPROXY_HPP
#define C2NG_GAME_PROXY_VCROVERVIEWPROXY_HPP

#include "util/requestsender.hpp"
#include "game/proxy/vcrdatabaseadaptor.hpp"
#include "game/vcr/overview.hpp"
#include "game/proxy/waitindicator.hpp"

namespace game { namespace proxy {

    /** Bidirectional proxy for VCR database access.
        Proxies access to a game::vcr::Overview.

        The underlying game::vcr::Database object is selected using a VcrDatabaseAdaptor instance provided by the caller.
        That adaptor also provides a few surrounding objects. */
    class VcrOverviewProxy {
     public:
        /** Constructor.
            \param sender Access to VcrDatabaseAdaptor */
        VcrOverviewProxy(util::RequestSender<VcrDatabaseAdaptor> sender);
        ~VcrOverviewProxy();

        /** Build diagram.
            \param [in]  ind  UI synchronisation
            \param [out] out  Diagram
            \see game::vcr::Overview::buildDiagram */
        void buildDiagram(WaitIndicator& ind, game::vcr::Overview::Diagram& out);

        /** Build score summary.
            \param [in]  ind  UI synchronisation
            \param [out] out  Summary
            \see game::vcr::Overview::buildScoreSummary */
        void buildScoreSummary(WaitIndicator& ind, game::vcr::Overview::ScoreSummary& out);

     private:
        class Trampoline;
        class TrampolineFromAdaptor;

        util::RequestSender<Trampoline> m_request;
    };

} }

#endif
