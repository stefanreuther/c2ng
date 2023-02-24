/**
  *  \file game/map/planetpredictor.cpp
  *  \brief Class game::map::PlanetPredictor
  */

#include <cmath>
#include <algorithm>
#include "game/map/planetpredictor.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/map/planeteffectors.hpp"
#include "game/map/planetformula.hpp"
#include "util/math.hpp"

using game::Element;
using game::config::HostConfiguration;
using game::map::Planet;
using util::PI;

namespace {
    void trimHappiness(Planet& pl)
    {
        // ex planacc.pas:ComputePlanetTurn.TrimHappiness
        int chappy = pl.getColonistHappiness().orElse(0);
        if (chappy < game::MIN_HAPPINESS) {
            chappy = game::MIN_HAPPINESS;
        }
        if (chappy > game::MAX_HAPPINESS) {
            chappy = game::MAX_HAPPINESS;
        }
        pl.setColonistHappiness(chappy);

        int nhappy = pl.getNativeHappiness().orElse(0);
        if (nhappy < game::MIN_HAPPINESS) {
            nhappy = game::MIN_HAPPINESS;
        }
        if (nhappy > game::MAX_HAPPINESS) {
            nhappy = game::MAX_HAPPINESS;
        }
        pl.setNativeHappiness(nhappy);
    }

    void doMining(Planet& pl,
                  const Element::Type el,
                  const game::config::HostConfiguration& config,
                  const game::HostVersion& host)
    {
        int numMines;
        if (pl.getNumBuildings(game::MineBuilding).get(numMines)) {
            int capacity;
            int32_t ground;
            int32_t mined;
            if (game::map::getMiningCapacity(pl, config, host, el, numMines).get(capacity)
                && pl.getOreGround(el).get(ground)
                && pl.getCargo(el).get(mined))
            {
                int32_t amount = std::min(int32_t(capacity), ground);
                pl.setCargo(el, mined + amount);
                pl.setOreGround(el, ground - amount);
            }
        }
    }

    void doTUDR(Planet& pl,
                const Element::Type el,
                const game::config::HostConfiguration& config)
    {
        /* FIXME: this is the PHost formula. THost? */
        pl.setOreGround(el, pl.getOreGround(el).orElse(0) + (pl.getOreDensity(el).orElse(0) * config[config.TransuraniumDecayRate]() + 50) / 100);
    }

    void doDecay(Planet& pl, game::PlanetaryBuilding kind, const game::config::HostConfiguration& config)
    {
        // ex planacc.pas:ComputePlanetTurn.Decay
        int planetOwner = 0;
        int max;
        int have;
        if (pl.getOwner().get(planetOwner)
            && getMaxBuildings(pl, kind, config).get(max)
            && pl.getNumBuildings(kind).get(have))
        {
            if (have > max) {
                have -= config[config.StructureDecayPerTurn](planetOwner);
                if (have < max) {
                    have = max;
                }
                pl.setNumBuildings(kind, have);
            }
        }
    }

    void doAssimilation(Planet& pl,
                        const game::config::HostConfiguration& config)
    {
        // ex planacc.pas:ComputePlanetTurn.Assimilate
        int planetOwner = 0;
        if (pl.getOwner().get(planetOwner) && config.getPlayerRaceNumber(planetOwner) == 6) {
            int nativeRace;
            if (pl.getNativeRace().get(nativeRace) && nativeRace != 0 && nativeRace != game::AmorphousNatives) {
                int32_t assim = pl.getCargo(Element::Colonists).orElse(0) * config[config.BorgAssimilationRate](planetOwner) / 100;
                int32_t natives = pl.getNatives().orElse(0);
                if (assim > natives) {
                    assim = natives;
                }
                pl.setCargo(Element::Colonists, pl.getCargo(Element::Colonists).orElse(0) + assim);
                pl.setNatives(natives - assim);
            }
        }
    }

