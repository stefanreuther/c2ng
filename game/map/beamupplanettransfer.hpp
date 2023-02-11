/**
  *  \file game/map/beamupplanettransfer.hpp
  *  \brief Class game::map::BeamUpPlanetTransfer
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

    /** "Beam Up Multiple" cargo transfer, planet side.
        Displays just the status, but does not generate any commands.
        Use together with BeamUpShipTransfer. */
    class BeamUpPlanetTransfer : public CargoContainer {
     public:
        /** Constructor.
            @param pl   Planet
            @param sh   Ship (for retrieving command)
            @param turn Turn (for retrieving command)
            @param config  Host configuration (for AllowBeamUpClans) */
        BeamUpPlanetTransfer(Planet& pl, const Ship& sh, Turn& turn, const game::config::HostConfiguration& config);

        /** Destructor. */
        ~BeamUpPlanetTransfer();

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
        Planet& m_planet;
        const game::config::HostConfiguration& m_config;

        util::Vector<int32_t,Element::Type> m_amount;
    };

} }

#endif
