/**
  *  \file game/map/beamupshiptransfer.hpp
  */
#ifndef C2NG_GAME_MAP_BEAMUPSHIPTRANSFER_HPP
#define C2NG_GAME_MAP_BEAMUPSHIPTRANSFER_HPP

#include "game/cargocontainer.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/interpreterinterface.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"
#include "util/vector.hpp"

namespace game { namespace map {

    class Ship;

    class BeamUpShipTransfer : public CargoContainer {
     public:
        BeamUpShipTransfer(Ship& sh,
                           const game::spec::ShipList& shipList,
                           Turn& turn,
                           const game::config::HostConfiguration& config);

        ~BeamUpShipTransfer();

        virtual String_t getName(afl::string::Translator& tx) const;
        virtual Flags_t getFlags() const;
        virtual bool canHaveElement(Element::Type type) const;
        virtual int32_t getMaxAmount(Element::Type type) const;
        virtual int32_t getMinAmount(Element::Type type) const;
        virtual int32_t getAmount(Element::Type type) const;
        virtual void commit();

     private:
        Ship& m_ship;
        const game::spec::ShipList& m_shipList;
        Turn& m_turn;
        const game::config::HostConfiguration& m_config;

        util::Vector<int32_t,Element::Type> m_amount;
    };

    void parseBeamUpCommand(util::Vector<int32_t,Element::Type>& out, Turn& turn, const Ship& ship, int factor);

} }

#endif
