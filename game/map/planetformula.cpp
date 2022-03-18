/**
  *  \file game/map/planetformula.cpp
  *  \brief Planet formulas
  */

#include <cmath>
#include <cstdlib>
#include "game/map/planetformula.hpp"
#include "game/map/planet.hpp"
#include "util/math.hpp"

using game::config::HostConfiguration;

namespace {
    /** Compute happiness change target for "Safe Tax" method. */
    int computeHappinessTarget(int happy)
    {
        // ex pdata.pas:OptimumTarget
        if (happy >= 100) {
            return 0;
        } else {
            int target = (100 - happy) / 6;
            if (happy < 100 && target == 0) {
                target = 1;
            }
            return target;
        }
    }

    /** Maximum number of structures.
        \param clans      clans on planet
        \param threshold  below threshold, growth is linear; above it's rootic :-) */
    game::LongProperty_t getMaxBuildingsFormula(game::LongProperty_t clansMaybe, int threshold)
    {
        // ex planacc.pas:Maximum0
        int32_t clans;
        if (clansMaybe.get(clans)) {
            if (clans <= threshold) {
                return clans;
            } else {
                /* the rounding function is actually ERnd, but that never
                   makes a difference; sqrt(INT) never ends in .5 exactly */
                return threshold + int(std::sqrt(double(clans - threshold)) + 0.5);
            }
        } else {
            return game::LongProperty_t();
        }
    }
}

game::LongProperty_t
game::map::getMaxBuildings(const Planet& p, PlanetaryBuilding kind, const game::config::HostConfiguration& config, LongProperty_t clans)
{
    // ex game/planetform.h:getMaxStructures
    int n;
    switch (kind) {
     case FactoryBuilding:
        return getMaxBuildingsFormula(clans, 100);
     case MineBuilding:
        return getMaxBuildingsFormula(clans, 200);
     case DefenseBuilding:
        return getMaxBuildingsFormula(clans, 50);
     case BaseDefenseBuilding:
        if (p.getOwner(n)) {
            return p.hasBase() ? config[config.MaximumDefenseOnBase](n) : 0;
        } else {
            return LongProperty_t();
        }
    }
    return LongProperty_t();
}

game::LongProperty_t
game::map::getMaxBuildings(const Planet& p, PlanetaryBuilding kind, const game::config::HostConfiguration& config)
{
    // ex game/planetform.h:getMaxStructures
    return getMaxBuildings(p, kind, config, p.getCargo(Element::Colonists));
}


/*
 *  Colonist formulas
 */

game::NegativeProperty_t
game::map::getColonistChange(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, int tax, int mifa)
{
    // ex game/planetform.h:getColonistChange
    // ex planint.pas:ColonistChange
    int32_t colos;
    int owner;
    int temp;
    if (pl.getCargo(Element::Colonists).get(colos) && pl.getOwner(owner) && pl.getTemperature().get(temp)) {
        double common = 1000 - 80*tax - std::sqrt(double(colos));
        bool crystal = (config.getPlayerRaceNumber(owner) == 7) && config[config.CrystalsPreferDeserts]();
        if (host.getKind() == HostVersion::PHost) {
            if (crystal) {
                return (int32_t(common - mifa/3.0 - (100-temp)/0.66) / 100);
            } else {
                return (int32_t(common - mifa/3.0 - std::abs(temp-50)/0.33) / 100);
            }
        } else {
            if (crystal) {
                return (int32_t(common - (mifa/3 + 3*(100-temp))) / 100);
            } else {
                return (int32_t(common - (mifa/3 + 3*std::abs(temp-50))) / 100);
            }
        }
    } else {
        return afl::base::Nothing;
    }
}

game::NegativeProperty_t
game::map::getColonistChange(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host)
{
    // ex game/planetform.h:getColonistChange
    int tax, mi, fa;
    if (pl.getColonistTax().get(tax) && pl.getNumBuildings(MineBuilding).get(mi) && pl.getNumBuildings(FactoryBuilding).get(fa)) {
        return getColonistChange(pl, config, host, tax, mi + fa);
    } else {
        return afl::base::Nothing;
    }
}

