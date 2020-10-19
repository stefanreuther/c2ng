/**
  *  \file game/map/planetformula.hpp
  */
#ifndef C2NG_GAME_MAP_PLANETFORMULA_HPP
#define C2NG_GAME_MAP_PLANETFORMULA_HPP

#include "game/types.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/hostversion.hpp"
#include "game/element.hpp"

namespace game { namespace map {

    class Planet;

    /** Get maximum number of structures for a planet.
        This uses a hyphotetical population. */
    LongProperty_t getMaxBuildings(const Planet& p, PlanetaryBuilding kind, const game::config::HostConfiguration& config, LongProperty_t clans);

    /** Get maximum number of structures for a planet.
        This uses the planet's current population. */
    LongProperty_t getMaxBuildings(const Planet& p, PlanetaryBuilding kind, const game::config::HostConfiguration& config);

    /*
     *  Colonist formulas
     */

    /** Colonists change.
        \param pl Planet
        \param tax desired tax rate
        \param mifa mines plus factories on planet */
    NegativeProperty_t getColonistChange(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, int tax, int mifa);
    NegativeProperty_t getColonistChange(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host);
    LongProperty_t getColonistDue(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, int tax);
    LongProperty_t getColonistDueLimited(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, int tax, int32_t& rem_inc);
    IntegerProperty_t getColonistSafeTax(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, int mifa);
    LongProperty_t getMaxSupportedColonists(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, int player);
    LongProperty_t getMaxSupportedColonists(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host);


    /*
     *  Native formulas
     */

    NegativeProperty_t getNativeChange(const Planet& pl, const HostVersion& host, int tax, int mifa);
    NegativeProperty_t getNativeChange(const Planet& pl, const HostVersion& host);
    LongProperty_t     getNativeDue(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, int tax);
    int32_t            getNativeDue(int tax, int race, int gov, int32_t pop, int owner, const game::config::HostConfiguration& config, const HostVersion& host);
    LongProperty_t     getNativeDueLimited(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, int tax, int32_t rem_inc);
    IntegerProperty_t  getNativeSafeTax(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, int mifa);
    IntegerProperty_t  getNativeBaseTax(const Planet& pl, int owner, const game::config::HostConfiguration& config, const HostVersion& host, int happyTarget);
    IntegerProperty_t  getNativeBaseTax(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, int happyTarget);
    LongProperty_t     getBovinoidSupplyContribution(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host);
    int32_t            getBovinoidSupplyContribution(int32_t pop, int owner, const game::config::HostConfiguration& config, const HostVersion& host);
    LongProperty_t     getBovinoidSupplyContributionLimited(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host);
    int32_t            getAmorphousBreakfast(const HostVersion& host, int happy);


    /*
     *  Mining formulas
     */
    IntegerProperty_t  getMiningCapacity(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, Element::Type type, int mines);
    IntegerProperty_t  getSensorVisibility(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host);


    /*
     *  Starbase formulas
     */
    int32_t getBaseTechCost(int player, int fromTech, int toTech, const game::config::HostConfiguration& config);
} }

#endif
