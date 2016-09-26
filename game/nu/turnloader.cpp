/**
  *  \file game/nu/turnloader.cpp
  */

#include "game/nu/turnloader.hpp"
#include "game/map/planet.hpp"
#include "game/game.hpp"
#include "game/turn.hpp"
#include "afl/base/countof.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/string/format.hpp"
#include "afl/except/invaliddataexception.hpp"
#include "game/map/ship.hpp"
#include "afl/string/string.hpp"
#include "game/timestamp.hpp"

namespace {

    const char LOG_NAME[] = "game.nu.turnloader";

    /** Check for known planet.
        A planet is known (possibly as unowned) if we have a sensible value in any of its fields.
        There is no explicit flag regarding this fact in the data. */
    bool isKnownPlanet(afl::data::Access p)
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

    size_t eatChar(afl::string::ConstStringMemory_t& mem, const char ch)
    {
        const char* p;
        size_t n = 0;
        while ((p = mem.at(n)) != 0 && *p == ch) {
            ++n;
        }
        mem.split(n);
        return n;
    }

    bool eatNumber(afl::string::ConstStringMemory_t& mem, int& out)
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

    bool eatMeridian(afl::string::ConstStringMemory_t& mem, bool& out)
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

    game::Timestamp convertTime(String_t nuTime)
    {
        // FIXME: this decodes "informaldate" format. Should we detect "formaldate" as well? So far that is only used in activities.
        // "6/22/2016 7:14:33 AM"
        afl::string::ConstStringMemory_t mem = afl::string::toMemory(nuTime);

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
            return game::Timestamp(year, month, day, hour, minute, second);
        } else {
            return game::Timestamp();
        }
    }
}


game::nu::TurnLoader::TurnLoader(afl::base::Ptr<GameState> gameState,
                                 afl::string::Translator& tx,
                                 afl::sys::LogListener& log)
    : m_gameState(gameState),
      m_translator(tx),
      m_log(log)
{ }

game::nu::TurnLoader::~TurnLoader()
{ }

game::TurnLoader::PlayerStatusSet_t
game::nu::TurnLoader::getPlayerStatus(int player, String_t& extra, afl::string::Translator& tx) const
{
    PlayerStatusSet_t result;
    afl::data::Access entry = m_gameState->loadGameListEntry();
    if (player == entry("player")("id").toInteger()) {
        result += Available;
        result += Playable;
        result += Primary;
        switch (entry("player")("turnstatus").toInteger()) {
         case 1:
            extra = tx.translateString("Turn viewed");
            break;
         case 2:
            extra = tx.translateString("Turn submitted");
            break;
         default:
            extra = tx.translateString("Result file available");
            break;
        }
    } else {
        extra.clear();
    }
    return result;
}

void
game::nu::TurnLoader::loadCurrentTurn(Turn& turn, Game& /*game*/, int player, Root& /*root*/)
{
    // FIXME: validate player

    // Load result
    afl::data::Access rst(m_gameState->loadResult());
    if (rst.isNull() || rst("success").toInteger() == 0) {
        throw std::runtime_error(m_translator.translateString("Unable to download result file"));
    }

    turn.universe().setTurnNumber(rst("rst")("game")("turn").toInteger());
    turn.universe().setTimestamp(convertTime(rst("rst")("settings")("hostcompleted").toString()));

    loadPlanets(turn.universe(), rst("rst")("planets"), PlayerSet_t(player));
    loadStarbases(turn.universe(), rst("rst")("starbases"), PlayerSet_t(player));
    loadShips(turn.universe(), rst("rst")("ships"), PlayerSet_t(player));
    loadMinefields(turn.universe(), rst("rst")("minefields"));
}

void
game::nu::TurnLoader::getHistoryStatus(int /*player*/, int turn, afl::base::Memory<HistoryStatus> status, const Root& /*root*/)
{
    /*
     *  Basic idea: be optimistic (WeaklyPositive) that we have a history result for each turn before the current one.
     *  FIXME: when we download a history result, cache it locally (using the regular Backup mechanism we use for v3)
     *  so we can give StronglyPositive answers and avoid network traffic later on.
     */

    // Fetch the result. This should not produce a network access, we already have it.
    afl::data::Access rst(m_gameState->loadResult());
    if (rst.isNull() || rst("success").toInteger() == 0) {
        // Bad result
        status.fill(Negative);
    } else {
        // OK, fill it
        int currentTurn = rst("rst")("game")("turn").toInteger();
        while (HistoryStatus* p = status.eat()) {
            if (turn >= 0 && turn < currentTurn) {
                *p = WeaklyPositive;
            } else {
                *p = Negative;
            }
            ++turn;
        }
    }
}

