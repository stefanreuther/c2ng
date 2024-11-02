/**
  *  \file game/nu/turnloader.cpp
  *  \brief Class game::nu::TurnLoader
  */

#include "game/nu/turnloader.hpp"

#include "afl/base/countof.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/except/invaliddataexception.hpp"
#include "afl/string/format.hpp"
#include "afl/string/string.hpp"
#include "game/game.hpp"
#include "game/map/ionstorm.hpp"
#include "game/map/minefield.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/parser/messageinformation.hpp"
#include "game/parser/messagevalue.hpp"
#include "game/session.hpp"
#include "game/timestamp.hpp"
#include "game/turn.hpp"
#include "game/vcr/classic/battle.hpp"
#include "game/vcr/classic/database.hpp"
#include "game/vcr/object.hpp"

using afl::data::Access;
using afl::string::ConstStringMemory_t;
using afl::string::Format;
using afl::string::Translator;
using afl::sys::LogListener;
using game::Element;
using game::IntegerProperty_t;
using game::PlayerSet_t;
using game::Turn;
using game::config::ConfigurationOption;
using game::map::IonStorm;
using game::map::Minefield;
using game::map::Planet;
using game::map::Point;
using game::map::Ship;
using game::vcr::classic::Battle;
using game::vcr::classic::Database;

namespace {

    const char LOG_NAME[] = "game.nu.turnloader";

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

    void addOptionalInteger(game::parser::MessageInformation& info, game::parser::MessageIntegerIndex ii, Access a, int minValue)
    {
        if (a.getValue() != 0) {
            int v = a.toInteger();
            if (v >= minValue) {
                info.addValue(ii, v);
            }
        }
    }

