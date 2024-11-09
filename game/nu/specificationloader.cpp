/**
  *  \file game/nu/specificationloader.cpp
  *  \brief Class game::nu::SpecificationLoader
  */

#include "game/nu/specificationloader.hpp"
#include "afl/data/access.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/string/format.hpp"
#include "afl/string/messages.hpp"
#include "afl/string/parse.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/root.hpp"
#include "game/spec/advantagelist.hpp"
#include "game/spec/basichullfunction.hpp"
#include "game/spec/beam.hpp"
#include "game/spec/engine.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/modifiedhullfunctionlist.hpp"
#include "game/spec/torpedolauncher.hpp"
#include "util/io.hpp"

namespace gs = game::spec;

using afl::data::Access;
using afl::string::Format;
using afl::sys::LogListener;
using game::PlayerSet_t;
using game::config::ConfigurationOption;
using game::config::HostConfiguration;
using game::spec::AdvantageList;
using game::spec::BasicHullFunction;
using game::spec::Hull;
using game::spec::ModifiedHullFunctionList;
using game::spec::ShipList;

namespace {
    /*
     *  Limits
     *
     *  Unlike v3 specification files, nu specification files can be sparse.
     *  Each component lists its Id, the Ids are not implicitly limited by the file size somehow.
     *  To avoid that the server can cause us to allocate unbounded amounts of memory, we limit the indexes.
     *  This is purely a self-protection measure and has no influence on actual behaviour; no other component knows these limits.
     *
     *  As of 20160829, the server uses hulls up to 3033, and beams/torpedoes/engines up to 10/10/9 as normal, so we've got pretty much room.
     *
     *  Our data structure is an array of pointers (PtrVector).
     *  With the current values - 169 hulls, 3033 pointers, 8 bytes/pointer - we have 143 bytes of overhead per hull,
     *  or, almost factor 2 given sizeof(game::spec::Hull) == 152.
     *  Should the server start generating larger hull indexes, consider changing BaseComponentVector into a sparse array or map.
     */
    const int MAX_HULLS = 20000;
    const int MAX_BEAMS = 100;
    const int MAX_TORPEDOES = 100;
    const int MAX_ENGINES = 100;

    const char LOG_NAME[] = "game.nu.specloader";


    // Comparator to compare hulls in a sensible way
    class CompareHulls {
     public:
        CompareHulls(const game::spec::HullVector_t& hulls)
            : m_hulls(hulls)
            { }
        bool operator()(int a, int b)
            {
                game::spec::Hull* aa = m_hulls.get(a);
                game::spec::Hull* bb = m_hulls.get(b);
                if (aa == bb) {
                    // Covers the a==b case and the both-out-of-bounds case
                    return a < b;
                } else if (aa == 0 || bb == 0) {
                    // Either is null
                    return aa < bb;
                } else {
                    int at = aa->getTechLevel();
                    int bt = bb->getTechLevel();
                    if (at == bt) {
                        return a < b;
                    } else {
                        return at < bt;
                    }
                }
            }
     private:
        const game::spec::HullVector_t& m_hulls;
    };

    void loadAdvantages(ShipList& sl, Access rst)
    {
        const Access in = rst("advantages");
        AdvantageList& out = sl.advantages();

        for (size_t i = 0, n = in.getArraySize(); i < n; ++i) {
            const Access a = in[i];
            AdvantageList::Item* p = out.add(a("id").toInteger());
            out.setName       (p, a("name").toString());
            out.setDescription(p, a("description").toString());

            // Consciously ignored:
            //   dur
            //   isbase
            //   locked
            //   mc
            //   mol
            //   tri
            //   value
            // As far as I can tell, these all deal with race design/custom advantages.
        }
    }