void
game::nu::TurnLoader::loadHistoryTurn(Turn& turn, Game& game, int player, int turnNumber, Root& root)
{
    (void) turn;
    (void) game;
    (void) player;
    (void) turnNumber;
    (void) root;
    throw std::runtime_error("!not implemented");
}

void
game::nu::TurnLoader::loadPlanets(game::map::Universe& univ, afl::data::Access planets, PlayerSet_t players)
{
    const int n = planets.getArraySize();
    m_log.write(m_log.Debug, LOG_NAME, afl::string::Format(m_translator.translateString("Loading %d planet%!1{s%}...").c_str(), n));
    for (int i = 0; i < n; ++i) {
        afl::data::Access in = planets[i];
        int id = in("id").toInteger();
        game::map::Planet* out = univ.planets().create(id);
        if (!out) {
            throw afl::except::InvalidDataException(afl::string::Format(m_translator.translateString("Invalid planet Id #%d").c_str(), id));
        }

        // Location and Name
        out->setPosition(game::map::Point(in("x").toInteger(), in("y").toInteger()));
        out->setName(in("name").toString());

        // Is this planet known?
        if (isKnownPlanet(in)) {
            int owner = in("ownerid").toInteger();
            if (players.contains(owner)) {
                out->addPlanetSource(players);
            }
            out->setFriendlyCode(in("friendlycode").toString());
            out->setNumBuildings(MineBuilding, in("mines").toInteger());
            out->setNumBuildings(FactoryBuilding, in("factories").toInteger());
            out->setNumBuildings(DefenseBuilding, in("defense").toInteger());
//             "targetmines": 0,
//             "targetfactories": 0,
//             "targetdefense": 0,
//             "builtmines": 0,
//             "builtfactories": 0,
//             "builtdefense": 0,
            out->setBuildBaseFlag(in("buildingstarbase").toInteger() != 0);
            out->setCargo(Element::Money, in("megacredits").toInteger());
            out->setCargo(Element::Supplies, in("supplies").toInteger());
//             "suppliessold": 0,
            out->setCargo(Element::Neutronium, in("neutronium").toInteger());
            out->setCargo(Element::Molybdenum, in("molybdenum").toInteger());
            out->setCargo(Element::Duranium, in("duranium").toInteger());
            out->setCargo(Element::Tritanium, in("tritanium").toInteger());

            out->setOreGround(Element::Neutronium, in("groundneutronium").toInteger());
            out->setOreGround(Element::Molybdenum, in("groundmolybdenum").toInteger());
            out->setOreGround(Element::Duranium,   in("groundduranium").toInteger());
            out->setOreGround(Element::Tritanium,  in("groundtritanium").toInteger());
            out->setOreDensity(Element::Neutronium, in("densityneutronium").toInteger());
            out->setOreDensity(Element::Molybdenum, in("densitymolybdenum").toInteger());
            out->setOreDensity(Element::Duranium,   in("densityduranium").toInteger());
            out->setOreDensity(Element::Tritanium,  in("densitytritanium").toInteger());
//             "totalneutronium": 0,
//             "totalmolybdenum": 0,
//             "totalduranium": 0,
//             "totaltritanium": 0,
//             "checkneutronium": 221,
//             "checkmolybdenum": 625,
//             "checkduranium": 3091,
//             "checktritanium": 1010,
//             "checkmegacredits": 34506,
//             "checksupplies": 1661,
            out->setTemperature(in("temp").toInteger());

            out->setOwner(owner);

            out->setCargo(Element::Colonists, in("clans").toInteger());
//             "colchange": 0,
            out->setColonistTax(in("colonisttaxrate").toInteger());
            out->setColonistHappiness(in("colonisthappypoints").toInteger());
//             "colhappychange": 8,
            out->setNatives(in("nativeclans").toInteger());
//             "nativechange": 0,
            out->setNativeGovernment(in("nativegovernment").toInteger());
//             "nativetaxvalue": 100, <--- FIXME: goes in tax computation?
            out->setNativeRace(in("nativetype").toInteger());
            out->setNativeTax(in("nativetaxrate").toInteger());
            out->setNativeHappiness(in("nativehappypoints").toInteger());
//             "nativehappychange": 3,
//             "infoturn": 77,
//             "debrisdisk": 0,
//             "flag": 2,
//             "readystatus": 0,
//             "targetx": 2894,
//             "targety": 2325,
//             "podhullid": 0,
//             "podspeed": 0,
//             "podcargo": 0,
//             "larva": 0,
//             "larvaturns": 0,
//             "img": "http:\/\/library.vgaplanets.nu\/planets\/46.png",
//             "nativeracename": "Insectoid",
//             "nativegovernmentname": "Feudal",
        }
    }
}

