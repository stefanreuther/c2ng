/**
  *  \file game/v3/loader.cpp
  *  \brief Class game::v3::Loader
  */

#include "game/v3/loader.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/string/format.hpp"
#include "game/alliance/hosthandler.hpp"
#include "game/alliance/phosthandler.hpp"
#include "game/game.hpp"
#include "game/map/basedata.hpp"
#include "game/map/ionstorm.hpp"
#include "game/map/minefield.hpp"
#include "game/map/minefieldtype.hpp"
#include "game/map/planet.hpp"
#include "game/map/planetdata.hpp"
#include "game/map/ship.hpp"
#include "game/map/ufotype.hpp"
#include "game/parser/messageinformation.hpp"
#include "game/v3/commandextra.hpp"
#include "game/v3/genfile.hpp"
#include "game/v3/inboxfile.hpp"
#include "game/v3/packer.hpp"
#include "game/v3/resultfile.hpp"
#include "game/v3/reverter.hpp"
#include "game/v3/structures.hpp"
#include "game/v3/utils.hpp"
#include "game/vcr/classic/database.hpp"
#include "game/vcr/flak/database.hpp"

using afl::base::Ref;
using afl::except::FileFormatException;
using afl::io::Stream;
using afl::string::Format;

namespace {
    const char LOG_NAME[] = "game.v3.loader";

    /** Check for dummy name.
        PHost can filter out ship names; we detect such names to avoid overwriting a known name by a dummy.
        \param name [in] Name, 20 bytes
        \param ship_id [in] Ship Id
        \return true iff it is a dummy name
        \todo maybe recognize other client's dummy names? */
    bool isDummyName(const String_t& name, int shipId)
    {
        // ex ccmain.pas:IsStubName
        return name == String_t(afl::string::Format("Ship %d", shipId));
    }

    struct KoreTargetHeader {
        char sig[4];
        game::v3::structures::UInt32_t num;
    };

    Stream::FileSize_t getShipXYSize(const game::v3::ResultFile& result)
    {
        Stream::FileSize_t start, end;
        if (result.getSectionOffset(result.ShipXYSection, start) && result.getSectionOffset(result.GenSection, end) && end > start) {
            return end - start;
        } else {
            return 0;
        }
    }
}

// Constructor.
game::v3::Loader::Loader(afl::charset::Charset& charset, afl::string::Translator& tx, afl::sys::LogListener& log)
    : m_charset(charset),
      m_translator(tx),
      m_log(log)
{ }

// Prepare universe.
void
game::v3::Loader::prepareUniverse(game::map::Universe& univ) const
{
    for (int i = 1; i <= structures::NUM_SHIPS; ++i) {
        univ.ships().create(i);
    }
    for (int i = 1; i <= structures::NUM_PLANETS; ++i) {
        univ.planets().create(i);
    }
    for (int i = 1; i <= structures::NUM_ION_STORMS; ++i) {
        univ.ionStorms().create(i);
    }
}

// Prepare turn.
void
game::v3::Loader::prepareTurn(Turn& turn, const Root& root, Session& session, int player) const
{
    // ex GGameTurn::postprocess (part)
    // FIXME: design problem? We have one reverter, one set of alliances.
    // This needs revision if we want to load multiple turns into one instance.
    // FIXME: merge with prepareUniverse()?

    // Reverter
    turn.universe().setNewReverter(new Reverter(turn, session));

    // Create CommandExtra. This allows further code to deal with PHost commands.
    CommandExtra::create(turn);

    // Alliances
    if (root.hostVersion().isPHost()) {
        turn.alliances().addNewHandler(new game::alliance::PHostHandler(root.hostVersion().getVersion(), turn, session, player), session.translator());
    } else {
        turn.alliances().addNewHandler(new game::alliance::HostHandler(root.hostVersion().getVersion(), turn, player), session.translator());
    }
}

