/**
  *  \file game/map/beamupplanettransfer.hpp
  */
#ifndef C2NG_GAME_MAP_BEAMUPPLANETTRANSFER_HPP
#define C2NG_GAME_MAP_BEAMUPPLANETTRANSFER_HPP

#include "game/cargocontainer.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/element.hpp"
#include "game/turn.hpp"
#include "util/vector.hpp"

namespace game { namespace map {

    class Planet;
    class Ship;

    class BeamUpPlanetTransfer : public CargoContainer {
     public:
        BeamUpPlanetTransfer(Planet& pl,
                             const Ship& sh,
                             Turn& turn,
                             const game::config::HostConfiguration& config);

        ~BeamUpPlanetTransfer();

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
        Planet& m_planet;
        Turn& m_turn;
        const game::config::HostConfiguration& m_config;

        util::Vector<int32_t,Element::Type> m_amount;
    };

} }

#endif
