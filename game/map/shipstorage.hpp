/**
  *  \file game/map/shipstorage.hpp
  *  \brief Class game::map::ShipStorage
  */
#ifndef C2NG_GAME_MAP_SHIPSTORAGE_HPP
#define C2NG_GAME_MAP_SHIPSTORAGE_HPP

#include "afl/base/signalconnection.hpp"
#include "afl/string/translator.hpp"
#include "game/cargocontainer.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/spec/shiplist.hpp"

namespace game { namespace map {

    class Ship;

    /** Ship cargo transfer.
        Implements CargoContainer for a played ship.
        Cargo can be transferred to/from cargo hold, including money and ammo. */
    class ShipStorage : public CargoContainer {
     public:
        /** Constructor.
            @param sh        Ship (must live longer than ShipStorage)
            @param shipList  Ship list (for cargo capacity; must live longer than ShipStorage) */
        ShipStorage(Ship& sh, const game::spec::ShipList& shipList);

        /** Destructor. */
        ~ShipStorage();

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
        const game::spec::ShipList& m_shipList;
        afl::base::SignalConnection m_changeConnection;
    };

} }

#endif