// Load planets.
void
game::v3::Loader::loadPlanets(game::map::Universe& univ, afl::io::Stream& file, int count, LoadMode mode, PlayerSet_t source) const
{
    // ex game/load.cc:loadPlanets
    m_log.write(m_log.Debug, LOG_NAME, afl::string::Format(m_translator.translateString("Loading %d planet%!1{s%}...").c_str(), count));
    Reverter* pReverter = dynamic_cast<Reverter*>(univ.getReverter());
    while (count > 0) {
        structures::Planet rawPlanet;
        file.fullRead(afl::base::fromObject(rawPlanet));

        const int planetId = rawPlanet.planetId;
        map::Planet* p = univ.planets().get(planetId);
        if (!p) {
            throw afl::except::FileFormatException(file, afl::string::Format(m_translator.translateString("Invalid planet Id #%d").c_str(), planetId));
        }

        // Unpack the planet
        game::map::PlanetData planetData;
        Packer(m_charset).unpackPlanet(planetData, rawPlanet);
        if (mode != LoadPrevious) {
            p->addCurrentPlanetData(planetData, source);
        }
        if (mode != LoadCurrent) {
            if (pReverter != 0) {
                pReverter->addPlanetData(planetId, planetData);
            }
        }
        --count;
    }
}

// Load planet coordinate.
void
game::v3::Loader::loadPlanetCoordinates(game::map::Universe& univ, afl::io::Stream& file) const
{
    // ex game/load.h:loadPlanetXY
    m_log.write(m_log.Debug, LOG_NAME, afl::string::Format(m_translator.translateString("Loading up to %d planet position%!1{s%}...").c_str(), structures::NUM_PLANETS));
    structures::Int16_t data[structures::NUM_PLANETS * 3];
    file.fullRead(afl::base::fromObject(data));
    for (int planetId = 1; planetId <= structures::NUM_PLANETS; ++planetId) {
        // FIXME: PCC2 checked chart config here.
        // pro: coordinate filtering is a v3 thing, and should be done in v3 code
        // con: doing the filtering in game::map::Planet::internalCheck only allows live map-reconfiguration to recover from errors
        game::map::Point pt(data[3*planetId-3], data[3*planetId-2]);
        game::map::Planet* p = univ.planets().get(planetId);
        if (!p) {
            throw afl::except::FileFormatException(file, afl::string::Format(m_translator.translateString("Invalid planet Id #%d").c_str(), planetId));
        }
        p->setPosition(pt);
    }
}

// Load planet names.
void
game::v3::Loader::loadPlanetNames(game::map::Universe& univ, afl::io::Stream& file) const
{
    // ex game/load.h:loadPlanetNames
    m_log.write(m_log.Debug, LOG_NAME, afl::string::Format(m_translator.translateString("Loading %d planet name%!1{s%}...").c_str(), structures::NUM_PLANETS));
    structures::String20_t data[structures::NUM_PLANETS];
    file.fullRead(afl::base::fromObject(data));
    for (int planetId = 1; planetId <= structures::NUM_PLANETS; ++planetId) {
        game::map::Planet* p = univ.planets().get(planetId);
        if (!p) {
            throw afl::except::FileFormatException(file, afl::string::Format(m_translator.translateString("Invalid planet Id #%d").c_str(), planetId));
        }
        p->setName(m_charset.decode(data[planetId-1]));
    }
}

// Load Ion Storm Names.
void
game::v3::Loader::loadIonStormNames(game::map::Universe& univ, afl::io::Stream& file) const
{
    // ex game/load.h:loadStormNames
    m_log.write(m_log.Debug, LOG_NAME, afl::string::Format(m_translator.translateString("Loading %d ion storm name%!1{s%}...").c_str(), structures::NUM_ION_STORMS));
    structures::String20_t data[structures::NUM_ION_STORMS];
    file.fullRead(afl::base::fromObject(data));
    for (int stormId = 1; stormId <= structures::NUM_ION_STORMS; ++stormId) {
        game::map::IonStorm* p = univ.ionStorms().get(stormId);
        if (!p) {
            throw afl::except::FileFormatException(file, afl::string::Format(m_translator.translateString("Invalid ion storm Id #%d").c_str(), stormId));
        }
        p->setName(m_charset.decode(data[stormId-1]));
    }
}

// Load starbases.
void
game::v3::Loader::loadBases(game::map::Universe& univ, afl::io::Stream& file, int count, LoadMode mode, PlayerSet_t source) const
{
    // ex game/load.h:loadBases
    m_log.write(m_log.Debug, LOG_NAME, afl::string::Format(m_translator.translateString("Loading %d starbase%!1{s%}...").c_str(), count));
    Reverter* pReverter = dynamic_cast<Reverter*>(univ.getReverter());
    while (count > 0) {
        structures::Base rawBase;
        file.fullRead(afl::base::fromObject(rawBase));

        const int baseId = rawBase.baseId;
        map::Planet* p = univ.planets().get(baseId);
        if (!p) {
            throw afl::except::FileFormatException(file, afl::string::Format(m_translator.translateString("Invalid starbase Id #%d").c_str(), baseId));
        }

        // Unpack the base
        game::map::BaseData baseData;
        Packer(m_charset).unpackBase(baseData, rawBase);

        if (mode != LoadPrevious) {
            p->addCurrentBaseData(baseData, source);
        }
        if (mode != LoadCurrent) {
            if (pReverter != 0) {
                pReverter->addBaseData(baseId, baseData);
            }
        }
        --count;
    }
}

