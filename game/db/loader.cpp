/**
  *  \file game/db/loader.cpp
  */

#include <cstring>
#include <algorithm>
#include "game/db/loader.hpp"
#include "afl/base/countof.hpp"
#include "afl/data/namemap.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/io/limitedstream.hpp"
#include "afl/string/format.hpp"
#include "game/db/drawingatommap.hpp"
#include "game/db/packer.hpp"
#include "game/db/structures.hpp"
#include "interpreter/savevisitor.hpp"
#include "interpreter/vmio/nullloadcontext.hpp"
#include "interpreter/vmio/nullsavecontext.hpp"
#include "interpreter/vmio/valueloader.hpp"
#include "util/atomtable.hpp"
#include "util/io.hpp"
#include "util/translation.hpp"

namespace {
    namespace gm = game::map;
    namespace gp = game::parser;
    namespace dt = game::db::structures;

    const char LOG_NAME[] = "game.db";

    const uint8_t colors[] = {
        0,
        1,   2,   3,   4,   5,   6,   7,   8,   9,  15,
        129, 131, 133, 135, 137, 139, 141, 143, 145, 147,
        130, 132, 134, 136, 138, 140, 142, 144, 146, 148
    };

    uint8_t convertColor(uint8_t externalColor)
    {
        for (size_t i = 0; i < countof(colors); ++i) {
            if (colors[i] == externalColor) {
                return uint8_t(i);
            }
        }
        return 10;
    }

    uint8_t convertToExternalColor(uint8_t internalColor)
    {
        if (internalColor < countof(colors)) {
            return colors[internalColor];
        } else {
            return 15;
        }
    }

    bool readUnitScoreHeader(afl::io::Stream& s, game::db::structures::UnitScoreHeader& ush)
    {
        /* Read size field */
        game::db::structures::UInt16_t size;
        if (s.read(size.m_bytes) != sizeof(size)) {
            return false;
        }

        size_t parsedSize = size;
        if (parsedSize < sizeof(ush)) {
            return false;
        }

        /* Read unit score header */
        if (s.read(afl::base::fromObject(ush)) != sizeof(ush)) {
            return false;
        }

        /* Skip possible trailing data */
        if (parsedSize > sizeof(ush)) {
            s.setPos(s.getPos() + parsedSize - sizeof(ush));
        }

        return true;
    }

    template<typename T>
    void saveTypeUnitScores(afl::io::Stream& out,
                            const game::map::ObjectVector<T>& vec,
                            size_t index)
    {
        for (int oid = 1, n = vec.size(); oid <= n; ++oid) {
            if (const T* p = vec.get(oid)) {
                int16_t value, turn;
                if (p->unitScores().get(index, value, turn)) {
                    dt::UnitScoreEntry entry;
                    entry.id = static_cast<int16_t>(oid);
                    entry.score = value;
                    entry.turn = turn;
                    out.fullWrite(afl::base::fromObject(entry));
                }
            }
        }
    }

    struct RecordState {
        game::db::structures::BlockHeader header;
        afl::io::Stream::FileSize_t headerPos;
    };

    void startRecord(afl::io::Stream& out, uint16_t type, RecordState& state)
    {
        state.headerPos = out.getPos();
        state.header.blockType = type;
        state.header.size = 0;
        out.fullWrite(afl::base::fromObject(state.header));
    }

    void endRecord(afl::io::Stream& out, RecordState& state)
    {
        afl::io::Stream::FileSize_t endPos = out.getPos();

        // Write updated header.
        state.header.size = static_cast<uint32_t>(endPos - state.headerPos - sizeof(state.header));
        out.setPos(state.headerPos);
        out.fullWrite(afl::base::fromObject(state.header));

        // Go back
        out.setPos(endPos);
    }