    game::Timestamp convertTime(String_t nuTime)
    {
        // FIXME: this decodes "informaldate" format. Should we detect "formaldate" as well? So far that is only used in activities. -> yes we should!
        // "6/22/2016 7:14:33 AM"
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
            return game::Timestamp(year, month, day, hour, minute, second);
        } else {
            return game::Timestamp();
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

    void loadPlanets(game::map::Universe& univ, Access planets, PlayerSet_t players, LogListener& log, Translator& tx)
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

    void loadStarbases(game::map::Universe& univ, Access bases, PlayerSet_t players, LogListener& log, Translator& tx)
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

    void loadShips(game::map::Universe& univ, Access ships, PlayerSet_t players, LogListener& log, Translator& tx)
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

    void loadMinefields(game::map::Universe& univ, Access p, LogListener& log, Translator& tx)
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

    void loadIonStorms(game::map::Universe& univ, Access p, LogListener& log, Translator& tx)
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

game::nu::TurnLoader::TurnLoader(afl::base::Ref<GameState> gameState,
                                 util::ProfileDirectory& profile,
                                 afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory)
    : m_gameState(gameState),
      m_profile(profile),
      m_defaultSpecificationDirectory(defaultSpecificationDirectory)
{ }

game::nu::TurnLoader::~TurnLoader()
{ }

game::TurnLoader::PlayerStatusSet_t
game::nu::TurnLoader::getPlayerStatus(int player, String_t& extra, afl::string::Translator& tx) const
{
    PlayerStatusSet_t result;
    Access entry = m_gameState->loadGameListEntryPreAuthenticated();
    if (player == entry("player")("id").toInteger()) {
        result += Available;
        result += Playable;
        result += Primary;
        switch (entry("player")("turnstatus").toInteger()) {
         case 1:
            extra = tx("Turn viewed");
            break;
         case 2:
            extra = tx("Turn submitted");
            break;
         default:
            extra = tx("Result file available");
            break;
        }
    } else {
        extra.clear();
    }
    return result;
}

std::auto_ptr<game::Task_t>
game::nu::TurnLoader::loadCurrentTurn(Turn& turn, Game& game, int player, Root& root, Session& session, std::auto_ptr<StatusTask_t> then)
{
    // FIXME: validate player
    class Task : public Task_t {
     public:
        Task(TurnLoader& parent, Turn& turn, Game& game, int player, Root& root, afl::sys::LogListener& log, Translator& tx, std::auto_ptr<StatusTask_t>& then)
            : m_parent(parent), m_turn(turn), m_game(game), m_player(player), m_root(root), m_log(log), m_translator(tx), m_then(then)
            { }

        virtual void call()
            {
                m_log.write(LogListener::Trace, LOG_NAME, "Task: loadCurrentTurn");
                try {
                    m_parent.doLoadCurrentTurn(m_turn, m_game, m_player, m_root, m_log, m_translator);
                    m_then->call(true);
                }
                catch (std::exception& e) {
                    m_log.write(LogListener::Error, LOG_NAME, String_t(), e);
                    m_then->call(false);
                }
            }
     private:
        TurnLoader& m_parent;
        Turn& m_turn;
        Game& m_game;
        int m_player;
        Root& m_root;
        afl::sys::LogListener& m_log;
        Translator& m_translator;
        std::auto_ptr<StatusTask_t> m_then;
    };
    return m_gameState->login(std::auto_ptr<Task_t>(new Task(*this, turn, game, player, root, session.log(), session.translator(), then)));
}

std::auto_ptr<game::Task_t>
game::nu::TurnLoader::saveCurrentTurn(const Turn& turn, const Game& game, PlayerSet_t players, SaveOptions_t opts, const Root& root, Session& session, std::auto_ptr<StatusTask_t> then)
{
    // FIXME
    (void) turn;
    (void) players;
    (void) opts;
    (void) root;

    game.expressionLists().saveRecentFiles(m_profile, session.log(), session.translator());

    return makeConfirmationTask(true, then);
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
    Access rst(m_gameState->loadResultPreAuthenticated());
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

std::auto_ptr<game::Task_t>
game::nu::TurnLoader::loadHistoryTurn(Turn& turn, Game& game, int player, int turnNumber, Root& root, Session& session, std::auto_ptr<StatusTask_t> then)
{
    (void) turn;
    (void) game;
    (void) player;
    (void) turnNumber;
    (void) root;

    session.log().write(LogListener::Error, LOG_NAME, "!FIXME: loadHistoryTurn not implemented");
    return makeConfirmationTask(false, then);
}

std::auto_ptr<game::Task_t>
game::nu::TurnLoader::saveConfiguration(const Root& root, afl::sys::LogListener& log, afl::string::Translator& tx, std::auto_ptr<Task_t> then)
{
    return defaultSaveConfiguration(root, &m_profile, log, tx, then);
}

String_t
game::nu::TurnLoader::getProperty(Property p)
{
    switch (p) {
     case LocalFileFormatProperty:
     case RemoteFileFormatProperty:
        // igpFileFormatLocal:
        return "Nu";

     case RootDirectoryProperty:
        // igpRootDirectory:
        return m_defaultSpecificationDirectory->getDirectoryName();
    }
    return String_t();
}

void
game::nu::TurnLoader::doLoadCurrentTurn(Turn& turn, Game& game, int player, Root& /*root*/, afl::sys::LogListener& log, afl::string::Translator& tx)
{
    // Load result
    Access rst(m_gameState->loadResultPreAuthenticated());
    if (rst.isNull() || rst("success").toInteger() == 0) {
        throw std::runtime_error(tx("Unable to download result file"));
    }

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

    turn.setTurnNumber(rst("rst")("game")("turn").toInteger());
    turn.setTimestamp(convertTime(rst("rst")("settings")("hostcompleted").toString()));

    // FIXME: loadCurrentDatabases()
    // must create all planets/ships before.

    // Expression lists
    game.expressionLists().loadRecentFiles(m_profile, log, tx);
    game.expressionLists().loadPredefinedFiles(m_profile, *m_defaultSpecificationDirectory, log, tx);

    loadPlanets   (turn.universe(), rst("rst")("planets"), PlayerSet_t(player),   log, tx);
    loadStarbases (turn.universe(), rst("rst")("starbases"), PlayerSet_t(player), log, tx);
    loadShips     (turn.universe(), rst("rst")("ships"), PlayerSet_t(player),     log, tx);
    loadMinefields(turn.universe(), rst("rst")("minefields"),                     log, tx);
    loadIonStorms (turn.universe(), rst("rst")("ionstorms"),                      log, tx);
    loadVcrs      (turn,            rst("rst")("vcrs"),                           log, tx);
}