    void doDemolish(Planet& pl, game::PlanetaryBuilding kind, int n)
    {
        int have;
        if (pl.getNumBuildings(kind).get(have)) {
            if (have > n) {
                pl.setNumBuildings(kind, have - n);
            } else {
                pl.setNumBuildings(kind, 0);
            }
        }
    }
}

game::map::PlanetPredictor::PlanetPredictor(const Planet& planet)
    : m_planet(planet)
{ }

void
game::map::PlanetPredictor::computeTurn(const PlanetEffectors& eff,
                                        const UnitScoreDefinitionList& planetScores,
                                        const game::config::HostConfiguration& config,
                                        const HostVersion& host)
{
    // ex computeTurn, pdata.pas:ComputePlanetTurn
    /* Our sequence will be:
       - hiss
       - lfm / gather-build        (maybe)
       - dmp                       (maybe)
       - free fighters             (maybe)
       - terraform
       - mining
       - supplies
       - tudr
       - happiness
       - tax
       - THost: assimilate
       - growth
       - cdr
       - amorphs
       - riots
       - PHost: assimilate */

    // Planet owner. If not known, treat as 0.
    int planetOwner = m_planet.getOwner().orElse(0);

    // Hiss
    if (config[config.AllowHiss]()) {
        int nhiss = std::min(eff.get(PlanetEffectors::Hiss), config[config.MaxShipsHissing]());

        nhiss *= config[config.HissEffectRate](planetOwner);

        m_planet.setColonistHappiness(m_planet.getColonistHappiness().orElse(0) + nhiss);
        m_planet.setNativeHappiness  (m_planet.getNativeHappiness()  .orElse(0) + nhiss);
        trimHappiness(m_planet);
    }

    // LFM, Gather-build, free fighters would be here

    // Terraform
    if (config[config.AllowScienceMissions]()) {
        int rate = config[config.TerraformRate](planetOwner);
        int temp = m_planet.getTemperature().orElse(0);
        if (temp > 50) {
            // Coolers
            temp -= eff.get(PlanetEffectors::CoolsTo50) * rate;
            if (temp < 50) {
                temp = 50;
            }
        }
        if (temp < 50) {
            // Heaters gonna heat
            temp += eff.get(PlanetEffectors::HeatsTo50) * rate;
            if (temp > 50) {
                temp = 50;
            }
        }
        /* Tholian heaters. Note that this is not 100% accurate. Given
           a 49F planet, one Bohemian (HeatTo50), one Onxy (HeatTo100).
           If the onyx goes first, we have 50F afterwards (because the
           bohemian sees a 50F planet and doesn't do anything). If the
           Bohemian goes first, we have 51F afterwards. */
        temp += eff.get(PlanetEffectors::HeatsTo100) * rate;
        if (temp > 100) {
            temp = 100;
        }
        m_planet.setTemperature(temp);
    }

    // Mining
    if (planetOwner > 0) {
        doMining(m_planet, Element::Neutronium, config, host);
        doMining(m_planet, Element::Tritanium,  config, host);
        doMining(m_planet, Element::Duranium,   config, host);
        doMining(m_planet, Element::Molybdenum, config, host);

        // Supplies
        int32_t fact = m_planet.getNumBuildings(FactoryBuilding).orElse(0)
            * config[config.ProductionRate](planetOwner) / 100;
        fact += getBovinoidSupplyContributionLimited(m_planet, config).orElse(0);
        m_planet.setCargo(Element::Supplies, m_planet.getCargo(Element::Supplies).orElse(0) + fact);
    }

    // TUDR
    doTUDR(m_planet, Element::Neutronium, config);
    doTUDR(m_planet, Element::Tritanium,  config);
    doTUDR(m_planet, Element::Duranium,   config);
    doTUDR(m_planet, Element::Molybdenum, config);

    // Happiness
    if (planetOwner > 0) {
        if (m_planet.getColonistHappiness().orElse(0) < 30) {
            m_planet.setColonistTax(0);
        }
        if (m_planet.getNativeHappiness().orElse(0) < 30) {
            m_planet.setNativeTax(0);
        }
        m_planet.setColonistHappiness(m_planet.getColonistHappiness().orElse(0) + getColonistChange(m_planet, config, host).orElse(0));
        m_planet.setNativeHappiness (m_planet.getNativeHappiness().orElse(0) + getNativeChange(m_planet, host).orElse(0));
        trimHappiness(m_planet);

        // Taxation
        int32_t limit = host.getPostTaxationHappinessLimit();
        int32_t income = 0;
        if (m_planet.getColonistHappiness().orElse(0) >= limit) {
            income += getColonistDue(m_planet, config, host, m_planet.getColonistTax().orElse(0)).orElse(0);
        }
        if (m_planet.getNativeHappiness().orElse(0) >= limit) {
            income += getNativeDueLimited(m_planet, config, host, m_planet.getNativeTax().orElse(0), 0x7FFFFFFF).orElse(0);
        }
        if (income > config[config.MaxPlanetaryIncome](planetOwner)) {
            income = config[config.MaxPlanetaryIncome](planetOwner);
        }
        m_planet.setCargo(Element::Money, m_planet.getCargo(Element::Money).orElse(0) + income);

        // PHost: Assimilation
        if (host.isPHost()) {
            doAssimilation(m_planet, config);
        }

        // Colonist growth
        limit = getMaxSupportedColonists(m_planet, config, host).orElse(0);
        if (host.isPHost()) {
            /* PHost 4.1/3.5 changed rounding in mining, as well as in overpopulation
               deaths; the latter implicitly due to the computation of deaths in
               persons, not clans. */
            const int ceilOrTrunc = host.isPHostRoundingMiningResults() ? 0 : 99;
            if (m_planet.getColonistHappiness().orElse(0) >= 70 && m_planet.getCargo(Element::Colonists).orElse(0) < limit) {
                double rate;
                if (config.getPlayerRaceNumber(planetOwner) == 7 && config[config.CrystalsPreferDeserts]()) {
                    if (config[config.CrystalSinTempBehavior]()) {
                        if (m_planet.getTemperature().orElse(0) < 15) {
                            rate = 0.0;
                        } else {
                            rate = 5.0 * std::sin(m_planet.getTemperature().orElse(0) * PI/200);
                        }
                    } else {
                        rate = (5*m_planet.getTemperature().orElse(0)) / 100.0;
                    }
                } else {
                    if (m_planet.getTemperature().orElse(0) < 15 || m_planet.getTemperature().orElse(0) > 84) {
                        rate = 0.0;
                    } else {
                        rate = 5.0 * std::sin(m_planet.getTemperature().orElse(0) * PI/100);
                    }
                }

                // Note this rounding behaviour is right
                int32_t growth = util::roundToInt(rate * m_planet.getCargo(Element::Colonists).orElse(0) / ((1 + m_planet.getColonistTax().orElse(0)/5.0) * 100))
                    * config[config.RaceGrowthRate](planetOwner) / 100;
                int32_t maxGrowth = limit - m_planet.getCargo(Element::Colonists).orElse(0);
                if (growth > maxGrowth) {
                    growth = maxGrowth;
                }
                m_planet.setCargo(Element::Colonists, m_planet.getCargo(Element::Colonists).orElse(0) + growth);
            }

            if (m_planet.getCargo(Element::Colonists).orElse(0) > limit && config[config.ClimateLimitsPopulation]()) {
                int32_t deaths = (config[config.ClimateDeathRate](planetOwner) * m_planet.getCargo(Element::Colonists).orElse(0) + ceilOrTrunc) / 100;
                int32_t maxDeaths = m_planet.getCargo(Element::Colonists).orElse(0) - limit;
                if (deaths > maxDeaths) {
                    deaths = maxDeaths;
                }
                m_planet.setCargo(Element::Colonists, m_planet.getCargo(Element::Colonists).orElse(0) - deaths);
            }

            // Native growth
            if (m_planet.getNativeRace().orElse(0) != 0) {
                bool likeItHot = (m_planet.getNativeRace().orElse(0) == SiliconoidNatives
                                  && config[config.CrystalsPreferDeserts]()
                                  && host.hasSiliconoidDesertAdvantage());
                if (!config[config.ClimateLimitsPopulation]()) {
                    limit = 156000;
                } else if (likeItHot) {
                    limit = m_planet.getTemperature().orElse(0) * 1000;
                } else {
                    limit = int32_t(std::sin(m_planet.getTemperature().orElse(0) * PI / 100) * 156000);
                }

                if (m_planet.getNativeHappiness().orElse(0) >= 70 && m_planet.getNatives().orElse(0) < limit) {
                    int32_t growth;
                    if (likeItHot) {
                        growth = (m_planet.getTemperature().orElse(0) * m_planet.getNatives().orElse(0)) / (500 * (m_planet.getNativeTax().orElse(0) + 5));
                    } else {
                        growth = int32_t((4*std::sin(m_planet.getTemperature().orElse(0) * PI/100) / (1 + m_planet.getNativeTax().orElse(0)/5.0)) * m_planet.getNatives().orElse(0)) / 100;
                    }
                    int32_t nn = m_planet.getNatives().orElse(0) + growth;
                    if (nn > limit) {
                        nn = limit;
                    }
                    m_planet.setNatives(nn);
                }

                if (m_planet.getNatives().orElse(0) > limit && config[config.ClimateLimitsPopulation]()) {
                    int32_t deaths = (config[config.NativeClimateDeathRate]() * m_planet.getNatives().orElse(0) + ceilOrTrunc) / 100;
                    int32_t nn = m_planet.getNatives().orElse(0) - deaths;
                    if (nn < limit) {
                        nn = limit;
                    }
                    m_planet.setNatives(nn);
                }
            }

            /* FIXME: Supply eat */
        } else {
            /* THost */
            int32_t growth = 0;
            const int planetTemp = m_planet.getTemperature().orElse(0);
            const int ownerRace = config.getPlayerRaceNumber(planetOwner);
            if (m_planet.getColonistHappiness().orElse(0) >= 70) {
                // These formulas have the form
                //     (temperature term) * colonists/20 * 5/(5+tax)
                // which simplifies to
                //     (temperature term) * colonists / (4 * (5+tax))
                // FIXME: should we check for CrystalsPreferDeserts here?
                if (ownerRace == 7) {
                    growth = util::divideAndRound(planetTemp * m_planet.getCargo(Element::Colonists).orElse(0), 400 * (m_planet.getColonistTax().orElse(0)+5));
                } else {
                    growth = util::roundToInt(std::sin((100 - planetTemp) * 0.0314) * m_planet.getCargo(Element::Colonists).orElse(0) / 4 / (m_planet.getColonistTax().orElse(0) + 5.0));
                }
            }

            int32_t maxPop = util::roundToInt(std::sin((100 - planetTemp) * 0.0314) * 100000);
            if (planetTemp <= 14 || planetTemp > 84) {
                if (ownerRace != 7 || !config[config.CrystalsPreferDeserts]()) {
                    growth = 0;
                    maxPop = std::max(1, m_planet.getCargo(Element::Colonists).orElse(0) - (m_planet.getCargo(Element::Colonists).orElse(0) * config[config.ClimateDeathRate](planetOwner) / 100));
                }
                if (planetTemp > 50) {
                    maxPop += (100-planetTemp) * 2;
                } else {
                    maxPop += (1+planetTemp)*2;
                }
            }
            if (ownerRace == 7 && config[config.CrystalsPreferDeserts]()) {
                maxPop = planetTemp * 1000;
            }
            if (planetTemp > 80 && (ownerRace >= 9 || ownerRace == 4)) {
                maxPop = std::max(maxPop, 60);
            }
            if (planetTemp < 20 && ownerRace == 10) {
                maxPop = 90000;
            }
            if (!config[config.ClimateLimitsPopulation]()) {
                maxPop = 100000;
            }

            if (config[config.AllowEatingSupplies](planetOwner) && m_planet.getCargo(Element::Colonists).orElse(0) > maxPop) {
                int32_t eaten = util::divideAndRoundToEven(m_planet.getCargo(Element::Colonists).orElse(0) - maxPop, 40, 1);
                int32_t maxEaten = m_planet.getCargo(Element::Supplies).orElse(0);
                if (eaten > maxEaten) {
                    eaten = maxEaten;
                }
                m_planet.setCargo(Element::Supplies, m_planet.getCargo(Element::Supplies).orElse(0) - eaten);
                maxPop = util::divideAndRoundToEven(m_planet.getCargo(Element::Supplies).orElse(0), 40, maxPop);
            }

            if (m_planet.getCargo(Element::Colonists).orElse(0) > 66000) {
                growth = util::divideAndRoundToEven(growth, 2, 0);
            }

            if ((planetTemp > 84 || planetTemp <= 14) && !(ownerRace == 7 && planetTemp > 50)) {
                growth = 0;
            }

            int32_t newPop = m_planet.getCargo(Element::Colonists).orElse(0);
            if (newPop < maxPop) {
                newPop += growth;
            }
            if (newPop > maxPop) {
                newPop = maxPop;
            }
            if (newPop > 250000) {
                newPop = 250000;
            }
            m_planet.setCargo(Element::Colonists, newPop);

            // Native Growth
            const int nativeRace = m_planet.getNativeRace().orElse(0);
            if (nativeRace != 0) {
                const bool siliconoid = (nativeRace == SiliconoidNatives);
                const int nativeHappiness = m_planet.getNativeHappiness().orElse(0);
                const int32_t nativePopulation = m_planet.getNatives().orElse(0);
                const int nativeTax = m_planet.getNativeTax().orElse(0);
                if (nativeHappiness < 70) {
                    growth = 0;
                } else if (siliconoid) {
                    growth = util::divideAndRoundToEven(planetTemp * nativePopulation * 5, 100 * 25 * (nativeTax + 5), 0);
                } else {
                    growth = util::roundToInt(std::sin((100-planetTemp) * 0.0314) * nativePopulation * 5
                                              / (25 * (nativeTax + 5)));
                }

                if (siliconoid) {
                    maxPop = 1000 * planetTemp;
                } else {
                    maxPop = util::roundToInt(std::sin((100-planetTemp) * 0.0314) * 150000);
                }

                if (nativePopulation > 66000) {
                    growth = util::divideAndRoundToEven(growth, 2, 0);
                }
                if (nativePopulation < maxPop) {
                    m_planet.setNatives(nativePopulation + growth);
                }
            }
        }

        // Structure decay
        doDecay(m_planet, MineBuilding, config);
        doDecay(m_planet, FactoryBuilding, config);
        doDecay(m_planet, DefenseBuilding, config);

        // Riots
        if (!host.isPHost()) {
            /* FIXME */
        } else {
            if ((m_planet.getCargo(Element::Colonists).orElse(0) > 0 && m_planet.getColonistHappiness().orElse(0) < 40)
                || (m_planet.getNatives().orElse(0) > 0 && m_planet.getNativeHappiness().orElse(0) < 40))
            {
                doDemolish(m_planet, FactoryBuilding, 10);
                doDemolish(m_planet, MineBuilding, 10);
            }

            int32_t colDeaths = (m_planet.getColonistHappiness().orElse(0) < 20
                                 ? m_planet.getCargo(Element::Colonists).orElse(0) * (40 - m_planet.getColonistHappiness().orElse(0)) / 5
                                 : m_planet.getNatives().orElse(0) > 0 && m_planet.getNativeHappiness().orElse(0) < 20
                                 ? m_planet.getCargo(Element::Colonists).orElse(0) * (40 - m_planet.getNativeHappiness().orElse(0)) / 25
                                 : 0);
            int32_t natDeaths = (m_planet.getNativeHappiness().orElse(0) < 20
                                 ? m_planet.getNatives().orElse(0) * (40 - m_planet.getNativeHappiness().orElse(0)) / 5
                                 : m_planet.getCargo(Element::Colonists).orElse(0) > 0 && m_planet.getColonistHappiness().orElse(0) < 20
                                 ? m_planet.getNatives().orElse(0) * (40 - m_planet.getColonistHappiness().orElse(0)) / 25
                                 : 0);

            // Colonist deaths
            colDeaths = (colDeaths+99) / 100;
            int32_t maxColDeaths = m_planet.getCargo(Element::Colonists).orElse(0);
            if (colDeaths > maxColDeaths) {
                colDeaths = maxColDeaths;
            }
            m_planet.setCargo(Element::Colonists, m_planet.getCargo(Element::Colonists).orElse(0) - colDeaths);

            // Native deaths
            natDeaths = (natDeaths+99) / 100;
            int32_t natives = m_planet.getNatives().orElse(0) - natDeaths;
            if (natives < 0) {
                natives = 0;
            }
            m_planet.setNatives(natives);
        }

        // Amorphs
        if (m_planet.getNatives().orElse(0) != 0 && m_planet.getNativeRace().orElse(0) == AmorphousNatives) {
            int32_t eaten = getAmorphousBreakfast(host, m_planet.getNativeHappiness().orElse(0));
            int32_t maxEaten = m_planet.getCargo(Element::Colonists).orElse(0);
            if (eaten > maxEaten) {
                eaten = maxEaten;
            }
            m_planet.setCargo(Element::Colonists, m_planet.getCargo(Element::Colonists).orElse(0) - eaten);
        }

        // THost assim
        if (!host.isPHost()) {
            doAssimilation(m_planet, config);
        }
    }

    // Experience
    // ex WPlanetGrowthTile::drawData (part)
    UnitScoreDefinitionList::Index_t expIndex;
    int16_t expPoints, expTurn;
    if (config[HostConfiguration::NumExperienceLevels]() > 0
        && planetScores.lookup(ScoreId_ExpPoints, expIndex)
        && m_planet.unitScores().get(expIndex, expPoints, expTurn))
    {
        int32_t newExpPoints = expPoints;
        newExpPoints += config[HostConfiguration::EPPlanetAging]();

        int happy = m_planet.getColonistHappiness().orElse(0);
        if (m_planet.getNatives().orElse(0) > 0) {
            happy = std::min(happy, m_planet.getNativeHappiness().orElse(0));
        }
        if (happy > 0) {
            newExpPoints += config[HostConfiguration::EPPlanetGovernment]() * happy / 100;
        }
        m_planet.unitScores().set(expIndex, int16_t(std::min(newExpPoints, 0x7FFF)), expTurn);
    }

    // Clean up
    if (m_planet.getNativeRace().orElse(0) == 0 || m_planet.getNatives().orElse(0) == 0) {
        m_planet.setNativeRace(0);
        m_planet.setNatives(0);
    }
    if (m_planet.getCargo(Element::Colonists).orElse(0) == 0 || planetOwner == 0) {
        m_planet.setCargo(Element::Colonists, 0);
        // FIXME: we cannot yet do that: m_planet.setOwner(mp16_t(0));
    }
}

game::map::Planet&
game::map::PlanetPredictor::planet()
{
    return m_planet;
}