void
game::v3::Loader::loadShipXY(game::map::Universe& univ, afl::io::Stream& file, afl::io::Stream::FileSize_t bytes, LoadMode /*mode*/, PlayerSet_t source, PlayerSet_t reject) const
{
    // ex game/load.cc:loadShipXY, ccmain.pas:LoadShipXY

    // Compute size of file
    static_assert(structures::NUM_SHIPS == 999, "NUM_SHIPS");
    size_t numShips = (bytes != 0 && bytes >= 999 * sizeof(structures::ShipXY)) ? 999 : 500;
    m_log.write(m_log.Debug, LOG_NAME, afl::string::Format(m_translator.translateString("Loading up to %d ship position%!1{s%}...").c_str(), numShips));

    // Read file in chunks
    const size_t CHUNK_SIZE = 100;
    Id_t id = 0;
    while (numShips > 0) {
        structures::ShipXY buffer[CHUNK_SIZE];
        size_t now = std::min(numShips, CHUNK_SIZE);
        file.fullRead(afl::base::fromObject(buffer).trim(now * sizeof(buffer[0])));
        for (size_t i = 0; i < now; ++i) {
            ++id;

            /* Detect bogus files made by Winplan999/Unpack999 when used with Host500.
               The SHIPXY file continues with a (mangled) copy of GENx.DAT which results in unlikely high coordinates.
               Only test for ship #501, to keep the risk of false positives low
               (if someone actually goes that far -- it's not forbidden after all).
               Stupid "solution" for stupid problem. */
            int x = buffer[i].x;
            int y = buffer[i].y;
            int owner = buffer[i].owner;
            int mass = buffer[i].mass;
            if (id == 501 && (x < 0 || x >= 0x3030 || owner >= 0x2020)) {
                return;
            }

            if (owner > 0 && owner <= structures::NUM_OWNERS && !reject.contains(owner)) {
                if (game::map::Ship* ship = univ.ships().get(id)) {
                    ship->addShipXYData(game::map::Point(x, y), owner, mass, source);
                }
            }
        }
        numShips -= now;
    }
}

void
game::v3::Loader::loadShips(game::map::Universe& univ, afl::io::Stream& file, int count, LoadMode mode, bool remapExplore, PlayerSet_t source) const
{
    m_log.write(m_log.Debug, LOG_NAME, afl::string::Format(m_translator.translateString("Loading %d ship%!1{s%}...").c_str(), count));
    Reverter* pReverter = dynamic_cast<Reverter*>(univ.getReverter());
    while (count > 0) {
        structures::Ship rawShip;
        file.fullRead(afl::base::fromObject(rawShip));

        const int shipId = rawShip.shipId;
        map::Ship* s = univ.ships().get(shipId);
        if (!s) {
            throw afl::except::FileFormatException(file, afl::string::Format(m_translator.translateString("Invalid ship Id #%d").c_str(), shipId));
        }

        // Unpack the ship
        map::ShipData shipData;
        Packer(m_charset).unpackShip(shipData, rawShip, remapExplore);

        if (mode != LoadPrevious) {
            s->addCurrentShipData(shipData, source);
        }
        if (mode != LoadCurrent) {
            if (pReverter != 0) {
                pReverter->addShipData(shipId, shipData);
            }
        }
        --count;
    }
}

// Load targets.
void
game::v3::Loader::loadTargets(game::map::Universe& univ, afl::io::Stream& file, int count, TargetFormat fmt, PlayerSet_t source, int turnNumber) const
{
    // ex game/load.cc:loadTargets, ccmain.pas:LoadTargets, ccmain.pas:LoadTargetFile
    m_log.write(m_log.Debug, LOG_NAME, afl::string::Format(m_translator.translateString("Loading %d visual contact%!1{s%}...").c_str(), count));
    while (count > 0) {
        game::v3::structures::ShipTarget target;
        file.fullRead(afl::base::fromObject(target));

        // Decrypt the target
        if (fmt == TargetEncrypted) {
            encryptTarget(target);
        }

        addTarget(univ, target, source, turnNumber);

        --count;
    }
}

