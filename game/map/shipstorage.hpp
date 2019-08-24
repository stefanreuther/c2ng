/**
  *  \file game/map/shipstorage.hpp
  */
#ifndef C2NG_GAME_MAP_SHIPSTORAGE_HPP
#define C2NG_GAME_MAP_SHIPSTORAGE_HPP

#include "game/cargocontainer.hpp"
#include "game/interpreterinterface.hpp"
#include "game/config/hostconfiguration.hpp"
#include "afl/base/signalconnection.hpp"
#include "game/spec/shiplist.hpp"

namespace game { namespace map {

    class Ship;

    class ShipStorage : public CargoContainer {
     public:
        ShipStorage(Ship& sh,
                    InterpreterInterface& iface,
                    const game::spec::ShipList& shipList);
        ~ShipStorage();

        virtual String_t getName(afl::string::Translator& tx) const;
        virtual Flags_t getFlags() const;
        virtual bool canHaveElement(Element::Type type) const;
        virtual int32_t getMaxAmount(Element::Type type) const;
        virtual int32_t getMinAmount(Element::Type type) const;
        virtual int32_t getAmount(Element::Type type) const;
        virtual void commit();

     private:
        Ship& m_ship;
        InterpreterInterface& m_interface;
        const game::spec::ShipList& m_shipList;
        afl::base::SignalConnection m_changeConnection;
    };

} }

#endif
