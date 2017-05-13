/**
  *  \file game/db/loader.cpp
  */

#include <cstring>
#include <algorithm>
#include "game/db/loader.hpp"
#include "game/db/structures.hpp"
#include "afl/except/fileformatexception.hpp"
#include "util/translation.hpp"
#include "afl/data/namemap.hpp"
#include "interpreter/vmio/valueloader.hpp"
#include "interpreter/vmio/nullloadcontext.hpp"
#include "game/db/drawingatommap.hpp"
#include "afl/io/limitedstream.hpp"
#include "util/io.hpp"
#include "afl/base/countof.hpp"
#include "afl/string/format.hpp"

namespace {
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

}

game::db::Loader::Loader(afl::charset::Charset& cs, interpreter::World& world)
    : m_charset(cs),
      m_world(world)
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
    structures::BlockHeader blockHeader;
    while (in.read(afl::base::fromObject(blockHeader)) == sizeof(blockHeader)) {
        uint32_t size = blockHeader.size;        /* not const to allow count-down loops */
        const afl::io::Stream::FileSize_t startPos = in.getPos();
        const afl::io::Stream::FileSize_t endPos = startPos + blockHeader.size;

        switch (blockHeader.blockType) {
    //      case rPlanetHistory: {
    //         /* a single planet record. Can have variable size, as we
    //            store more fields than PCC 1.x, and possibly extend it
    //            in the future. However, we expect a minimum size. */
    //         if (size >= 93) {
    //             /* 93 = size of planet data, plus timestamps */
    //             char tmp[TDbPlanet::size];
    //             zeroFill(tmp);
    //             if (size > TDbPlanet::size)
    //                 size = TDbPlanet::size;
    //             s.readT(tmp, size);

    //             TDbPlanet dbp;
    //             dbp.known_to_have_natives = 0;
    //             getPartialStructure(tmp, dbp, size);
    //             if (univ.isValidPlanetId(dbp.planet.planet_id))
    //                 univ.getPlanet(dbp.planet.planet_id).addHistoryData(dbp);
    //         } else {
    //             /* no known program writes these files */
    //         }
    //         break;
    //      }
    //      case rShipHistory: {
    //         if (size >= TDbShip::size) {
    //             /* Single ship history entry */
    //             TDbShip ship;
    //             getStructureT(s, ship);
    //             if (univ.isValidShipId(ship.ship.ship_id))
    //                 univ.getShip(ship.ship.ship_id).addShipHistoryData(ship);
    //         }
    //         break;
    //      }
    //      case rShipTrack: {
    //         if (size >= TDbShipTrack::size) {
    //             /* One TDbShipTrack, plus many TDbShipTrackEntry's */
    //             TDbShipTrack header;
    //             TDbShipTrackEntry entry;
    //             getStructureT(s, header);
    //             size -= header.size;
    //             if (univ.isValidShipId(header.id)) {
    //                 int turn = header.ref_turn;
    //                 while (size >= TDbShipTrackEntry::size) {
    //                     getStructureT(s, entry);
    //                     size -= entry.size;
    //                     univ.getShip(header.id).addShipTrackEntry(entry, turn);
    //                     --turn;
    //                 }
    //             }
    //         }
    //         break;
    //      }
    //      case rMinefield:
    //         while (size >= TDbMinefield::size) {
    //             /* read it */
    //             TDbMinefield dbm;
    //             getStructureT(s, dbm);
    //             size -= TDbMinefield::size;

    //             /* enter into database */
    //             TMinefieldReport report;
    //             report.x               = dbm.x;
    //             report.y               = dbm.y;
    //             report.owner           = dbm.owner;
    //             report.web             = dbm.type;
    //             report.units_known     = true;
    //             report.type_known      = true;
    //             report.radius_or_units = dbm.units;
    //             report.turn            = dbm.turn;
    //             report.why             = GMinefield::mfa_None;
    //             univ.ty_minefields.addMinefieldReport(dbm.id, report);
    //         }
    //         break;
         case structures::rPainting: {
            afl::io::LimitedStream ss(in.createChild(), startPos, size);
            loadDrawings(ss, turn.universe().drawings(), atom_translation);
            atom_translation.clear();
            break;
         }

         case structures::rAutoBuild: {
            int id = 0;
            while (size >= sizeof(structures::AutobuildSettings)) {
                /* read it */
                structures::AutobuildSettings abs;
                in.fullRead(afl::base::fromObject(abs));
                size -= uint32_t(sizeof(abs));

                /* enter into database */
                ++id;
                if (game::map::Planet* pl = turn.universe().planets().get(id)) {
                    pl->setAutobuildGoal(MineBuilding, abs.goal[0]);
                    pl->setAutobuildSpeed(MineBuilding, abs.speed[0]);
                    pl->setAutobuildGoal(FactoryBuilding, abs.goal[1]);
                    pl->setAutobuildSpeed(FactoryBuilding, abs.speed[1]);
                    pl->setAutobuildGoal(DefenseBuilding, abs.goal[2]);
                    pl->setAutobuildSpeed(DefenseBuilding, abs.speed[2]);
                    pl->setAutobuildGoal(BaseDefenseBuilding, abs.goal[3]);
                    pl->setAutobuildSpeed(BaseDefenseBuilding, abs.speed[3]);
                }
            }
            if (size != 0) {
                log().write(afl::sys::LogListener::Warn, LOG_NAME, _("Autobuild record has unexpected size"));
            }
            break;
         }

         case structures::rShipProperty:
            if (acceptProperties) {
                afl::io::LimitedStream ss(in.createChild(), startPos, size);
                loadPropertyRecord(ss, ShipScope, turn.universe(), shipPropertyNames, m_world.shipPropertyNames(), valueLoader);
            }
            break;

         case structures::rPlanetProperty:
            if (acceptProperties) {
                afl::io::LimitedStream ss(in.createChild(), startPos, size);
                loadPropertyRecord(ss, PlanetScope, turn.universe(), planetPropertyNames, m_world.planetPropertyNames(), valueLoader);
            }
            break;

         case structures::rShipScore: {
            afl::io::LimitedStream ss(in.createChild(), startPos, size);
            loadUnitScoreRecord(ss, ShipScope, turn.universe(), game.shipScores());
            break;
         }

         case structures::rPlanetScore: {
            afl::io::LimitedStream ss(in.createChild(), startPos, size);
            loadUnitScoreRecord(ss, PlanetScope, turn.universe(), game.planetScores());
            break;
         }

         case structures::rPaintingTags: {
            if (!atom_translation.isEmpty()) {
                log().write(afl::sys::LogListener::Warn, LOG_NAME, _("Text record appears at unexpected place"));
            }
            afl::io::LimitedStream ss(in.createChild(), startPos, size);
            atom_translation.clear();
            atom_translation.load(ss, m_charset, m_world.atomTable());
            break;
         }
    //      case rUfoHistory: {
    //         if (size >= TDbUfo::size) {
    //             TDbUfo ufo;
    //             getStructureT(s, ufo);
    //             univ.ty_ufos.addHistoryData(ufo);
    //         }
    //         break;
    //      }
         default:
            ++ignored_entries;
        }

        in.setPos(endPos);
    }
    if (ignored_entries != 0) {
        log().write(afl::sys::LogListener::Info, LOG_NAME, afl::string::Format(_("%d database record%!1{s have%| has%} been ignored").c_str(), ignored_entries));
    }
}