    // /** Compute atom map. Populates a GDrawingAtomMap object, used for
    //     chart file I/O and filtering.
    //     \param map   [out] Object to populate
    //     \param flags [in] Options, bitfield of IncludeNumeric, IncludeInvisible. */
    void computeAtomMap(game::db::DrawingAtomMap& out,
                        const game::map::DrawingContainer& drawings,
                        const util::AtomTable& atoms)
    {
        // ex GDrawingContainer::computeAtomMap [removed flags]
        for (game::map::DrawingContainer::Iterator_t it = drawings.begin(), end = drawings.end(); it != end; ++it) {
            if (const game::map::Drawing* pd = *it) {
                util::Atom_t a = pd->getTag();
                if (atoms.isAtom(a)) {
                    out.add(a);
                }
            }
        }
    }
}

game::db::Loader::Loader(afl::charset::Charset& cs, interpreter::World& world, afl::string::Translator& tx)
    : m_charset(cs),
      m_world(world),
      m_translator(tx)
{ }

// /** Load starchart database.
//     \param s   Stream to read from
//     \param trn Turn to populate with information
//     \param acceptProperties true to accept properties, false to skip them.
//                Properties are not stored in the trn proper, so this offers
//                to ignore properties from the file, avoiding overwriting
//                global properties. */
void
game::db::Loader::load(afl::io::Stream& in, Turn& turn, Game& game, bool acceptProperties)
{
    // ex game/db.h:loadChartDatabase

    int ignored_entries = 0;

    // Read header
    structures::Header header;
    in.fullRead(afl::base::fromObject(header));
    if (std::memcmp(header.signature, structures::SIGNATURE, sizeof(structures::SIGNATURE)) != 0) {
        throw afl::except::FileFormatException(in, _("File is missing required signature"));
    }
    log().write(afl::sys::LogListener::Debug, LOG_NAME, _("Loading starchart database..."));

    // A ValueLoader
    interpreter::vmio::NullLoadContext loadContext;
    interpreter::vmio::ValueLoader valueLoader(m_charset, loadContext);

    // Read property names
    afl::data::NameMap planetPropertyNames;
    afl::data::NameMap shipPropertyNames;
    valueLoader.loadNames(planetPropertyNames, in, header.numPlanetProperties);
    valueLoader.loadNames(shipPropertyNames, in, header.numShipProperties);

    // Set database turn number
    // ex GGameTurn::addDatabaseTurn
    if (header.turnNumber > turn.getDatabaseTurnNumber()) {
        turn.setDatabaseTurnNumber(header.turnNumber);
    }

    // Read blocks, main loop
    in.setPos(header.dataStart);
    DrawingAtomMap atom_translation;
    dt::BlockHeader blockHeader;
    while (in.read(afl::base::fromObject(blockHeader)) == sizeof(blockHeader)) {
        uint32_t size = blockHeader.size;        /* not const to allow count-down loops */
        const afl::io::Stream::FileSize_t startPos = in.getPos();
        const afl::io::Stream::FileSize_t endPos = startPos + blockHeader.size;

        switch (blockHeader.blockType) {
         case dt::rPlanetHistory: {
            // A single planet record. Can have variable size, as we store more fields than PCC 1.x,
            // and possibly extend it in the future. However, we expect a minimum size.
            if (size >= 93) {
                // 93 = size of planet data, plus timestamps */
                dt::Planet planet;
                afl::base::fromObject(planet).fill(0);
                in.fullRead(afl::base::fromObject(planet).trim(size));

                Packer(turn, m_charset).addPlanet(planet);
            } else {
                /* no known program writes these files */
            }
            break;
         }

         case dt::rShipHistory: {
            if (size >= sizeof(dt::Ship)) {
                // Single ship history entry
                dt::Ship ship;
                in.fullRead(afl::base::fromObject(ship));

                Packer(turn, m_charset).addShip(ship);
            }
            break;
         }

         case dt::rShipTrack: {
            if (size >= sizeof(dt::ShipTrackHeader)) {
                // One header plus many entries
                dt::ShipTrackHeader header;
                dt::ShipTrackEntry entry;
                in.fullRead(afl::base::fromObject(header));
                size -= uint32_t(sizeof(dt::ShipTrackHeader));

                int id = header.id;
                int turnNumber = header.turn;
                while (size >= sizeof(dt::ShipTrackEntry)) {
                    in.fullRead(afl::base::fromObject(entry));
                    size -= uint32_t(sizeof(dt::ShipTrackEntry));
                    Packer(turn, m_charset).addShipTrack(id, turnNumber, entry);
                    --turnNumber;
                }
            }
            break;
         }

         case dt::rMinefield: {
            dt::Minefield dbm;
            while (size >= sizeof(dbm)) {
                in.fullRead(afl::base::fromObject(dbm));
                size -= uint32_t(sizeof(dbm));

                // Add to database
                if (gm::Minefield* mf = turn.universe().minefields().create(dbm.id)) {
                    mf->addReport(gm::Point(dbm.x, dbm.y),
                                  dbm.owner,
                                  dbm.type != 0 ? gm::Minefield::IsWeb : gm::Minefield::IsMine,
                                  gm::Minefield::UnitsKnown,
                                  dbm.units,
                                  dbm.turn,
                                  gm::Minefield::NoReason);
                }
            }
            break;
         }

         case dt::rPainting: {
            afl::io::LimitedStream ss(in.createChild(), startPos, size);
            loadDrawings(ss, turn.universe().drawings(), atom_translation);
            atom_translation.clear();
            break;
         }

         case dt::rAutoBuild: {
            int id = 0;
            while (size >= sizeof(dt::AutobuildSettings)) {
                /* read it */
                dt::AutobuildSettings abs;
                in.fullRead(afl::base::fromObject(abs));
                size -= uint32_t(sizeof(abs));

                /* enter into database */
                ++id;
                if (gm::Planet* pl = turn.universe().planets().get(id)) {
                    for (size_t i = 0; i < NUM_PLANETARY_BUILDING_TYPES; ++i) {
                        pl->setAutobuildGoal(PlanetaryBuilding(i), abs.goal[i]);
                        pl->setAutobuildSpeed(PlanetaryBuilding(i), abs.speed[i]);
                    }
                }
            }
            if (size != 0) {
                log().write(afl::sys::LogListener::Warn, LOG_NAME, _("Autobuild record has unexpected size"));
            }
            break;
         }

         case dt::rShipProperty:
            if (acceptProperties) {
                afl::io::LimitedStream ss(in.createChild(), startPos, size);
                loadPropertyRecord(ss, ShipScope, turn.universe(), shipPropertyNames, m_world.shipPropertyNames(), valueLoader);
            }
            break;

         case dt::rPlanetProperty:
            if (acceptProperties) {
                afl::io::LimitedStream ss(in.createChild(), startPos, size);
                loadPropertyRecord(ss, PlanetScope, turn.universe(), planetPropertyNames, m_world.planetPropertyNames(), valueLoader);
            }
            break;

         case dt::rShipScore: {
            afl::io::LimitedStream ss(in.createChild(), startPos, size);
            loadUnitScoreRecord(ss, ShipScope, turn.universe(), game.shipScores());
            break;
         }

         case dt::rPlanetScore: {
            afl::io::LimitedStream ss(in.createChild(), startPos, size);
            loadUnitScoreRecord(ss, PlanetScope, turn.universe(), game.planetScores());
            break;
         }

         case dt::rPaintingTags: {
            if (!atom_translation.isEmpty()) {
                log().write(afl::sys::LogListener::Warn, LOG_NAME, _("Text record appears at unexpected place"));
            }
            afl::io::LimitedStream ss(in.createChild(), startPos, size);
            atom_translation.clear();
            atom_translation.load(ss, m_charset, m_world.atomTable());
            break;
         }

         case dt::rUfoHistory: {
            if (size >= sizeof(dt::Ufo)) {
                dt::Ufo ufo;
                in.fullRead(afl::base::fromObject(ufo));
                Packer(turn, m_charset).addUfo(ufo);
            }
            break;
         }

         default:
            ++ignored_entries;
        }

        in.setPos(endPos);
    }
    if (ignored_entries != 0) {
        log().write(afl::sys::LogListener::Info, LOG_NAME, afl::string::Format(_("%d database record%!1{s have%| has%} been ignored").c_str(), ignored_entries));
    }
}