game::LongProperty_t
game::map::getColonistDue(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, int tax)
{
    // ex game/planetform.h:getColonistDue
    // ex planacc.pas:ColonistDue
    // Note that these formulas differ in rounding only. PHost uses
    // `Round', THost uses `ERnd' aka `I-don't-care-how-it-rounds'.
    int owner;
    int32_t colos;
    if (pl.getOwner(owner) && pl.getCargo(Element::Colonists).get(colos)) {
        const int rate = config[config.ColonistTaxRate](owner);
        if (host.getKind() == HostVersion::PHost) {
            return util::divideAndRound(util::divideAndRound(colos * tax, 1000) * rate, 100);
        } else {
            return util::divideAndRoundToEven(util::divideAndRoundToEven(colos * tax, 1000, 0) * rate, 100, 0);
        }
    } else {
        return afl::base::Nothing;
    }
}

game::LongProperty_t
game::map::getColonistDueLimited(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, int tax, int32_t& rem_inc)
{
    // ex game/planetform.h:getColonistDueLimited, pdata.pas:LimitColonists, pdata.pas:ColonistDueLimited
    int owner;
    int32_t due;
    if (pl.getOwner(owner) && getColonistDue(pl, config, host, tax).get(due)) {
        const int32_t max = config[config.MaxPlanetaryIncome](owner);
        if (due < max) {
            rem_inc = max - due;
            return due;
        } else {
            rem_inc = 0;
            return max;
        }
    } else {
        rem_inc = 0;
        return afl::base::Nothing;
    }
}

game::IntegerProperty_t
game::map::getColonistSafeTax(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, int mifa)
{
    // ex game/planetform.h:getColonistSafeTax, pdata.pas:OptimizeTaxes
    int owner, happy, temp;
    int32_t colos;
    if (pl.getOwner(owner)
        && pl.getCargo(Element::Colonists).get(colos)
        && pl.getColonistHappiness().get(happy)
        && pl.getTemperature().get(temp))
    {
        // Compute result
        int taxlimit = host.getColonistTaxRateLimit(owner, config);
        int tax = taxlimit;
        if (happy < 70) {
            // Use tax 0 for unhappy colonists
            tax = 0;
        } else {
            // Figure out maximum tax rate yielding a usable happiness:
            int target = computeHappinessTarget(happy);
            int value;
            while (tax > 0 && getColonistChange(pl, config, host, tax, mifa).get(value) && value < target) {
                --tax;
            }
        }

        // If higher tax rate produces the same happiness change, use that.
        // This applies when the happiness change goal cannot be reached,
        // so let's use a tax rate that gets some income instead of 0%.
        int a, b;
        while (tax < taxlimit
               && getColonistChange(pl, config, host, tax, mifa).get(a)
               && getColonistChange(pl, config, host, tax+1, mifa).get(b)
               && a == b)
        {
            ++tax;
        }

        // If lower tax rates produce the same income, use that:
        int32_t limit;
        int32_t income;
        if (getColonistDueLimited(pl, config, host, tax, limit).get(income)) {
            while (tax > 0 && getColonistDueLimited(pl, config, host, tax-1, limit).isSame(income)) {
                --tax;
            }
        }

        return tax;
    } else {
        return afl::base::Nothing;
    }
}

