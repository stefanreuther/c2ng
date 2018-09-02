/**
  *  \file game/maint/difficultyrater.cpp
  */

#include <cmath>
#include <stdexcept>
#include "game/maint/difficultyrater.hpp"
#include "afl/base/countof.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/string.hpp"
#include "game/config/integervalueparser.hpp"
#include "game/v3/structures.hpp"

namespace gt = game::v3::structures;

namespace {
    /** Average cost of a ship.
        A ship list has a difficulty rating of 100% if a ship costs on average this many T+D+M+$. */
    const double AVG_SHIP_COST = 2000.0;

    /** Average minerals on planet.
        A universe has a difficulty rating of 100% if a planet has on average this many T+D+M in core and on surface combined. */
    const double AVG_MINERALS = 1800.0;

    /** Average natives on planet.
        A universe has a difficulty rating of 100% if a planet has on average this many native clans. */
    const double AVG_NATIVES = 25000.0;


    /** Find best engine: first one that does warp 9 at 120% fuel usage. */
    bool findBestEngine(afl::io::Directory& dir, gt::Engine& engine)
    {
        try {
            afl::base::Ref<afl::io::Stream> s = dir.openFile("engspec.dat", afl::io::FileSystem::OpenRead);
            for (int i = 1; i <= gt::NUM_ENGINE_TYPES; ++i) {
                s->fullRead(afl::base::fromObject(engine));
                if (engine.fuelFactors[8] <= 120*81) {
                    break;
                }
            }
            return true;
        }
        catch (afl::except::FileProblemException& e) {
            return false;
        }
    }

    /** Find best beam: best bang. */
    bool findBestBeam(afl::io::Directory& dir, gt::Beam& beam)
    {
        try {
            afl::base::Ref<afl::io::Stream> s = dir.openFile("beamspec.dat", afl::io::FileSystem::OpenRead);
            s->fullRead(afl::base::fromObject(beam));
            for (int i = 2; i <= gt::NUM_BEAM_TYPES; ++i) {
                gt::Beam b2;
                s->fullRead(afl::base::fromObject(b2));
                if (b2.killPower + b2.damagePower > beam.killPower + beam.damagePower) {
                    beam = b2;
                }
            }
            return true;
        }
        catch (afl::except::FileProblemException& e) {
            return false;
        }
    }

    /** Find best torpedo: best bang. */
    bool findBestTorpedo(afl::io::Directory& dir, gt::Torpedo& torp)
    {
        try {
            afl::base::Ref<afl::io::Stream> s = dir.openFile("torpspec.dat", afl::io::FileSystem::OpenRead);
            s->fullRead(afl::base::fromObject(torp));
            for (int i = 2; i <= gt::NUM_TORPEDO_TYPES; ++i) {
                gt::Torpedo t2;
                s->fullRead(afl::base::fromObject(t2));
                if (t2.killPower + t2.damagePower > torp.killPower + torp.damagePower) {
                    torp = t2;
                }
            }
            return true;
        }
        catch (afl::except::FileProblemException& e) {
            return false;
        }
    }


    int32_t sumCost(const gt::Cost& c)
    {
        return int32_t(c.tritanium) + c.duranium + c.molybdenum + c.money;
    }
}


const size_t game::maint::DifficultyRater::Config_MAX;

game::maint::DifficultyRater::OptionValue::OptionValue()
    : game::config::CollapsibleIntegerArrayOption<NUM_PLAYERS>(game::config::IntegerValueParser::instance)
{ }


// /** Constructor. */
game::maint::DifficultyRater::DifficultyRater()
    : config_known(),
      shiplist_known(false),
      shiplist_average_cost(0)
{
    // ex DifficultyRater::DifficultyRater
}

game::maint::DifficultyRater::~DifficultyRater()
{ }

void
game::maint::DifficultyRater::addShipList(afl::io::Directory& dir)
{
    // ex DifficultyRater::addShipList
    // Find best possible equipment
    gt::Engine bestEngine;
    if (!findBestEngine(dir, bestEngine)) {
        return;
    }

    gt::Beam bestBeam;
    if (!findBestBeam(dir, bestBeam)) {
        return;
    }

    gt::Torpedo bestTorpedo;
    if (!findBestTorpedo(dir, bestTorpedo)) {
        return;
    }

    // Process hulls
    try {
        afl::base::Ref<afl::io::Stream> s = dir.openFile("hullspec.dat", afl::io::FileSystem::OpenRead);
        int32_t totalCost = 0;
        int32_t count = 0;
        gt::Hull hull;
        while (s->read(afl::base::fromObject(hull)) == sizeof(hull)) {
            totalCost += hull.tritanium + hull.duranium + hull.molybdenum + hull.money;
            totalCost += hull.numEngines * sumCost(bestEngine.cost);
            totalCost += hull.maxBeams * sumCost(bestBeam.cost);
            totalCost += hull.maxLaunchers * sumCost(bestTorpedo.launcherCost);
            ++count;
        }
        if (count > 0) {
            shiplist_known = true;
            shiplist_average_cost = totalCost / count;
        }
    }
    catch (afl::except::FileProblemException& e) { }
}

