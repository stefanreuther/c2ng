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
#include "game/spec/beam.hpp"
#include "game/spec/engine.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/torpedolauncher.hpp"
#include "util/io.hpp"

namespace gs = game::spec;

using afl::data::Access;
using afl::string::Format;
using afl::sys::LogListener;
using game::config::ConfigurationOption;
using game::config::HostConfiguration;

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

        // Map all Nu settings under their original names
        addAllOptions(config, game, "nu.game.");
        addAllOptions(config, settings, "nu.");

        // Mark everything as sourced in game
        config.setAllOptionsSource(ConfigurationOption::Game);
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

                    loadConfig(m_root.hostConfiguration(), rst("rst"));

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