// Add a target.
void
game::v3::Loader::addTarget(game::map::Universe& univ, const game::v3::structures::ShipTarget& target, PlayerSet_t source, int turnNumber) const
{
    const int shipId = target.shipId;
    map::Ship* s = univ.ships().get(shipId);
    if (!s) {
        m_log.write(m_log.Error, LOG_NAME, afl::string::Format(m_translator.translateString("Invalid ship Id #%d for visual contact. Target will be ignored").c_str(), shipId));
    } else {
        // Convert to message information
        namespace gp = game::parser;
        gp::MessageInformation info(gp::MessageInformation::Ship, shipId, turnNumber);

        // Simple values
        info.addValue(gp::mi_Owner, target.owner);
        info.addValue(gp::mi_Speed, target.warpFactor);
        info.addValue(gp::mi_X, target.x);
        info.addValue(gp::mi_Y, target.y);
        info.addValue(gp::mi_ShipHull, target.hullType);

        // Heading
        int heading = target.heading;
        if (heading >= 0) {
            info.addValue(gp::mi_Heading, heading);
        }

        // Name (optional)
        String_t name = m_charset.decode(target.name);
        if (!isDummyName(name, shipId)) {
            info.addValue(gp::ms_Name, name);
        }

        s->addMessageInformation(info, source);
    }
}

// Load Minefields from KORE-style file.
void
game::v3::Loader::loadKoreMinefields(game::map::Universe& univ, afl::io::Stream& file, int count, int player, int turnNumber) const
{
    // ex game/load.cc:loadKoreMinefields
    m_log.write(m_log.Debug, LOG_NAME, afl::string::Format(m_translator.translateString("Loading up to %d minefield%!1{s%}...").c_str(), count));

    // We're loading a KORE file, so all minefields for this player are known.
    game::map::MinefieldType& ty = univ.minefields();
    ty.setAllMinefieldsKnown(player);

    // Read the file
    for (int i = 1; i <= count; ++i) {
        structures::KoreMine mf;
        file.fullRead(afl::base::fromObject(mf));
        if (mf.ownerTypeFlag != 0) {
            // Use get() if radius is 0; we don't want the minefield to start existing in this case
            if (game::map::Minefield* p = (mf.radius == 0 ? ty.get(i) : ty.create(i))) {
                // Figure out type/owner. 12 is a Tholian web, for other races we don't know the type.
                int owner;
                game::map::Minefield::TypeReport type;
                if (mf.ownerTypeFlag == 12) {
                    owner = 7;
                    type = game::map::Minefield::IsWeb;
                } else {
                    owner = mf.ownerTypeFlag;
                    type = game::map::Minefield::UnknownType;
                }

                p->addReport(game::map::Point(mf.x, mf.y),
                             owner, type,
                             game::map::Minefield::RadiusKnown, mf.radius,
                             turnNumber,
                             game::map::Minefield::MinefieldScanned);
            }
        }
    }
}

void
game::v3::Loader::loadKoreIonStorms(game::map::Universe& univ, afl::io::Stream& file, int count) const
{
    // ex game/load.cc:loadKoreIonStorms
    m_log.write(m_log.Debug, LOG_NAME, afl::string::Format(m_translator.translateString("Loading up to %d ion storm%!1{s%}...").c_str(), count));
    for (int i = 1; i <= count; ++i) {
        structures::KoreStorm st;
        file.fullRead(afl::base::fromObject(st));
        if (st.voltage > 0 && st.radius > 0) {
            game::map::IonStorm* s = univ.ionStorms().get(i);
            if (!s) {
                m_log.write(m_log.Error, LOG_NAME, afl::string::Format(m_translator.translateString("Invalid ion storm Id #%d. Storm will be ignored").c_str(), i));
            } else {
                s->setPosition(game::map::Point(st.x, st.y));
                s->setRadius(st.radius);
                s->setVoltage(st.voltage);
                s->setSpeed(st.warpFactor);
                s->setHeading(st.heading);
                s->setIsGrowing((st.voltage & 1) != 0);
            }
        }
    }
}

void
game::v3::Loader::loadKoreExplosions(game::map::Universe& univ, afl::io::Stream& file, int count) const
{
    // ex game/load.cc:loadKoreExplosions
    m_log.write(m_log.Debug, LOG_NAME, afl::string::Format(m_translator.translateString("Loading up to %d explosion%!1{s%}...").c_str(), count));

    for (int i = 1; i <= count; ++i) {
        structures::KoreExplosion kx;
        file.fullRead(afl::base::fromObject(kx));
        int x = kx.x;
        int y = kx.y;
        if (x != 0 || y != 0) {
            univ.explosions().add(game::map::Explosion(i, game::map::Point(x, y)));
        }
    }
}