void
game::nu::TurnLoader::loadStarbases(game::map::Universe& univ, afl::data::Access bases, PlayerSet_t players)
{
    using game::map::Planet;

    const int n = bases.getArraySize();
    m_log.write(m_log.Debug, LOG_NAME, afl::string::Format(m_translator.translateString("Loading %d starbase%!1{s%}...").c_str(), n));
    for (int i = 0; i < n; ++i) {
        afl::data::Access in = bases[i];

        // Get planet. This does not create planet objects!
        int id = in("planetid").toInteger();
        Planet* out = univ.planets().get(id);
        if (!out) {
            throw afl::except::InvalidDataException(afl::string::Format(m_translator.translateString("Invalid planet Id #%d").c_str(), id));
        }

        int owner;
        if (out->getOwner(owner) && players.contains(owner)) {
            // It is an own base
            out->addBaseSource(players);
            out->setNumBuildings(BaseDefenseBuilding, in("defense").toInteger());
            out->setBaseDamage(in("damage").toInteger());
            out->setBaseTechLevel(Planet::EngineTech,  in("enginetechlevel").toInteger());
            out->setBaseTechLevel(Planet::HullTech,    in("hulltechlevel").toInteger());
            out->setBaseTechLevel(Planet::BeamTech,    in("beamtechlevel").toInteger());
            out->setBaseTechLevel(Planet::TorpedoTech, in("torptechlevel").toInteger());
            out->setCargo(Element::Fighters, in("fighters").toInteger());
            out->setBaseShipyardOrder(in("shipmission").toInteger(), in("targetshipid").toInteger());
            out->setBaseMission(in("mission").toInteger()); // FIXME: mission1target!

            // "builtdefense": 0,
            // "hulltechup": 0,
            // "enginetechup": 0,
            // "beamtechup": 0,
            // "torptechup": 0,
            // "builtfighters": 0,
            // "mission1target": 0,
            // "raceid": 0, <- unused?

            // "buildbeamid": 0,
            // "buildengineid": 0,
            // "buildtorpedoid": 0,
            // "buildhullid": 0,
            // "buildbeamcount": 0,
            // "buildtorpcount": 0,
            // "isbuilding": false,
            // "starbasetype": 0,
            // "infoturn": 31,
            // "readystatus": 0,
        } else {
            // FIXME: allied base? What to do with these?
        }
    }
}