    void loadPlayerAdvantages(ShipList& sl, Access rst)
    {
        // Our logic:
        //   use players[].activeadvantages
        //   if that is not known, use races[players[].raceid].baseadvantages
        //   add hardcoded values
        // Nu has an additional check (campaignmode || presetadvantages) before using activeadvantages.
        // Also, Nu hardcodes not only addition, but also removal of advantages
        // (e.g. 79 is always taken from settings, never from activeadvantages/baseadvantages).
        // I believe this implementation is more flexible because it trusts the server;
        // if it sends a value, we expect that to be a correct one.
        const Access players = rst("players");
        const Access settings = rst("settings");
        AdvantageList& out = sl.advantages();

        for (size_t playerIndex = 0, numPlayers = players.getArraySize(); playerIndex < numPlayers; ++playerIndex) {
            const Access thisPlayer = players[playerIndex];
            const int playerNr = thisPlayer("id").toInteger();
            const int raceNr = thisPlayer("raceid").toInteger();

            // Fetch activeadvantages
            afl::data::IntegerList_t adv;
            util::toIntegerList(adv, thisPlayer("activeadvantages"));

            // If still empty, fetch from spec
            if (adv.empty()) {
                util::toIntegerList(adv, util::findArrayItemById(rst("races"), "id", raceNr)("baseadvantages"));
            }

            // Hardcoded
            switch (raceNr) {
             case 1:
                if (settings("quantumtorpedos").toInteger()) {
                    adv.push_back(79);
                }
                break;
             case 3:
                if (settings("superspyadvanced").toInteger()) {
                    adv.push_back(62);
                }
                if (settings("cloakandintercept").toInteger()) {
                    adv.push_back(63);
                }
                break;
             case 4:
                if (settings("fascistdoublebeams").toInteger()) {
                    adv.push_back(36);
                }
                break;
             case 8:
                if (settings("starbasefightertransfer").toInteger()) {
                    adv.push_back(57);
                }
                if (settings("galacticpower").toInteger()) {
                    adv.push_back(77);
                }
                break;
            }

            // Mark them
            for (size_t i = 0, n = adv.size(); i < n; ++i) {
                out.addPlayer(out.find(adv[i]), playerNr);
            }
        }
    }

    void addAllOptions(HostConfiguration& out, Access in, String_t prefix)
    {
        afl::data::StringList_t settingNames;
        in.getHashKeys(settingNames);
        for (size_t i = 0, n = settingNames.size(); i < n; ++i) {
            if (settingNames[i] != "id") {
                out.setOption(prefix + settingNames[i], in(settingNames[i]).toString(), ConfigurationOption::Game);
            }
        }
    }

    void loadConfig(HostConfiguration& config, Access rst)
    {
        // PlayerRace:
        const Access players = rst("players");
        for (size_t i = 0, n = players.getArraySize(); i < n; ++i) {
            const int playerId = players[i]("id").toInteger();
            const int raceId = players[i]("raceid").toInteger();
            if (playerId != 0 && raceId != 0) {
                config[HostConfiguration::PlayerRace].set(playerId, raceId);
                config[HostConfiguration::PlayerSpecialMission].set(playerId, raceId);
            }
        }

        // From game:
        const Access game = rst("game");
        config[HostConfiguration::GameName].set(game("name").toString());

        // From settings:
        const Access settings = rst("settings");
        config[HostConfiguration::AllowGravityWells]      .set(!settings("nowarpwells").toInteger());
        config[HostConfiguration::AllowMinefields]        .set(!settings("nominefields").toInteger());
        config[HostConfiguration::AllowShipCloning]       .set( settings("cloningenabled").toInteger());
        config[HostConfiguration::AllowWraparoundMap]     .set( settings("sphere").toInteger());
        config[HostConfiguration::CloakFailureRate]       .set( settings("cloakfail").toInteger());
        config[HostConfiguration::IonStormActivity]       .set( settings("maxions").toInteger());
        config[HostConfiguration::NumShips]               .set( settings("shiplimit").toInteger());
        config[HostConfiguration::ScanRange]              .set( settings("shipscanrange").toInteger());
        config[HostConfiguration::StructureDecayOnUnowned].set( settings("structuredecayrate").toInteger());
        config[HostConfiguration::StructureDecayPerTurn]  .set( settings("structuredecayrate").toInteger());

        // Hardcoded
        config[HostConfiguration::MaxPlanetaryIncome].set(5000);
        config[HostConfiguration::AllowSuperRefit].set(1);              // Configured by Advantage #3
        config[HostConfiguration::MaximumWebMinefieldRadius].set(150);  // Advantage 20 (Web Mines) says limit is always 150
        config[HostConfiguration::SensorRange].set(200);
        config[HostConfiguration::DarkSenseRange].set(200);

        // Map all Nu settings under their original names
        addAllOptions(config, game, "nu.game.");
        addAllOptions(config, settings, "nu.");

        // Mark everything as sourced in game
        config.setAllOptionsSource(ConfigurationOption::Game);
    }

