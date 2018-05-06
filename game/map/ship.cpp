/**
  *  \file game/map/ship.cpp
  */

#include "game/map/ship.hpp"
#include "afl/string/format.hpp"
#include "game/map/universe.hpp"
#include "game/parser/messagevalue.hpp"
#include "util/math.hpp"

namespace {
    template<typename PropertyType, typename ValueType>
    void updateField(int& fieldTime, int time, PropertyType& fieldValue, ValueType value)
    {
        if (fieldTime <= time || !fieldValue.isValid()) {
            fieldValue = value;
            if (fieldTime < time) {
                fieldTime = time;
            }
        }
    }
}


game::map::Ship::Ship(int id)
    : MapObject(),
      m_id(id),
      m_scannedMass(),
      m_scannedHeading(),
      m_specialFunctions(),
      m_remoteControlFlag(),
      m_kind(NoShip),
      m_fleetNumber(0),
      m_fleetName(),
      m_currentData(),
      m_historyData(),
      m_shipSource(),
      m_targetSource(),
      m_xySource(),
      m_unitScores()
{
    m_historyTimestamps[0] = 0;
    m_historyTimestamps[1] = 0;
}

game::map::Ship::~Ship()
{ }

void
game::map::Ship::addCurrentShipData(const ShipData& data, PlayerSet_t source)
{
    m_currentData = data;
    m_shipSource += source;
// FIXME: missing behaviours
// /** Add SHIP.DAT entry. This assumes that if we see a ship through
//     several .dat files, we get the same one each time.
//     \param data [in] Parsed .dat file entry
//     \param source [in] Source flags, i.e. bitfield of which file this record is from */
// void
// GShip::addShipData(const TShip& data, GPlayerSet source)
// {
//     // Set hull through regular setter to get reset behaviour
//     setHullId(mp16_t(data.hull));
// }
}

// FIXME: retire
// void
// game::map::Ship::addPreviousShipData(const ShipData& data) REVERTER
// {
//     m_previousData = data;
// }

// /** Add TARGET.DAT entry. This assumes that if we see a ship through
//     several .dat files, we get the same one each time. This does, however,
//     handle the case that one record is seen through PHost's ship name filter
//     and the other one isn't, and uses the better ship name.
//     \param target [in] Parsed TARGET.DAT file entry
//     \param source [in] Source flags, i.e. bitfield of which file this record is from */
// void
// GShip::addTargetData(const TShipTarget& target, GPlayerSet source)
// {
//     // Record that we know it
//     target_source |= source;

//     // Update ship unless we already saw it this turn
//     if (ship_source.empty()) {
//         // Set hull through regular setter to get reset behaviour
//         setHullId(mp16_t(target.hull));

//         // Set remainder
//         ShipInfo& info = createShipInfo();
//         info.data.owner       = target.owner;
//         info.data.warp        = target.warp;
//         info.data.x           = target.x;
//         info.data.y           = target.y;
//         info.data.waypoint_dx = mn16_t().getRawValue();
//         info.data.waypoint_dy = mn16_t().getRawValue();
//         info.heading          = target.heading;

//         // If the new target is better than the one we have, use that
//         if (isDummyName(info.data.name, map_info.id) && !isDummyName(target.name, map_info.id)) {
//             std::memcpy(info.data.name, target.name, sizeof(target.name));
//         }
//     }
// }

void
game::map::Ship::addShipXYData(Point pt, int owner, int mass, PlayerSet_t source)
{
    // ex GShip::addShipXYData
    // Record that we know it
    m_xySource += source;

    // Update ship.
    // FIXME: if m_shipSource.empty() but existing values are invalid, initialize them anyway for order independence
    if (m_shipSource.empty()) {
        m_currentData.x          = pt.getX();
        m_currentData.y          = pt.getY();
        m_currentData.waypointDX = NegativeProperty_t();
        m_currentData.waypointDY = NegativeProperty_t();
        m_currentData.owner      = owner;
        m_scannedMass            = mass;
    }
}