void
game::v3::Loader::loadInbox(game::msg::Inbox& inbox, afl::io::Stream& file, int turn) const
{
    InboxFile parser(file, m_charset, m_translator);
    const size_t n = parser.getNumMessages();
    m_log.write(m_log.Debug, LOG_NAME, afl::string::Format(m_translator("Loading %d incoming message%!1{s%}...").c_str(), n));
    for (size_t i = 0; i < n; ++i) {
        String_t msgText(parser.loadMessage(i));
        int msgTurn = turn;
        if (msgText.size() > 2 && msgText.compare(0, 2, "(o", 2) == 0) {
            --msgTurn;
        }
        inbox.addMessage(msgText, msgTurn);
    }
}

void
game::v3::Loader::loadBattles(game::Turn& turn, afl::io::Stream& file, const game::config::HostConfiguration& config) const
{
    afl::base::Ptr<game::vcr::classic::Database> db = new game::vcr::classic::Database();
    db->load(file, config, m_charset);
    if (db->getNumBattles() != 0) {
        m_log.write(m_log.Debug, LOG_NAME, afl::string::Format(m_translator.translateString("Loaded %d combat recording%!1{s%}...").c_str(), db->getNumBattles()));
        turn.setBattles(db);
    }
}

void
game::v3::Loader::loadFlakBattles(game::Turn& turn, afl::io::Directory& gameDir, int playerNr)
{
    // ex maybeLoadFlakVcrs
    if (turn.getBattles().get() != 0) {
        // We already have regular combat, no need to look for FLAK
        return;
    }

    String_t fileName = afl::string::Format("flak%d.dat", playerNr);
    afl::base::Ptr<afl::io::Stream> s = gameDir.openFileNT(fileName, afl::io::FileSystem::OpenRead);
    if (s.get() == 0) {
        // No FLAK combat
        return;
    }

    afl::base::Ptr<game::vcr::flak::Database> db(new game::vcr::flak::Database());
    try {
        db->load(*s, m_charset, m_translator);
        if (db->getTimestamp() != turn.getTimestamp()) {
            m_log.write(afl::sys::LogListener::Error, LOG_NAME, afl::string::Format("%s is from a different turn. File will be ignored.", fileName));
            return;
        }
        if (db->getNumBattles() != 0) {
            m_log.write(afl::sys::LogListener::Debug, LOG_NAME, afl::string::Format(m_translator("Loaded %d combat recording%!1{s%} (FLAK)..."), db->getNumBattles()));
            turn.setBattles(db);
        }
    }
    catch (std::exception& e) {
        m_log.write(afl::sys::LogListener::Error, LOG_NAME, m_translator("Error loading FLAK combat"), e);
    }
}

void
game::v3::Loader::loadUfos(game::map::Universe& univ, afl::io::Stream& file, int firstId, int count) const
{
    // ex game/load.h:loadUfos, GUfoType::addUfoData, GUfo::addUfoData
    game::map::UfoType& ufos = univ.ufos();
    for (int i = 0; i < count; ++i) {
        structures::Ufo in;
        file.fullRead(afl::base::fromObject(in));
        if (in.color != 0) {
            // uc.addUfoData(first_id + i, ufo);
            if (game::map::Ufo* out = ufos.addUfo(firstId+i, in.typeCode, in.color)) {
                out->setName(m_charset.decode(in.name));
                out->setInfo1(m_charset.decode(in.info1));
                out->setInfo2(m_charset.decode(in.info2));
                out->setPosition(game::map::Point(in.x, in.y));
                out->setSpeed(int(in.warpFactor));
                if (in.heading >= 0) {
                    out->setHeading(int(in.heading));
                } else {
                    out->setHeading(afl::base::Nothing);
                }
                out->setPlanetRange(int(in.planetRange));
                out->setShipRange(int(in.shipRange));
                out->setRadius(int(in.radius));
                out->setIsSeenThisTurn(true);
            }
        }
    }
}

