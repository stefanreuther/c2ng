/**
  *  \file game/proxy/cargotransfersetupproxy.hpp
  *  \brief Class game::proxy::CargoTransferSetupProxy
  */
#ifndef C2NG_GAME_PROXY_CARGOTRANSFERSETUPPROXY_HPP
#define C2NG_GAME_PROXY_CARGOTRANSFERSETUPPROXY_HPP

#include "game/actions/cargotransfersetup.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/session.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    /** Cargo transfer setup proxy.

        This is a bidirectional, synchronous proxy that allows creating game::actions::CargoTransferSetup objects.
        These objects can be used to set up cargo transfer. */
    class CargoTransferSetupProxy {
     public:
        /** Constructor.
            \param gameSender Game sender */
        CargoTransferSetupProxy(util::RequestSender<Session> gameSender);

        /** Construct from a planet and ship.
            \param link WaitIndicator
            \param planetId Planet Id
            \param shipId Ship Id
            \return setup
            \see game::actions::CargoTransferSetup::fromPlanetShip() */
        game::actions::CargoTransferSetup createPlanetShip(WaitIndicator& link, Id_t planetId, Id_t shipId);

        /** Construct from a two ships.
            \param link WaitIndicator
            \param leftId First ship Id
            \param rightId Second ship Id
            \return setup
            \see game::actions::CargoTransferSetup::fromShipShip() */
        game::actions::CargoTransferSetup createShipShip(WaitIndicator& link, Id_t leftId, Id_t rightId);

        /** Construct for jettison.
            \param link WaitIndicator
            \param shipId Ship Id
            \return setup
            \see game::actions::CargoTransferSetup::fromShipJettison() */
        game::actions::CargoTransferSetup createShipJettison(WaitIndicator& link, Id_t shipId);

        /** Construct for beam-up-multiple mission.
            \param link WaitIndicator
            \param shipId Ship Id
            \return setup
            \see game::actions::CargoTransferSetup::fromShipBeamUp() */
        game::actions::CargoTransferSetup createShipBeamUp(WaitIndicator& link, Id_t shipId);

     private:
        util::RequestSender<Session> m_gameSender;
    };

} }

#endif
