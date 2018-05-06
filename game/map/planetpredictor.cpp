/**
  *  \file game/map/planetpredictor.cpp
  */

#include <cmath>
#include "game/map/planetpredictor.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/map/planeteffectors.hpp"
#include "game/map/planetformula.hpp"
#include "util/math.hpp"

using game::Element;
using game::map::Planet;
using util::PI;

namespace {
    void trimHappiness(Planet& pl)
    {
        int chappy = pl.getColonistHappiness().orElse(0);
        if (chappy < -300) {
            chappy = -300;
        }
        if (chappy > 100) {
            chappy = 100;
        }
        pl.setColonistHappiness(chappy);

        int nhappy = pl.getNativeHappiness().orElse(0);
        if (nhappy < -300) {
            nhappy = -300;
        }
        if (nhappy > 100) {
            nhappy = 100;
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
        int planetOwner = 0;
        int max;
        int have;
        if (pl.getOwner(planetOwner)
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
        int planetOwner = 0;
        if (pl.getOwner(planetOwner) && config.getPlayerRaceNumber(planetOwner) == 6) {
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
                                        const game::config::HostConfiguration& config,
                                        const HostVersion& host)
{
    // ex computeTurn
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
    int planetOwner = 0;
    m_planet.getOwner(planetOwner);

    // Hiss
    if (config[config.AllowHiss]()) {
        int nhiss = eff.get(PlanetEffectors::Hiss);
        if (host.isPHost() && nhiss > config[config.MaxShipsHissing]()) {
            nhiss = config[config.MaxShipsHissing]();
        }

        nhiss *= config[config.HissEffectRate](planetOwner);

        m_planet.setColonistHappiness(m_planet.getColonistHappiness().orElse(0) + nhiss);
        m_planet.setNativeHappiness  (m_planet.getNativeHappiness()  .orElse(0) + nhiss);
        trimHappiness(m_planet);
    }

    // LFM, Gather-build, free fighters would be here

    // Terraform
    if (config[config.AllowScienceMissions]()) {
        int rate = host.isPHost() ? config[config.TerraformRate](planetOwner) : 1;
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
        int32_t fact = m_planet.getNumBuildings(FactoryBuilding).orElse(0);
        if (host.isPHost()) {
            fact = fact * config[config.ProductionRate](planetOwner) / 100;
        }
        fact += getBovinoidSupplyContributionLimited(m_planet, config, host).orElse(0);
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
            /* The following is a straight port of the PCC 1.x version.
               Like that, it's incomplete and sucks. Original comment follows:

               This code sucks dead hamsters through nanotubes.
               This is almost a bootleg of Tim's code.
               It should be rewritten in a cleaner way. */
            int32_t growth = 0;
            if (m_planet.getColonistHappiness().orElse(0) >= 70) {
                if (config.getPlayerRaceNumber(planetOwner) == 7 && config[config.CrystalsPreferDeserts]()) {
                    if (m_planet.getTemperature().orElse(0) > 14) {
                        growth = util::divideAndRound(m_planet.getTemperature().orElse(0) * m_planet.getCargo(Element::Colonists).orElse(0), 400 * (m_planet.getColonistTax().orElse(0)+5));
                    }
                } else {
                    growth = util::roundToInt(std::sin((100 - m_planet.getTemperature().orElse(0)) * 0.0314) * m_planet.getCargo(Element::Colonists).orElse(0) / 4 / (m_planet.getColonistTax().orElse(0) + 5.0));
                }
            }

            int32_t maxPop, realMaxPop;
            if (!config[config.ClimateLimitsPopulation]()) {
                maxPop     = 100000;
                realMaxPop = 100000;
            } else if (config.getPlayerRaceNumber(planetOwner) == 7 && config[config.CrystalsPreferDeserts]()) {
                maxPop     = 1000 * m_planet.getTemperature().orElse(0);
                realMaxPop = maxPop;
            } else {
                maxPop     = util::roundToInt(100000 * std::sin((100 - m_planet.getTemperature().orElse(0)) * 0.0314));
                realMaxPop = maxPop;
                if (m_planet.getTemperature().orElse(0) > 84 || m_planet.getTemperature().orElse(0) <= 14) {
                    realMaxPop = 1;
                    growth = 0;
                    maxPop = m_planet.getCargo(Element::Colonists).orElse(0) * (100 - config[config.ClimateDeathRate](planetOwner)) / 100;
                    if (maxPop < 1) {
                        maxPop = 1;
                    }
                    if (m_planet.getTemperature().orElse(0) > 50) {
                        maxPop += 2*(100 - m_planet.getTemperature().orElse(0));
                        realMaxPop += 2*(100 - m_planet.getTemperature().orElse(0));
                    } else {
                        maxPop += 2*(1 + m_planet.getTemperature().orElse(0));
                        realMaxPop += 2*(1 + m_planet.getTemperature().orElse(0));
                    }
                }
                if (m_planet.getTemperature().orElse(0) > 80 && (config.getPlayerRaceNumber(planetOwner) >= 9 || config.getPlayerRaceNumber(planetOwner) == 4)) {
                    if (maxPop < 60) {
                        maxPop = realMaxPop = 60;
                    }
                }
                if (m_planet.getTemperature().orElse(0) < 20 && config.getPlayerRaceNumber(planetOwner) == 10) {
                    if (maxPop < 200) {
                        maxPop = realMaxPop = 90000;
                    }
                }
            }

            if (m_planet.getCargo(Element::Colonists).orElse(0) > maxPop && config[config.AllowEatingSupplies](planetOwner) && m_planet.getCargo(Element::Supplies).orElse(0) > 0) {
                int32_t eaten = 1 + (m_planet.getCargo(Element::Colonists).orElse(0) - maxPop) / 40;
                int32_t maxEaten = m_planet.getCargo(Element::Supplies).orElse(0);
                if (eaten > maxEaten) {
                    eaten = maxEaten;
                }
                m_planet.setCargo(Element::Supplies, m_planet.getCargo(Element::Supplies).orElse(0) - eaten);
                maxPop += 40*m_planet.getCargo(Element::Supplies).orElse(0);
            }

            /* FIXME: this is not a universal rule. Check it. */
            if (m_planet.getCargo(Element::Colonists).orElse(0) > 50000) {
                growth /= 2;
            }

            // This is not a 1:1 conversion from PCC 1.x.
            if (config.getPlayerRaceNumber(planetOwner) != 7 && (m_planet.getTemperature().orElse(0) > 84 || m_planet.getTemperature().orElse(0) < 14)) {
                growth = 0;
            }
            if (maxPop > 250000) {
                maxPop = 250000;
            }

            int32_t newPop = m_planet.getCargo(Element::Colonists).orElse(0) + growth;
            if (newPop > maxPop) {
                newPop = maxPop;
            }
            m_planet.setCargo(Element::Colonists, newPop);
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
