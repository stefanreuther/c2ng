/**
  *  \file game/map/shiptransporter.hpp
  *  \brief Class game::map::ShipTransporter
  */
#ifndef C2NG_GAME_MAP_SHIPTRANSPORTER_HPP
#define C2NG_GAME_MAP_SHIPTRANSPORTER_HPP

#include "afl/base/signalconnection.hpp"
#include "afl/string/translator.hpp"
#include "game/cargocontainer.hpp"
#include "game/hostversion.hpp"
#include "game/map/ship.hpp"

namespace game { namespace map {

    class Universe;

    /** Ship transporter.
        Implements CargoContainer for a ship's transporter (ship/ship, ship/planet, jettison). */
    class ShipTransporter : public CargoContainer {
     public:
        /** Constructor.
            @param sh          Ship (must live longer than ShipTransporter)
            @param type        Transporter type to use
            @param targetId    Target unit Id
            @param univ        Universe (must live longer than ShipTransporter)
            @param hostVersion Host version (HostVersion::hasParallelShipTransfers()) */
        ShipTransporter(Ship& sh,
                        Ship::Transporter type,
                        Id_t targetId,
                        const Universe& univ,
                        HostVersion hostVersion);

        // CargoContainer:
        virtual String_t getName(afl::string::Translator& tx) const;
        virtual String_t getInfo1(afl::string::Translator& tx) const;
        virtual String_t getInfo2(afl::string::Translator& tx) const;
        virtual Flags_t getFlags() const;
        virtual bool canHaveElement(Element::Type type) const;
        virtual int32_t getMaxAmount(Element::Type type) const;
        virtual int32_t getMinAmount(Element::Type type) const;
        virtual int32_t getAmount(Element::Type type) const;
        virtual void commit();

     private:
        Ship& m_ship;
        Ship::Transporter m_type;
        Id_t m_targetId;
        const Universe& m_universe;
        bool m_allowParallelTransfer;

        afl::base::SignalConnection m_changeConnection;
    };

} }

#endif