// /** Save starchart database.
//     \param s   Stream to save into
//     \param trn Turn to save */
void
game::db::Loader::save(afl::io::Stream& out, Turn& turn, Game& game, const game::spec::ShipList& shipList)
{
    // ex saveChartDatabase
    // Prepare initial header
    const afl::io::Stream::FileSize_t pos = out.getPos();
    structures::Header header;
    afl::base::fromObject(header).fill(0);

    // Write preliminary header
    size_t numPlanetProperties = m_world.planetPropertyNames().getNumNames();
    size_t numShipProperties = m_world.shipPropertyNames().getNumNames();
    header.turnNumber = static_cast<uint16_t>(turn.getTurnNumber());
    header.numPlanetProperties = static_cast<uint16_t>(numPlanetProperties);
    header.numShipProperties = static_cast<uint16_t>(numShipProperties);
    out.fullWrite(afl::base::fromObject(header));
    interpreter::SaveVisitor::saveNames(out, m_world.planetPropertyNames(), numPlanetProperties, m_charset);
    interpreter::SaveVisitor::saveNames(out, m_world.shipPropertyNames(), numShipProperties, m_charset);

    // Prepare final header
    afl::base::Memory<char>(header.signature).copyFrom(structures::SIGNATURE);
    header.dataStart = static_cast<uint16_t>(out.getPos());

    const game::map::Universe& univ = turn.universe();
    RecordState rs;

    // Write the drawings.
    // For drawings that have atoms attached, we must store the atom mapping first, to be able to restore it.
    // Whereas PCC 1.x stores the actual atom values verbatim, we emulate that to support our larger value range.
    {
        DrawingAtomMap map;
        computeAtomMap(map, univ.drawings(), m_world.atomTable());
        if (!map.isEmpty()) {
            startRecord(out, dt::rPaintingTags, rs);
            map.save(out, m_charset, m_world.atomTable(), log(), m_translator);
            endRecord(out, rs);
        }
        startRecord(out, dt::rPainting, rs);
        saveDrawings(out, univ.drawings(), map);
        endRecord(out, rs);
    }

    // Write the minefields.
    // PCC 1.x stores only one minefield per record.
    // This record has been specified as being capable of holding many since ever,
    // and all other CHART.CC programs seem to handle that just fine, so we'll happily store them all in one.
    startRecord(out, dt::rMinefield, rs);
    for (Id_t id = univ.minefields().findNextIndex(0); id != 0; id = univ.minefields().findNextIndex(id)) {
        if (const gm::Minefield* mf = univ.minefields().get(id)) {
            gm::Point pos;
            int owner = 0;
            if (mf->getPosition(pos) && mf->getOwner(owner)) {
                dt::Minefield dbm;
                dbm.id    = static_cast<int16_t>(id);
                dbm.x     = static_cast<int16_t>(pos.getX());
                dbm.y     = static_cast<int16_t>(pos.getY());
                dbm.owner = static_cast<int16_t>(owner);
                dbm.units = mf->getUnitsLastSeen();
                dbm.type  = mf->isWeb();
                dbm.turn  = static_cast<int16_t>(mf->getTurnLastSeen());
                out.fullWrite(afl::base::fromObject(dbm));
            }
        }
    }
    endRecord(out, rs);

    // Write autobuild settings
    startRecord(out, dt::rAutoBuild, rs);
    for (Id_t id = 1, n = univ.planets().size(); id <= n; ++id) {
        dt::AutobuildSettings abs;
        if (const gm::Planet* pl = univ.planets().get(id)) {
            for (size_t i = 0; i < NUM_PLANETARY_BUILDING_TYPES; ++i) {
                abs.goal[i] = static_cast<int16_t>(pl->getAutobuildGoal(PlanetaryBuilding(i)));
                abs.speed[i] = static_cast<int8_t>(pl->getAutobuildSpeed(PlanetaryBuilding(i)));
            }
        } else {
            for (size_t i = 0; i < NUM_PLANETARY_BUILDING_TYPES; ++i) {
                abs.goal[i] = 0;
                abs.speed[i] = 0;
            }
        }
        out.fullWrite(afl::base::fromObject(abs));
    }
    endRecord(out, rs);

    // Write planets
    for (Id_t id = 1, n = univ.planets().size(); id <= n; ++id) {
        const gm::Planet* pl = univ.planets().get(id);
        if (pl != 0 && pl->hasAnyPlanetData()) {
            dt::Planet dbp;
            Packer(turn, m_charset).packPlanet(dbp, *pl);
            startRecord(out, dt::rPlanetHistory, rs);
            out.fullWrite(afl::base::fromObject(dbp));
            endRecord(out, rs);
        }
        savePropertyRecord(out, dt::rPlanetProperty, id, m_world.planetProperties().get(id));
    }

    // Write ships
    for (Id_t id = 1, n = univ.ships().size(); id <= n; ++id) {
        const gm::Ship* sh = univ.ships().get(id);
        if (sh != 0 && sh->hasAnyShipData()) {
            // History Data
            dt::Ship dbs;
            Packer(turn, m_charset).packShip(dbs, *sh);
            startRecord(out, dt::rShipHistory, rs);
            out.fullWrite(afl::base::fromObject(dbs));
            endRecord(out, rs);
        }

        // Track data
        int turnNr = sh->getHistoryNewestLocationTurn();
        if (turnNr > 0) {
            dt::ShipTrackHeader th;
            th.id   = static_cast<int16_t>(sh->getId());
            th.turn = static_cast<int16_t>(turnNr);
            startRecord(out, dt::rShipTrack, rs);
            out.fullWrite(afl::base::fromObject(th));

            while (const gm::ShipHistoryData::Track* p = sh->getHistoryLocation(turnNr)) {
                dt::ShipTrackEntry te;
                te.x       = static_cast<int16_t>(p->x.orElse(-1));
                te.y       = static_cast<int16_t>(p->y.orElse(-1));
                if (turnNr == turn.getTurnNumber()) {
                    // FIXME: this distinction should be done by Ship
                    te.speed   = static_cast<int8_t>(sh->getWarpFactor().orElse(-1));
                    te.heading = static_cast<int16_t>(sh->getHeading().orElse(-1));
                    te.mass    = static_cast<int16_t>(sh->getMass(shipList).orElse(-1));
                } else {
                    te.speed   = static_cast<int8_t>(p->speed.orElse(-1));
                    te.heading = static_cast<int16_t>(p->heading.orElse(-1));
                    te.mass    = static_cast<int16_t>(p->mass.orElse(-1));
                }
                out.fullWrite(afl::base::fromObject(te));
                --turnNr;
            }
            endRecord(out, rs);
        }

        // Property data
        savePropertyRecord(out, dt::rShipProperty, id, m_world.shipProperties().get(id));
    }

    // Write unit scores
    saveUnitScores(out, dt::rPlanetScore, PlanetScope, game.planetScores(), turn.universe());
    saveUnitScores(out, dt::rShipScore,   ShipScope,   game.shipScores(), turn.universe());

    // Write Ufos
    for (Id_t id = univ.ufos().findNextIndex(0); id != 0; id = univ.ufos().findNextIndex(id)) {
        const gm::Ufo* pUfo = const_cast<gm::UfoType&>(univ.ufos()).getObjectByIndex(id);
        if (pUfo != 0 && pUfo->isStoredInHistory()) {
            dt::Ufo ufo;
            Packer(turn, m_charset).packUfo(ufo, *pUfo);

            startRecord(out, dt::rUfoHistory, rs);
            out.fullWrite(afl::base::fromObject(ufo));
            endRecord(out, rs);
        }
    }

    // Write final header
    out.setPos(pos);
    out.fullWrite(afl::base::fromObject(header));
}

