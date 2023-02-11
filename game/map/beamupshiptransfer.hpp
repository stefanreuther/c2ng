/**
  *  \file game/map/beamupshiptransfer.hpp
  *  \brief Class game::map::BeamUpShipTransfer
  */
#ifndef C2NG_GAME_MAP_BEAMUPSHIPTRANSFER_HPP
#define C2NG_GAME_MAP_BEAMUPSHIPTRANSFER_HPP

#include "afl/string/translator.hpp"
#include "game/cargocontainer.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/interpreterinterface.hpp"
#include "game/map/configuration.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"
#include "util/vector.hpp"

namespace game { namespace map {

    class Ship;

    /** "Beam Up Multiple" cargo transfer, ship side.
        This side implements creating the command.
        Use together with BeamUpPlanetTransfer. */
    class BeamUpShipTransfer : public CargoContainer {
     public:
        /** Constructor.
            @param sh        Ship
            @param shipList  Ship list (for cargo capacity)
            @param turn      Turn (for retrieving/updating command)
            @param mapConfig Map configuration (for mission update)
            @param config    Host configuration (for AllowBeamUpClans, ExtMissionsStartAt) */
        BeamUpShipTransfer(Ship& sh,
                           const game::spec::ShipList& shipList,
                           Turn& turn,
                           const game::map::Configuration& mapConfig,
                           const game::config::HostConfiguration& config);

        /** Destructor. */
        ~BeamUpShipTransfer();

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
        Turn& m_turn;
        const game::map::Configuration& m_mapConfig;
        const game::config::HostConfiguration& m_config;

        util::Vector<int32_t,Element::Type> m_amount;
    };

    /** Parse "beam up" command.
        Parses the ship's "beam up" (Command::Beamup) command, and stores its cargo amount in the given vector,
        scaled by the factor.

        @param [out] out      Result produced here
        @param [in]  turn     Turn (for retrieving command)
        @param [in]  ship     Ship (for retrieving command)
        @param [in]  factor   Factor */
    void parseBeamUpCommand(util::Vector<int32_t,Element::Type>& out, const Turn& turn, const Ship& ship, int factor);

} }

#endif
