/**
  *  \file game/nu/specificationloader.cpp
  */

#include "game/nu/specificationloader.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/beam.hpp"
#include "game/spec/torpedolauncher.hpp"
#include "game/spec/engine.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "game/root.hpp"

namespace gs = game::spec;

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
}

game::nu::SpecificationLoader::SpecificationLoader(afl::base::Ref<GameState> gameState,
                                                   afl::string::Translator& tx,
                                                   afl::sys::LogListener& log)
    : m_gameState(gameState),
      m_translator(tx),
      m_log(log)
{ }

game::nu::SpecificationLoader::~SpecificationLoader()
{ }

void
game::nu::SpecificationLoader::loadShipList(game::spec::ShipList& list, Root& root)
{
    afl::data::Access rst(m_gameState->loadResultPreAuthenticated());

    loadRaceNames(root, rst("rst")("players"), rst("rst")("races"));

    loadHulls(list, rst("rst")("hulls"));
    loadBeams(list, rst("rst")("beams"));
    loadTorpedoes(list, rst("rst")("torpedos"));
    loadEngines(list, rst("rst")("engines"));

    loadDefaultHullAssignments(list, rst("rst")("players"), rst("rst")("races"));
    loadRaceHullAssignments(list, rst("rst")("racehulls"), rst("rst")("player")("id").toInteger());

    // FIXME: process these attributes:
    // BasicHullFunctionList& basicHullFunctions();
    // ModifiedHullFunctionList& modifiedHullFunctions();
    // HullFunctionAssignmentList& racialAbilities();
    // StandardComponentNameProvider& componentNamer();
    // FriendlyCodeList& friendlyCodes();
    // MissionList& missions();
}

void
game::nu::SpecificationLoader::loadHulls(game::spec::ShipList& list, afl::data::Access p)
{
    for (size_t i = 0, n = p.getArraySize(); i < n; ++i) {
        afl::data::Access in = p[i];
        int nr = in("id").toInteger();
        if (nr <= 0 || nr > MAX_HULLS) {
            m_log.write(m_log.Warn, LOG_NAME, afl::string::Format(m_translator.translateString("Invalid hull number %d, component has been ignored").c_str(), nr));
        } else if (gs::Hull* out = list.hulls().create(nr)) {
            // Component:
            out->setMass(in("mass").toInteger());
            out->setTechLevel(in("techlevel").toInteger());
            out->cost().set(gs::Cost::Money, in("cost").toInteger());
            out->cost().set(gs::Cost::Tritanium, in("tritanium").toInteger());
            out->cost().set(gs::Cost::Duranium, in("duranium").toInteger());
            out->cost().set(gs::Cost::Molybdenum, in("molybdenum").toInteger());
            out->setName(in("name").toString());

            // Hull:
            out->setExternalPictureNumber(1); // FIXME!
            out->setInternalPictureNumber(1); // FIXME!
            out->setMaxFuel(in("fueltank").toInteger());
            out->setMaxCrew(in("crew").toInteger());
            out->setNumEngines(in("engines").toInteger());
            out->setMaxCargo(in("cargo").toInteger());
            out->setNumBays(in("fighterbays").toInteger());
            out->setMaxLaunchers(in("launchers").toInteger());
            out->setMaxBeams(in("beams").toInteger());

            // FIXME: process these attributes:
            //             "cancloak":false,

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
            m_log.write(m_log.Warn, LOG_NAME, afl::string::Format(m_translator.translateString("Invalid hull number %d, component has been ignored").c_str(), nr));
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
            m_log.write(m_log.Warn, LOG_NAME, afl::string::Format(m_translator.translateString("Invalid beam number %d, component has been ignored").c_str(), nr));
        } else if (gs::Beam* out = list.beams().create(nr)) {
            // Component:
            out->setMass(in("mass").toInteger());
            out->setTechLevel(in("techlevel").toInteger());
            out->cost().set(gs::Cost::Money, in("cost").toInteger());
            out->cost().set(gs::Cost::Tritanium, in("tritanium").toInteger());
            out->cost().set(gs::Cost::Duranium, in("duranium").toInteger());
            out->cost().set(gs::Cost::Molybdenum, in("molybdenum").toInteger());
            out->setName(in("name").toString());

            // Weapon:
            out->setKillPower(in("crewkill").toInteger());
            out->setDamagePower(in("damage").toInteger());
        } else {
            m_log.write(m_log.Warn, LOG_NAME, afl::string::Format(m_translator.translateString("Invalid beam number %d, component has been ignored").c_str(), nr));
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
            m_log.write(m_log.Warn, LOG_NAME, afl::string::Format(m_translator.translateString("Invalid torpedo number %d, component has been ignored").c_str(), nr));
        } else if (gs::TorpedoLauncher* out = list.launchers().create(nr)) {
            // Component:
            out->setMass(in("mass").toInteger());
            out->setTechLevel(in("techlevel").toInteger());
            out->cost().set(gs::Cost::Money, in("launchercost").toInteger());
            out->cost().set(gs::Cost::Tritanium, in("tritanium").toInteger());
            out->cost().set(gs::Cost::Duranium, in("duranium").toInteger());
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
        } else {
            m_log.write(m_log.Warn, LOG_NAME, afl::string::Format(m_translator.translateString("Invalid torpedo number %d, component has been ignored").c_str(), nr));
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
            m_log.write(m_log.Warn, LOG_NAME, afl::string::Format(m_translator.translateString("Invalid engine number %d, component has been ignored").c_str(), nr));
        } else if (gs::Engine* out = list.engines().create(nr)) {
            // Component:
            out->setMass(0);
            out->setTechLevel(in("techlevel").toInteger());
            out->cost().set(gs::Cost::Money, in("cost").toInteger());
            out->cost().set(gs::Cost::Tritanium, in("tritanium").toInteger());
            out->cost().set(gs::Cost::Duranium, in("duranium").toInteger());
            out->cost().set(gs::Cost::Molybdenum, in("molybdenum").toInteger());
            out->setName(in("name").toString());

            // Engine
            for (int i = 1; i <= 9; ++i) {
                out->setFuelFactor(i, in(afl::string::Format("warp%d", i)).toInteger());
            }
        } else {
            m_log.write(m_log.Warn, LOG_NAME, afl::string::Format(m_translator.translateString("Invalid engine number %d, component has been ignored").c_str(), nr));
        }
    }
}