inline afl::sys::LogListener&
game::db::Loader::log()
{
    return m_world.logListener();
}


// /** Load drawings from file.
//     \param in     [in] Stream to read from (must yield EOF at end of data)
//     \param map    [in] Maps external tags to internal
//     \param expire [in] Current turn for expiration; -1 to not expire anything */
void
game::db::Loader::loadDrawings(afl::io::Stream& in, game::map::DrawingContainer& container, const DrawingAtomMap& map)
{
    // ex GDrawingContainer::load
    dt::Drawing d;
    while (in.read(afl::base::fromObject(d)) == sizeof(d)) {
        /* Parse what we have so far: */
        const int kind             = d.type & 127;
        const bool has_comment     = (d.type & 0x80) != 0;

        /* If it has a comment, read that too */
        String_t comment;
        if (has_comment) {
            comment = util::loadPascalString(in, m_charset);
        }

        /* Check type */
        // FIXME: isolate internal/external representation
        if (kind < 0 || kind > gm::Drawing::MarkerDrawing) {
            continue;
        }

        /* Might be valid */
        std::auto_ptr<gm::Drawing> t(new gm::Drawing(gm::Point(d.x1, d.y1), gm::Drawing::Type(kind)));
        t->setColor(convertColor(d.color));
        t->setTag(map.get(d.tag));
        t->setExpire(d.expirationTurn);
        t->setComment(comment);
        switch (kind) {
         case gm::Drawing::LineDrawing:
         case gm::Drawing::RectangleDrawing:
            t->setPos2(gm::Point(d.x2, d.y2));
            break;
         case gm::Drawing::CircleDrawing:
            t->setCircleRadius(d.x2);
            break;
         case gm::Drawing::MarkerDrawing:
            t->setMarkerKind(d.x2);
            break;
        }
        container.addNew(t.release());
    }
}