void
game::map::Ship::addMessageInformation(const game::parser::MessageInformation& info, PlayerSet_t source)
{
    // ex GShip::addMessageInformation (expanded!)
    namespace gp = game::parser;

    // Check acceptance of information for possibly current ship
    bool isCurrent = !m_shipSource.empty();
    const int turn = info.getTurnNumber();

    for (gp::MessageInformation::Iterator_t i = info.begin(); i != info.end(); ++i) {
        if (gp::MessageIntegerValue_t* iv = dynamic_cast<gp::MessageIntegerValue_t*>(*i)) {
            switch (iv->getIndex()) {
             case gp::mi_Owner:
                if (!isCurrent) {
                    updateField(m_historyTimestamps[RestTime], turn, m_currentData.owner, iv->getValue());
                }
                break;

             case gp::mi_Speed:
                // FIXME: add intelligence: merge into history track
                if (!isCurrent) {
                    updateField(m_historyTimestamps[RestTime], turn, m_currentData.warpFactor, iv->getValue());
                }
                break;

             case gp::mi_X:
                // FIXME: add intelligence: merge into history track
                if (!isCurrent) {
                    updateField(m_historyTimestamps[RestTime], turn, m_currentData.x, iv->getValue());
                }
                break;

             case gp::mi_Y:
                // FIXME: add intelligence: merge into history track
                if (!isCurrent) {
                    updateField(m_historyTimestamps[RestTime], turn, m_currentData.y, iv->getValue());
                }
                break;

             case gp::mi_ShipHull:
                if (!isCurrent) {
                    updateField(m_historyTimestamps[RestTime], turn, m_currentData.hullType, iv->getValue());
                }
                break;

             case gp::mi_Heading:
                // FIXME: add intelligence: merge into history track
                if (!isCurrent) {
                    updateField(m_historyTimestamps[RestTime], turn, m_scannedHeading, iv->getValue());
                }
                break;

             // FIXME:
             // case gp::mi_ShipRemoteFlag:
             //    addRCEntry(iv->getValue());
             //    break;

             default:
                break;
            }
        } else if (gp::MessageStringValue_t* sv = dynamic_cast<gp::MessageStringValue_t*>(*i)) {
            switch (sv->getIndex()) {
             case gp::ms_Name:
                if (isCurrent) {
                    updateField(m_historyTimestamps[RestTime], turn, m_currentData.friendlyCode, sv->getValue());
                }
                break;

             default:
                break;
            }
        } else {
            // what?
        }
    }

    // Mark ship dirty
    markDirty();

    // Count as target
    // Rule-wise, this value is more or less irrelevant once m_shipSource is set, but it helps in reconstructing target files.
    m_targetSource += source;
}

void
game::map::Ship::getCurrentShipData(ShipData& out) const
{
    out = m_currentData;
}


// /** Do internal checks for this ship.
//     Internal checks do not require a partner to interact with.
//     This will fix the problems, and display appropriate messages.
//     It will also fill in the ship kind. */
void
game::map::Ship::internalCheck()
{
    // ex GShip::internalCheck
    // figure out what kind we are
    if (!m_shipSource.empty()) {
        // current ship
        m_kind = CurrentShip;
    } else if (!m_targetSource.empty()) {
        // current target
        m_kind = CurrentTarget;
    } else if (!m_xySource.empty()) {
        // nonvisual contact
        m_kind = CurrentUnknown;
    } else if (m_currentData.owner.isValid()) {
        // history ship
        m_kind = HistoryShip;
    } else {
        // unknown
        m_kind = NoShip;
    }

    // FIXME: c2ng additional checks:
    // - if it's not CurrentShip, copy current to previous

    // FIXME: things that could be checked in here:
    // - make sure owner is known, nonzero for everything but NoShip
}

// /** Combined checks, phase 1.
//     This will do all post-processing which needs a partner to interact with.
//     It requires the playability to be filled in. */
void
game::map::Ship::combinedCheck1(Universe& univ, int turnNumber)
{
    // FIXME: remove parameter?
    (void) univ;
    // ex GShip::combinedCheck1
    // Update ages
    if (hasFullShipData()) {
        m_historyTimestamps[MilitaryTime] = m_historyTimestamps[RestTime] = turnNumber;
    }

    // Update ship track.
    // The simplest way is to generate a whole new record and have the regular code assimilate it.
    if (isVisible()) {
        // TDbShipTrackEntry e;
        // e.x       = ship_info->data.x;
        // e.y       = ship_info->data.y;
        // e.speed   = getWarp().getRawValue();
        // e.heading = getHeading().getRawValue();
        // e.mass    = getMass().getRawValue();
        // addShipTrackEntry(e, univ.getTurnNumber());
    }
}


String_t
game::map::Ship::getName(Name which, afl::string::Translator& tx, InterpreterInterface& iface) const
{
    // ex GShip::getName
    // Figure out plain name
    String_t plainName;
    m_currentData.name.get(plainName);

    // Is that just what we wanted?
    if (which == PlainName) {
        return plainName;
    }

    // Add detail
    String_t result;
    if (plainName.empty()) {
        result = afl::string::Format(tx.translateString("Ship #%d").c_str(), m_id);
    } else {
        result = afl::string::Format(tx.translateString("Ship #%d: %s").c_str(), m_id, plainName);
    }

    if (which == DetailedName) {
        String_t comment = iface.getComment(InterpreterInterface::Ship, m_id);
        if (!comment.empty()) {
            result += ": ";
            result += comment;
        } else {
            int owner;
            String_t ownerName;
            if (getOwner(owner) && iface.getPlayerAdjective(owner, ownerName)) {
                int hullNr;
                String_t hullName;
                if (getHull().get(hullNr) && iface.getHullShortName(hullNr, hullName)) {
                    result += afl::string::Format(" (%s %s)", ownerName, hullName);
                } else {
                    result += afl::string::Format(tx.translateString(" (%s starship)").c_str(), ownerName);
                }
            }
        }
    }
    return result;
}

game::Id_t
game::map::Ship::getId() const
{
    // ex GShip::getId
    return m_id;
}

bool
game::map::Ship::getOwner(int& result) const
{
    // ex GShip::getOwner
    return m_currentData.owner.get(result);
}

bool
game::map::Ship::getPosition(Point& result) const
{
    // ex GShip::getPos
    int x, y;
    if (m_currentData.x.get(x) && m_currentData.y.get(y)) {
        result.setX(x);
        result.setY(y);
        return true;
    } else {
        return false;
    }
}

