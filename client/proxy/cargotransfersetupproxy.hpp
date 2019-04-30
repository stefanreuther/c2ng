/**
  *  \file client/proxy/cargotransfersetupproxy.hpp
  */
#ifndef C2NG_CLIENT_PROXY_CARGOTRANSFERSETUPPROXY_HPP
#define C2NG_CLIENT_PROXY_CARGOTRANSFERSETUPPROXY_HPP

#include "client/downlink.hpp"
#include "game/session.hpp"
#include "game/actions/cargotransfersetup.hpp"
#include "util/requestsender.hpp"

namespace client { namespace proxy {

    class CargoTransferSetupProxy {
     public:
        CargoTransferSetupProxy(util::RequestSender<game::Session> gameSender);

        game::actions::CargoTransferSetup createPlanetShip(Downlink& link, int planetId, int shipId);

        game::actions::CargoTransferSetup createShipShip(Downlink& link, int leftId, int rightId);

        game::actions::CargoTransferSetup createShipJettison(Downlink& link, int shipId);

        game::actions::CargoTransferSetup createShipBeamUp(Downlink& link, int shipId);

     private:
        util::RequestSender<game::Session> m_gameSender;
    };

} }

#endif