    void setConfigValue(HostConfiguration::StandardOption_t& opt, PlayerSet_t players, int value)
    {
        for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
            if (players.contains(i)) {
                opt.set(i, value);
            }
        }
    }

    void setImplicitConfiguration(HostConfiguration& config, const AdvantageList& advList)
    {
        /* It is unclear to what extent these abilities serve just for documentation or actually affect the configuration
           (i.e. is the 200% ColonistTaxRate actually triggered by advantage 2, or by raceid=1?).
           The choice of mapping is therefore more or less arbitrary/guesswork. */

        // ColonistTaxRate
        //   2 -> 200% Taxing (Fed)
        config[HostConfiguration::ColonistTaxRate].set(100);
        setConfigValue(config[HostConfiguration::ColonistTaxRate], advList.getPlayers(advList.find(2)), 200);
        config[HostConfiguration::NativeTaxRate].copyFrom(config[HostConfiguration::ColonistTaxRate]);

        // RaceMiningRate
        //   4 -> 70% (Fed)
        //   31 -> 200% (Lizard) or settings.mining200adjustment <-FIXME
        config[HostConfiguration::RaceMiningRate].set(100);
        setConfigValue(config[HostConfiguration::RaceMiningRate], advList.getPlayers(advList.find(4)), 70);
        setConfigValue(config[HostConfiguration::RaceMiningRate], advList.getPlayers(advList.find(31)), 200);

        // GroundKillFactor
        //   80 -> 5X ("Shock Troops")
        //   12 -> 15X (Klingon)
        //   6 -> 30X (Lizard)
        config[HostConfiguration::GroundKillFactor].set(1);
        setConfigValue(config[HostConfiguration::GroundKillFactor], advList.getPlayers(advList.find(80)), 5);
        setConfigValue(config[HostConfiguration::GroundKillFactor], advList.getPlayers(advList.find(12)), 15);
        setConfigValue(config[HostConfiguration::GroundKillFactor], advList.getPlayers(advList.find(6)), 30);

        // GroundDefenseFactor
        //   81 -> 5X ("Fortress")
        //   13 -> 5X (Klingon)
        //   7 -> 15X (Lizard)
        config[HostConfiguration::GroundDefenseFactor].set(1);
        setConfigValue(config[HostConfiguration::GroundDefenseFactor], advList.getPlayers(advList.find(81)), 5);
        setConfigValue(config[HostConfiguration::GroundDefenseFactor], advList.getPlayers(advList.find(13)), 5);
        setConfigValue(config[HostConfiguration::GroundDefenseFactor], advList.getPlayers(advList.find(7)), 15);

        // PlayerSpecialMission
        //   3 -> Super Refit
        //   8 -> Hiss
        //   9 -> Super Spy
        //   11 -> Pillage
        //   14 -> Rob
        //   19 -> Self repair
        //   20 -> Lay web
        //   22 -> Dark Sense
        //   26 -> RGA
        // No setting for race 9/11?
        setConfigValue(config[HostConfiguration::PlayerSpecialMission], advList.getPlayers(advList.find(3)), 1);
        setConfigValue(config[HostConfiguration::PlayerSpecialMission], advList.getPlayers(advList.find(8)), 2);
        setConfigValue(config[HostConfiguration::PlayerSpecialMission], advList.getPlayers(advList.find(9)), 3);
        setConfigValue(config[HostConfiguration::PlayerSpecialMission], advList.getPlayers(advList.find(11)), 4);
        setConfigValue(config[HostConfiguration::PlayerSpecialMission], advList.getPlayers(advList.find(14)), 5);
        setConfigValue(config[HostConfiguration::PlayerSpecialMission], advList.getPlayers(advList.find(19)), 6);
        setConfigValue(config[HostConfiguration::PlayerSpecialMission], advList.getPlayers(advList.find(20)), 7);
        setConfigValue(config[HostConfiguration::PlayerSpecialMission], advList.getPlayers(advList.find(22)), 8);
        setConfigValue(config[HostConfiguration::PlayerSpecialMission], advList.getPlayers(advList.find(26)), 10);

        // PlayerRace
        // These are mostly set from players.raceid already
        //   1 -> Fed Crew Bonus
        //   5 -> Lizard 150% bonus
        //   15 -> Privateer Triple Beam Kill
        //   17 -> Assimilation
        PlayerSet_t adv1Set = advList.getPlayers(advList.find(1));
        setConfigValue(config[HostConfiguration::PlayerRace], adv1Set, 1);
        setConfigValue(config[HostConfiguration::PlayerRace], advList.getPlayers(advList.find(5)), 2);
        setConfigValue(config[HostConfiguration::PlayerRace], advList.getPlayers(advList.find(15)), 5);
        setConfigValue(config[HostConfiguration::PlayerRace], advList.getPlayers(advList.find(17)), 6);

        // AllowFedCombatBonus
        config[HostConfiguration::AllowFedCombatBonus].set(!adv1Set.empty());

        // AllowDeluxeSuperSpy
        config[HostConfiguration::AllowDeluxeSuperSpy].set(!advList.getPlayers(advList.find(10)).empty());

        // FreeFighters
        //   56 -> 1
        //   55 -> 2
        //   54 -> 3
        //   53 -> 4
        //   23 -> 5
        config[HostConfiguration::FreeFighters].set(0);
        setConfigValue(config[HostConfiguration::FreeFighters], advList.getPlayers(advList.find(56)), 1);
        setConfigValue(config[HostConfiguration::FreeFighters], advList.getPlayers(advList.find(55)), 2);
        setConfigValue(config[HostConfiguration::FreeFighters], advList.getPlayers(advList.find(54)), 3);
        setConfigValue(config[HostConfiguration::FreeFighters], advList.getPlayers(advList.find(53)), 4);
        setConfigValue(config[HostConfiguration::FreeFighters], advList.getPlayers(advList.find(23)), 5);

        // CrystalsPreferDeserts
        config[HostConfiguration::CrystalsPreferDeserts].set(!advList.getPlayers(advList.find(21)).empty());

        // UnitsPerTorpRate
        config[HostConfiguration::UnitsPerTorpRate].set(100);
        setConfigValue(config[HostConfiguration::UnitsPerTorpRate], advList.getPlayers(advList.find(24)), 400);

        // AllowBuildFighters
        config[HostConfiguration::AllowBuildFighters].set(0);
        setConfigValue(config[HostConfiguration::AllowBuildFighters], advList.getPlayers(advList.find(25)), 1);

        // FighterSweepRate/Range
        const PlayerSet_t adv29Set = advList.getPlayers(advList.find(29));
        config[HostConfiguration::FighterSweepRate].set(0);
        config[HostConfiguration::FighterSweepRange].set(0);
        setConfigValue(config[HostConfiguration::FighterSweepRate], adv29Set, 20);
        setConfigValue(config[HostConfiguration::FighterSweepRange], adv29Set, 100);

        // AntiCloakImmunity
        config[HostConfiguration::AntiCloakImmunity].set(0);
        setConfigValue(config[HostConfiguration::AntiCloakImmunity], advList.getPlayers(advList.find(32)), 1);

        // MaximumMinefieldRadius
        //   47 -> 100 ly
        //   48 -> 150 ly
        config[HostConfiguration::MaximumMinefieldRadius].set(0);
        setConfigValue(config[HostConfiguration::MaximumMinefieldRadius], advList.getPlayers(advList.find(49)), 100);
        setConfigValue(config[HostConfiguration::MaximumMinefieldRadius], advList.getPlayers(advList.find(48)), 150);
        config[HostConfiguration::MaximumWebMinefieldRadius].copyFrom(config[HostConfiguration::MaximumMinefieldRadius]);

        // AllowShipCloning
        // (alternatively map to Unclonable ability?)
        config[HostConfiguration::AllowShipCloning].set(!advList.getPlayers(advList.find(51)).empty());

        // Intentionally not handled for now:
        //   18 (Recover Minerals)
        //   21 (Desert Worlds)
        //   27 (Dark Sense Defense)
        //   30 (Arctic Planet Colonists)
        //   33 (Diplomatic Spies)
        //   34 (Red Storm Cloud)
        //   35 (Plunder Planet) -> increase efficiency of pillage
        //   36 (2X Faster Beams)
        //   37 (Ion Starbase Shield)
        //   38 (Starbase Money Transfer) -> unlocks SB mission 7=send, 8=receive
        //   39 (Starbase Mine Laying) -> unlocks SB mission 9=lay, 10=lay web
        //   40 (Starbase Mine Sweeping) -> unlocks SB mission 11=sweep
        //   41 (Starbase Fighter Sweeping) -> unlocks SB mission 11=sweep
        //   42 (Energy Defense Field) -> unlocks "edf" fcode
        //   43 (Fighter Patrol Routes)
        //   44 (Destroy Planet)
        //   45 (Star Cluster Radiation Immunity)
        //   46 (Debris Disk Defense)
        //   47 (Improved Desert Habitation)
        //   50 (Super Spy Command)
        //   52 (Advanced Cloning)
        //   57 (Starbase Fighter Transfer)
        //   61 (Dark Detection)
        //   62 (Super Spy Advanced)
        //   63 (Cloak and Intercept)
        //   64 (Ship Building Planets)
        //   65 (Swarming)
        //   66 (Rock Attacks)
        //   67 (Reduced Diplomacy)
        //   68 (Psychic Scanning)
        //   69 (Rob Fighters)
        //   70 (Hardened Mines)
        //   71 (Build Clans) -> unlock mission 27=build robots
        //   72 (Dense Minefields)
        //   73 (Hide In Warp Well) -> unlock mission 28=hide
        //   74 (Enhanced Recycle) -> can probably be mapped through PALRecyclingPer10KT?
        //   75 (Pleasure Planets)
        //   76 (Internal Temp Regulation) -> array-ized ClimateLimitsPopulation?
        //   77 (Galactic Power)
        //   78 (Minefields Save Fuel)
        //   79 (Quantum Torpedos)
        //   83 -> something with larva
        //   85 -> unlocks mission 29=lay hidden minefield, hardwired to privateer only
        //   86 -> unlocks mission 30=call to this hive, hardwired to hull 115, race 12
        //   87 -> something with build points / combineable ships
    }

    void addAbilityToAllHulls(game::spec::HullVector_t& hulls, PlayerSet_t players, ModifiedHullFunctionList::Function_t ability)
    {
        if (!players.empty()) {
            for (Hull* p = hulls.findNext(0); p != 0; p = hulls.findNext(p->getId())) {
                p->changeHullFunction(ability, players, PlayerSet_t(), true);
            }
        }
    }

    void setImplicitHullFunctions(game::spec::HullVector_t& hulls, const game::spec::ModifiedHullFunctionList& modList, const AdvantageList& advList)
    {
        // Boarding
        addAbilityToAllHulls(hulls, advList.getPlayers(advList.find(16)), modList.getFunctionIdFromHostId(BasicHullFunction::Boarding));

        // Planet Immunity
        addAbilityToAllHulls(hulls, advList.getPlayers(advList.find(28)), modList.getFunctionIdFromHostId(BasicHullFunction::PlanetImmunity));
    }
}