// /** Check whether this ship is visible.
//     If it is visible, it is displayed on the map. */
bool
game::map::Ship::isVisible() const
{
    // ex GShip::isVisible
    return m_kind == CurrentShip
        || m_kind == CurrentTarget
        || m_kind == CurrentUnknown
        || m_kind == GuessedShip;
}

// /** Check whether this ship is reliably visible (to a player).
//     A ship can be unreliably visible if it guessed.
//     It can also be reliably visible to one player but not another one if they are not allied.

//     \param forPlayer Player to ask question for: is this ship known to that player,
//     and will host accept orders relating to it?
//     If zero, check whether ship is seen reliably by anyone. */
bool
game::map::Ship::isReliablyVisible(const int forPlayer) const
{
    // GShip::isReliablyVisible
    PlayerSet_t set = m_targetSource | m_shipSource | m_xySource;
    if (forPlayer == 0) {
        return !set.empty();
    } else {
        return set.contains(forPlayer);
    }
}

// /** Get ship source flags.
//     This is the set of players whose SHIP file contains a copy of this ship (usually a unit set). */
game::PlayerSet_t
game::map::Ship::getShipSource() const
{
    // ex GShip::getShipSource
    return m_shipSource;
}

void
game::map::Ship::addShipSource(PlayerSet_t p)
{
    m_shipSource += p;
}

// /** Get kind of this ship.
//     The kind determines how complete and reliable this ship's data is. */
game::map::Ship::Kind
game::map::Ship::getShipKind() const
{
    // ex GShip::getShipKind
    return m_kind;
}

// /** Check whether we have any data about this ship. */
bool
game::map::Ship::hasAnyShipData() const
{
    // ex GShip::hasAnyShipData
    // Note that this is implemented differently than in PCC2!
    // PCC2: check whether any historic data is available
    // c2ng: check for known owner
    return m_currentData.owner.isValid();
}

// /** Check whether we have full, playable data. */
bool
game::map::Ship::hasFullShipData() const
{
    // ex GShip::hasFullShipData
    return !m_shipSource.empty();
}

// FIXME: port
// /** Get history timestamp. 0 means not set. */
// int
// GShip::getHistoryTimestamp(TDbShipTimestamp which_one) const
// {
//     if (ship_info != 0) {
//         return ship_info->time[which_one];
//     } else {
//         return 0;
//     }
// }

// FIXME: port
// /** Get ship history entry for a turn.
//     \param turn Turn number */
// const TDbShipTrackEntry&
// GShip::getHistoryEntry(int turn) const
// {
//     if (ship_info != 0 && turn <= ship_info->track_turn && turn > ship_info->track_turn - NUM_SHIP_TRACK_ENTRIES) {
//         return ship_info->track[ship_info->track_turn - turn];
//     } else {
//         return blank_ship_track;
//     }
// }

// FIXME: port
// /** Get newest history turn. This is the newest turn for which we
//     (may) have information. Iteration can use
//     "for (t = getHistoryNewestTurn(); t > getHistoryOldestTurn(); --t)". */
// int
// GShip::getHistoryNewestTurn() const
// {
//     if (ship_info != 0) {
//         return ship_info->track_turn;
//     } else {
//         return 0;
//     }
// }

// FIXME: port
// /** Get oldest history turn. This is the turn before the oldest one
//     for which we may have information.

//     Note that this may return zero or a negative number if the history
//     buffer reaches back to before the game start.

//     \see getHistoryNewestTurn() */
// int
// GShip::getHistoryOldestTurn() const
// {
//     if (ship_info != 0) {
//         return ship_info->track_turn - NUM_SHIP_TRACK_ENTRIES;
//     } else {
//         return 0;
//     }
// }

// 
// /*
//  *  Type Accessors
//  */

// /** Get ship mass. */
game::IntegerProperty_t
game::map::Ship::getMass(const game::spec::ShipList& shipList) const
{
    // ex GShip::getMass() const
    if (m_kind == CurrentShip) {
        return getShipMass(m_currentData, shipList);
    } else {
        return m_scannedMass;
    }
}

// /** Get hull Id. */
game::IntegerProperty_t
game::map::Ship::getHull() const
{
    // ex GShip::getHullId() const
    return m_currentData.hullType;
}

// /** Set hull Id. */
void
game::map::Ship::setHull(IntegerProperty_t h)
{
    // ex GShip::setHullId, GShip::ShipInfo::init
// FIXME: sanitize
//     // Sanitize. Hull=0 is in some databases.
//     if (h.getRawValue() == 0) {
//         h = mp16_t();
//     }

    int oldValue = 0, newValue = 0;
    if (getHull().get(oldValue) && h.get(newValue) && oldValue != newValue) {
        // It's a hull change. Everything you know is wrong.
        clearShipHistory(m_historyData);
        m_historyTimestamps[MilitaryTime] = 0;
        m_historyTimestamps[RestTime] = 0;
        m_scannedHeading = NegativeProperty_t();
        m_currentData = ShipData();
    }
    m_currentData.hullType = h;
    markDirty();
}

// 
// /*
//  *  Location accessors
//  */

// /** Get Id of planet we're orbiting.
//     \return planet id, 0 if none. */
// int
// GShip::getOrbitPlanetId() const
// {
//     return map_info.orbit;
// }