game::LongProperty_t
game::map::getMaxSupportedColonists(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, int player)
{
    // ex game/planetform.h:getMaxSupportedColonists, planacc.pas:SupportedClans
    int race = config.getPlayerRaceNumber(player);
    bool crystal = (race == 7 && config[config.CrystalsPreferDeserts]());
    int32_t limit;
    int temp;
    if (host.getKind() == HostVersion::PHost) {
        if (!config[config.ClimateLimitsPopulation]()) {
            return 250000;
        }
        if (!pl.getTemperature().get(temp)) {
            return afl::base::Nothing;
        }
        if (crystal) {
            if (config[config.CrystalSinTempBehavior]())
                if (temp >= 15)
                    return (int32_t(100000 * std::sin(temp * util::PI / 200)));
                else
                    return (3 + temp * config[config.MaxColTempSlope]() / 100);
            else
                return std::max(1, 1000 * temp);
        }

        if (temp >= 85)
            limit = config[config.MaxColTempSlope]() * (100-temp) / 100 + 1;
        else if (temp <= 14)
            limit = config[config.MaxColTempSlope]() * temp / 100 + 3;
        else
            limit = int32_t(100000 * std::sin(temp * util::PI / 100));
    } else {
        if (!config[config.ClimateLimitsPopulation]()) {
            return 100000;
        }
        if (!pl.getTemperature().get(temp)) {
            return afl::base::Nothing;
        }
        if (crystal)
            return (1000 * temp);

        // THost before 3.13a probably does not have this
        if (temp >= 85)
            limit = 2 * (100 - temp) + 1;
        else if (temp <= 14)
            limit = 2 * temp + 3;
        else
            limit = int32_t(100000 * std::sin((100 - temp) * 0.0314) + 0.5);
    }

    // THost before 3.22 has an additional "&& limit < 200" here, making this apply to temp <= 14 only.
    if (race == 10 && temp <= 19 && limit < 90000)
        limit = 90000;
    if ((race == 4 || race >= 9) && temp >= 84 && limit < 60)
        limit = 60;

    return limit;
}

game::LongProperty_t
game::map::getMaxSupportedColonists(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host)
{
    // ex game/planetform.h:getMaxSupportedColonists
    int owner;
    if (pl.getOwner(owner)) {
        return getMaxSupportedColonists(pl, config, host, owner);
    } else {
        return afl::base::Nothing;
    }
}

int
game::map::getHissEffect(int shipOwner, int numShips, const game::config::HostConfiguration& config, const HostVersion& host)
{
    // ex client/tiles/planetgrowth.cc:getHissEffect, client/dlg-tax.cc:getHissEffect
    if (config[HostConfiguration::AllowHiss]()) {
        if (host.isPHost()) {
            numShips = std::min(numShips, config[HostConfiguration::MaxShipsHissing]());
        }
        return numShips * config[HostConfiguration::HissEffectRate](shipOwner);
    } else {
        return 0;
    }
}


/*
 *  Native formulas
 */

game::NegativeProperty_t
game::map::getNativeChange(const Planet& pl, const HostVersion& host, int tax, int mifa)
{
    // ex game/planetform.h:getNativeChange
    // ex planint.pas:NativesChange
    // Change to PCC2: checks pop>0
    int gov, race;
    int32_t pop;
    if (pl.getNativeGovernment().get(gov) && pl.getNatives().get(pop) && pl.getNativeRace().get(race)) {
        if (pop > 0) {
            double industryTerm;
            if (host.getKind() == HostVersion::PHost) {
                industryTerm = 0.5 * mifa;    // float division
            } else {
                industryTerm = mifa / 2;      // integer division
            }

            int change = int32_t(500 + 50 * gov - 85 * tax - industryTerm - std::sqrt(double(pop))) / 100;
            if (race == AvianNatives) {
                change += 10;
            }
            return change;
        } else {
            return afl::base::Nothing;
        }
    } else {
        return afl::base::Nothing;
    }
}

game::NegativeProperty_t
game::map::getNativeChange(const Planet& pl, const HostVersion& host)
{
    // ex game/planetform.h:getNativeChange
    int ntax, mi, fa;
    if (pl.getNativeTax().get(ntax) && pl.getNumBuildings(MineBuilding).get(mi) && pl.getNumBuildings(FactoryBuilding).get(fa)) {
        return getNativeChange(pl, host, ntax, mi+fa);
    } else {
        return afl::base::Nothing;
    }
}

game::LongProperty_t
game::map::getNativeDue(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, int tax)
{
    // ex game/planetform.h:getNativeDue
    int race, gov, owner;
    int32_t pop;
    if (pl.getNativeRace().get(race) && pl.getNativeGovernment().get(gov) && pl.getOwner(owner) && pl.getNatives().get(pop)) {
        return getNativeDue(tax, race, gov, pop, owner, config, host);
    } else {
        return afl::base::Nothing;
    }
}

