/**
  *  \file game/db/fleetloader.cpp
  *  \brief Class game::db::FleetLoader
  *
  *  <b>PCC2 comment re Fleets:</b>
  *
  *  Fleets are groups of ships with a common waypoint.
  *
  *  Fleet attributes are stored in GShip objects:
  *  - the fleet Id is the ship Id of the leader
  *  - the fleet has an optional name/comment which is stored in the leader
  *
  *  Fleet invariants:
  *  - if the leader has mission "Intercept X", all members have mission "Intercept X",
  *    and the same warp speed
  *    . if X is a member of that fleet, it has no waypoint and no speed.
  *  - otherwise, all ships have the same waypoint and the same warp speed.
  *    . as an exception, a ship that is being towed has no waypoint and no speed.
  *
  *  Loading:
  *  - loadFleets(): Fleet data is loaded from fleetX.cc. At this point, everything
  *    we have to make sure is that we do not stomp on other fleetX.cc which might
  *    be loaded in parallel, but we do not have access to derived information,
  *    just shipX.dat data and source flags.
  *  - postprocessFleet(): This is called after ship derived information has been
  *    set. At this point, we can assume that data is syntactically correct, but
  *    it could still happen that a fleet member might been gone. However, we can
  *    use the full API and data set here.
  */

#include <cstring>
#include "game/db/fleetloader.hpp"
#include "afl/base/growablememory.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/bits/pack.hpp"
#include "afl/bits/uint16le.hpp"
#include "afl/bits/value.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/format.hpp"
#include "game/map/ship.hpp"
#include "util/io.hpp"
#include "util/translation.hpp"

using afl::string::Format;

namespace {
    typedef afl::bits::Value<afl::bits::UInt16LE> UInt16_t;

    const char*const FLEETFILE = "fleet%d.cc";

    const uint16_t Name_Flag = 0x8000;

    struct FleetFileHeader {
        uint8_t magic[8];
        uint8_t version;
    };
    static_assert(sizeof(FleetFileHeader) == 9, "sizeof FleetFileHeader");

    static const char MAGIC[] = { 'C', 'C', 'f', 'l', 'e', 'e', 't', 26 };
    static_assert(sizeof(MAGIC) == sizeof(FleetFileHeader().magic), "sizeof MAGIC");


    /** Loading: Check whether ship is a valid fleet member for a given player.
        For use during construction.
        This checks source flags, not isPlayable(), because only source flags are valid at this time.
        \param sh Ship to check
        \param player Player to check for */
    bool isValidFleetMember(const game::map::Ship& sh, const int player)
    {
        // ex game/fleet.cc:isValidFleetMember
        int shipOwner;
        return sh.getShipSource().contains(player)
            && sh.getOwner(shipOwner)
            && shipOwner == player
            && sh.getFleetNumber() == 0;
    }

    /** Loading: Build fleet from data.
        This takes fleet membership data loaded from the fleet file and places it into the universe,
        avoiding conflicts with existing fleets.
        \param univ     [in/out] Universe
        \param fid      [in] Fleet Id, corresponding to an existing fleet
        \param player   [in] Player number
        \param fleetNrs [in/out] Fleet numbers loaded from file, indexed [0,NUM_SHIPS). Used slots will be zeroed.
        \param nameNrs  [in/out] Name pointers, indexed [0,NUM_SHIPS). A nonzero entry means to load a name.
                        Positive means to make this name the name of the corresponding fleet,
                        negative means to ignore the name. */
    void buildFleet(game::map::Universe& univ,
                    const uint16_t fid,
                    const int player,
                    const afl::base::Memory<uint16_t> fleetNrs,
                    const afl::base::Memory<int16_t> nameNrs)
    {
        // ex game/fleet.cc:buildFleet
        // Find new fleet Id
        uint16_t newfid = 0;
        game::map::Ship* fidShip = univ.ships().get(fid);
        if (fidShip != 0 && *fleetNrs.at(fid-1) == fid && isValidFleetMember(*fidShip, player)) {
            // Keep fleet Id (also keep names)
            newfid = fid;
        } else {
            for (uint16_t i = 1; i <= fleetNrs.size(); ++i) {
                if (*fleetNrs.at(i-1) == fid
                    && univ.ships().get(i) != 0
                    && isValidFleetMember(*univ.ships().get(i), player))
                {
                    // Found another ship that can get this fleet Id
                    newfid = i;
                    break;
                }
            }
        }

        // Find new name
        if (newfid == 0) {
            // Nothing found, fleet got completely annihilated, we have to delete it.
            if (int16_t* pNameNr = nameNrs.at(fid-1)) {
                if (*pNameNr != 0) {
                    *pNameNr = -1;
                }
            }
        } else if (newfid != fid) {
            // Id changed, move name:
            // - Place fid's name in newfid.
            if (int16_t* pNameNr = nameNrs.at(fid-1)) {
                if (*pNameNr != 0) {
                    *pNameNr = newfid;
                }
            }
            // - Discard newfid's name.
            if (int16_t* pNameNr = nameNrs.at(newfid-1)) {
                if (*pNameNr != 0) {
                    *pNameNr = -1;
                }
            }
        } else {
            // Id remains the same.
        }

        // Build fleet and strike out of fleetNrs
        for (size_t i = 1; i <= fleetNrs.size(); ++i) {
            if (*fleetNrs.at(i-1) == fid) {
                game::map::Ship* sh = univ.ships().get(game::Id_t(i));
                if (sh != 0 && isValidFleetMember(*sh, player)) {
                    sh->setFleetNumber(newfid);
                }
                *fleetNrs.at(i-1) = 0;
            }
        }
    }
}