// /** Check whether ship wants to be cloned.
//     This does not whether cloning actually is permitted (registeredness,
//     hull restrictions, player restrictions). */
// bool
// GShip::isCloningAt(const GPlanet& planet) const
// {
//     return ship_info != 0
//         && planet.getPos() == getPos()
//         && (ship_info->data.fcode[0] == 'c' && ship_info->data.fcode[1] == 'l' && ship_info->data.fcode[2] == 'n');
// }

// 
// /*
//  *  Owner accessors
//  */

// /** Get real owner of ship. */
game::IntegerProperty_t
game::map::Ship::getRealOwner() const
{
    // ex GShip::getRealOwner
    int n;
    if (m_remoteControlFlag > 0) {
        return m_remoteControlFlag;
    } else if (getOwner(n)) {
        return n;
    } else {
        return IntegerProperty_t();
    }
}

// /** Get ship's remote control flag. */
int
game::map::Ship::getRemoteControlFlag() const
{
    // ex GShip::getRemoteControlFlag
    return m_remoteControlFlag;
}

// /** Get waypoint.
//     If the waypoint is not known, the GPoint's components will be unknown if interpreted as mp16_t. */
afl::base::Optional<game::map::Point>
game::map::Ship::getWaypoint() const
{
    // ex GShip::getWaypoint
    int x, y, dx, dy;
    if (m_currentData.x.get(x) && m_currentData.y.get(y) && m_currentData.waypointDX.get(dx) && m_currentData.waypointDY.get(dy)) {
        return Point(x+dx, y+dy);
    } else {
        return afl::base::Nothing;
    }
}

// /** Set waypoint.
//     We can set a waypoint only if the ship is visible, i.e. has coordinates.
//     If the ship is not visible, the call is ignored. */
void
game::map::Ship::setWaypoint(afl::base::Optional<Point> pt)
{
    // ex GShip::setWaypoint
    // Note different condition than in PCC2; PCC2 checks isVisible()
    int x, y;
    if (const Point* p = pt.get()) {
        if (m_currentData.x.get(x) && m_currentData.y.get(y)) {
            m_currentData.waypointDX = p->getX() - x;
            m_currentData.waypointDY = p->getY() - y;
            markDirty();
        }
    }
}

void
game::map::Ship::clearWaypoint()
{
    if (!m_currentData.waypointDX.isSame(0) || !m_currentData.waypointDY.isSame(0)) {
        m_currentData.waypointDX = 0;
        m_currentData.waypointDY = 0;
        markDirty();
    }
}


// /** Get waypoint X displacement. */
game::NegativeProperty_t
game::map::Ship::getWaypointDX() const
{
    // ex GShip::getWaypointDX
    return m_currentData.waypointDX;
}

// /** Get waypoint Y displacement. */
game::NegativeProperty_t
game::map::Ship::getWaypointDY() const
{
    // ex GShip::getWaypointDY
    return m_currentData.waypointDY;
}

// /** Get ship's heading vector.
//     \return value from [0,360), corresponding to VGAP's heading angles.
//     Returns unknown value if ship doesn't move or heading not known */
game::IntegerProperty_t
game::map::Ship::getHeading() const
{
    int dx, dy;
    if (m_currentData.waypointDX.get(dx) && m_currentData.waypointDY.get(dy)) {
        if (dx == 0 && dy == 0) {
            return afl::base::Nothing;
        } else {
            return int(util::getHeadingDeg(dx, dy));
        }
    } else {
        return m_scannedHeading;
    }
}

// /** Get warp factor.
//     \return warp factor or unknown */
game::IntegerProperty_t
game::map::Ship::getWarpFactor() const
{
    // ex GShip::getWarp
    return m_currentData.warpFactor;
}

// /** Set warp factor. */
void
game::map::Ship::setWarpFactor(IntegerProperty_t warp)
{
    // ex GShip::setWarp
    if (!warp.isSame(m_currentData.warpFactor)) {
        m_currentData.warpFactor = warp;
        markDirty();
    }
}

// /** Check whether ship is hyperdriving.
//     \return true iff ship has hyperdrive and tries to activate it */
bool
game::map::Ship::isHyperdriving(const UnitScoreDefinitionList& scoreDefinitions,
                                const game::spec::ShipList& shipList,
                                const game::config::HostConfiguration& config) const
{
    // ex GShip::isHyperdriving
    int warp;
    String_t fc;
    return getShipKind() == CurrentShip
        && hasSpecialFunction(game::spec::HullFunction::Hyperdrive, scoreDefinitions, shipList, config)
        && getWarpFactor().get(warp)
        && warp > 0
        && getFriendlyCode().get(fc)
        && fc == "HYP";
}


/*
 *  Equipment accessors
 */

// /** Get engine type.
//     \return engine type [1,NUM_ENGINES], or unknown */
game::IntegerProperty_t
game::map::Ship::getEngineType() const
{
    // ex GShip::getEngineType
    return m_currentData.engineType;
}

// /** Set engine type (for history).
//     \param engine engine type [1,NUM_ENGINES], or unknown */
void
game::map::Ship::setEngineType(IntegerProperty_t engineType)
{
    // ex GShip::setEngineType
    m_currentData.engineType = engineType;
    markDirty();
}

// /** Get beam type.
//     \return beam type [1,NUM_BEAMS], or unknown */
game::IntegerProperty_t
game::map::Ship::getBeamType() const
{
    // ex GShip::getBeamType
    return m_currentData.beamType;
}