game::nu::SpecificationLoader::SpecificationLoader(afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory,
                                                   afl::base::Ref<GameState> gameState,
                                                   afl::string::Translator& tx,
                                                   afl::sys::LogListener& log)
    : m_defaultSpecificationDirectory(defaultSpecificationDirectory),
      m_gameState(gameState),
      m_translator(tx),
      m_log(log)
{ }

game::nu::SpecificationLoader::~SpecificationLoader()
{ }

std::auto_ptr<game::Task_t>
game::nu::SpecificationLoader::loadShipList(game::spec::ShipList& list, Root& root, std::auto_ptr<StatusTask_t> then)
{
    class Task : public Task_t {
     public:
        Task(SpecificationLoader& parent, game::spec::ShipList& list, Root& root, std::auto_ptr<StatusTask_t>& then)
            : m_parent(parent), m_shipList(list), m_root(root), m_then(then)
            { }
        virtual void call()
            {
                try {
                    m_parent.m_log.write(LogListener::Trace, LOG_NAME, "Task: loadShipList");
                    afl::data::Access rst(m_parent.m_gameState->loadResultPreAuthenticated());

                    loadAdvantages(m_shipList, rst("rst"));
                    loadPlayerAdvantages(m_shipList, rst("rst"));
                    loadConfig(m_root.hostConfiguration(), rst("rst"));
                    setImplicitConfiguration(m_root.hostConfiguration(), m_shipList.advantages());
                    setImplicitHullFunctions(m_shipList.hulls(), m_shipList.modifiedHullFunctions(), m_shipList.advantages());

                    m_parent.loadRaceNames(m_root, rst("rst")("players"), rst("rst")("races"));

                    m_parent.loadHullFunctionDefinitions(m_shipList);
                    m_parent.loadHulls    (m_shipList, rst("rst")("hulls"));
                    m_parent.loadBeams    (m_shipList, rst("rst")("beams"));
                    m_parent.loadTorpedoes(m_shipList, rst("rst")("torpedos"));
                    m_parent.loadEngines  (m_shipList, rst("rst")("engines"));

                    m_parent.loadDefaultHullAssignments(m_shipList, rst("rst")("players"),   rst("rst")("races"));
                    m_parent.loadRaceHullAssignments   (m_shipList, rst("rst")("racehulls"), rst("rst")("player")("id").toInteger());

                    // FIXME: process these attributes:
                    // HullFunctionAssignmentList& racialAbilities();
                    // StandardComponentNameProvider& componentNamer();
                    // FriendlyCodeList& friendlyCodes();
                    // MissionList& missions();

                    return m_then->call(true);
                }
                catch (std::exception& e) {
                    m_parent.m_log.write(LogListener::Error, LOG_NAME, String_t(), e);
                    return m_then->call(false);
                }
            }
     private:
        SpecificationLoader& m_parent;
        game::spec::ShipList& m_shipList;
        Root& m_root;
        std::auto_ptr<StatusTask_t> m_then;
    };
    return m_gameState->login(std::auto_ptr<Task_t>(new Task(*this, list, root, then)));
}