int32_t
game::map::getNativeDue(int tax, int race, int gov, int32_t pop, int owner, const game::config::HostConfiguration& config, const HostVersion& host)
{
    // ex game/planetform.h:getNativeDue
    // ex planacc.pas:NativesDue
    int32_t due;
    if (host.getKind() == HostVersion::PHost) {
        due = util::divideAndRound(util::divideAndRound(tax * gov * pop, 5000) * config[config.NativeTaxRate](owner), 100);
    } else {
        due = util::divideAndRoundToEven(util::divideAndRoundToEven(tax * gov * pop, 5000, 0) * config[config.ColonistTaxRate](owner), 100, 0);
    }
    if (race == InsectoidNatives) {
        return 2*due;
    } else {
        return due;
    }
}

game::LongProperty_t
game::map::getNativeDueLimited(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, int tax, int32_t rem_inc)
{
    // ex planacc.pas:LimitCollection
    int race, owner;
    if (pl.getNativeRace().get(race) && pl.getOwner(owner)) {
        /* amorphs don't pay */
        if (race == AmorphousNatives) {
            return 0;
        }

        /* cyborgs can only tax to 20 */
        int limit = host.getNativeTaxRateLimit(owner, config);
        if (tax > limit)
            tax = limit;

        /* normal formulas here */
        int32_t due;
        if (!getNativeDue(pl, config, host, tax).get(due)) {
            return afl::base::Nothing;
        }
        int32_t colos;
        if (!pl.getCargo(Element::Colonists).get(colos)) {
            return afl::base::Nothing;
        }

        if (host.getKind() == HostVersion::PHost) {
            if (race == InsectoidNatives) {
                colos *= 2;
            }
            colos = util::divideAndRound(colos * config[config.NativeTaxRate](owner), 100);
        } else {
            colos = colos * config[config.ColonistTaxRate](owner) / 100;
            if (race == InsectoidNatives) {
                colos *= 2;
            }
        }

        if (due > colos)
            due = colos;
        if (due > rem_inc)
            due = rem_inc;
        return due;
    } else {
        return afl::base::Nothing;
    }
}

game::IntegerProperty_t
game::map::getNativeSafeTax(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, int mifa)
{
    // ex game/planetform.h:getNativeSafeTax, pdata.pas:OptimizeTaxes
    // Validate inputs
    int owner, race, gov, happy;
    int32_t cpop, npop;
    if (pl.getOwner(owner) && pl.getCargo(Element::Colonists).get(cpop)
        && pl.getNativeRace().get(race) && pl.getNatives().get(npop)
        && pl.getNativeGovernment().get(gov) && pl.getNativeHappiness().get(happy)
        && npop > 0)
    {
        // Compute result
        int taxlimit = host.getNativeTaxRateLimit(owner, config);
        int tax = taxlimit;
        if (happy < 70 || race == AmorphousNatives) {
            // Use tax 0 for unhappy natives or Amorphs
            tax = 0;
        } else {
            // Figure out maximum tax rate yielding a usable happiness:
            int target = computeHappinessTarget(happy);
            int value;
            while (tax > 0 && getNativeChange(pl, host, tax, mifa).get(value) && value < target) {
                --tax;
            }
        }

        // If higher tax rate produces the same happiness change, use that.
        // This applies when the happiness change goal cannot be reached,
        // so let's use a tax rate that gets some income instead of 0%.
        int a, b;
        while (tax < taxlimit
               && getNativeChange(pl, host, tax, mifa).get(a)
               && getNativeChange(pl, host, tax+1, mifa).get(b)
               && a == b)
        {
            ++tax;
        }

        // If lower tax rates produce the same income, use that.
        // Note that the original code tried (and failed) to handle the relation
        // between colonist tax, native tax, and MaxPlanetaryIncome here. Doing
        // this correctly means handling the (assumed) colonist tax rate here,
        // which would complicate matters too much for my taste. This code matters
        // in two places: where safe-tax income hits MaxPlanetaryIncome (rare),
        // and where population is low enough to make rounding effects matter.
        int32_t limit = 0x7FFFFFFF;
        int32_t income;
        if (getNativeDueLimited(pl, config, host, tax, limit).get(income)) {
            while (tax > 0 && getNativeDueLimited(pl, config, host, tax-1, limit).isSame(income)) {
                --tax;
            }
        }

        return tax;
    } else {
        return afl::base::Nothing;
    }
}

