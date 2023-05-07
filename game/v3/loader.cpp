/**
  *  \file game/v3/loader.cpp
  *  \brief Class game::v3::Loader
  */

#include "game/v3/loader.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/except/assertionfailedexception.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/string/format.hpp"
#include "game/alliance/hosthandler.hpp"
#include "game/alliance/phosthandler.hpp"
#include "game/config/configurationparser.hpp"
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
#include "game/v3/commandcontainer.hpp"
#include "game/v3/commandextra.hpp"
#include "game/v3/genextra.hpp"
#include "game/v3/genfile.hpp"
#include "game/v3/hconfig.hpp"
#include "game/v3/inboxfile.hpp"
#include "game/v3/packer.hpp"
#include "game/v3/registrationkey.hpp"
#include "game/v3/resultfile.hpp"
#include "game/v3/reverter.hpp"
#include "game/v3/structures.hpp"
#include "game/v3/trn/turnprocessor.hpp"
#include "game/v3/turnfile.hpp"
#include "game/v3/utils.hpp"
#include "game/vcr/classic/database.hpp"
#include "game/vcr/flak/database.hpp"
#include "util/translation.hpp"

using afl::base::Ref;
using afl::except::FileFormatException;
using afl::except::checkAssertion;
using afl::io::FileSystem;
using afl::io::Stream;
using afl::string::Format;
using afl::sys::LogListener;
using game::config::ConfigurationOption;

namespace gt = game::v3::structures;

namespace {
    const char LOG_NAME[] = "game.v3.loader";

    const int DEFAULT_PHOST_VERSION = MKVERSION(4,1,0);
    const int DEFAULT_HOST_VERSION = MKVERSION(3,22,26);

    struct KoreTargetHeader {
        char sig[4];
        gt::UInt32_t num;
    };

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

    /* Extract commands from a message.
       This figures out the PHost commands from a message a player sent to himself.
       \param trn     Game turn object
       \param text    Decoded message text
       \param player  Player number
       \returns Message text without commands. Might be empty if the message consists entirely of commands. */
    String_t extractCommands(game::Turn& trn, String_t text, int player)
    {
        String_t::size_type n = text.find_first_not_of(' ');
        if (n != text.npos && text[n] == '<') {
            // it's a genuine message. Don't parse.
            return text;
        }

        String_t s;
        do {
            String_t now = afl::string::strFirst(text, "\n");
            if (game::v3::Command::isMessageIntroducer(now)) {
                // the rest is a message.
                return s + text;
            }
            if (game::v3::Command* cmd = game::v3::Command::parseCommand(now, false, false)) {
                game::v3::CommandExtra::create(trn).create(player).addNewCommand(cmd);
            } else {
                s += now;
                s += '\n';
            }
        } while (afl::string::strRemove(text, "\n"));

        String_t::size_type x = s.length();
        while (x > 0 && s[x-1] == '\n') {
            --x;
        }
        if (x < s.length()) {
            s.erase(x);
        }
        return s;
    }

