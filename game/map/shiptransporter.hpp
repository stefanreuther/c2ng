/**
  *  \file game/map/shiptransporter.hpp
  */
#ifndef C2NG_GAME_MAP_SHIPTRANSPORTER_HPP
#define C2NG_GAME_MAP_SHIPTRANSPORTER_HPP

#include "afl/base/signalconnection.hpp"
#include "game/cargocontainer.hpp"
#include "game/interpreterinterface.hpp"
#include "game/map/ship.hpp"
#include "game/hostversion.hpp"

namespace game { namespace map {

    class Universe;

    class ShipTransporter : public CargoContainer {
     public:
        ShipTransporter(Ship& sh,
                        Ship::Transporter type,
                        Id_t targetId,
                        InterpreterInterface& iface,
                        const Universe& univ,
                        HostVersion hostVersion);

        virtual String_t getName(afl::string::Translator& tx) const;
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
        InterpreterInterface& m_interface;
        const Universe& m_universe;
        bool m_allowParallelTransfer;

        afl::base::SignalConnection m_changeConnection;
    };

} }

#endif
