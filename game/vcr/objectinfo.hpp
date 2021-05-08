/**
  *  \file game/vcr/objectinfo.hpp
  *  \brief VCR Object Information
  */
#ifndef C2NG_GAME_VCR_OBJECTINFO_HPP
#define C2NG_GAME_VCR_OBJECTINFO_HPP

#include "afl/string/translator.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/spec/shiplist.hpp"
#include "util/range.hpp"

namespace game { namespace vcr {

    class Object;

    typedef util::Range<int> Range_t;

    /** Information about a planet.
        Ranges can be
        - empty if no valid value can be found
        - unit range if value is known exactly
        - larger range if a range can be determined */
    struct PlanetInfo {
        /** true if remaining properties are valid. */
        bool isValid;

        /** true if planet is known to have a starbase.
            false can mean no base, or base entirely passive (e.g. all tech-1, no ammo) */
        bool hasBase;

        /** Planet mass. */
        int mass;

        /** Range for planetary defense posts. */
        Range_t defense;

        /** Range for starbase defense posts. */
        Range_t baseDefense;

        /** Range for number of starbase fighters. */
        Range_t numBaseFighters;

        /** Range for starbase tech level.
            Values from PCC2:
            - tech_state=0 -> empty range (no valid value found)
            - tech_state=1 -> unit range (exact value known)
            - tech_state=2/3 -> larger range (upper limit known) */
        Range_t baseBeamTech;

        /** Maximum number of base fighters. */
        int maxBaseFighters;

        /** Maximum base defense. */
        int maxBaseDefense;

        PlanetInfo()
            : isValid(false), hasBase(false), mass(0), defense(), baseDefense(), numBaseFighters(), baseBeamTech(),
              maxBaseFighters(0), maxBaseDefense(0)
            { }
    };

    typedef std::pair<String_t, String_t> ShipInfoItem_t;

    /** Information about a ship.
        Contains textual information about a ship in the `.first` positions,
        comparing it to the hull's values in the `.second` position. */
    struct ShipInfo {
        ShipInfoItem_t primary;
        ShipInfoItem_t secondary;
        ShipInfoItem_t ammo;
        ShipInfoItem_t crew;
        ShipInfoItem_t experienceLevel;
        ShipInfoItem_t techLevel;
        ShipInfoItem_t mass;
        ShipInfoItem_t shield;
        ShipInfoItem_t damage;
        ShipInfoItem_t fuel;
        ShipInfoItem_t engine;
    };

    /** Retrieve derived information about a planet in combat.
        \param [out] result  Result
        \param [in]  in      VCR object
        \param [in]  config  Host configuration */
    void describePlanet(PlanetInfo& result, const Object& in, const game::config::HostConfiguration& config);

    /** Retrieve derived information about a ship in combat.
        \param [out] result       Result
        \param [in]  in           VCR object
        \param [in]  shipList     Ship List (for looking up components)
        \param [in]  pAssumedHull Compare against this hull if non-null
        \param [in]  withESB      Whether ESB is active in this fight, see Battle::isESBActive
        \param [in]  config       Host configuration
        \param [in]  tx           Translator
        \param [in]  fmt          Number formatter */
    void describeShip(ShipInfo& result, const Object& in, const game::spec::ShipList& shipList,
                      const game::spec::Hull* pAssumedHull,
                      bool withESB,
                      const game::config::HostConfiguration& config,
                      afl::string::Translator& tx,
                      util::NumberFormatter fmt);

} }


#endif