// /** Set beam type (for history).
//     \param type beam type [1,NUM_BEAMS], or unknown */
void
game::map::Ship::setBeamType(IntegerProperty_t type)
{
    // ex GShip::setBeamType
    m_currentData.beamType = type;
    markDirty();
}

// /** Get number of beams.
//     \return beam count, or unknown */
game::IntegerProperty_t
game::map::Ship::getNumBeams() const
{
    // ex GShip::getNumBeams
    if (m_currentData.beamType.isSame(0)) {
        return 0;
    } else {
        return m_currentData.numBeams;
    }
}

// /** Set number of beams (for history).
//     \param count beam count, or unknown */
void
game::map::Ship::setNumBeams(IntegerProperty_t count)
{
    // ex GShip::setNumBeams
    m_currentData.numBeams = count;
    markDirty();
}

// /** Get bay count.
//     \return bay count, or unknown */
game::IntegerProperty_t
game::map::Ship::getNumBays() const
{
    // ex GShip::getNumBays
    return m_currentData.numBays;
}

// /** Set bay count (for history).
//     \param count bay count, or unknown */
void
game::map::Ship::setNumBays(IntegerProperty_t count)
{
    // ex GShip::setNumBays
    m_currentData.numBays = count;
    markDirty();
}

// /** Get torpedo type.
//     \return torpedo type [1,NUM_TORPS], or unknown */
game::IntegerProperty_t
game::map::Ship::getTorpedoType() const
{
    // ex GShip::getTorpType
    return m_currentData.launcherType;
}

// /** Set torpedo type (for history).
//     \param type beam type [1,NUM_TORPS], or unknown */
void
game::map::Ship::setTorpedoType(IntegerProperty_t type)
{
    // ex GShip::setTorpType
    m_currentData.launcherType = type;
    markDirty();
}

// /** Get torpedo launcher count.
//     \return torpedo launcher count, or unknown */
game::IntegerProperty_t
game::map::Ship::getNumLaunchers() const
{
    // ex GShip::getNumTorpLaunchers
    if (m_currentData.launcherType.isSame(0)) {
        return 0;
    } else {
        return m_currentData.numLaunchers;
    }
}

// /** Set torpedo launcher count (for history).
//     \param count torpedo launcher count, or unknown */
void
game::map::Ship::setNumLaunchers(IntegerProperty_t count)
{
    // ex GShip::setNumTorpLaunchers
    m_currentData.numLaunchers = count;
    markDirty();
}


// 
// /*
//  *  Mission accessors
//  */

// /** Set ship name.
//     \param str New name */
void
game::map::Ship::setName(const String_t& str)
{
    // ex GShip::setName
    m_currentData.name = str;
    markDirty();
}


// /** Get ship mission.
//     \return mission number, or unknown */
game::IntegerProperty_t
game::map::Ship::getMission() const
{
    // ex GShip::getMission
    return m_currentData.mission;
}

// /** Set ship mission.
//     \param m Mission number
//     \param i Intercept number
//     \param t Tow number

//     FIXME: should this take a TMissionData object? */
void
game::map::Ship::setMission(IntegerProperty_t m, IntegerProperty_t i, IntegerProperty_t t)
{
    // ex GShip::setMission
    if (!m.isSame(m_currentData.mission) || !i.isSame(m_currentData.missionInterceptParameter) || !t.isSame(m_currentData.missionTowParameter)) {
        m_currentData.mission                   = m;
        m_currentData.missionInterceptParameter = i;
        m_currentData.missionTowParameter       = t;
        markDirty();
    }
}

// /** Get tow number. */
// /** Get intercept number. */
game::IntegerProperty_t
game::map::Ship::getMissionParameter(MissionParameter which) const
{
    // ex GShip::getTowId, GShip::getInterceptId
    return which == InterceptParameter
        ? m_currentData.missionInterceptParameter
        : m_currentData.missionTowParameter;
}

// /** Get primary enemy. */
game::IntegerProperty_t
game::map::Ship::getPrimaryEnemy() const
{
    // ex GShip::getPrimaryEnemy
    return m_currentData.primaryEnemy;
}

// /** Set primary enemy.
//     \param pe New primary enemy, [0,NUM_PLAYERS] */
void
game::map::Ship::setPrimaryEnemy(IntegerProperty_t pe)
{
    // ex GShip::setPrimaryEnemy
    m_currentData.primaryEnemy = pe;
    markDirty();
}

// /** Get ship damage.
//     \return damage level, or unknown */
game::IntegerProperty_t
game::map::Ship::getDamage() const
{
    // ex GShip::getDamage
    return m_currentData.damage;
}

// /** Set ship damage.
//     \param damage damage level, or unknown */
void
game::map::Ship::setDamage(IntegerProperty_t damage)
{
    // ex GShip::setDamage
    m_currentData.damage = damage;
    markDirty();
}

// /** Get ship crew.
//     \return number of crewmen, or unknown */
game::IntegerProperty_t
game::map::Ship::getCrew() const
{
    // ex GShip::getCrew
    return m_currentData.crew;
}

// /** Set ship crew.
//     \param crew number of crewmen, or unknown */
void
game::map::Ship::setCrew(IntegerProperty_t crew)
{
    // ex GShip::setCrew
    m_currentData.crew = crew;
    markDirty();
}