void
game::db::Loader::loadPropertyRecord(afl::io::Stream& in, Scope scope, game::map::Universe& univ, const afl::data::NameMap& dbNames, afl::data::NameMap& liveNames, interpreter::vmio::ValueLoader& valueLoader)
{
    // ex game/db.cc:loadPropertyRecord

    // Header: Id + count
    dt::PropertyHeader header;
    if (in.read(afl::base::fromObject(header)) != sizeof(header)) {
        log().write(afl::sys::LogListener::Warn, LOG_NAME, _("Property record has unexpected size and has been ignored"));
        return;
    }

    // Get pointer to live property table
    afl::data::Segment* liveProperties = 0;
    int id = header.id;
    switch (scope) {
     case ShipScope:
        if (univ.ships().get(id) != 0) {
            liveProperties = m_world.shipProperties().create(id);
        }
        break;

     case PlanetScope:
        if (univ.planets().get(id) != 0) {
            liveProperties = m_world.planetProperties().create(id);
        }
        break;
    }
    if (liveProperties == 0) {
        log().write(afl::sys::LogListener::Warn, LOG_NAME, afl::string::Format(_("Property record has invalid Id (%d) and has been ignored").c_str(), id));
        return;
    }

    // Read data into temporary store first
    afl::data::Segment dbValues;
    valueLoader.load(dbValues, in, 0, header.numProperties);

    // Copy to game object
    // FIXME: why is NameMap::Index_t a uint32_t?
    size_t limit = std::min(dbValues.size(), size_t(dbNames.getNumNames()));
    for (size_t i = 0, n = limit; i < n; ++i) {
        afl::data::Value* dbValue = dbValues[i];
        const String_t&   dbName  = dbNames.getNameByIndex(i);

        // Address remapping
        afl::data::Segment::Index_t liveIndex = liveNames.getIndexByName(dbName);
        if (dbValue != 0 && liveIndex == liveNames.nil) {
            liveIndex = liveNames.add(dbName);
        }

        // Store value
        if (liveIndex != liveNames.nil) {
            liveProperties->set(liveIndex, dbValue);
        }
    }
}