afl::base::Ref<afl::io::Stream>
game::nu::SpecificationLoader::openSpecificationFile(const String_t& fileName)
{
    return m_defaultSpecificationDirectory->openFile(fileName, afl::io::FileSystem::OpenRead);
}

void
game::nu::SpecificationLoader::loadHullFunctionDefinitions(game::spec::ShipList& list)
{
    // We load the basic function definitions in the same way as for V3.
    // This enables subsequent code to use it, in particular, the hull "cancloak" flag.
    // We do not ever define modified functions.
    list.basicHullFunctions().clear();
    afl::base::Ptr<afl::io::Stream> ps = m_defaultSpecificationDirectory->openFileNT("hullfunc.usr", afl::io::FileSystem::OpenRead);
    if (ps.get()) {
        list.basicHullFunctions().load(*ps, m_translator, m_log);
    }
    ps = m_defaultSpecificationDirectory->openFileNT("hullfunc.cc", afl::io::FileSystem::OpenRead);
    if (ps.get()) {
        list.basicHullFunctions().load(*ps, m_translator, m_log);
    }
}

void
game::nu::SpecificationLoader::loadHulls(game::spec::ShipList& list, afl::data::Access p)
{
    for (size_t i = 0, n = p.getArraySize(); i < n; ++i) {
        afl::data::Access in = p[i];
        int nr = in("id").toInteger();
        if (nr <= 0 || nr > MAX_HULLS) {
            m_log.write(LogListener::Warn, LOG_NAME, Format(m_translator("Invalid hull number %d, component has been ignored"), nr));
        } else if (gs::Hull* out = list.hulls().create(nr)) {
            // Component:
            out->setMass(in("mass").toInteger());
            out->setTechLevel(in("techlevel").toInteger());
            out->cost().set(gs::Cost::Money,      in("cost").toInteger());
            out->cost().set(gs::Cost::Tritanium,  in("tritanium").toInteger());
            out->cost().set(gs::Cost::Duranium,   in("duranium").toInteger());
            out->cost().set(gs::Cost::Molybdenum, in("molybdenum").toInteger());
            out->setName(in("name").toString());

            // Hull:
            out->setExternalPictureNumber(1); // FIXME!
            out->setInternalPictureNumber(1); // FIXME!
            out->setMaxFuel     (in("fueltank").toInteger());
            out->setMaxCrew     (in("crew").toInteger());
            out->setNumEngines  (in("engines").toInteger());
            out->setMaxCargo    (in("cargo").toInteger());
            out->setNumBays     (in("fighterbays").toInteger());
            out->setMaxLaunchers(in("launchers").toInteger());
            out->setMaxBeams    (in("beams").toInteger());

            if (in("cancloak").toInteger() != 0) {
                out->changeHullFunction(list.modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::Cloak),
                                        PlayerSet_t::allUpTo(MAX_PLAYERS), PlayerSet_t(), true);
            }

            // Other abilities:
            //  29,31,3033,1047: adv cloak (no fuel usage)
            //  109,1023,1049: chamaeleon
            //  97,104,105: alchemy
            //  108: "matrix"
            //  1089: "command"
            //  56,1055: chunnel initiate
            //  108,56,1055: ?
            //  56: chunnel
            //  56,1054,51,1055: chunnel target
            //  51,77,87,110: hyp (inconsistency in rendering for 110?)
            //  1090: repair
            //  [200,300): horwasp specials; not really ships
            //  205: accelerator pod (not really a ship)
            //  84,96,9,1084: bioscan (inconsistency in rendering for 1084?)
            //  70: fighter receiver (with advantage 57)
            //  70: destroy planet (with advantage 44)
            //  113: push/pull mine field
            //  111: tantrum
            //  112: not renameable(?)
            //  6,33,34,35,36,37,38,39,40,41,68,93,1068,1093,1033,1006,2006,1068,3033,2033,1041,1039,107,1037,1038,2038: no radiation
            //  29,31: reduced radiation
            //  39,41,1034,1039,1041: pop/trg
            //  115,116: something with neutronium?

            // Other attributes:
            //   dur, tri, mol, mc, advantage - cost of optional hulls during race building
            //   parentid                     - if improved version, link to original
            //   special, description         - plaintext hullfuncs
            //   isbase                       - true if default hull of any race
            //   academy                      - available in "academy game" (?)
        } else {
            m_log.write(LogListener::Warn, LOG_NAME, Format(m_translator("Invalid hull number %d, component has been ignored"), nr));
        }
    }
}