// /** Get friendly code. */
game::StringProperty_t
game::map::Ship::getFriendlyCode() const
{
    // ex GShip::getFCode, GShip::isFCodeKnown
    return m_currentData.friendlyCode;
}

// /** Set friendly code. */
void
game::map::Ship::setFriendlyCode(StringProperty_t fc)
{
    // ex GShip::setFCode
    m_currentData.friendlyCode = fc;
    markDirty();
}

// 
// /*
//  *  Cargo accessors
//  */

// /** Get ammunition on ship, raw version. This does not honor reservations.
//     It returns the amount of torpedoes/fighters, regardless of their type. */
game::IntegerProperty_t
game::map::Ship::getAmmo() const
{
    // ex GShip::getAmmoRaw; also replaces GShip::getAmmo
    return m_currentData.ammo;
}

void
game::map::Ship::setAmmo(IntegerProperty_t amount)
{
    m_currentData.ammo = amount;
    markDirty();
}

// /** Get cargo amount on ship, raw version. This does not honor reservations. */
game::IntegerProperty_t
game::map::Ship::getCargo(Element::Type type) const
{
    // ex GShip::getCargoRaw, GShip::getCargo
    int numBays, expectedType, numLaunchers, launcherType;
    switch (type) {
     case Element::Neutronium:
        return m_currentData.neutronium;
     case Element::Tritanium:
        return m_currentData.tritanium;
     case Element::Duranium:
        return m_currentData.duranium;
     case Element::Molybdenum:
        return m_currentData.molybdenum;
     case Element::Fighters:
        if (getNumBays().get(numBays)) {
            if (numBays > 0) {
                // I know it has bays, so ammo is number of fighters
                return m_currentData.ammo;
            } else {
                // I know it has no fighters
                return 0;
            }
        } else {
            // I don't know whether it has fighters
            return afl::base::Nothing;
        }
     case Element::Colonists:
        return m_currentData.colonists;
     case Element::Supplies:
        return m_currentData.supplies;
     case Element::Money:
        return m_currentData.money;
     default:
        if (Element::isTorpedoType(type, expectedType)) {
            if (getTorpedoType().get(launcherType)) {
                if (launcherType == expectedType) {
                    // Asking correct torpedo type
                    return m_currentData.ammo;
                } else {
                    // Asking wrong torpedo type
                    return 0;
                }
            } else if (getNumLaunchers().get(numLaunchers) && numLaunchers == 0) {
                // Asking any type, and we know we don't have torpedoes
                return 0;
            } else {
                // Nothing known
                return afl::base::Nothing;
            }
        } else {
            // I don't know what cargo type this is, but I don't have it.
            return 0;
        }
    }
}

void
game::map::Ship::setCargo(Element::Type type, IntegerProperty_t amount)
{
    int numBays, expectedType, launcherType;
    switch (type) {
     case Element::Neutronium:
        m_currentData.neutronium = amount;
        break;
     case Element::Tritanium:
        m_currentData.tritanium = amount;
        break;
     case Element::Duranium:
        m_currentData.duranium = amount;
        break;
     case Element::Molybdenum:
        m_currentData.molybdenum = amount;
        break;
     case Element::Fighters:
        if (getNumBays().get(numBays) && numBays > 0) {
            // I know it has bays, so ammo is number of fighters
            m_currentData.ammo = amount;
        }
        break;
     case Element::Colonists:
        m_currentData.colonists = amount;
        break;
     case Element::Supplies:
        m_currentData.supplies = amount;
        break;
     case Element::Money:
        m_currentData.money = amount;
        break;
     default:
        if (Element::isTorpedoType(type, expectedType)
            && getTorpedoType().get(launcherType)
            && launcherType == expectedType)
        {
            m_currentData.ammo = amount;
        }
        break;
    }
    markDirty();
}

game::LongProperty_t
game::map::Ship::getFreeCargo(const game::spec::ShipList& list) const
{
    int hull, t, d, m, ammo, col, sup;
    if (m_currentData.hullType.get(hull)
        && m_currentData.tritanium.get(t) && m_currentData.duranium.get(d) && m_currentData.molybdenum.get(m)
        && m_currentData.ammo.get(ammo) && m_currentData.colonists.get(col) && m_currentData.supplies.get(sup))
    {
        if (const game::spec::Hull* p = list.hulls().get(hull)) {
            return p->getMaxCargo() - t - d - m - ammo - col - sup;
        } else {
            return LongProperty_t();
        }
    } else {
        return LongProperty_t();
    }
}


// /** Check whether a transporter is active. An active transporter has a
//     full transfer order. */
bool
game::map::Ship::isTransporterActive(Transporter which) const
{
    // ex GShip::isTransporterActive
    const ShipData::Transfer& tr = which==UnloadTransporter ? m_currentData.unload : m_currentData.transfer;
    return m_kind == CurrentShip && isTransferActive(tr);
}

game::IntegerProperty_t
game::map::Ship::getTransporterTargetId(Transporter which) const
{
    if (m_kind == CurrentShip) {
        const ShipData::Transfer& tr = which==UnloadTransporter ? m_currentData.unload : m_currentData.transfer;
        return tr.targetId;
    } else {
        return IntegerProperty_t();
    }
}