game::IntegerProperty_t
game::map::getNativeBaseTax(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, int happyTarget)
{
    // ex game/planetform.h:getNativeBaseTax
    // ex envscan.pas:BaseTax
    int owner;
    if (pl.getOwner(owner)) {
        return getNativeBaseTax(pl, owner, config, host, happyTarget);
    } else {
        return afl::base::Nothing;
    }
}

game::IntegerProperty_t
game::map::getNativeBaseTax(const Planet& pl, int owner, const game::config::HostConfiguration& config, const HostVersion& host, int happyTarget)
{
    // Check preconditions
    int race, gov;
    int32_t pop;
    if (pl.getNativeRace().get(race) && pl.getNatives().get(pop) && pl.getNativeGovernment().get(gov)) {
        // No taxes if natives wouldn't pay anyway, or there are none
        if (pop == 0 || race == AmorphousNatives) {
            return 0;
        }

        // Compute result
        const int mifa = 100;
        int tax = host.getNativeTaxRateLimit(owner, config);
        while (tax > 0) {
            /* Host 3.22.040 formula:
                 Trunc(1000.0 - sqrt(Planet.Natives) - 85 * Planet.NativeTax
                              - (Planet.Mines + .Factories) DIV 2
                              - 50*(10-Planet.NativeGov))
                   DIV 100
               change:=5 + Trunc(spi/2 - (Sqrt(Natives) + 85*LONGINT(tax) + 100/2) / 100);
               change:=5 + Trunc(50*spi - (85*LONGINT(tax) + 50) - Sqrt(Natives)) DIV 100;
               since MIFA is even, the same formula can be used for PHost and THost. */
            int change = int(500 - (mifa/2) + (50*gov) - (85*tax) - std::sqrt(double(pop))) / 100;
            if (race == AvianNatives) {
                change += 10;
            }
            if (change >= happyTarget) {
                break;
            }
            --tax;
        }
        return tax;
    } else {
        return afl::base::Nothing;
    }
}

game::LongProperty_t
game::map::getBovinoidSupplyContribution(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host)
{
    // ex game/planetform.h:getBovinoidSupplyContribution
    // Change to PCC2: PCC2 returns 0, we return unknown
    int race, owner;
    int32_t pop;
    if (pl.getOwner(owner) && pl.getNativeRace().get(race) && pl.getNatives().get(pop)) {
        if (race == BovinoidNatives) {
            return getBovinoidSupplyContribution(pop, owner, config, host);
        } else {
            return 0;
        }
    } else {
        return afl::base::Nothing;
    }
}

int32_t
game::map::getBovinoidSupplyContribution(int32_t pop, int owner, const game::config::HostConfiguration& config, const HostVersion& host)
{
    // ex game/planetform.h:getBovinoidSupplyContribution
    // FIXME: for ultimate accuracy, we have to consider factories as
    // well. PHost does "(bovi + factories) * ProductionRate", not
    // "bovi * ProductionRate + factories * ProductionRate".
    if (host.getKind() == HostVersion::PHost) {
        return (pop / 100) * config[config.ProductionRate](owner) / 100;
    } else {
        return pop / 100;
    }
}