void
game::v3::Loader::loadCommonFiles(afl::io::Directory& gameDir, afl::io::Directory& specDir, game::map::Universe& univ, int player) const
{
    {
        afl::base::Ptr<Stream> file = gameDir.openFileNT(Format("xyplan%d.dat", player), afl::io::FileSystem::OpenRead);
        if (file.get() == 0) {
            file = specDir.openFile("xyplan.dat", afl::io::FileSystem::OpenRead).asPtr();
        }
        loadPlanetCoordinates(univ, *file);
    }

    // planet.nm
    {
        Ref<Stream> file = specDir.openFile("planet.nm", afl::io::FileSystem::OpenRead);
        loadPlanetNames(univ, *file);
    }

    // storm.nm
    {
        Ref<Stream> file = specDir.openFile("storm.nm", afl::io::FileSystem::OpenRead);
        loadIonStormNames(univ, *file);
    }
}

void
game::v3::Loader::loadResult(Turn& turn, const Root& root, Game& game, afl::io::Stream& file, int player) const
{
    // ex game/load-rst.cc:loadResult
    structures::Int16_t n;

    ResultFile result(file, m_translator);
    PlayerSet_t source(player);

    // Gen
    GenFile gen;
    result.seekToSection(ResultFile::GenSection);
    gen.loadFromResult(file);
    if (gen.getPlayerId() != player) {
        throw FileFormatException(file, Format(m_translator.translateString("File is owned by player %d, should be %d").c_str(), gen.getPlayerId(), player));
    }
//     trn.setHaveData(player);
//     trn.setGen(player, GGen(gen));
    gen.copyScoresTo(game.scores());
    turn.setTurnNumber(gen.getTurnNumber());
    turn.setTimestamp(gen.getTimestamp());

    // Ships
    result.seekToSection(ResultFile::ShipSection);
    file.fullRead(afl::base::fromObject(n));
    loadShips(turn.universe(), file, n, LoadBoth, !root.hostVersion().isMissionAllowed(1), source);

    // Targets
    result.seekToSection(ResultFile::TargetSection);
    file.fullRead(afl::base::fromObject(n));
    loadTargets(turn.universe(), file, n, TargetPlaintext, source, turn.getTurnNumber());

    // Planets
    result.seekToSection(ResultFile::PlanetSection);
    file.fullRead(afl::base::fromObject(n));
    loadPlanets(turn.universe(), file, n, LoadBoth, source);

    // Starbases
    result.seekToSection(ResultFile::BaseSection);
    file.fullRead(afl::base::fromObject(n));
    loadBases(turn.universe(), file, n, LoadBoth, source);

    // Messages
    result.seekToSection(ResultFile::MessageSection);
    loadInbox(turn.inbox(), file, gen.getTurnNumber());

    // SHIPXY (must be after SHIP) <-- FIXME: why this comment (from PCC2)?
    result.seekToSection(ResultFile::ShipXYSection);
    loadShipXY(turn.universe(), file, getShipXYSize(result), LoadBoth, source, PlayerSet_t());

    // VCRs
    result.seekToSection(ResultFile::VcrSection);
    loadBattles(turn, file, root.hostConfiguration());

    // Windows part of RST
    Stream::FileSize_t pos;
    if (result.getSectionOffset(ResultFile::KoreSection, pos)) {
        // KORE
        file.setPos(pos);
        loadKoreMinefields(turn.universe(), file, 500, player, turn.getTurnNumber());
        loadKoreIonStorms(turn.universe(), file, 50);
        loadKoreExplosions(turn.universe(), file, 50);
//         player_racenames.load(s); /* FIXME: configurable? */
//         host_racenames = player_racenames;
        file.setPos(pos + 500*8+600+50*4+682);
        loadUfos(turn.universe(), file, 1, 100);

        file.setPos(pos + 500*8+600+50*4+682+7800);
        KoreTargetHeader kth;
        if (file.read(afl::base::fromObject(kth)) == sizeof(kth) && std::memcmp(kth.sig, "1120", 4) == 0) {
            uint32_t n = kth.num;
            if (n > uint32_t(game::v3::structures::NUM_SHIPS)) {
                throw FileFormatException(file, m_translator.translateString("Unbelievable number of visual contacts"));
            }
            loadTargets(turn.universe(), file, n, TargetEncrypted, source, turn.getTurnNumber());
        }
    }

    if (result.getSectionOffset(ResultFile::SkoreSection, pos)) {
        // SKORE
        file.setPos(pos);
        if (file.read(afl::base::fromObject(n)) == sizeof(n) && n > 100) {
            loadUfos(turn.universe(), file, 101, n - 100);
        }
    }
}
