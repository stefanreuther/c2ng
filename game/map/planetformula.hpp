/**
  *  \file game/map/planetformula.hpp
  *  \brief Planet formulas
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

    /** Get colonists happiness change.
        @param pl     Planet
        @param config Host configuration
        @param host   Host version
        @param tax    Tax rate
        @param mifa   Mines plus factories on planet
        @return Happiness change; unknown if preconditions not met */
    NegativeProperty_t getColonistChange(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, int tax, int mifa);

    /** Get colonists happiness change, using actual tax rate/buildings.
        @param pl     Planet
        @param config Host configuration
        @param host   Host version
        @return Happiness change; unknown if preconditions not met */
    NegativeProperty_t getColonistChange(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host);

    /** Get colonist tax due amount, as requested.
        @param pl     Planet
        @param config Host configuration
        @param host   Host version
        @param tax    Tax rate
        @return Amount; unknown if preconditions not met */
    LongProperty_t getColonistDue(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, int tax);

    /** Get colonist tax income.
        @param [in]  pl      Planet
        @param [in]  config  Host configuration
        @param [in]  host    Host version
        @param [in]  tax     Tax rate
        @param [out] rem_inc Amount of taxes which we can still get from the natives (regarding MaxPlanetaryIncome)
        @return Amount collected; unknown if preconditions not met */
    LongProperty_t getColonistDueLimited(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, int tax, int32_t& rem_inc);

    /** Get colonist "safe tax" rate.
        @param pl     Planet
        @param config Host configuration
        @param host   Host version
        @param mifa   Assume this many mines and factories
        @return Safe tax value, or unknown if preconditions not met */
    IntegerProperty_t getColonistSafeTax(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, int mifa);

    /** Maximum population on planet, for a race.
        @param pl      Planet
        @param config  Host configuration
        @param host    Host version
        @param player  player number
        @return maximum population in clans, may be unknown. */
    LongProperty_t getMaxSupportedColonists(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, int player);

    /** Maximum population on planet, for actual planet owner.
        @param pl      Planet
        @param config  Host configuration
        @param host    Host version
        @return maximum population in clans, may be unknown. */
    LongProperty_t getMaxSupportedColonists(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host);

    /** Get hiss effect.
        @param shipOwner Ship owner
        @param numShips  Number of ships hissing
        @param config    Host configuration
        @return Total hiss effect */
    int getHissEffect(int shipOwner, int numShips, const game::config::HostConfiguration& config);


    /*
     *  Native formulas
     */

    /** Get native happiness change.
        @param pl    Planet
        @param host  Host version
        @param tax   Desired tax rate
        @param mifa  mines plus factories on planet
        @return happiness change, unknown if preconditions not met */
    NegativeProperty_t getNativeChange(const Planet& pl, const HostVersion& host, int tax, int mifa);

    /** Get native happiness change, for actual tax rate/buildings.
        @param pl    Planet
        @param host  Host version
        @returns happiness change for current situation, unknown if preconditions not met. */
    NegativeProperty_t getNativeChange(const Planet& pl, const HostVersion& host);

    /** Get native tax amount, as requested, for this planet.
        This returns the amount we're asking from them, not what we'll get.
        @param pl     Planet
        @param config Host configuration
        @param host   Host version
        @param tax    Tax rate
        @return native tax due amount */
    LongProperty_t getNativeDue(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, int tax);

    /** Get native tax amount, as requested, parameterized.
        This returns the amount we're asking from them, not what we'll get.
        \param tax    tax rate
        \param race   native race
        \param gov    government factor (SPI)
        \param pop    native clans
        \param owner  owner (for production rates)
        \param config Host configuration
        \param host   Host version
        \return native tax amount due */
    int32_t getNativeDue(int tax, int race, int gov, int32_t pop, int owner, const game::config::HostConfiguration& config, const HostVersion& host);

    /** Get native tax amount, limited. Limits the tax income by income
        limit and available colonists.
        @param pl      Planet
        @param config  Host configuration
        @param host    Host version
        @param tax     tax rate
        @param rem_inc remaining allowed income; output of getColonistDueLimited().
        @return native tax amount due, limited; unknown if preconditions not met */
    LongProperty_t getNativeDueLimited(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, int tax, int32_t rem_inc);

    /** Get native "safe tax" rate.
        @param pl     Planet
        @param config Host configuration
        @param host   Host version
        @param mifa   Assume this many mines and factories
        @return Safe tax value, or unknown if preconditions not met */
    IntegerProperty_t getNativeSafeTax(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, int mifa);

    /** Get native "base tax" rate.
        The base tax assumes a fixed mines/factories count to make results comparable between planets.
        @param pl          Planet
        @param owner       Planet owner
        @param config      Host configuration
        @param host        Host version
        @param happyTarget Happiness target (0=base tax, -30=max tax)
        @return tax rate, unknown if preconditions not met */
    IntegerProperty_t getNativeBaseTax(const Planet& pl, int owner, const game::config::HostConfiguration& config, const HostVersion& host, int happyTarget);

    /** Get native "base tax" rate, for actual owne.r
        The base tax assumes a fixed mines/factories count to make results comparable between planets.
        @param pl          Planet
        @param config      Host configuration
        @param host        Host version
        @param happyTarget Happiness target (0=base tax, -30=max tax)
        @return tax rate, unknown if preconditions not met */
    IntegerProperty_t getNativeBaseTax(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, int happyTarget);

    /** Get Bovinoid supply contribution.
        Returns the nominal contribution not limited by colonists.
        @param pl     Planet
        @param config Host configuration
        @return Supply contribution, unknown if preconditions not met */
    LongProperty_t getBovinoidSupplyContribution(const Planet& pl, const game::config::HostConfiguration& config);

    /** Get Bovinoid supply contribution, parameterized.
        @param pop    Native population
        @param owner  Planet owner
        @param config Host configuration
        @return Supply contribution */
    int32_t getBovinoidSupplyContribution(int32_t pop, int owner, const game::config::HostConfiguration& config);

    /** Get Bovinoid supply contribution.
        This returns the amount of supplies finally collected, limited by what colonists can collect.
        @param pl     Planet
        @param config Host configuration
        @return Supply contribution, unknown if preconditions not met */
    LongProperty_t getBovinoidSupplyContributionLimited(const Planet& pl, const game::config::HostConfiguration& config);

    /** Get Amorphous colonist breakfast.
        Returns the number of clans eaten by our lovely amorphous natives when they have a happiness as specified.
        @param host  Host version
        @param happy Native happiness
        @return number of clans eaten */
    int32_t getAmorphousBreakfast(const HostVersion& host, int happy);


    /*
     *  Mining formulas
     */

    /** Get mining capacity.
        @param pl     Planet
        @param config Host configuration
        @param host   Host version
        @param type   Mineral type
        @param mines  Number of mines on planet
        @return Mining capacity; may be unknown */
    IntegerProperty_t getMiningCapacity(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, Element::Type type, int mines);

    /** Get sensor visibility.
        @param pl     Planet
        @param config Host configuration
        @param host   Host version
        @return Sensor visibility in % (0=invisible, 100=always visible); unknown if preconditions not met */
    IntegerProperty_t  getSensorVisibility(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host);


    /*
     *  Starbase formulas
     */

    /** Compute cost for a tech level upgrade.
        Returns the cost for upgrading from %fromTech to %toTech.
        @param player Player to compute this for
        @param fromTech Old (low) tech
        @param toTech New (high) tech
        @param config Host configuration
        @return Cost in megacredits */
    int32_t getBaseTechCost(int player, int fromTech, int toTech, const game::config::HostConfiguration& config);
} }

#endif