void
game::db::Loader::loadUnitScoreRecord(afl::io::Stream& in, Scope scope, game::map::Universe& univ, UnitScoreDefinitionList& defs)
{
    // ex game/db.cc:loadUnitScoreRecord
    dt::UnitScoreHeader ush;
    if (readUnitScoreHeader(in, ush)) {
        /* Read content */
        UnitScoreDefinitionList::Definition def;
        def.name = m_charset.decode(ush.name);
        def.id = ush.scoreType;
        def.limit = ush.scoreLimit;
        UnitScoreDefinitionList::Index_t index = defs.add(def);

        dt::UnitScoreEntry entry;
        while (in.read(afl::base::fromObject(entry)) == sizeof(entry)) {
            int id = entry.id;
            UnitScoreList* target = 0;
            switch (scope) {
             case ShipScope:
                if (gm::Ship* sh = univ.ships().get(id)) {
                    target = &sh->unitScores();
                }
                break;
             case PlanetScope:
                if (gm::Planet* pl = univ.planets().get(id)) {
                    target = &pl->unitScores();
                }
                break;
            }
            if (target != 0) {
                target->merge(index, entry.score, entry.turn);
            }
        }
    } else {
        log().write(afl::sys::LogListener::Warn, LOG_NAME, _("Unit score record is invalid"));
    }
}