game::LongProperty_t
game::map::getBovinoidSupplyContributionLimited(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host)
{
    // ex game/planetform.h:getBovinoidSupplyContributionLimited
    // FIXME: the same problem as in getBovinoidSupplyContribution(int,int)
    // appears here. We *could* add a workaround here (compute totals,
    // compute factory contribution, compute difference), but is it
    // really worth to do that for an accuracy of +/-1 with a config
    // option no-one uses?
    //      -- update: it's going to be used in PList 2.5, so someday
    //         it should be supported
    int32_t clans, due;
    int owner;
    if (pl.getOwner(owner) && pl.getCargo(Element::Colonists).get(clans) && getBovinoidSupplyContribution(pl, config, host).get(due)) {
        int32_t limit = host.getKind() == HostVersion::PHost ? clans * config[config.ProductionRate](owner) / 100 : clans;
        if (due < limit) {
            return due;
        } else {
            return limit;
        }
    } else {
        return afl::base::Nothing;
    }
}

int32_t
game::map::getAmorphousBreakfast(const HostVersion& host, int happy)
{
    // ex game/planetform.h:getAmorphousBreakfast, planacc.pas:AmorphousBreakfast
    if (host.getKind() == HostVersion::PHost) {
        if (happy >= 70)
            return 5;
        else if (happy >= 50)
            return 20;
        else
            return 40;
    } else {
        if (happy > 90)
            return 5;
        else
            return 95 - happy;
    }
}

/*
 *  Mining Formulas
 */

game::IntegerProperty_t
game::map::getMiningCapacity(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host, Element::Type type, int mines)
{
    // ex ccmain.pas:MiningCapacity
    int density;
    if (pl.getOreDensity(type).get(density)) {
        // Mining rate
        int owner;
        int mining_rate;
        if (pl.getOwner(owner)) {
            mining_rate = config[config.RaceMiningRate](owner);
        } else {
            mining_rate = 100;
        }

        // Native races
        int32_t pop;
        int reptile_factor;
        if (pl.getNativeRace().isSame(ReptilianNatives) && pl.getNatives().get(pop) && pop > 0) {
            reptile_factor = 2;
        } else {
            reptile_factor = 1;
        }

        // Host-dependant formula
        if (host.getKind() != HostVersion::PHost) {
            // Tim
            return util::divideAndRoundToEven(util::divideAndRoundToEven(density * int32_t(mines), 100, 0) * mining_rate, 100, 0) * reptile_factor;
        } else {
            // Andrew
            int add = (host.isPHostRoundingMiningResults() ? 50 : 0);
            return (((density * int32_t(mining_rate) + add) / 100) * reptile_factor * mines + add) / 100;
        }
    } else {
        return afl::base::Nothing;
    }
}

game::IntegerProperty_t
game::map::getSensorVisibility(const Planet& pl, const game::config::HostConfiguration& config, const HostVersion& host)
{
    int mines, factories, defense;
    if (pl.getNumBuildings(MineBuilding).get(mines)
        && pl.getNumBuildings(FactoryBuilding).get(factories)
        && pl.getNumBuildings(DefenseBuilding).get(defense))
    {
        // Parameters
        int dfu, mfd, ffd;
        if (host.isPHost()) {
            dfu = config[HostConfiguration::DefenseForUndetectable]();
            mfd = config[HostConfiguration::MinesForDetectable]();
            ffd = config[HostConfiguration::FactoriesForDetectable]();
        } else {
            dfu = 15;
            mfd = 21;
            ffd = 16;
        }

        if (defense >= dfu
            || dfu <= 0                   // avoid division by zero in pathological case
            || (mines < mfd && factories < ffd))
        {
            return 0;
        } else {
            return 100 - (defense * 100) / dfu;
        }
    } else {
        return afl::base::Nothing;
    }
}

int32_t
game::map::getBaseTechCost(int player, int fromTech, int toTech, const game::config::HostConfiguration& config)
{
    // ex accessor.pas:TechCost
    /* Going from tech i to i+1 costs 100*i mc, thus going from 0 to i
       costs 100*\sum[0,i] i = 100*(i*(i-1)/2) = 50*i*(i-1).

       Using BaseTechCost, the formula becomes
           BaseTechCost*(i*(i-1)/2)
       Because either i or i-1 will be even, the division by two will
       always work out. */
    int fromVal = fromTech * (fromTech-1);
    int toVal   = toTech   * (toTech-1);
    return config[config.BaseTechCost](player) * (toVal - fromVal) / 2;
}