void
game::nu::SpecificationLoader::loadBeams(game::spec::ShipList& list, afl::data::Access p)
{
    for (size_t i = 0, n = p.getArraySize(); i < n; ++i) {
        afl::data::Access in = p[i];
        int nr = in("id").toInteger();
        if (nr <= 0 || nr > MAX_BEAMS) {
            m_log.write(LogListener::Warn, LOG_NAME, Format(m_translator("Invalid beam number %d, component has been ignored"), nr));
        } else if (gs::Beam* out = list.beams().create(nr)) {
            // Component:
            out->setMass(in("mass").toInteger());
            out->setTechLevel(in("techlevel").toInteger());
            out->cost().set(gs::Cost::Money,      in("cost").toInteger());
            out->cost().set(gs::Cost::Tritanium,  in("tritanium").toInteger());
            out->cost().set(gs::Cost::Duranium,   in("duranium").toInteger());
            out->cost().set(gs::Cost::Molybdenum, in("molybdenum").toInteger());
            out->setName(in("name").toString());

            // Weapon:
            out->setKillPower  (in("crewkill").toInteger());
            out->setDamagePower(in("damage").toInteger());
        } else {
            m_log.write(LogListener::Warn, LOG_NAME, Format(m_translator("Invalid beam number %d, component has been ignored"), nr));
        }
    }
}