    /* Try to guess a game name */
    void guessGameName(game::config::StringOption& gameName, afl::io::Directory& dir, afl::charset::Charset& cs)
    {
        // ex cc.pas:FillInGameName
        // Nothing to do if option was set
        if (gameName.wasSet()) {
            return;
        }

        // Check for vpwork directory
        String_t dirTitle = afl::string::strLCase(dir.getTitle());
        static_assert(game::v3::structures::NUM_GAMESTAT_SLOTS == 8, NUM_GAMESTAT_SLOTS);
        if (dirTitle.size() == 7 && dirTitle >= "vpwork1" && dirTitle <= "vpwork8") {
            const int slotNr = dirTitle[6] - '1';
            try {
                afl::base::Ptr<afl::io::Directory> parent = dir.getParentDirectory();
                if (parent.get() != 0) {
                    game::v3::structures::GameStatFile file;
                    parent->openFile("gamestat.dat", afl::io::FileSystem::OpenRead)
                        ->fullRead(afl::base::fromObject(file));

                    String_t configuredName = cs.decode(file.slots[slotNr].name);
                    if (!configuredName.empty()) {
                        gameName.setAndMarkUpdated(configuredName, game::config::ConfigurationOption::Game);
                        return;
                    }
                }
            }
            catch (...) { }
        }

        // Use directory name
        gameName.setAndMarkUpdated(dir.getTitle(), game::config::ConfigurationOption::Game);
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
    for (int i = 1; i <= gt::NUM_SHIPS; ++i) {
        univ.ships().create(i);
    }
    for (int i = 1; i <= gt::NUM_PLANETS; ++i) {
        univ.planets().create(i);
    }
    for (int i = 1; i <= gt::NUM_ION_STORMS; ++i) {
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
        turn.alliances().addNewHandler(new game::alliance::PHostHandler(turn, root, player), session.translator());
    } else {
        turn.alliances().addNewHandler(new game::alliance::HostHandler(root.hostVersion().getVersion(), turn, player), session.translator());
    }
}

// Load planets.
void
game::v3::Loader::loadPlanets(game::map::Universe& univ, afl::io::Stream& file, int count, LoadMode mode, PlayerSet_t source) const
{
    // ex game/load.cc:loadPlanets
    m_log.write(LogListener::Debug, LOG_NAME, afl::string::Format(m_translator("Loading %d planet%!1{s%}..."), count));
    Reverter* pReverter = dynamic_cast<Reverter*>(univ.getReverter());
    while (count > 0) {
        gt::Planet rawPlanet;
        file.fullRead(afl::base::fromObject(rawPlanet));

        const int planetId = rawPlanet.planetId;
        map::Planet* p = univ.planets().get(planetId);
        if (!p) {
            throw afl::except::FileFormatException(file, afl::string::Format(m_translator("Invalid planet Id #%d"), planetId));
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
    m_log.write(LogListener::Debug, LOG_NAME, afl::string::Format(m_translator("Loading up to %d planet position%!1{s%}..."), gt::NUM_PLANETS));
    gt::Int16_t data[gt::NUM_PLANETS * 3];
    file.fullRead(afl::base::fromObject(data));
    for (int planetId = 1; planetId <= gt::NUM_PLANETS; ++planetId) {
        // FIXME: PCC2 checked chart config here.
        // pro: coordinate filtering is a v3 thing, and should be done in v3 code
        // con: doing the filtering in game::map::Planet::internalCheck only allows live map-reconfiguration to recover from errors
        game::map::Point pt(data[3*planetId-3], data[3*planetId-2]);
        game::map::Planet* p = univ.planets().get(planetId);
        if (!p) {
            throw afl::except::FileFormatException(file, afl::string::Format(m_translator("Invalid planet Id #%d"), planetId));
        }
        p->setPosition(pt);
    }
}

// Load planet names.
void
game::v3::Loader::loadPlanetNames(game::map::Universe& univ, afl::io::Stream& file) const
{
    // ex game/load.h:loadPlanetNames
    m_log.write(LogListener::Debug, LOG_NAME, afl::string::Format(m_translator("Loading %d planet name%!1{s%}..."), gt::NUM_PLANETS));
    gt::String20_t data[gt::NUM_PLANETS];
    file.fullRead(afl::base::fromObject(data));
    for (int planetId = 1; planetId <= gt::NUM_PLANETS; ++planetId) {
        game::map::Planet* p = univ.planets().get(planetId);
        if (!p) {
            throw afl::except::FileFormatException(file, afl::string::Format(m_translator("Invalid planet Id #%d"), planetId));
        }
        p->setName(m_charset.decode(data[planetId-1]));
    }
}

// Load Ion Storm Names.
void
game::v3::Loader::loadIonStormNames(game::map::Universe& univ, afl::io::Stream& file) const
{
    // ex game/load.h:loadStormNames, ccload.pas:LoadStormNames
    m_log.write(LogListener::Debug, LOG_NAME, afl::string::Format(m_translator("Loading %d ion storm name%!1{s%}..."), gt::NUM_ION_STORMS));
    gt::String20_t data[gt::NUM_ION_STORMS];
    file.fullRead(afl::base::fromObject(data));
    for (int stormId = 1; stormId <= gt::NUM_ION_STORMS; ++stormId) {
        game::map::IonStorm* p = univ.ionStorms().get(stormId);
        if (!p) {
            throw afl::except::FileFormatException(file, afl::string::Format(m_translator("Invalid ion storm Id #%d"), stormId));
        }
        p->setName(m_charset.decode(data[stormId-1]));
    }
}

// Load starbases.
void
game::v3::Loader::loadBases(game::map::Universe& univ, afl::io::Stream& file, int count, LoadMode mode, PlayerSet_t source) const
{
    // ex game/load.h:loadBases, ccload.pas:LoadBases
    m_log.write(LogListener::Debug, LOG_NAME, afl::string::Format(m_translator("Loading %d starbase%!1{s%}..."), count));
    Reverter* pReverter = dynamic_cast<Reverter*>(univ.getReverter());
    while (count > 0) {
        gt::Base rawBase;
        file.fullRead(afl::base::fromObject(rawBase));

        const int baseId = rawBase.baseId;
        map::Planet* p = univ.planets().get(baseId);
        if (!p) {
            throw afl::except::FileFormatException(file, afl::string::Format(m_translator("Invalid starbase Id #%d"), baseId));
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
    static_assert(gt::NUM_SHIPS == 999, "NUM_SHIPS");
    size_t numShips = (bytes != 0 && bytes >= 999 * sizeof(gt::ShipXY)) ? 999 : 500;
    m_log.write(LogListener::Debug, LOG_NAME, afl::string::Format(m_translator("Loading up to %d ship position%!1{s%}..."), numShips));

    // Read file in chunks
    const size_t CHUNK_SIZE = 100;
    Id_t id = 0;
    while (numShips > 0) {
        gt::ShipXY buffer[CHUNK_SIZE];
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

            if (owner > 0 && owner <= gt::NUM_OWNERS && !reject.contains(owner)) {
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
    m_log.write(LogListener::Debug, LOG_NAME, afl::string::Format(m_translator("Loading %d ship%!1{s%}..."), count));
    Reverter* pReverter = dynamic_cast<Reverter*>(univ.getReverter());
    while (count > 0) {
        gt::Ship rawShip;
        file.fullRead(afl::base::fromObject(rawShip));

        const int shipId = rawShip.shipId;
        map::Ship* s = univ.ships().get(shipId);
        if (!s) {
            throw afl::except::FileFormatException(file, afl::string::Format(m_translator("Invalid ship Id #%d"), shipId));
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
    m_log.write(LogListener::Debug, LOG_NAME, afl::string::Format(m_translator("Loading %d visual contact%!1{s%}..."), count));
    while (count > 0) {
        gt::ShipTarget target;
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
        m_log.write(LogListener::Error, LOG_NAME, afl::string::Format(m_translator("Invalid ship Id #%d for visual contact. Target will be ignored"), shipId));
    } else {
        // Convert to message information
        namespace gp = game::parser;
        gp::MessageInformation info(gp::MessageInformation::Ship, shipId, turnNumber);

        // Simple values
        info.addValue(gp::mi_Owner, target.owner);
        info.addValue(gp::mi_WarpFactor, target.warpFactor);
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
    m_log.write(LogListener::Debug, LOG_NAME, afl::string::Format(m_translator("Loading up to %d minefield%!1{s%}..."), count));

    // We're loading a KORE file, so all minefields for this player are known.
    game::map::MinefieldType& ty = univ.minefields();
    ty.setAllMinefieldsKnown(player);

    // Read the file
    for (int i = 1; i <= count; ++i) {
        gt::KoreMine mf;
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
    m_log.write(LogListener::Debug, LOG_NAME, afl::string::Format(m_translator("Loading up to %d ion storm%!1{s%}..."), count));
    for (int i = 1; i <= count; ++i) {
        gt::KoreStorm st;
        file.fullRead(afl::base::fromObject(st));
        if (st.voltage > 0 && st.radius > 0) {
            game::map::IonStorm* s = univ.ionStorms().get(i);
            if (!s) {
                m_log.write(LogListener::Error, LOG_NAME, afl::string::Format(m_translator("Invalid ion storm Id #%d. Storm will be ignored"), i));
            } else {
                s->setPosition(game::map::Point(st.x, st.y));
                s->setRadius(st.radius);
                s->setVoltage(st.voltage);
                s->setWarpFactor(st.warpFactor);
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
    m_log.write(LogListener::Debug, LOG_NAME, afl::string::Format(m_translator("Loading up to %d explosion%!1{s%}..."), count));

    for (int i = 1; i <= count; ++i) {
        gt::KoreExplosion kx;
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
    m_log.write(LogListener::Debug, LOG_NAME, afl::string::Format(m_translator("Loading %d incoming message%!1{s%}..."), n));
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
        m_log.write(LogListener::Debug, LOG_NAME, afl::string::Format(m_translator("Loaded %d combat recording%!1{s%}..."), db->getNumBattles()));
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
    afl::base::Ptr<afl::io::Stream> s = gameDir.openFileNT(fileName, FileSystem::OpenRead);
    if (s.get() == 0) {
        // No FLAK combat
        return;
    }

    afl::base::Ptr<game::vcr::flak::Database> db(new game::vcr::flak::Database());
    try {
        db->load(*s, m_charset, m_translator);
        if (db->getTimestamp() != turn.getTimestamp()) {
            m_log.write(LogListener::Error, LOG_NAME, afl::string::Format("%s is from a different turn. File will be ignored.", fileName));
            return;
        }
        if (db->getNumBattles() != 0) {
            m_log.write(LogListener::Debug, LOG_NAME, afl::string::Format(m_translator("Loaded %d combat recording%!1{s%} (FLAK)..."), db->getNumBattles()));
            turn.setBattles(db);
        }
    }
    catch (std::exception& e) {
        m_log.write(LogListener::Error, LOG_NAME, m_translator("Error loading FLAK combat"), e);
    }
}

void
game::v3::Loader::loadUfos(game::map::Universe& univ, afl::io::Stream& file, int firstId, int count) const
{
    // ex game/load.h:loadUfos, GUfoType::addUfoData, GUfo::addUfoData
    game::map::UfoType& ufos = univ.ufos();
    for (int i = 0; i < count; ++i) {
        gt::Ufo in;
        file.fullRead(afl::base::fromObject(in));
        if (in.color != 0) {
            // uc.addUfoData(first_id + i, ufo);
            if (game::map::Ufo* out = ufos.addUfo(firstId+i, in.typeCode, in.color)) {
                out->setName(m_charset.decode(in.name));
                out->setInfo1(m_charset.decode(in.info1));
                out->setInfo2(m_charset.decode(in.info2));
                out->setPosition(game::map::Point(in.x, in.y));
                out->setWarpFactor(int(in.warpFactor));
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
game::v3::Loader::loadPConfig(Root& root, afl::io::Stream& pconfig, afl::base::Ptr<afl::io::Stream> shiplist, game::config::ConfigurationOption::Source source)
{
    // ex game/config.cc:loadPConfig
    // Configure parser
    game::config::ConfigurationParser parser(m_log, m_translator, root.hostConfiguration(), source);
    parser.setCharsetNew(m_charset.clone());

    // Load pconfig.src (mandatory)
    m_log.write(LogListener::Info, LOG_NAME, afl::string::Format(m_translator("Reading configuration from %s..."), pconfig.getName()));
    parser.setSection("phost", true);
    parser.parseFile(pconfig);

    // Load shiplist.txt (optional)
    if (shiplist.get() != 0) {
        m_log.write(LogListener::Info, LOG_NAME, afl::string::Format(m_translator("Reading configuration from %s..."), shiplist->getName()));
        parser.setSection("phost", false);
        parser.parseFile(*shiplist);
    }

    // Postprocess
    root.hostConfiguration().setDependantOptions();

    // Update host version guess
    HostVersion& host = root.hostVersion();
    if (host.getKind() == HostVersion::Unknown) {
        host.set(HostVersion::PHost, DEFAULT_PHOST_VERSION);
        m_log.write(LogListener::Info, LOG_NAME, afl::string::Format(m_translator("Host version not known, assuming %s"), host.toString()));
    }
}

void
game::v3::Loader::loadHConfig(Root& root, afl::io::Stream& hconfig, game::config::ConfigurationOption::Source source)
{
    // ex game/config.cc:loadHConfig, Config::assignFromHConfigImage
    // FIXME: do host version guessing in this function
    if (hconfig.getSize() > 10*sizeof(gt::HConfig)) {
        // FIXME: log only?
        throw afl::except::FileFormatException(hconfig, m_translator("File has invalid size"));
    }

    // Read hconfig
    m_log.write(LogListener::Info, LOG_NAME, afl::string::Format(m_translator("Reading configuration from %s..."), hconfig.getName()));

    gt::HConfig image;
    size_t size = hconfig.read(afl::base::fromObject(image));
    unpackHConfig(image, size, root.hostConfiguration(), source);

    // Postprocess
    root.hostConfiguration().setDependantOptions();

    // Update host version guess
    HostVersion& host = root.hostVersion();
    if (host.getKind() == HostVersion::Unknown) {
        host.set(HostVersion::Host, DEFAULT_HOST_VERSION);
        m_log.write(LogListener::Info, LOG_NAME, afl::string::Format(m_translator("Host version not known, assuming %s"), host.toString()));
    }
}

void
game::v3::Loader::loadRaceMapping(Root& root, afl::io::Stream& file, game::config::ConfigurationOption::Source source)
{
    // ex ccload.pas:LoadSRaceMappings, loadRaceMapping
    gt::Int16_t mapping[gt::NUM_PLAYERS];
    if (file.read(afl::base::fromObject(mapping)) == sizeof(mapping)) {
        // Load configuration option
        game::config::HostConfiguration& config = root.hostConfiguration();
        for (int i = 1; i <= gt::NUM_PLAYERS; ++i) {
            config[config.PlayerRace].set(i, mapping[i-1]);
        }
        config[config.PlayerSpecialMission].copyFrom(config[config.PlayerRace]);
        config[config.PlayerRace].setSource(source);
        config[config.PlayerSpecialMission].setSource(source);

        // Update host version guess
        HostVersion& host = root.hostVersion();
        if (host.getKind() == HostVersion::Unknown) {
            host.set(HostVersion::SRace, DEFAULT_HOST_VERSION);
            m_log.write(LogListener::Info, LOG_NAME, afl::string::Format(m_translator("Host version not known, assuming %s"), host.toString()));
        }
    }
}

void
game::v3::Loader::loadCommonFiles(afl::io::Directory& gameDir, afl::io::Directory& specDir, game::map::Universe& univ, int player) const
{
    // xyplan.dat
    // ex ccload.pas:LoadPlanetXY
    // FIXME: PCC1 shows a warning if there's a possible conflict between xyplan.dat/xyplanX.dat
    {
        afl::base::Ptr<Stream> file = gameDir.openFileNT(Format("xyplan%d.dat", player), FileSystem::OpenRead);
        if (file.get() == 0) {
            file = specDir.openFile("xyplan.dat", FileSystem::OpenRead).asPtr();
        }
        loadPlanetCoordinates(univ, *file);
    }

    // planet.nm
    {
        Ref<Stream> file = specDir.openFile("planet.nm", FileSystem::OpenRead);
        loadPlanetNames(univ, *file);
    }

    // storm.nm
    {
        Ref<Stream> file = specDir.openFile("storm.nm", FileSystem::OpenRead);
        loadIonStormNames(univ, *file);
    }
}

void
game::v3::Loader::loadResult(Turn& turn, const Root& root, Game& game, afl::io::Stream& file, int player) const
{
    // ex game/load-rst.cc:loadResult
    gt::Int16_t n;

    ResultFile result(file, m_translator);
    PlayerSet_t source(player);

    // Gen
    GenFile gen;
    result.seekToSection(ResultFile::GenSection);
    gen.loadFromResult(file);
    if (gen.getPlayerId() != player) {
        throw FileFormatException(file, Format(m_translator("File is owned by player %d, should be %d"), gen.getPlayerId(), player));
    }
    GenExtra::create(turn).create(player) = gen;
    // FIXME:    trn.setHaveData(player);
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
    loadShipXY(turn.universe(), file, result.getNumShipCoordinates() * sizeof(gt::ShipXY), LoadBoth, source, PlayerSet_t());

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
            if (n > uint32_t(gt::NUM_SHIPS)) {
                throw FileFormatException(file, m_translator("Unbelievable number of visual contacts"));
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

void
game::v3::Loader::loadTurnfile(Turn& turn, const Root& root, afl::io::Stream& file, int player) const
{
    // ex game/load-trn.cc:loadTurn
    // Load, validate, and log.
    TurnFile f(m_charset, m_translator, file);
    if (f.getPlayer() != player) {
        throw FileFormatException(file, Format(m_translator("Turn file belongs to player %d"), f.getPlayer()));
    }
    if (f.getFeatures().contains(TurnFile::TaccomFeature)) {
        m_log.write(LogListener::Info, LOG_NAME, Format(m_translator("Turn file contains %d attachment%!1{s%}"), f.getNumFiles()));
    }

    // Use TurnProcessor to load the turn file.
    class LocalTurnProcessor : public game::v3::trn::TurnProcessor {
     public:
        LocalTurnProcessor(Turn& turn, Stream& file, int player, bool remapExplore, const Loader& parent)
            : m_turn(turn),
              m_file(file),
              m_player(player),
              m_remapExplore(remapExplore),
              m_parent(parent)
            { }
        void fail(const char* tpl, int arg)
            { throw FileFormatException(m_file, Format(m_parent.m_translator(tpl), arg)); }
        virtual void handleInvalidCommand(int code)
            { fail(N_("Turn file contains invalid command code %d"), code); }
        virtual void validateShip(int id)
            {
                if (const game::map::Ship* sh = m_turn.universe().ships().get(id)) {
                    if (!sh->getShipSource().contains(m_player)) {
                        fail(N_("Turn file refers to ship %d which is not ours"), id);
                    }
                } else {
                    fail(N_("Turn file refers to non-existant ship %d"), id);
                }
            }
        virtual void validatePlanet(int id)
            {
                if (const game::map::Planet* pl = m_turn.universe().planets().get(id)) {
                    if (!pl->getPlanetSource().contains(m_player)) {
                        fail(N_("Turn file refers to planet %d which is not ours"), id);
                    }
                } else {
                    fail(N_("Turn file refers to non-existant planet %d"), id);
                }
            }
        virtual void validateBase(int id)
            {
                if (const game::map::Planet* pl = m_turn.universe().planets().get(id)) {
                    if (!pl->getBaseSource().contains(m_player)) {
                        fail(N_("Turn file refers to starbase %d which is not ours"), id);
                    }
                } else {
                    fail(N_("Turn file refers to non-existant starbase %d"), id);
                }
            }

        virtual void getShipData(int id, Ship_t& out, afl::charset::Charset& charset)
            {
                if (game::map::Ship* sh = m_turn.universe().ships().get(id)) {
                    game::map::ShipData data;
                    sh->getCurrentShipData(data);
                    Packer(charset).packShip(out, id, data, m_remapExplore);
                }
            }

        virtual void getPlanetData(int id, Planet_t& out, afl::charset::Charset& charset)
            {
                if (game::map::Planet* pl = m_turn.universe().planets().get(id)) {
                    game::map::PlanetData data;
                    pl->getCurrentPlanetData(data);
                    Packer(charset).packPlanet(out, id, data);
                }
            }

        virtual void getBaseData(int id, Base_t& out, afl::charset::Charset& charset)
            {
                if (game::map::Planet* pl = m_turn.universe().planets().get(id)) {
                    game::map::BaseData data;
                    pl->getCurrentBaseData(data);
                    const int owner = pl->getOwner().orElse(0);
                    Packer(charset).packBase(out, id, data, owner);
                }
            }

        virtual void storeShipData(int id, const Ship_t& in, afl::charset::Charset& charset)
            {
                if (game::map::Ship* sh = m_turn.universe().ships().get(id)) {
                    game::map::ShipData data;
                    Packer(charset).unpackShip(data, in, m_remapExplore);
                    sh->addCurrentShipData(data, PlayerSet_t(m_player));
                }
            }

        virtual void storePlanetData(int id, const Planet_t& in, afl::charset::Charset& charset)
            {
                if (game::map::Planet* pl = m_turn.universe().planets().get(id)) {
                    game::map::PlanetData data;
                    Packer(charset).unpackPlanet(data, in);
                    pl->addCurrentPlanetData(data, PlayerSet_t(m_player));
                }
            }

        virtual void storeBaseData(int id, const Base_t& in, afl::charset::Charset& charset)
            {
                if (game::map::Planet* pl = m_turn.universe().planets().get(id)) {
                    game::map::BaseData data;
                    Packer(charset).unpackBase(data, in);
                    pl->addCurrentBaseData(data, PlayerSet_t(m_player));
                }
            }

        virtual void addMessage(int to, String_t text)
            {
                if (to > 0 && to <= structures::NUM_OWNERS) {
                    if (to == structures::NUM_OWNERS) {
                        to = 0;
                    }
                    m_parent.addMessage(m_turn, text, m_player, PlayerSet_t(to));
                }
            }

        virtual void addNewPassword(const NewPassword_t& pass)
            {
                if (GenFile* p = GenExtra::get(m_turn, m_player)) {
                    p->setNewPasswordData(pass);
                }
            }

        virtual void addAllianceCommand(String_t text)
            { CommandExtra::create(m_turn).create(m_player).addCommand(Command::TAlliance, 0, text); }
     private:
        Turn& m_turn;
        Stream& m_file;
        const int m_player;
        bool m_remapExplore;
        const Loader& m_parent;
    };

    const bool remapExplore = !root.hostVersion().isMissionAllowed(1);
    LocalTurnProcessor(turn, file, player, remapExplore, *this).handleTurnFile(f, m_charset);
}

void
game::v3::Loader::saveTurnFile(TurnFile& thisTurn, const Turn& turn, int player, const Root& root) const
{
    const char*const LOCATION = "Loader::saveTurnFile";

    // Obtain reverter
    const game::map::Universe& u = turn.universe();
    Reverter* rev = dynamic_cast<Reverter*>(u.getReverter());
    checkAssertion(rev != 0, "Reverter exists", LOCATION);

    // Obtain key
    const RegistrationKey* key = dynamic_cast<const RegistrationKey*>(&root.registrationKey());
    checkAssertion(key != 0, "Key exists", LOCATION);

    thisTurn.setFeatures(TurnFile::FeatureSet_t(TurnFile::WinplanFeature));
    thisTurn.setRegistrationKey(*key, turn.getTurnNumber());

    // Make commands
    const game::map::Ship* pAllianceShip = 0;
    Packer pack(m_charset);
    const bool remapExplore = !root.hostVersion().isMissionAllowed(1);
    for (int i = 1; i <= structures::NUM_SHIPS; ++i) {
        const game::map::Ship* pShip = u.ships().get(i);
        const game::map::ShipData* pOldShip = rev->getShipData(i);
        if (pShip != 0 && pOldShip != 0 && pShip->getShipSource().contains(player)) {
            if (pAllianceShip == 0) {
                pAllianceShip = pShip;
            }

            // Get ship data
            game::map::ShipData newShip;
            pShip->getCurrentShipData(newShip);

            // Convert into blobs
            structures::Ship rawOldShip, rawNewShip;
            pack.packShip(rawOldShip, i, *pOldShip, remapExplore);
            pack.packShip(rawNewShip, i,   newShip, remapExplore);

            // Make commands
            thisTurn.makeShipCommands(i, rawOldShip, rawNewShip);
        }
    }
    for (int i = 1; i <= structures::NUM_PLANETS; ++i) {
        const game::map::Planet* pPlanet = u.planets().get(i);
        const game::map::PlanetData* pOldPlanet = rev->getPlanetData(i);
        if (pPlanet != 0 && pOldPlanet != 0 && pPlanet->getPlanetSource().contains(player)) {
            // Get planet data
            game::map::PlanetData newPlanet;
            pPlanet->getCurrentPlanetData(newPlanet);

            // Convert into blobs
            structures::Planet rawOldPlanet, rawNewPlanet;
            pack.packPlanet(rawOldPlanet, i, *pOldPlanet);
            pack.packPlanet(rawNewPlanet, i,   newPlanet);

            // Make commands
            thisTurn.makePlanetCommands(i, rawOldPlanet, rawNewPlanet);
        }
    }
    for (int i = 1; i <= structures::NUM_PLANETS; ++i) {
        const game::map::Planet* pPlanet = u.planets().get(i);
        const game::map::BaseData* pOldBase = rev->getBaseData(i);
        if (pPlanet != 0 && pOldBase != 0 && pPlanet->getBaseSource().contains(player)) {
            // Get starbase data
            game::map::BaseData newBase;
            pPlanet->getCurrentBaseData(newBase);

            const int owner = pPlanet->getOwner().orElse(0);

            // Convert into blobs
            structures::Base rawOldBase, rawNewBase;
            pack.packBase(rawOldBase, i, *pOldBase, owner);
            pack.packBase(rawNewBase, i,   newBase, owner);

            // Make commands
            thisTurn.makeBaseCommands(i, rawOldBase, rawNewBase);
        }
    }

    // Messages
    thisTurn.sendOutbox(turn.outbox(), player, m_translator, root.playerList(), m_charset);

    // Command messages
    if (const CommandExtra* cx = CommandExtra::get(turn)) {
        if (const CommandContainer* cc = cx->get(player)) {
            String_t accum;
            for (CommandContainer::ConstIterator_t i = cc->begin(); i != cc->end(); ++i) {
                if (const Command* pc = *i) {
                    if (pc->getCommand() == Command::TAlliance) {
                        if (pAllianceShip == 0) {
                            m_log.write(m_log.Warn, LOG_NAME, Format(m_translator("Player %d has no ship; alliance changes not transmitted"), player));
                        } else {
                            thisTurn.sendTHostAllies(pc->getArg(), pAllianceShip->getId(), pAllianceShip->getFriendlyCode().orElse(""));
                        }
                    } else {
                        const String_t text = pc->getCommandText();
                        if (!text.empty() && text[0] != '$') {
                            if (accum.size() + text.size() > 500) {
                                thisTurn.sendMessage(player, player, accum, m_charset);
                                accum.clear();
                            }
                            accum += text;
                            accum += '\n';
                        }
                    }
                }
            }
            if (!accum.empty()) {
                thisTurn.sendMessage(player, player, accum, m_charset);
            }
        }
    }

    // New password
    if (const GenFile* gen = GenExtra::get(turn, player)) {
        afl::base::ConstBytes_t newPassword = gen->getNewPasswordData();
        if (!newPassword.empty()) {
            thisTurn.addCommand(tcm_ChangePassword, 0, newPassword);
        }
    }

    thisTurn.update();
}

void
game::v3::Loader::loadConfiguration(Root& root, afl::io::Directory& dir)
{
    game::config::HostConfiguration& config = root.hostConfiguration();
    config.setDefaultValues();

    // FIXME: PCC1 shows warning if fewer than 70 pconfig keys
    // FIXME: PCC1 shows warning if both PCONFIG.SRC and FRIDAY.DAT

    // Check pconfig.src
    afl::base::Ptr<afl::io::Stream> file = dir.openFileNT("pconfig.src", FileSystem::OpenRead);
    if (file.get() != 0) {
        // OK, PHost
        loadPConfig(root, *file, dir.openFileNT("shiplist.txt", FileSystem::OpenRead), ConfigurationOption::Game);
    } else {
        // SRace
        file = root.gameDirectory().openFileNT("friday.dat", FileSystem::OpenRead);
        if (file.get() != 0) {
            loadRaceMapping(root, *file, ConfigurationOption::Game);
        }

        // Regular host config
        file = dir.openFileNT("hconfig.hst", FileSystem::OpenRead);
        if (file.get() != 0) {
            loadHConfig(root, *file, ConfigurationOption::Game);
        } else {
            m_log.write(LogListener::Warn, LOG_NAME, m_translator("No host configuration file found, using defaults"));
        }
    }

    root.hostVersion().setImpliedHostConfiguration(config);

    // FLAK
    game::vcr::flak::loadConfiguration(root.flakConfiguration(), dir, m_log, m_translator);

    // If we still do not have a game name, try to guess one
    guessGameName(root.hostConfiguration()[game::config::HostConfiguration::GameName], dir, m_charset);
}

/** Add message from message file.
    This decides whether the message is a command message or a normal message,
    and places it in the appropriate part of the game turn object (outbox, command list).

    \param turn        Turn
    \param text        Message text (decoded)
    \param sender      Sender of message
    \param receivers   Receivers of message

    \note This only recognizes messages to one receiver as command messages.
    It is possible (but unlikely) that someone sends a message to theirselves and someone else.
    Our Maketurn will make sure that the message comes out as a real text message.
    However, with Winplan's maketurn, the message will be interpreted by PHost.  */
void
game::v3::Loader::addMessage(Turn& turn, String_t text, int sender, PlayerSet_t receivers) const
{
    // ex game/load.cc:addMessageFromFile
    if (receivers == PlayerSet_t(sender)) {
        // It's a message to us. Is it a command message?
        text = extractCommands(turn, text, sender);
        if (text.empty()) {
            return;
        }
    }
    turn.outbox().addMessageFromFile(sender, text, receivers);
}