void
game::maint::DifficultyRater::addConfigurationDirectory(afl::io::Directory& dir)
{
    // ex DifficultyRater::addConfigDirectory
    // Process config files
    static const char files[][13] = { "pconfig.src", "shiplist.txt", "amaster.src", "pmaster.cfg" };
    for (size_t i = 0; i < countof(files); ++i) {
        afl::base::Ptr<afl::io::Stream> s = dir.openFileNT(files[i], afl::io::FileSystem::OpenRead);
        if (s.get() != 0) {
            addConfigurationFile(*s);
        }
    }
}

void
game::maint::DifficultyRater::addConfigurationFile(afl::io::Stream& s)
{
    // ex DifficultyRater::addConfigFile
    afl::io::TextFile tf(s);
    String_t line;
    String_t prefix;
    while (tf.readLine(line)) {
        line = afl::string::strTrim(line);
        game::config::ConfigurationOption::removeComment(line);
        if (line.empty()) {
            // ignore
        } else if (line[0] == '%') {
            // section delimiter
            prefix = afl::string::strTrim(line.substr(1)) + ".";
        } else {
            String_t::size_type n = line.find('=');
            if (n != line.npos) {
                addConfigurationValue(prefix + afl::string::strRTrim(line.substr(0, n)), afl::string::strLTrim(line.substr(n+1)));
            }
        }
    }
}

void
game::maint::DifficultyRater::addConfigurationValue(String_t name, String_t value)
{
    // ex DifficultyRater::addConfig
    // FIXME: make this return false if the option is not recognized
    // Mapping of names to config keys
    struct Map {
        const char* name;
        Config what;
    };
    static Map mapping[] = {
        { "amaster.nativerange",               Master_NativeRanges },
        { "amaster.nativesonplanetfrequency",  Master_NativeFrequency },
        { "amaster.planetcorerangesalternate", Master_CoreRangesAlternate },
        { "amaster.planetcorerangesusual",     Master_CoreRangesUsual },
        { "amaster.planetcoreusualfrequency",  Master_CoreUsualFrequency },
        { "amaster.planetsurfaceranges",       Master_SurfaceRanges },
        { "phost.colonisttaxrate",             Host_ColonistTaxRate },
        { "phost.hisseffectrate",              Host_HissEffectRate },            // optional
        { "phost.nativetaxrate",               Host_NativeTaxRate },             // optional
        { "phost.playerrace",                  Host_PlayerRace },                // optional
        { "phost.playerspecialmission",        Host_PlayerSpecialMission },      // optional
        { "phost.productionrate",              Host_ProductionRate },
        { "phost.raceminingrate",              Host_MiningRate },
        { "phost.racetaxrate",                 Host_ColonistTaxRate },           // old name
        { "pmaster.nativeclansrange",          Master_NativeClansRange },
        { "pmaster.nativesonplanetfrequency",  Master_NativeFrequency },
        { "pmaster.planetcorerangesalternate", Master_CoreRangesAlternate },
        { "pmaster.planetcorerangesusual",     Master_CoreRangesUsual },
        { "pmaster.planetcoreusualfrequency",  Master_CoreUsualFrequency },
        { "pmaster.planetsurfaceranges",       Master_SurfaceRanges },
    };

    // Look it up
    size_t index = Config_MAX;
    for (size_t i = 0; i < sizeof(mapping)/sizeof(mapping[0]); ++i) {
        if (afl::string::strCaseCompare(mapping[i].name, name) == 0) {
            index = mapping[i].what;
            break;
        }
    }

    // Found? Assign it.
    if (index != Config_MAX) {
        try {
            config_values[index].set(value);
            config_known += Config(index);
        }
        catch (std::exception& e) {
            // Value was invalid, ignore
        }
    }
}

bool
game::maint::DifficultyRater::isRatingKnown(Rating which) const
{
    // ex DifficultyRater::isRatingKnown
    switch (which) {
     case ShiplistRating:
        return shiplist_known;

     case MineralRating:
        return config_known.contains(Master_CoreRangesUsual)
            && config_known.contains(Master_CoreRangesAlternate)
            && config_known.contains(Master_CoreUsualFrequency)
            && config_known.contains(Master_SurfaceRanges);

     case NativeRating:
        return config_known.contains(Master_NativeFrequency)
            && (config_known.contains(Master_NativeRanges)
                || config_known.contains(Master_NativeClansRange));

     case ProductionRating:
        /* optional: Host_HissEffectRate, Host_NativeTaxRate, Host_PlayerRace, Host_PlayerSpecialMission */
        return config_known.contains(Host_ProductionRate)
            && config_known.contains(Host_MiningRate)
            && config_known.contains(Host_ColonistTaxRate);
    }

    return false;
}