void
game::nu::SpecificationLoader::loadTorpedoes(game::spec::ShipList& list, afl::data::Access p)
{
    for (size_t i = 0, n = p.getArraySize(); i < n; ++i) {
        afl::data::Access in = p[i];
        int nr = in("id").toInteger();
        if (nr <= 0 || nr > MAX_TORPEDOES) {
            m_log.write(LogListener::Warn, LOG_NAME, Format(m_translator("Invalid torpedo number %d, component has been ignored"), nr));
        } else if (gs::TorpedoLauncher* out = list.launchers().create(nr)) {
            // Component:
            out->setMass(in("mass").toInteger());
            out->setTechLevel(in("techlevel").toInteger());
            out->cost().set(gs::Cost::Money,      in("launchercost").toInteger());
            out->cost().set(gs::Cost::Tritanium,  in("tritanium").toInteger());
            out->cost().set(gs::Cost::Duranium,   in("duranium").toInteger());
            out->cost().set(gs::Cost::Molybdenum, in("molybdenum").toInteger());
            out->setName(in("name").toString());

            // Weapon:
            out->setKillPower(in("crewkill").toInteger());
            out->setDamagePower(in("damage").toInteger());

            // Torpedo:
            out->torpedoCost().set(gs::Cost::Money, in("torpedocost").toInteger());
            out->torpedoCost().set(gs::Cost::Tritanium, 1);
            out->torpedoCost().set(gs::Cost::Duranium, 1);
            out->torpedoCost().set(gs::Cost::Molybdenum, 1);

            // FIXME: deal with combatrange field (300 for normal, 340 for Quantum Torpedo, but may be not present)
        } else {
            m_log.write(LogListener::Warn, LOG_NAME, Format(m_translator("Invalid torpedo number %d, component has been ignored"), nr));
        }
    }
}