void
game::nu::TurnLoader::loadShips(game::map::Universe& univ, afl::data::Access ships, PlayerSet_t players)
{
    const int n = ships.getArraySize();
    m_log.write(m_log.Debug, LOG_NAME, afl::string::Format(m_translator.translateString("Loading %d ship%!1{s%}...").c_str(), n));
    for (int i = 0; i < n; ++i) {
        afl::data::Access in = ships[i];
        int id = in("id").toInteger();
        game::map::Ship* out = univ.ships().create(id);
        if (!out) {
            throw afl::except::InvalidDataException(afl::string::Format(m_translator.translateString("Invalid ship Id #%d").c_str(), id));
        }

        int owner = in("ownerid").toInteger();
        out->setName(in("name").toString());
        out->setWarpFactor(in("warp").toInteger());

        // Set SHIPXY data. This will make the ship visible.
        out->addShipXYData(game::map::Point(in("x").toInteger(), in("y").toInteger()),
                           owner,
                           in("mass").toInteger(),
                           players);
                           
        out->setHull(in("hullid").toInteger());

        if (players.contains(owner)) {
            out->addShipSource(players);
            out->setFriendlyCode(in("friendlycode").toString());
            out->setBeamType(in("beamid").toInteger());
            out->setNumBeams(in("beams").toInteger());
            out->setNumBays(in("bays").toInteger());
            out->setTorpedoType(in("torpedoid").toInteger());
            out->setNumLaunchers(in("torps").toInteger());
            out->setEngineType(in("engineid").toInteger());

            // Mission: we have mission1target and mission2target, but the latter is not used.
            //   Mission           mission1target goes in
            //     6 "Tow"           tow [ship id here]
            //     7 "Intercept"     intercept [ship id]
            //   [12 "Send fighters" intercept [ship id if [-999,+999], planet id otherwise] -- base mission]
            //    15 "Repair ship"   intercept [ship id here]
            //    18 "Send fighters" intercept [ship id if [-999,+999], 0=all, planet id otherwise]
            //    20 "Cloak+Int"     intercept [ship id]
            // Nu missions are off-by-one.
            int mission = in("mission").toInteger();
            int arg = in("mission1target").toInteger();
            out->setMission(mission + 1,
                            mission != 6 ? arg : 0,
                            mission == 6 ? arg : 0);

            out->setPrimaryEnemy(in("enemy").toInteger());
            out->setDamage(in("damage").toInteger());
            out->setCrew(in("crew").toInteger());
            out->setCargo(Element::Colonists, in("clans").toInteger());
            out->setCargo(Element::Neutronium, in("neutronium").toInteger());
            out->setCargo(Element::Tritanium, in("tritanium").toInteger());
            out->setCargo(Element::Duranium, in("duranium").toInteger());
            out->setCargo(Element::Molybdenum, in("molybdenum").toInteger());
            out->setCargo(Element::Supplies, in("supplies").toInteger());
            out->setAmmo(in("ammo").toInteger());
            out->setCargo(Element::Money, in("megacredits").toInteger());
            out->setWaypoint(game::map::Point(in("targetx").toInteger(), in("targety").toInteger()));
        } else {
            // FIXME: can we report a subset?
        }

        // Not handled:
        //     "mission2target": 0,

        //     "transferclans": 0,
        //     "transferneutronium": 0,
        //     "transferduranium": 0,
        //     "transfertritanium": 0,
        //     "transfermolybdenum": 0,
        //     "transfersupplies": 0,
        //     "transferammo": 0,
        //     "transfermegacredits": 0,
        //     "transfertargetid": 0,
        //     "transfertargettype": 0,

        //     "heading": -1,
        //     "turn": 0,
        //     "turnkilled": 0,
        //     "experience": 0,
        //     "infoturn": 83,
        //     "podhullid": 0,
        //     "podcargo": 0,
        //     "goal": 0,
        //     "goaltarget": 0,
        //     "goaltarget2": 0,
        //     "waypoints": [],
        //     "history": [],
        //     "iscloaked": false,
        //     "readystatus": 0,
    }
}


void
game::nu::TurnLoader::loadMinefields(game::map::Universe& univ, afl::data::Access p)
{
    using game::map::Minefield;

    const int n = p.getArraySize();
    m_log.write(m_log.Debug, LOG_NAME, afl::string::Format(m_translator.translateString("Loading %d minefield%!1{s%}...").c_str(), n));
    for (int i = 0; i < n; ++i) {
        afl::data::Access in = p[i];

        int id = in("id").toInteger();
        if (Minefield* out = univ.minefields().create(id)) {
            out->addReport(game::map::Point(in("x").toInteger(), in("y").toInteger()),
                           in("ownerid").toInteger(),
                           in("isweb").toInteger() ? Minefield::IsWeb : Minefield::IsMine,
                           Minefield::UnitsKnown,
                           in("units").toInteger(),
                           in("infoturn").toInteger(),
                           Minefield::MinefieldScanned);
            // FIXME: should we report old (infoturn < current turn) reports here?
            // FIXME: should we validate the .radius attribute against the .units?
            // FIXME: unhandled attribute: "friendlycode": "652"
        } else {
            m_log.write(m_log.Error, LOG_NAME, afl::string::Format(m_translator.translateString("Invalid minefield Id #%d, minefield has been ignored").c_str(), id));
        }
    }
}
