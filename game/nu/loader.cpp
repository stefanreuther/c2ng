/**
  *  \file game/nu/loader.cpp
  *  \brief Class game::nu::Loader
  */

#include "game/nu/loader.hpp"
#include "afl/base/countof.hpp"
#include "afl/except/invaliddataexception.hpp"
#include "afl/string/format.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/config/configurationoption.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/element.hpp"
#include "game/map/universe.hpp"
#include "game/player.hpp"
#include "game/playerlist.hpp"
#include "game/root.hpp"
#include "game/spec/advantagelist.hpp"
#include "game/spec/basichullfunction.hpp"
#include "game/spec/beam.hpp"
#include "game/spec/cost.hpp"
#include "game/spec/engine.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/modifiedhullfunctionlist.hpp"
#include "game/spec/torpedolauncher.hpp"
#include "game/vcr/classic/battle.hpp"
#include "game/vcr/classic/database.hpp"
#include "util/io.hpp"

using afl::data::Access;
using afl::string::ConstStringMemory_t;
using afl::string::Format;
using afl::string::Translator;
using afl::sys::LogListener;
using game::Element;
using game::Player;
using game::PlayerSet_t;
using game::Root;
using game::Turn;
using game::config::ConfigurationOption;
using game::config::HostConfiguration;
using game::map::IonStorm;
using game::map::Minefield;
using game::map::Planet;
using game::map::Point;
using game::map::Ship;
using game::map::Universe;
using game::spec::AdvantageList;
using game::spec::BasicHullFunction;
using game::spec::Beam;
using game::spec::Cost;
using game::spec::Engine;
using game::spec::Hull;
using game::spec::ModifiedHullFunctionList;
using game::spec::ShipList;
using game::spec::TorpedoLauncher;
using game::vcr::classic::Battle;
using game::vcr::classic::Database;

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

    const char LOG_NAME[] = "game.nu";


    /*
     *  Low Level
     */

    size_t eatChar(ConstStringMemory_t& mem, const char ch)
    {
        const char* p;
        size_t n = 0;
        while ((p = mem.at(n)) != 0 && *p == ch) {
            ++n;
        }
        mem.split(n);
        return n;
    }

    bool eatNumber(ConstStringMemory_t& mem, int& out)
    {
        out = 0;
        const char* p;
        bool ok = false;
        while ((p = mem.at(0)) != 0 && (*p >= '0' && *p <= '9')) {
            out = 10*out + (*p - '0');
            mem.split(1);
            ok = true;
        }
        return ok;
    }

    bool eatMeridian(ConstStringMemory_t& mem, bool& out)
    {
        static const char AM[] = {'A','M'}, PM[] = {'P','M'};
        if (mem.equalContent(AM)) {
            out = false;
            return true;
        } else if (mem.equalContent(PM)) {
            out = true;
            return true;
        } else {
            return false;
        }
    }

    /*
     *  Ship List
     */

    // Comparator to compare hulls in a sensible way
    class CompareHulls {
     public:
        CompareHulls(const game::spec::HullVector_t& hulls)
            : m_hulls(hulls)
            { }
        bool operator()(int a, int b)
            {
                const Hull* aa = m_hulls.get(a);
                const Hull* bb = m_hulls.get(b);
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
        for (Hull* p = hulls.findNext(0); p != 0; p = hulls.findNext(p->getId())) {
            p->changeHullFunction(ability, players, PlayerSet_t::allUpTo(game::MAX_PLAYERS) - players, true);
        }
    }

    void setImplicitHullFunctions(game::spec::HullVector_t& hulls, const game::spec::ModifiedHullFunctionList& modList, const AdvantageList& advList)
    {
        // Boarding
        addAbilityToAllHulls(hulls, advList.getPlayers(advList.find(16)), modList.getFunctionIdFromHostId(BasicHullFunction::Boarding));

        // Planet Immunity
        addAbilityToAllHulls(hulls, advList.getPlayers(advList.find(28)), modList.getFunctionIdFromHostId(BasicHullFunction::PlanetImmunity));
    }

    void loadRaceNames(Root& root, Access players, Access races, LogListener& log, Translator& tx)
    {
        for (size_t i = 0, n = players.getArraySize(); i < n; ++i) {
            Access in = players[i];
            int nr = in("id").toInteger();
            if (Player* out = root.playerList().get(nr)) {
                // Update race name (if it fails, keep the dummy set up by GameFolder).
                Access race = util::findArrayItemById(races, "id", in("raceid").toInteger());
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
                log.write(LogListener::Warn, LOG_NAME, Format(tx("Invalid player number %d, entry has been ignored"), nr));
            }
        }
        root.playerList().notifyListeners();
    }

    void loadHulls(ShipList& list, Access p, LogListener& log, Translator& tx)
    {
        for (size_t i = 0, n = p.getArraySize(); i < n; ++i) {
            Access in = p[i];
            int nr = in("id").toInteger();
            if (nr <= 0 || nr > MAX_HULLS) {
                log.write(LogListener::Warn, LOG_NAME, Format(tx("Invalid hull number %d, component has been ignored"), nr));
            } else if (Hull* out = list.hulls().create(nr)) {
                // Component:
                out->setMass(in("mass").toInteger());
                out->setTechLevel(in("techlevel").toInteger());
                out->cost().set(Cost::Money,      in("cost").toInteger());
                out->cost().set(Cost::Tritanium,  in("tritanium").toInteger());
                out->cost().set(Cost::Duranium,   in("duranium").toInteger());
                out->cost().set(Cost::Molybdenum, in("molybdenum").toInteger());
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
                    out->changeHullFunction(list.modifiedHullFunctions().getFunctionIdFromHostId(BasicHullFunction::Cloak),
                                            PlayerSet_t::allUpTo(game::MAX_PLAYERS), PlayerSet_t(), true);
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
                log.write(LogListener::Warn, LOG_NAME, Format(tx("Invalid hull number %d, component has been ignored"), nr));
            }
        }
    }

    void loadBeams(ShipList& list, Access p, LogListener& log, Translator& tx)
    {
        for (size_t i = 0, n = p.getArraySize(); i < n; ++i) {
            Access in = p[i];
            int nr = in("id").toInteger();
            if (nr <= 0 || nr > MAX_BEAMS) {
                log.write(LogListener::Warn, LOG_NAME, Format(tx("Invalid beam number %d, component has been ignored"), nr));
            } else if (Beam* out = list.beams().create(nr)) {
                // Component:
                out->setMass(in("mass").toInteger());
                out->setTechLevel(in("techlevel").toInteger());
                out->cost().set(Cost::Money,      in("cost").toInteger());
                out->cost().set(Cost::Tritanium,  in("tritanium").toInteger());
                out->cost().set(Cost::Duranium,   in("duranium").toInteger());
                out->cost().set(Cost::Molybdenum, in("molybdenum").toInteger());
                out->setName(in("name").toString());

                // Weapon:
                out->setKillPower  (in("crewkill").toInteger());
                out->setDamagePower(in("damage").toInteger());
            } else {
                log.write(LogListener::Warn, LOG_NAME, Format(tx("Invalid beam number %d, component has been ignored"), nr));
            }
        }
    }

    void loadTorpedoes(ShipList& list, Access p, LogListener& log, Translator& tx)
    {
        for (size_t i = 0, n = p.getArraySize(); i < n; ++i) {
            Access in = p[i];
            int nr = in("id").toInteger();
            if (nr <= 0 || nr > MAX_TORPEDOES) {
                log.write(LogListener::Warn, LOG_NAME, Format(tx("Invalid torpedo number %d, component has been ignored"), nr));
            } else if (TorpedoLauncher* out = list.launchers().create(nr)) {
                // Component:
                out->setMass(in("mass").toInteger());
                out->setTechLevel(in("techlevel").toInteger());
                out->cost().set(Cost::Money,      in("launchercost").toInteger());
                out->cost().set(Cost::Tritanium,  in("tritanium").toInteger());
                out->cost().set(Cost::Duranium,   in("duranium").toInteger());
                out->cost().set(Cost::Molybdenum, in("molybdenum").toInteger());
                out->setName(in("name").toString());

                // Weapon:
                out->setKillPower(in("crewkill").toInteger());
                out->setDamagePower(in("damage").toInteger());

                int range = in("combatrange").toInteger();
                if (range != 0) {
                    // Normal for new RSTs
                    out->setFiringRangeBonus(range - 300);
                } else if (nr == 11) {
                    // Old RST that is missing the parameter, but has the Quantum Torpedos
                    out->setFiringRangeBonus(40);
                } else {
                    // Normal
                    out->setFiringRangeBonus(0);
                }

                // Torpedo:
                out->torpedoCost().set(Cost::Money, in("torpedocost").toInteger());
                out->torpedoCost().set(Cost::Tritanium, 1);
                out->torpedoCost().set(Cost::Duranium, 1);
                out->torpedoCost().set(Cost::Molybdenum, 1);
            } else {
                log.write(LogListener::Warn, LOG_NAME, Format(tx("Invalid torpedo number %d, component has been ignored"), nr));
            }
        }
    }

    void loadEngines(ShipList& list, Access p, LogListener& log, Translator& tx)
    {
        for (size_t i = 0, n = p.getArraySize(); i < n; ++i) {
            Access in = p[i];
            int nr = in("id").toInteger();
            if (nr <= 0 || nr > MAX_ENGINES) {
                log.write(LogListener::Warn, LOG_NAME, Format(tx("Invalid engine number %d, component has been ignored"), nr));
            } else if (Engine* out = list.engines().create(nr)) {
                // Component:
                out->setMass(0);
                out->setTechLevel(in("techlevel").toInteger());
                out->cost().set(Cost::Money,      in("cost").toInteger());
                out->cost().set(Cost::Tritanium,  in("tritanium").toInteger());
                out->cost().set(Cost::Duranium,   in("duranium").toInteger());
                out->cost().set(Cost::Molybdenum, in("molybdenum").toInteger());
                out->setName(in("name").toString());

                // Engine
                for (int i = 1; i <= 9; ++i) {
                    out->setFuelFactor(i, in(Format("warp%d", i)).toInteger());
                }
            } else {
                log.write(LogListener::Warn, LOG_NAME, Format(tx("Invalid engine number %d, component has been ignored"), nr));
            }
        }
    }

    /* Load default hull assignments.
       Nu does not provide a truehull record for each player; we only see the default race definitions.
       This populates the HullAssignmentList with the given defaults. */
    void loadDefaultHullAssignments(ShipList& list, Access players, Access races)
    {
        for (size_t player = 0, nplayers = players.getArraySize(); player < nplayers; ++player) {
            // Get raceid
            Access p = players[player];
            int raceId = p("raceid").toInteger();
            int playerId = p("id").toInteger();

            // Get associated race
            Access r = util::findArrayItemById(races, "id", raceId);

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

    void loadRaceHullAssignments(ShipList& list, Access racehulls, int player)
    {
        list.hullAssignments().clearPlayer(player);
        for (size_t i = 0, n = racehulls.getArraySize(); i < n; ++i) {
            list.hullAssignments().add(player, int(i+1), racehulls[i].toInteger());
        }
    }

    /*
     *  Turn Data
     */

    /** Check for known planet.
        A planet is known (possibly as unowned) if we have a sensible value in any of its fields.
        There is no explicit flag regarding this fact in the data. */
    bool isKnownPlanet(Access p)
    {
        if (p("friendlycode").toString() != "?""?""?") {
            return true;
        }
        const char*const FIELDS[] = {
            "mines",
            "factories",
            "defense",
            "neutronium",
            "tritanium",
            "duranium",
            "molybdenum",
            "clans",
            "supplies",
            "megacredits",
            "groundneutronium",
            "groundtritanium",
            "groundduranium",
            "groundmolybdenum",
            "densityneutronium",
            "densitytritanium",
            "densityduranium",
            "densitymolybdenum",
            "colonisttaxrate",
            "nativetaxrate",
            "colonisthappypoints",
            "nativehappypoints",
            "nativegovernment",
            "nativeclans",
            "nativetype",
        };
        for (size_t i = 0; i < countof(FIELDS); ++i) {
            if (p(FIELDS[i]).toInteger() > 0) {
                return true;
            }
        }
        return false;
    }

    void addOptionalInteger(game::parser::MessageInformation& info, game::parser::MessageIntegerIndex ii, Access a, int minValue)
    {
        if (a.getValue() != 0) {
            int v = a.toInteger();
            if (v >= minValue) {
                info.addValue(ii, v);
            }
        }
    }

    game::vcr::Object unpackVcrObject(Access p, int owner, bool isPlanet)
    {
        game::vcr::Object obj;

        obj.setMass(p("mass").toInteger());
        obj.setShield(p("shield").toInteger());
        obj.setDamage(p("damage").toInteger());
        obj.setCrew(p("crew").toInteger());
        obj.setId(p("objectid").toInteger());
        obj.setOwner(owner);
        obj.setRace(p("raceid").toInteger());
        obj.setHull(p("hullid").toInteger());
        obj.setBeamType(p("beamid").toInteger());
        obj.setNumBeams(p("beamcount").toInteger());
        obj.setTorpedoType(p("torpedoid").toInteger());
        obj.setNumTorpedoes(p("torpedos").toInteger());
        obj.setNumLaunchers(p("launchercount").toInteger());
        obj.setNumBays(p("baycount").toInteger());
        obj.setNumFighters(p("fighters").toInteger());
        obj.setExperienceLevel(0);
        obj.setBeamKillRate(p("beamkillbonus").toInteger());
        obj.setBeamChargeRate(p("beamchargerate").toInteger());
        obj.setTorpMissRate(p("torpmisspercent").toInteger());
        obj.setTorpChargeRate(p("torpchargerate").toInteger());
        obj.setCrewDefenseRate(p("crewdefensepercent").toInteger());
        obj.setIsPlanet(isPlanet);

        obj.setName(p("name").toString());

        // FIXME: synthesize attributes:
        //   obj.setPicture()

        // FIXME: handle attributes:
        //   temperature
        //   hasstarbase

        // Consciously ignored:
        //   vcrid
        //   side
        //   id

        return obj;
    }

    void unpackTransporter(Ship& out, Ship::Transporter which, int id, Access in)
    {
        out.setTransporterTargetId(which, id);
        out.setTransporterCargo(which, Element::Neutronium, in("transferneutronium").toInteger());
        out.setTransporterCargo(which, Element::Tritanium,  in("transfertritanium").toInteger());
        out.setTransporterCargo(which, Element::Duranium,   in("transferduranium").toInteger());
        out.setTransporterCargo(which, Element::Molybdenum, in("transfermolybdenum").toInteger());
        out.setTransporterCargo(which, Element::Colonists,  in("transferclans").toInteger());
        out.setTransporterCargo(which, Element::Supplies,   in("transfersupplies").toInteger());

        // FIXME: unhandled:
        //   transferammo
        //   transfermegacredits
    }

    void loadPlanets(Universe& univ, Access planets, PlayerSet_t players, LogListener& log, Translator& tx)
    {
        const size_t n = planets.getArraySize();
        log.write(LogListener::Debug, LOG_NAME, Format(tx("Loading %d planet%!1{s%}..."), n));
        for (size_t i = 0; i < n; ++i) {
            Access in = planets[i];
            int id = in("id").toInteger();
            Planet* out = univ.planets().create(id);
            if (!out) {
                throw afl::except::InvalidDataException(Format(tx("Invalid planet Id #%d"), id));
            }

            // Location and Name
            out->setPosition(Point(in("x").toInteger(), in("y").toInteger()));
            out->setName(in("name").toString());

            // Is this planet known?
            if (isKnownPlanet(in)) {
                int owner = in("ownerid").toInteger();
                if (players.contains(owner)) {
                    out->addPlanetSource(players);
                }
                out->setFriendlyCode(in("friendlycode").toString());
                out->setNumBuildings(game::MineBuilding,    in("mines").toInteger());
                out->setNumBuildings(game::FactoryBuilding, in("factories").toInteger());
                out->setNumBuildings(game::DefenseBuilding, in("defense").toInteger());
                out->setBuildBaseFlag(in("buildingstarbase").toInteger() != 0);
                out->setCargo(Element::Money,      in("megacredits").toInteger());
                out->setCargo(Element::Supplies,   in("supplies").toInteger());
                out->setCargo(Element::Neutronium, in("neutronium").toInteger());
                out->setCargo(Element::Molybdenum, in("molybdenum").toInteger());
                out->setCargo(Element::Duranium,   in("duranium").toInteger());
                out->setCargo(Element::Tritanium,  in("tritanium").toInteger());

                out->setOreGround(Element::Neutronium, in("groundneutronium").toInteger());
                out->setOreGround(Element::Molybdenum, in("groundmolybdenum").toInteger());
                out->setOreGround(Element::Duranium,   in("groundduranium").toInteger());
                out->setOreGround(Element::Tritanium,  in("groundtritanium").toInteger());
                out->setOreDensity(Element::Neutronium, in("densityneutronium").toInteger());
                out->setOreDensity(Element::Molybdenum, in("densitymolybdenum").toInteger());
                out->setOreDensity(Element::Duranium,   in("densityduranium").toInteger());
                out->setOreDensity(Element::Tritanium,  in("densitytritanium").toInteger());
                out->setTemperature(in("temp").toInteger());

                out->setOwner(owner);

                out->setCargo(Element::Colonists, in("clans").toInteger());
                out->setColonistTax(in("colonisttaxrate").toInteger());
                out->setColonistHappiness(in("colonisthappypoints").toInteger());
                out->setNatives(in("nativeclans").toInteger());
                out->setNativeGovernment(in("nativegovernment").toInteger());
                out->setNativeRace(in("nativetype").toInteger());
                out->setNativeTax(in("nativetaxrate").toInteger());
                out->setNativeHappiness(in("nativehappypoints").toInteger());

                // FIXME: TODO:
                //   builtdefense       -- undo
                //   builtfactories     -- undo
                //   builtmines         -- undo
                //   flag               -- homeworld flag, could relax tech limits?
                //   nativetaxvalue     -- hull #106 special effect
                //   readystatus        -- sync with selection?
                //   suppliessold       -- undo
                //   totalduranium      -- history
                //   totalmolybdenum    -- history
                //   totalneutronium    -- history
                //   totaltritanium     -- history

                // Consciously ignored (computed internally)
                //   colchange
                //   colhappychange
                //   img
                //   nativechange
                //   nativehappychange
                //   nativeracename
                //   nativegovernmentname

                // Unknown:
                //   burrowsize
                //   checkduranium
                //   checkmegacredits
                //   checkmolybdenum
                //   checkneutronium
                //   checksupplies
                //   checktritanium
                //   debrisdisk
                //   developmentlevel
                //   infoturn
                //   larva
                //   larvaturns
                //   podcargo
                //   podhullid
                //   podspeed
                //   targetdefense
                //   targetfactories
                //   targetmines
                //   targetx
                //   targety
            }
        }
    }

    void loadStarbases(Universe& univ, Access bases, PlayerSet_t players, LogListener& log, Translator& tx)
    {
        const size_t n = bases.getArraySize();
        log.write(LogListener::Debug, LOG_NAME, Format(tx("Loading %d starbase%!1{s%}..."), n));
        for (size_t i = 0; i < n; ++i) {
            Access in = bases[i];

            // Get planet. This does not create planet objects!
            int id = in("planetid").toInteger();
            Planet* out = univ.planets().get(id);
            if (!out) {
                throw afl::except::InvalidDataException(Format(tx("Invalid planet Id #%d"), id));
            }

            int owner;
            if (out->getOwner().get(owner) && players.contains(owner)) {
                // It is an own base
                out->addBaseSource(players);
                out->setNumBuildings(game::BaseDefenseBuilding, in("defense").toInteger());
                out->setBaseDamage(in("damage").toInteger());
                out->setBaseTechLevel(game::EngineTech,  in("enginetechlevel").toInteger());
                out->setBaseTechLevel(game::HullTech,    in("hulltechlevel").toInteger());
                out->setBaseTechLevel(game::BeamTech,    in("beamtechlevel").toInteger());
                out->setBaseTechLevel(game::TorpedoTech, in("torptechlevel").toInteger());
                out->setCargo(Element::Fighters, in("fighters").toInteger());
                out->setBaseShipyardOrder(in("shipmission").toInteger(), in("targetshipid").toInteger());
                out->setBaseMission(in("mission").toInteger());

                // FIXME: TODO
                //   beamtechup            -- undo
                //   buildbeamcount        -- ship build
                //   buildbeamid           -- ship build
                //   buildengineid         -- ship build
                //   buildhullid           -- ship build
                //   buildtorpcount        -- ship build
                //   buildtorpedoid        -- ship build
                //   builtdefense          -- undo
                //   builtfighters         -- undo
                //   enginetechup          -- undo
                //   hulltechup            -- undo
                //   isbuilding            -- ship build
                //   mission1target        -- extra property
                //   readystatus           -- sync with selection?
                //   starbasetype          -- limits storage
                //   torptechup            -- undo

                // Consciously ignored:
                //   raceid                -- unused

                // Unknown:
                //   infoturn
            } else {
                // FIXME: allied base? What to do with these?
            }
        }
    }

    void loadShips(Universe& univ, Access ships, PlayerSet_t players, LogListener& log, Translator& tx)
    {
        const size_t n = ships.getArraySize();
        log.write(LogListener::Debug, LOG_NAME, Format(tx("Loading %d ship%!1{s%}..."), n));
        for (size_t i = 0; i < n; ++i) {
            const Access in = ships[i];
            const int id = in("id").toInteger();
            Ship*const out = univ.ships().create(id);
            if (!out) {
                throw afl::except::InvalidDataException(Format(tx("Invalid ship Id #%d"), id));
            }

            // Main data
            const int owner = in("ownerid").toInteger();
            out->setName(in("name").toString());

            // Set SHIPXY data. This will make the ship visible.
            out->addShipXYData(Point(in("x").toInteger(), in("y").toInteger()),
                               owner,
                               in("mass").toInteger(),
                               players);

            // Hull
            const int hullNr = in("hullid").toInteger();
            if (hullNr > 0) {
                out->setHull(hullNr);
            }

            if (players.contains(owner)) {
                out->addShipSource(players);
                out->setFriendlyCode(in("friendlycode").toString());
                out->setBeamType    (in("beamid").toInteger());
                out->setNumBeams    (in("beams").toInteger());
                out->setNumBays     (in("bays").toInteger());
                out->setTorpedoType (in("torpedoid").toInteger());
                out->setNumLaunchers(in("torps").toInteger());
                out->setEngineType  (in("engineid").toInteger());
                out->setWarpFactor  (in("warp").toInteger());

                // Mission: differences to classic:
                // - Nu packs the Tow target in mission1target, so we swap for that mission (and only that).
                // - Missions are off-by-one.
                // Extra missions are different.
                //   Mission           mission1target goes in
                //     6 "Tow"           tow [ship id here]
                //     7 "Intercept"     intercept [ship id]
                //   [12 "Send fighters" intercept [ship id if [-999,+999], planet id otherwise] -- base mission]
                //    15 "Repair ship"   intercept [ship id here]
                //    18 "Send fighters" intercept [ship id if [-999,+999], 0=all, planet id otherwise]
                //    20 "Cloak+Int"     intercept [ship id]
                int mission = in("mission").toInteger();
                int arg1 =    in("mission1target").toInteger();
                int arg2 =    in("mission2target").toInteger();
                out->setMission(mission + 1,
                                mission != 6 ? arg1 : arg2,
                                mission == 6 ? arg1 : arg2);

                out->setPrimaryEnemy(in("enemy").toInteger());
                out->setDamage      (in("damage").toInteger());
                out->setCrew        (in("crew").toInteger());
                out->setAmmo        (in("ammo").toInteger());

                out->setCargo(Element::Colonists,  in("clans").toInteger());
                out->setCargo(Element::Neutronium, in("neutronium").toInteger());
                out->setCargo(Element::Tritanium,  in("tritanium").toInteger());
                out->setCargo(Element::Duranium,   in("duranium").toInteger());
                out->setCargo(Element::Molybdenum, in("molybdenum").toInteger());
                out->setCargo(Element::Supplies,   in("supplies").toInteger());
                out->setCargo(Element::Money,      in("megacredits").toInteger());

                out->setWaypoint(Point(in("targetx").toInteger(), in("targety").toInteger()));

                switch (in("transfertargettype").toInteger()) {
                 case 1:
                    unpackTransporter(*out, Ship::UnloadTransporter, in("transfertargetid").toInteger(), in);
                    break;
                 case 2:
                    unpackTransporter(*out, Ship::TransferTransporter, in("transfertargetid").toInteger(), in);
                    break;
                 case 3:
                    unpackTransporter(*out, Ship::UnloadTransporter, 0, in);
                    break;
                }
            } else {
                // Foreign ship
                game::parser::MessageInformation info(game::parser::MessageInformation::Ship, id, in("turn").toInteger());
                addOptionalInteger(info, game::parser::mi_Damage,          in("damage"),   0);
                addOptionalInteger(info, game::parser::mi_ShipCrew,        in("crew"),     0);
                addOptionalInteger(info, game::parser::mi_Heading,         in("heading"),  0);
                addOptionalInteger(info, game::parser::mi_WarpFactor,      in("warp"),     0);
                addOptionalInteger(info, game::parser::mi_ShipEngineType,  in("engineid"), 1);
                if (info.begin() != info.end()) {
                    out->addMessageInformation(info, players);
                }
            }

            // FIXME TODO:
            //   readystatus         -- sync with selection?

            // Unknown:
            //   experience
            //   goal
            //   goaltarget
            //   goaltarget2
            //   infoturn
            //   podcargo
            //   podhullid
            //   turn
            //   turnkilled
            //   waypoints

            // Consciously ignored:
            //   history            -- handled internally
            //   iscloaked          -- handled internally
        }
    }

    void loadMinefields(Universe& univ, Access p, LogListener& log, Translator& tx)
    {
        const size_t n = p.getArraySize();
        log.write(LogListener::Debug, LOG_NAME, Format(tx("Loading %d minefield%!1{s%}..."), n));
        for (size_t i = 0; i < n; ++i) {
            Access in = p[i];

            int id = in("id").toInteger();
            if (Minefield* out = univ.minefields().create(id)) {
                out->addReport(Point(in("x").toInteger(), in("y").toInteger()),
                               in("ownerid").toInteger(),
                               in("isweb").toInteger() ? Minefield::IsWeb : Minefield::IsMine,
                               Minefield::UnitsKnown,
                               in("units").toInteger(),
                               in("infoturn").toInteger(),
                               Minefield::MinefieldScanned);
                // Consciously ignored:
                //   radius       -- use units instead
                //   friendlycode -- handled internally
            } else {
                log.write(LogListener::Warn, LOG_NAME, Format(tx("Invalid minefield Id #%d, minefield has been ignored"), id));
            }
        }
    }

    void loadIonStorms(Universe& univ, Access p, LogListener& log, Translator& tx)
    {
        const size_t n = p.getArraySize();
        log.write(LogListener::Debug, LOG_NAME, Format(tx("Loading %d ion storm%!1{s%}..."), n));
        for (size_t i = 0; i < n; ++i) {
            Access in = p[i];

            int id = in("id").toInteger();
            if (IonStorm* out = univ.ionStorms().create(id)) {
                // Note that Nu ion storms have no names.
                // Nu uses Ids outside the 1..50 range, so we cannot just fill in our canned names.
                out->setPosition(Point(in("x").toInteger(), in("y").toInteger()));
                out->setRadius    (in("radius").toInteger());
                out->setVoltage   (in("voltage").toInteger());
                out->setWarpFactor(in("warp").toInteger());
                out->setHeading   (in("heading").toInteger());
                out->setIsGrowing (in("isgrowing").toInteger());

                // FIXME: unknown:
                //   parentid
            } else {
                log.write(LogListener::Warn, LOG_NAME, Format(tx("Invalid ion storm Id #%d. Storm will be ignored"), id));
            }
        }
    }

    void loadVcrs(Turn& turn, Access p, LogListener& log, Translator& tx)
    {
        afl::base::Ptr<Database> db = new Database();

        const size_t n = p.getArraySize();
        for (size_t i = 0; i < n; ++i) {
            Access in = p[i];

            Battle* b = db->addNewBattle(new Battle(unpackVcrObject(in("left"),  in("leftownerid").toInteger(),  false),
                                                    unpackVcrObject(in("right"), in("rightownerid").toInteger(), in("battletype").toInteger() != 0),
                                                    uint16_t(in("seed").toInteger()),
                                                    0 /* signature, not relevant */
                                             ));
            b->setType(game::vcr::classic::NuHost, 0);
            b->setPosition(Point(in("x").toInteger(), in("y").toInteger()));

            // Ignored fields: turn, id
        }
        if (db->getNumBattles() != 0) {
            log.write(LogListener::Debug, LOG_NAME, Format(tx("Loaded %d combat recording%!1{s%}..."), db->getNumBattles()));
            turn.setBattles(db);
        }
    }
}

game::nu::Loader::Loader(afl::string::Translator& tx, afl::sys::LogListener& log)
    : m_translator(tx),
      m_log(log)
{ }

void
game::nu::Loader::loadShipList(game::spec::ShipList& shipList, Root& root, afl::data::Access in) const
{
    loadAdvantages(shipList, in("rst"));
    loadPlayerAdvantages(shipList, in("rst"));
    loadConfig(root.hostConfiguration(), in("rst"));
    setImplicitConfiguration(root.hostConfiguration(), shipList.advantages());

    loadRaceNames(root, in("rst")("players"), in("rst")("races"), m_log, m_translator);

    loadHulls    (shipList, in("rst")("hulls"),    m_log, m_translator);
    loadBeams    (shipList, in("rst")("beams"),    m_log, m_translator);
    loadTorpedoes(shipList, in("rst")("torpedos"), m_log, m_translator);
    loadEngines  (shipList, in("rst")("engines"),  m_log, m_translator);

    loadDefaultHullAssignments(shipList, in("rst")("players"),   in("rst")("races"));
    loadRaceHullAssignments   (shipList, in("rst")("racehulls"), in("rst")("player")("id").toInteger());

    // Must be after hulls!
    setImplicitHullFunctions(shipList.hulls(), shipList.modifiedHullFunctions(), shipList.advantages());

    // FIXME: process these attributes:
    // HullFunctionAssignmentList& racialAbilities();
    // StandardComponentNameProvider& componentNamer();
    // FriendlyCodeList& friendlyCodes();
    // MissionList& missions();
}

void
game::nu::Loader::loadTurn(Turn& turn, PlayerSet_t playerSet, afl::data::Access in) const
{
    // rst attributes:
    // - settings
    // - game
    // - player
    // - players
    // - scores
    // - maps
    // - planets
    // - ships
    // - ionstorms
    // - nebulas
    // - stars
    // - artifacts
    // - wormholes
    // - starbases
    // - stock
    // - minefields
    // - relations
    // - messages
    // - mymessages
    // - cutscenes
    // - notes
    // - vcrs
    // - races
    // - hulls
    // - racehulls
    // - beams
    // - engines
    // - torpedos
    // - advantages
    // - activebadges
    // - badgechange

    turn.setTurnNumber(in("rst")("game")("turn").toInteger());
    turn.setTimestamp(loadTime(in("rst")("settings")("hostcompleted")));

    loadPlanets   (turn.universe(), in("rst")("planets"),   playerSet, m_log, m_translator);
    loadStarbases (turn.universe(), in("rst")("starbases"), playerSet, m_log, m_translator);
    loadShips     (turn.universe(), in("rst")("ships"),     playerSet, m_log, m_translator);
    loadMinefields(turn.universe(), in("rst")("minefields"),           m_log, m_translator);
    loadIonStorms (turn.universe(), in("rst")("ionstorms"),            m_log, m_translator);
    loadVcrs      (turn,            in("rst")("vcrs"),                 m_log, m_translator);
}

game::Timestamp
game::nu::Loader::loadTime(afl::data::Access a)
{
    // FIXME: this decodes "informaldate" format. Should we detect "formaldate" as well? So far that is only used in activities. -> yes we should!
    // "6/22/2016 7:14:33 AM"
    String_t nuTime = a.toString();
    ConstStringMemory_t mem = afl::string::toMemory(nuTime);

    // Skip initial whitespace, for robustness
    eatChar(mem, ' ');

    // Parse numbers
    int month, day, year, hour, minute, second;
    bool meridian;
    if (eatNumber(mem, month)
        && eatChar(mem, '/') == 1
        && eatNumber(mem, day)
        && eatChar(mem, '/') == 1
        && eatNumber(mem, year)
        && eatChar(mem, ' ') > 0
        && eatNumber(mem, hour)
        && eatChar(mem, ':') == 1
        && eatNumber(mem, minute)
        && eatChar(mem, ':') == 1
        && eatNumber(mem, second)
        && eatChar(mem, ' ') > 0
        && eatMeridian(mem, meridian))
    {
        if (hour == 12) {
            // 12 AM = 0:00, 12 PM = 12:00
            if (!meridian) {
                hour -= 12;
            }
        } else {
            // 5 AM = 5:00, 5 PM = 17:00
            if (meridian) {
                hour += 12;
            }
        }
        return Timestamp(year, month, day, hour, minute, second);
    } else {
        return Timestamp();
    }
}