// /** Save drawings in file
//     \param out [in] Stream to write to
//     \param map [in] Maps internal tags to external */
void
game::db::Loader::saveDrawings(afl::io::Stream& out, const game::map::DrawingContainer& container, const DrawingAtomMap& map)
{
    // ex GDrawingContainer::save
    for (game::map::DrawingContainer::Iterator_t i = container.begin(), end = container.end(); i != end; ++i) {
        if (game::map::Drawing* p = *i) {
            dt::Drawing d;

            d.type           = static_cast<int8_t>(p->getType());
            d.color          = convertToExternalColor(p->getColor());
            d.x1             = static_cast<int16_t>(p->getPos().getX());
            d.y1             = static_cast<int16_t>(p->getPos().getY());
            d.x2             = static_cast<int16_t>(p->getPos2().getX());
            d.y2             = static_cast<int16_t>(p->getPos2().getY());
            d.tag            = map.getExternalValue(p->getTag());
            d.expirationTurn = static_cast<int16_t>(p->getExpire());

            if (!p->getComment().empty()) {
                d.type |= 0x80;
                out.fullWrite(afl::base::fromObject(d));
                util::storePascalStringTruncate(out, p->getComment(), m_charset);
            } else {
                out.fullWrite(afl::base::fromObject(d));
            }
        }
    }
}

void
game::db::Loader::savePropertyRecord(afl::io::Stream& out, uint16_t type, game::Id_t id, const afl::data::Segment* pData)
{
    // ex writePropertyRecord
    if (pData != 0) {
        size_t n = pData->getNumUsedSlots();
        if (n != 0) {
            dt::PropertyHeader ph;
            ph.id            = static_cast<uint16_t>(id);
            ph.numProperties = static_cast<uint16_t>(n);

            RecordState rs;
            startRecord(out, type, rs);
            out.fullWrite(afl::base::fromObject(ph));

            interpreter::vmio::NullSaveContext ctx;
            interpreter::SaveVisitor::save(out, *pData, n, m_charset, ctx);
            endRecord(out, rs);
        }
    }
}

void
game::db::Loader::saveUnitScores(afl::io::Stream& out, uint16_t type, Scope scope, const UnitScoreDefinitionList& defs, const game::map::Universe& univ)
{
    // ex writeUnitScores
    // // FIXME: it would make sense to drop empty records. This implementation will
    // // write them out, keeping outdated score definitions around for ages.
    for (UnitScoreDefinitionList::Index_t index = 0, maxIndex = defs.getNumScores(); index < maxIndex; ++index) {
        if (const UnitScoreDefinitionList::Definition* pDef = defs.get(index)) {
            // Record control
            RecordState rs;
            startRecord(out, type, rs);

            // Header
            dt::UInt16_t size;
            dt::UnitScoreHeader header;
            size = sizeof(dt::UnitScoreHeader);
            header.name = m_charset.encode(afl::string::toMemory(pDef->name));
            header.scoreType = pDef->id;
            header.scoreLimit = pDef->limit;
            out.fullWrite(afl::base::fromObject(size));
            out.fullWrite(afl::base::fromObject(header));

            switch (scope) {
             case ShipScope:
                saveTypeUnitScores(out, univ.ships(), index);
                break;

             case PlanetScope:
                saveTypeUnitScores(out, univ.planets(), index);
                break;
            }

            endRecord(out, rs);
        }
    }
}