void
game::map::Ship::setTransporterTargetId(Transporter which, IntegerProperty_t id)
{
    ShipData::Transfer& tr = which==UnloadTransporter ? m_currentData.unload : m_currentData.transfer;
    tr.targetId = id;
    markDirty();
}

game::IntegerProperty_t
game::map::Ship::getTransporterCargo(Transporter which, Element::Type type) const
{
    if (m_kind == CurrentShip) {
        const ShipData::Transfer& tr = which==UnloadTransporter ? m_currentData.unload : m_currentData.transfer;
        switch (type) {
         case Element::Neutronium: return tr.neutronium;
         case Element::Tritanium:  return tr.tritanium;
         case Element::Duranium:   return tr.duranium;
         case Element::Molybdenum: return tr.molybdenum;
         case Element::Colonists:  return tr.colonists;
         case Element::Supplies:   return tr.supplies;
         default:                  return 0;
        }
    } else {
        return IntegerProperty_t();
    }
}

void
game::map::Ship::setTransporterCargo(Transporter which, Element::Type type, IntegerProperty_t amount)
{
    ShipData::Transfer& tr = which==UnloadTransporter ? m_currentData.unload : m_currentData.transfer;
    switch (type) {
     case Element::Neutronium: tr.neutronium = amount; break;
     case Element::Tritanium:  tr.tritanium  = amount; break;
     case Element::Duranium:   tr.duranium   = amount; break;
     case Element::Molybdenum: tr.molybdenum = amount; break;
     case Element::Colonists:  tr.colonists  = amount; break;
     case Element::Supplies:   tr.supplies   = amount; break;
     default:                  break;
    }
    markDirty();
}


// /** Check whether a transporter is cancellable. A transporter is
//     cancellable if its content can be moved back into main cargo room
//     without overloading the ship. */
// bool
// GShip::isTransporterCancellable(Transporter which_one) const throw()
// {
//     // FIXME: this does not guarantee correctness in all cases: Take a
//     // ship with 10 cargo room left, and 10 kt in the transporter. A
//     // pending transaction adds 10 to the ship (i.e. it got the
//     // guarantee that it'll be able to add this). When we now do
//     // cancelTranporter() and commit that transaction, the ship is
//     // overloaded.
//     if (hasFullShipData()) {
//         const TShipTransfer& t = getTransporter(which_one);
//         long sum = (long) t.ore[0] + t.ore[1] + t.ore[2] + t.ore[3] + t.supplies + t.colonists;
//         return sum == 0 || sum <= getFreeCargo();
//     } else {
//         return false;
//     }
// }

// /** Cancel a transporter. Moves its contents back to main cargo room.
//     \pre isTransporterCancellable(which_one) */
void
game::map::Ship::cancelTransporter(Transporter which)
{
    // ex GShip::cancelTranporter
    ShipData::Transfer& tr = getTransporter(which);

    m_currentData.neutronium = m_currentData.neutronium.orElse(0) + tr.neutronium.orElse(0);
    m_currentData.tritanium  = m_currentData.tritanium.orElse(0)  + tr.tritanium.orElse(0);
    m_currentData.duranium   = m_currentData.duranium.orElse(0)   + tr.duranium.orElse(0);
    m_currentData.molybdenum = m_currentData.molybdenum.orElse(0) + tr.molybdenum.orElse(0);
    m_currentData.colonists  = m_currentData.colonists.orElse(0)  + tr.colonists.orElse(0);
    m_currentData.supplies   = m_currentData.supplies.orElse(0)   + tr.supplies.orElse(0);

    tr.neutronium = 0;
    tr.tritanium  = 0;
    tr.duranium   = 0;
    tr.molybdenum = 0;
    tr.supplies   = 0;
    tr.colonists  = 0;
    tr.targetId   = 0;

    markDirty();
}

// 
// /*
//  *  Fleet accessors
//  */

// /** Set number of the fleet this ship is in.
//     This function just sets the internal flag.
//     Do not use this function directly; use ::setFleetNumber(GUniverse&,int,int) instead.
//     That function will update all dependant information. */
void
game::map::Ship::setFleetNumber(int fno)
{
    // ex GShip::setFleetNumber
    m_fleetNumber = fno;
    markDirty();
}

// /** Get number of the fleet this ship is in.
//     \return fleet number (a ship Id), or 0 */
int
game::map::Ship::getFleetNumber() const
{
    // ex GShip::getFleetNumber
    return m_fleetNumber;
}

// /** Set name of the fleet led by this ship. */
void
game::map::Ship::setFleetName(String_t name)
{
    // ex GShip::setFleetName
    m_fleetName = name;
    markDirty();
}

// /** Get name of the fleet led by this ship (if any). */
const String_t&
game::map::Ship::getFleetName() const
{
    // ex GShip::getFleetName
    return m_fleetName;
}

// /** Check for fleet leader. */
bool
game::map::Ship::isFleetLeader() const
{
    // ex GShip::isFleetLeader
    return m_fleetNumber == m_id;
}

// /** Check for fleet member.
//     \return true if this is a fleet member (but not a leader) */
bool
game::map::Ship::isFleetMember() const
{
    // ex GShip::isFleetMember
    return m_fleetNumber != 0 && m_fleetNumber != m_id;
}




// void
// GShip::setPredictedPos(GPoint pt)
// {
//     map_info.predicted_pos = pt;
// }