// Constructor.
game::db::FleetLoader::FleetLoader(afl::charset::Charset& cs)
    : m_charset(cs)
{ }

// Load fleets.
void
game::db::FleetLoader::load(afl::io::Directory& dir, game::map::Universe& univ, int playerNumber)
{
    // ex game/fleet.cc:loadFleets, ccmain.pas:LoadFleets
    // Open file
    afl::base::Ptr<afl::io::Stream> s = dir.openFileNT(Format(FLEETFILE, playerNumber), afl::io::FileSystem::OpenRead);
    if (s.get() == 0) {
        return;
    }

    // Read header
    FleetFileHeader header;
    s->fullRead(afl::base::fromObject(header));
    if (std::memcmp(header.magic, MAGIC, sizeof(MAGIC)) != 0 || (header.version != 0 && header.version != 1)) {
        throw afl::except::FileFormatException(*s, _("Invalid file header"));
    }

    // Get count
    uint16_t nFleets = 500;
    if (header.version == 1) {
        UInt16_t count;
        s->fullRead(count.m_bytes);
        nFleets = count;
    }

    // Get bits
    afl::base::GrowableBytes_t fleetData;
    fleetData.resize(nFleets * 2);
    s->fullRead(fleetData);

    // Extract fleet numbers.
    afl::base::GrowableMemory<uint16_t> fleetNrs;
    fleetNrs.resize(nFleets);
    afl::bits::unpackArray<afl::bits::UInt16LE>(fleetNrs, fleetData);

    // Prepare names. nameNrs is nonzero to load a name at this place,
    // positive to load it into that ship.
    afl::base::GrowableMemory<int16_t> nameNrs;
    nameNrs.resize(nFleets);
    for (size_t i = 0; i < nFleets; ++i) {
        if (*fleetNrs.at(i) & Name_Flag) {
            *fleetNrs.at(i) = uint16_t(*fleetNrs.at(i) & ~Name_Flag);
            if (*fleetNrs.at(i) == i+1) {
                // Fleet leader
                *nameNrs.at(i) = int16_t(i+1);
            } else {
                // Fleet member (comment invalid here)
                *nameNrs.at(i) = -1;
            }
        } else {
            // No comment
            *nameNrs.at(i) = 0;
        }
    }

    // Postprocess the fleets
    for (size_t i = 0; i < nFleets; ++i) {
        if (uint16_t nr = *fleetNrs.at(i)) {
            buildFleet(univ, nr, playerNumber, fleetNrs, nameNrs);
        }
    }

    // Load comments
    for (size_t i = 0; i < nFleets; ++i) {
        if (*nameNrs.at(i) != 0) {
            String_t comment = util::loadPascalString(*s, m_charset);
            if (*nameNrs.at(i) > 0) {
                if (game::map::Ship* sh = univ.ships().get(*nameNrs.at(i))) {
                    sh->setFleetName(comment);
                }
            }
        }
    }
}

// Save fleets.
void
game::db::FleetLoader::save(afl::io::Directory& dir, const game::map::Universe& univ, int playerNumber)
{
    // ex game/fleet.cc:saveFleets
    // Limit: process at least 500 ships even if universe has fewer.
    const Id_t maxShipId = std::max(500, univ.ships().size());

    // Build fleet list and find highest fleet member
    afl::base::GrowableMemory<UInt16_t> fleetNrs;
    fleetNrs.resize(maxShipId);
    fleetNrs.toBytes().fill(0);

    Id_t highestFleetMember = 0;
    for (Id_t i = 1; i <= maxShipId; ++i) {
        if (const game::map::Ship* pShip = univ.ships().get(i)) {
            int shipOwner;
            if (pShip->getShipSource().contains(playerNumber) && pShip->getOwner(shipOwner) && shipOwner == playerNumber) {
                uint16_t fleetNr = static_cast<uint16_t>(pShip->getFleetNumber());
                if (fleetNr != 0) {
                    if (pShip->isFleetLeader() && !pShip->getFleetName().empty()) {
                        fleetNr |= Name_Flag;
                    }
                    *fleetNrs.at(i-1) = fleetNr;
                    highestFleetMember = i;
                }
            }
        }
    }

    // If there is no fleet, erase the fleet file
    const String_t fileName = afl::string::Format(FLEETFILE, playerNumber);
    if (highestFleetMember == 0) {
        dir.eraseNT(fileName);
        return;
    }

    // Create file
    afl::base::Ref<afl::io::Stream> s = dir.openFile(fileName, afl::io::FileSystem::Create);

    // Decide upon file format (Host999 or normal)
    FleetFileHeader header;
    std::memcpy(header.magic, MAGIC, sizeof(MAGIC));
    Id_t numFleets;
    if (highestFleetMember <= 500) {
        header.version = 0;
        numFleets = 500;
        s->fullWrite(afl::base::fromObject(header));
    } else {
        header.version = 1;
        numFleets = maxShipId;
        s->fullWrite(afl::base::fromObject(header));

        UInt16_t rawCount;
        rawCount = static_cast<uint16_t>(numFleets);
        s->fullWrite(rawCount.m_bytes);
    }

    // Fleet data
    s->fullWrite(fleetNrs.subrange(0, numFleets).toBytes());

    // Comments
    for (Id_t i = 1; i <= numFleets; ++i) {
        if ((*fleetNrs.at(i-1) & Name_Flag) != 0) {
            const game::map::Ship* pShip = univ.ships().get(i);
            util::storePascalStringTruncate(*s, pShip ? pShip->getFleetName() : String_t(), m_charset);
        }
    }
}