afl::sys::LogListener&
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
    structures::DatabaseDrawing d;
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
        if (kind < 0 || kind > game::map::Drawing::MarkerDrawing) {
            continue;
        }

        /* Might be valid */
        std::auto_ptr<game::map::Drawing> t(new game::map::Drawing(game::map::Point(d.x1, d.y1), game::map::Drawing::Type(kind)));
        t->setColor(convertColor(d.color));
        t->setTag(map.get(d.tag));
        t->setExpire(d.expirationTurn);
        t->setComment(comment);
        switch (kind) {
         case game::map::Drawing::LineDrawing:
         case game::map::Drawing::RectangleDrawing:
            t->setPos2(game::map::Point(d.x2, d.y2));
            break;
         case game::map::Drawing::CircleDrawing:
            t->setCircleRadius(d.x2);
            break;
         case game::map::Drawing::MarkerDrawing:
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
    structures::PropertyHeader header;
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
    structures::UnitScoreHeader ush;
    if (readUnitScoreHeader(in, ush)) {
        /* Read content */
        UnitScoreDefinitionList::Definition def;
        def.name = m_charset.decode(ush.name);
        def.id = ush.scoreType;
        def.limit = ush.scoreLimit;
        UnitScoreDefinitionList::Index_t index = defs.add(def);

        structures::UnitScoreEntry entry;
        while (in.read(afl::base::fromObject(entry)) == sizeof(entry)) {
            int id = entry.id;
            UnitScoreList* target = 0;
            switch (scope) {
             case ShipScope:
                if (game::map::Ship* sh = univ.ships().get(id)) {
                    target = &sh->unitScores();
                }
                break;
             case PlanetScope:
                if (game::map::Planet* pl = univ.planets().get(id)) {
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