void
game::nu::SpecificationLoader::loadEngines(game::spec::ShipList& list, afl::data::Access p)
{
    for (size_t i = 0, n = p.getArraySize(); i < n; ++i) {
        afl::data::Access in = p[i];
        int nr = in("id").toInteger();
        if (nr <= 0 || nr > MAX_ENGINES) {
            m_log.write(LogListener::Warn, LOG_NAME, Format(m_translator("Invalid engine number %d, component has been ignored"), nr));
        } else if (gs::Engine* out = list.engines().create(nr)) {
            // Component:
            out->setMass(0);
            out->setTechLevel(in("techlevel").toInteger());
            out->cost().set(gs::Cost::Money,      in("cost").toInteger());
            out->cost().set(gs::Cost::Tritanium,  in("tritanium").toInteger());
            out->cost().set(gs::Cost::Duranium,   in("duranium").toInteger());
            out->cost().set(gs::Cost::Molybdenum, in("molybdenum").toInteger());
            out->setName(in("name").toString());

            // Engine
            for (int i = 1; i <= 9; ++i) {
                out->setFuelFactor(i, in(Format("warp%d", i)).toInteger());
            }
        } else {
            m_log.write(LogListener::Warn, LOG_NAME, Format(m_translator("Invalid engine number %d, component has been ignored"), nr));
        }
    }
}

/* Load default hull assignments.
   Nu does not provide a truehull record for each player; we only see the default race definitions.
   This populates the HullAssignmentList with the given defaults. */
void
game::nu::SpecificationLoader::loadDefaultHullAssignments(game::spec::ShipList& list, afl::data::Access players, afl::data::Access races)
{
    for (size_t player = 0, nplayers = players.getArraySize(); player < nplayers; ++player) {
        // Get raceid
        afl::data::Access p = players[player];
        int raceId = p("raceid").toInteger();
        int playerId = p("id").toInteger();

        // Get associated race
        afl::data::Access r = util::findArrayItemById(races, "id", raceId);

        // Get base hulls which are cleverly encoded as a string
        afl::data::IntegerList_t hulls;
        util::toIntegerList(hulls, r("basehulls"));

        // Sort into sensible order (for users; not required for turn file validity)
        std::sort(hulls.begin(), hulls.end(), CompareHulls(list.hulls()));

        // Populate this player's entry
        for (size_t i = 0, n = hulls.size(); i < n; ++i) {
            list.hullAssignments().add(playerId, int(i+1), hulls[i]);
        }
    }
}

void
game::nu::SpecificationLoader::loadRaceHullAssignments(game::spec::ShipList& list, afl::data::Access racehulls, int player)
{
    list.hullAssignments().clearPlayer(player);
    for (size_t i = 0, n = racehulls.getArraySize(); i < n; ++i) {
        list.hullAssignments().add(player, int(i+1), racehulls[i].toInteger());
    }
}

void
game::nu::SpecificationLoader::loadRaceNames(Root& root, afl::data::Access players, afl::data::Access races)
{
    for (size_t i = 0, n = players.getArraySize(); i < n; ++i) {
        afl::data::Access in = players[i];
        int nr = in("id").toInteger();
        if (Player* out = root.playerList().get(nr)) {
            // Update race name (if it fails, keep the dummy set up by GameFolder).
            afl::data::Access race = util::findArrayItemById(races, "id", in("raceid").toInteger());
            if (!race.isNull()) {
                out->setName(Player::LongName,      race("name").toString());
                out->setName(Player::ShortName,     race("shortname").toString());
                out->setName(Player::AdjectiveName, race("adjective").toString());
                out->setOriginalNames();
            }

            // Other names
            out->setName(Player::UserName,     in("username").toString());
            out->setName(Player::EmailAddress, in("email").toString());
        } else {
            m_log.write(LogListener::Warn, LOG_NAME, Format(m_translator("Invalid player number %d, entry has been ignored"), nr));
        }
    }
    root.playerList().notifyListeners();
}