double
game::maint::DifficultyRater::getRating(Rating which) const
{
    // ex DifficultyRater::getRating
    if (!isRatingKnown(which)) {
        return 1.00;
    } else {
        switch (which) {
         case ShiplistRating:
            return std::pow(double(shiplist_average_cost) / AVG_SHIP_COST, 0.33);

         case MineralRating:
            return std::pow(AVG_MINERALS / getAverageMinerals(), 0.33);

         case NativeRating:
            return std::pow(AVG_NATIVES / getAverageNatives(), 0.33);

         case ProductionRating:
            return 100.0 / getAverageVPI();
        }
        return 1.00;
    }
}

double
game::maint::DifficultyRater::getTotalRating() const
{
    // ex DifficultyRater::getTotalRating
    return getRating(ShiplistRating)
        * getRating(MineralRating)
        * getRating(NativeRating)
        * getRating(ProductionRating);
}

// /** This computes the average amount of minerals on planets. A game is easier
//     when it has many minerals.

//     AMASTER distinguishes between usual and alternate minerals. We assume that
//     less frequent of the two is the bigger value. AMASTER's selection frequency
//     is shifted 3/4 towards the 0%/100% point, to avoid that the exceptional high
//     value dominates the regular value (reading: the exceptional high value needs
//     extra logistics to use).

//     In addition, we add average surface minerals. */
double
game::maint::DifficultyRater::getAverageMinerals() const
{
    // ex DifficultyRater::getAverageMinerals
    double averageMinerals = 0;

    // Iterate through slots 2..4, i.e. T/D/M; skip slot 1, i.e. N.
    for (int slot = 2; slot <= 4; ++slot) {
        // Core:
        int usual_max = config_values[Master_CoreRangesUsual](slot+4);
        int usual_min = config_values[Master_CoreRangesUsual](slot);
        int alt_max   = config_values[Master_CoreRangesAlternate](slot+4);
        int alt_min   = config_values[Master_CoreRangesAlternate](slot);

        double usual_freq = config_values[Master_CoreUsualFrequency](slot) / 100.0;
        if (usual_freq > 0.5) {
            usual_freq = 1 - (1 - usual_freq)*0.25;
        } else {
            usual_freq *= 0.25;
        }

        averageMinerals += ((usual_max + usual_min) * usual_freq + (alt_max + alt_min) * (1.0 - usual_freq)) / 2.0;

        // Surface:
        int sfc_min = config_values[Master_SurfaceRanges](slot);
        int sfc_max = config_values[Master_SurfaceRanges](slot+4);

        averageMinerals += (sfc_max + sfc_min)/2.0;
    }

    return averageMinerals;
}

// /** Computes the average number of natives on a planet, and that's about it.
//     The idea is to get an average income, which happens to scale linearly
//     with the number of natives.

//     This assumes that all natives are equally valuable, i.e. the added
//     benefit of Ins/Rep/Bov cancels out Amo, and governments are equally
//     distributed. If desired, we could check those, too. */
double
game::maint::DifficultyRater::getAverageNatives() const
{
    // ex DifficultyRater::getAverageNatives
    int averageClansX2 = config_known.contains(Master_NativeRanges)
        ? 10*(config_values[Master_NativeRanges](1) + config_values[Master_NativeRanges](2))
        : (config_values[Master_NativeClansRange](1) + config_values[Master_NativeClansRange](2));
    return config_values[Master_NativeFrequency](1) * averageClansX2 / 200.0;
}

/** Compute average VPI. */
double
game::maint::DifficultyRater::getAverageVPI() const
{
    // ex DifficultyRater::getAverageVPI
    int totalProd = 0;
    int totalMini = 0;
    int totalCtax = 0;
    int totalNtax = 0;
    int totalHiss = 0;

    const double DIVI = NUM_PLAYERS;

    for (int i = 1; i <= NUM_PLAYERS; ++i) {
        // Get config, process defaults
        int prod = config_values[Host_ProductionRate](i);
        int mini = config_values[Host_MiningRate](i);
        int ctax = config_values[Host_ColonistTaxRate](i);
        int ntax = config_known.contains(Host_NativeTaxRate)        ? config_values[Host_NativeTaxRate](i)        : ctax;
        int race = config_known.contains(Host_PlayerRace)           ? config_values[Host_PlayerRace](i)           : i+1;
        int misn = config_known.contains(Host_PlayerSpecialMission) ? config_values[Host_PlayerSpecialMission](i) : race;
        int hiss = misn == 2 ? config_known.contains(Host_HissEffectRate) ? config_values[Host_HissEffectRate](i) : 5 : 0;

        totalProd += prod;
        totalMini += mini;
        totalCtax += ctax;
        totalNtax += ntax;
        totalHiss += hiss;
    }

    // VPI formula
    return -8.0
        + 50*std::pow(1 + (totalProd/DIVI - 100)/100, 0.66)
        + 50*std::sqrt(totalMini/DIVI/100)
        + std::sqrt((totalNtax/DIVI + totalCtax/DIVI)/2*(1+totalHiss/DIVI/5))*0.8;
}