// GPoint
// GShip::getPredictedPos() const
// {
//     return map_info.predicted_pos;
// }

// 
// /*
//  *  Function accessors
//  */

void
game::map::Ship::addShipSpecialFunction(game::spec::ModifiedHullFunctionList::Function_t function)
{
    // ex GShip::addShipFunction
    m_specialFunctions.push_back(function);
    markDirty();
}

// /** Check whether this ship can do special function.
//     \param basic_function A basic ship function, e.g. hf_Cloak. */
bool
game::map::Ship::hasSpecialFunction(int basicFunction,
                                    const UnitScoreDefinitionList& scoreDefinitions,
                                    const game::spec::ShipList& shipList,
                                    const game::config::HostConfiguration& config) const
{
    // ex GShip::canDoSpecial
    // Do we know the hull?
    int hullNr;
    if (!getHull().get(hullNr)) {
        return false;
    }
    const game::spec::Hull* hull = shipList.hulls().get(hullNr);
    if (!hull) {
        return false;
    }

    // Figure out experience level
    int16_t expLevel;
    int16_t expTurn;
    UnitScoreList::Index_t expIndex;
    if (!scoreDefinitions.lookup(ScoreId_ExpLevel, expIndex) || !unitScores().get(expIndex, expLevel, expTurn)) {
        expLevel = 0;
        expTurn = 0;
    }

    // Do we know the owner?
    int owner;
    if (!getRealOwner().get(owner)) {
        return false;
    }

    // Check class functions
    const game::spec::ModifiedHullFunctionList& mhf = shipList.modifiedHullFunctions();
    if (hull->getHullFunctions(true).getPlayersThatCan(basicFunction, mhf, shipList.basicHullFunctions(), config, *hull, ExperienceLevelSet_t(expLevel), true).contains(owner)) {
        return true;
    }

    // Check this ship's functions
    ExperienceLevelSet_t allLevels(ExperienceLevelSet_t::allUpTo(config[config.NumExperienceLevels]()));
    for (size_t i = 0, n = m_specialFunctions.size(); i < n; ++i) {
        game::spec::HullFunction f;
        if (mhf.getFunctionDefinition(mhf.getFunctionIdFromHostId(m_specialFunctions[i]), f)
            && f.getBasicFunctionId() == basicFunction
            && (f.getLevels().contains(allLevels) || f.getLevels().contains(expLevel)))
        {
            // We accept the function if it is available on our current level, or on all configured levels.
            // The latter condition is normally redundant, but provides a sensible fallback
            // in case we do not know the current levels for some reasons (like: missing util.dat).
            return true;
        }
    }

    // Not found
    return false;
}

// /** Enumerate this ship's special functions (only functions assigned to this
//     ship, not including class functions). */
void
game::map::Ship::enumerateShipFunctions(game::spec::HullFunctionList& list,
                                        const game::spec::ShipList& shipList) const
{
    // ex GShip::enumShipSpecificSpecials
    const game::spec::ModifiedHullFunctionList& mhf = shipList.modifiedHullFunctions();
    for (size_t i = 0, n = m_specialFunctions.size(); i < n; ++i) {
        game::spec::HullFunction f;
        if (mhf.getFunctionDefinition(mhf.getFunctionIdFromHostId(m_specialFunctions[i]), f)) {
            list.add(f);
        }
    }
}

// /** Check whether this ship has any ship-specific functions. */
bool
game::map::Ship::hasAnyShipSpecialFunctions() const
{
    // ex GShip::hasAnyShipSpecificSpecials
    return !m_specialFunctions.empty();
}

// 
// /*
//  *  Data access
//  */

// /** Create ShipInfo if it doesn't exist yet. Returns reference to it. */
// GShip::ShipInfo&
// GShip::createShipInfo()
// {
//     if (!ship_info) {
//         ship_info = new ShipInfo(map_info.id);
//         if (map_info.kind == NoShip) {
//             map_info.kind = HistoryShip;
//         }
//     }
//     return *ship_info;
// }

// /** Get read-only ship data. */
// const TShip&
// GShip::getDisplayedShip() const
// {
//     return ship_info != 0 ? ship_info->data : blank_ship;
// }

// /** Adjust ship track info to start no later than turn.
//     \post turn <= info.track_turn */
// void
// GShip::adjustShipTrack(int turn)
// {
//     ShipInfo& info = createShipInfo();
//     if (turn > info.track_turn) {
//         int adjust = turn - info.track_turn;
//         for (int i = NUM_SHIP_TRACK_ENTRIES-1; i >= 0; --i) {
//             info.track[i] = (i >= adjust
//                              ? info.track[i - adjust]
//                              : blank_ship_track);
//         }
//         info.track_turn = turn;
//     }
// }

game::UnitScoreList&
game::map::Ship::unitScores()
{
    return m_unitScores;
}

const game::UnitScoreList&
game::map::Ship::unitScores() const
{
    return m_unitScores;
}

game::map::ShipData::Transfer&
game::map::Ship::getTransporter(Transporter which)
{
    // ex GShip::getTransporter
    return which==UnloadTransporter ? m_currentData.unload : m_currentData.transfer;
}

const game::map::ShipData::Transfer&
game::map::Ship::getTransporter(Transporter which) const
{
    // ex GShip::getTransporter
    return which==UnloadTransporter ? m_currentData.unload : m_currentData.transfer;
}