void
game::nu::SpecificationLoader::loadDefaultHullAssignments(game::spec::ShipList& list, afl::data::Access players, afl::data::Access races)
{
    for (size_t player = 0, nplayers = players.getArraySize(); player < nplayers; ++player) {
        // Get raceid
        afl::data::Access p = players[player];
        int raceId = p("raceid").toInteger();
        int playerId = p("id").toInteger();

        // Get associated race
        afl::data::Access r;
        for (size_t race = 0, nraces = races.getArraySize(); race < nraces; ++race) {
            if (races[race]("id").toInteger() == raceId) {
                r = races[race];
                break;
            }
        }

        // Get base hulls which are cleverly encoded as a string
        String_t hullsAsString = r("basehulls").toString();
        std::vector<int> hulls;
        do {
            int n;
            if (afl::string::strToInteger(afl::string::strFirst(hullsAsString, ","), n)) {
                hulls.push_back(n);
            }
        } while (afl::string::strRemove(hullsAsString, ","));

        // Sort into sensible order (for users; not required for turn file validity)
        std::sort(hulls.begin(), hulls.end(), CompareHulls(list.hulls()));

        // Populate one entry
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
            afl::data::Access race;
            int raceNr = in("raceid").toInteger();
            for (size_t j = 0, n = races.getArraySize(); j < n; ++j) {
                if (races[j]("id").toInteger() == raceNr) {
                    race = races[j];
                    break;
                }
            }
            if (!race.isNull()) {
                out->setName(Player::LongName, race("name").toString());
                out->setName(Player::ShortName, race("shortname").toString());
                out->setName(Player::AdjectiveName, race("adjective").toString());
                out->setOriginalNames();
            }

            // Other names
            out->setName(Player::UserName, in("username").toString());
            out->setName(Player::EmailAddress, in("email").toString());

            // Other attributes:
            // "status":1,
            // "statusturn":0,
            // "accountid":860,
            // "teamid":0,
            // "prioritypoints":0,
            // "joinrank":0,
            // "finishrank":0,
            // "turnjoined":1,
            // "turnready":false,
            // "turnreadydate":"",
            // "turnstatus":1,
            // "turnsmissed":0,
            // "turnsmissedtotal":0,
            // "turnsholiday":0,
            // "turnsearly":0,
            // "turn":1,
            // "timcontinuum":0,
            // "activehulls":"",
            // "activeadvantages":"",
            // "savekey":"",
            // "tutorialid":1,
            // "tutorialtaskid":0,
            // "megacredits":0,
            // "duranium":0,
            // "tritanium":0,
            // "molybdenum":0,
            // "id":1
        } else {
            m_log.write(m_log.Warn, LOG_NAME, afl::string::Format(m_translator.translateString("Invalid player number %d, entry has been ignored").c_str(), nr));
        }
    }
    root.playerList().notifyListeners();
}
