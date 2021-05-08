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
        /** Prepared information about a conflict.
            FIXME: this needs additional handling for transfer to planet */
        struct ConflictInfo {
            Id_t fromId;                 /**< Originating unit Id. */
            Id_t toId;                   /**< Target unit Id. */
            String_t fromName;           /**< Originating unit name. */
            String_t toName;             /**< Target unit name. */
            ConflictInfo()
                : fromId(), toId(), fromName(), toName()
                { }
        };

        /** Constructor.
            \param gameSender Game sender */
        CargoTransferSetupProxy(util::RequestSender<Session> gameSender);


        /*
         *  Construction/modification
         */

        /** Construct from a planet and ship.
            \param link WaitIndicator
            \param planetId Planet Id
            \param shipId Ship Id
            \see game::actions::CargoTransferSetup::fromPlanetShip() */
        void createPlanetShip(WaitIndicator& link, Id_t planetId, Id_t shipId);

        /** Construct from a two ships.
            \param link WaitIndicator
            \param leftId First ship Id
            \param rightId Second ship Id
            \see game::actions::CargoTransferSetup::fromShipShip() */
        void createShipShip(WaitIndicator& link, Id_t leftId, Id_t rightId);

        /** Construct for jettison.
            \param link WaitIndicator
            \param shipId Ship Id
            \see game::actions::CargoTransferSetup::fromShipJettison() */
        void createShipJettison(WaitIndicator& link, Id_t shipId);

        /** Construct for beam-up-multiple mission.
            \param link WaitIndicator
            \param shipId Ship Id
            \see game::actions::CargoTransferSetup::fromShipBeamUp() */
        void createShipBeamUp(WaitIndicator& link, Id_t shipId);

        /** Swap sides.
            Reverses the order in which results will be produced in get().
            \see game::actions::CargoTransferSetup::swapSides() */
        void swapSides();

        /** Cancel conflicting transfer.
            This will modify the underlying universe.
            If getConflictInfo() returned null, this is a no-op.
            \param link WaitIndicator
            \see game::actions::CargoTransferSetup::cancelConflictingTransfer() */
        void cancelConflictingTransfer(WaitIndicator& link);


        /*
         *  Inquiry
         */

        /** Check for conflicting transfer.
            \return information if a conflict exists, null if no more conflicts */
        const ConflictInfo* getConflictInfo() const;

        /** Get constructed setup.
            \return setup */
        game::actions::CargoTransferSetup get() const;

     private:
        struct Status {
            game::actions::CargoTransferSetup setup;
            ConflictInfo conflict;
        };
        Status m_status;
        util::RequestSender<Session> m_gameSender;

        static void checkConflict(Session& s, Status& st);
    };

} }

#endif
