/**
  *  \file game/actions/cargotransfersetup.hpp
  *  \brief Class game::actions::CargoTransferSetup
  */
#ifndef C2NG_GAME_ACTIONS_CARGOTRANSFERSETUP_HPP
#define C2NG_GAME_ACTIONS_CARGOTRANSFERSETUP_HPP

#include "game/hostversion.hpp"
#include "game/map/object.hpp"
#include "game/map/universe.hpp"
#include "game/spec/shiplist.hpp"

namespace game { namespace actions {

    class CargoTransfer;

    /** Cargo transfer setup logic.
        This class supports a multitude of usecases for validation and setup of bilateral cargo transfers.
        It can be used to configure a CargoTransfer object and will create the correct CargoContainer descendants,
        using regular cargo holds (ShipStorage, PlanetStorage) and transporters (ShipTransporter).

        This class builds bilateral cargo transfers (two participants), known as "left" and "right".
        Those are accessible as indexes 0 and 1 in the CargoTransfer, respectively.
        You can swap sides after building using swapSides() before calling build().

        A planet-ship transfer can use a proxy, which is an (invisible) third participant.
        This means, planet cargo is transferred directly into the ship transporter.

        - Find possible cargo transfer partners:
          - Iterate through possibilities. Call constructor function. Check status and act accordingly.
        - Find possible proxies for A to B:
          - Call constructor function. Iterate through possible proxies. Call isValidProxy().
        - Make a transfer from A to B:
          - Call constructor function. Call build(). If the transfer is invalid, build() throws.
        - Make a transfer from A to B with maximum user interaction:
          - Call constructor function.
          - Check status (getStatus()). If proxy needed, provide a proxy.
          - Call getConflictingTransferShipId(). Warn/confirm if needed.
          - Call build().

        This is a data class that does not hold any references and can be copied as needed. */
    class CargoTransferSetup {
     public:
        /** Status. */
        enum Result {
            Ready,              ///< Setup is complete. Check getConflictingTransferShipId() if needed, then build().
            NeedProxy,          ///< Setup can be completed by adding a proxy. Call setProxy().
            Impossible          ///< Invalid setup.
        };

        /** Default constructor.
            Makes a setup that reports Impossible. */
        CargoTransferSetup();

        /** Construct from a planet and ship.
            This validates the ship and planet's Id, visibility/playability status and position.
            After build(), the planet will appear as CargoTransfer::get(0), the ship will appear as CargoTransfer::get(1).
            \param univ Universe
            \param planetId Planet Id
            \param shipId Ship Id
            \return setup */
        static CargoTransferSetup fromPlanetShip(const game::map::Universe& univ, int planetId, int shipId);

        /** Construct from two ships.
            This validates the ships' Ids, visibility/playability status and position.
            After build(), the left ship will appear as CargoTransfer::get(0), the right ship will appear as CargoTransfer::get(1).
            \param univ Universe
            \param leftId First ship Id
            \param rightId Second ship Id
            \return setup */
        static CargoTransferSetup fromShipShip(const game::map::Universe& univ, int leftId, int rightId);

        /** Construct for jettison.
            This validates the ships' Ids, visibility/playability status and position (jettison not allowed when orbiting a planet).
            After build(), the ship will appear as CargoTransfer::get(0), the jettison transporter will appear as CargoTransfer::get(1).
            \param univ Universe
            \param shipId Ship Id
            \return setup */
        static CargoTransferSetup fromShipJettison(const game::map::Universe& univ, int shipId);

        /** Swap sides.
            Reverses the order in which results will be produced in build(). */
        void swapSides();

        /** Get setup status.
            \return status */
        Result getStatus() const;

        /** Check valid proxy.
            \param univ Universe
            \param shipId Ship Id
            \retval true the ship is a valid proxy
            \retval false the ship is not a valid proxy, or no proxy was required */
        bool isValidProxy(const game::map::Universe& univ, int shipId) const;

        /** Set proxy.
            \param univ Universe
            \param shipId Ship Id
            \return true if ship was accepted (see isValidProxy()) */
        bool setProxy(const game::map::Universe& univ, Id_t shipId);

        /** Check for conflicting transfer.
            If this setup requires a ship to transfer cargo to ship X,
            but that ship is already transferring elsewhere, this is a conflicting transfer.
            \param univ Universe
            \return ship Id if any, 0 if no more conflicts */
        Id_t getConflictingTransferShipId(const game::map::Universe& univ) const;

        /** Cancel conflicting transfer.
            \param univ Universe
            \param shipId ship Id */
        void cancelConflictingTransfer(game::map::Universe& univ, Id_t shipId);

        /** Build CargoTransfer action.
            \param action   [out] Target action. Must be constructed and empty.
            \param univ     [in/out] Universe
            \param iface    [in] Interface (needed to construct CargoContainer descendants)
            \param config   [in] Host configuration (needed to construct CargoContainer descendants)
            \param shipList [in] Ship list (needed to construct CargoContainer descendants)
            \param version  [in] Host version (needed to construct CargoContainer descendants)
            \throw Exception if setup is incomplete/impossible */
        void build(CargoTransfer& action,
                   game::map::Universe& univ,
                   InterpreterInterface& iface,
                   const game::config::HostConfiguration& config,
                   const game::spec::ShipList& shipList,
                   const game::HostVersion& version);

     private:
        enum Action {
            Invalid,
            UsePlanetStorage,   // Use PlanetStorage(this side)
            UseShipStorage,     // Use ShipStorage(this side)
            UseOtherUnload,     // Use ShipTransporter(other side, UnloadTransporter, this side)
            UseOtherTransfer,   // Use ShipTransporter(other side, TransferTransporter, this side)
            UseProxyTransfer    // Use ShipTransporter(proxy, UnloadTransporter, this side)
        };

        enum Side {
            Left,
            Right,
            Proxy
        };

        Action m_actions[2];
        Id_t   m_ids[3];

        CargoTransferSetup(Action leftAction, int leftId, Action rightAction, int rightId);

        bool checkProxyPlanet(const game::map::Universe& univ, int shipId, int planetId) const;
    };

} }

#endif
