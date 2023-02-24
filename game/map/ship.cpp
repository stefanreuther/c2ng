/**
  *  \file game/map/ship.cpp
  *  \brief Class game::map::Ship
  */

#include "game/map/ship.hpp"
#include "afl/string/format.hpp"
#include "game/map/universe.hpp"
#include "game/parser/messagevalue.hpp"
#include "util/math.hpp"

namespace {
    template<typename PropertyType, typename ValueType>
    void updateField(int& fieldTime, int time, bool flag, PropertyType& fieldValue, ValueType value)
    {
        if (flag && (fieldTime <= time || !fieldValue.isValid())) {
            fieldValue = value;
            if (fieldTime < time) {
                fieldTime = time;
            }
        }
    }
}


/*
 *  Construction
 */

game::map::Ship::Ship(Id_t id)
    : Object(id),
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
      m_unitScores(),
      m_messages()
{
    m_historyTimestamps[0] = 0;
    m_historyTimestamps[1] = 0;
}

game::map::Ship::~Ship()
{ }


/*
 *  Load and Save
 */

void
game::map::Ship::addCurrentShipData(const ShipData& data, PlayerSet_t source)
{
    // Set hull through regular setter to update history
    setHull(data.hullType);

    // Take over everything
    m_currentData = data;
    m_shipSource += source;
}

void
game::map::Ship::addShipXYData(Point pt, int owner, int mass, PlayerSet_t source)
{
    // ex GShip::addShipXYData
    // Record that we know it
    m_xySource += source;

    // Update ship.
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
    // ex GShip::addMessageInformation (expanded!), GShip::addShipHistoryData
    namespace gp = game::parser;

    // Check acceptance of information for possibly current ship
    bool isCurrent = !m_shipSource.empty();
    const int turn = info.getTurnNumber();

    for (gp::MessageInformation::Iterator_t i = info.begin(); i != info.end(); ++i) {
        if (const gp::MessageIntegerValue_t* iv = dynamic_cast<gp::MessageIntegerValue_t*>(*i)) {
            switch (iv->getIndex()) {
             case gp::mi_Owner:
                // Update RestTime with owner.
                // PCC1 does not always update a timestamp here.
                updateField(m_historyTimestamps[RestTime], turn, !isCurrent, m_currentData.owner, iv->getValue());
                break;

             case gp::mi_ShipWaypointDX:
                updateField(m_historyTimestamps[RestTime], turn, !isCurrent, m_currentData.waypointDX, iv->getValue());
                break;

             case gp::mi_ShipWaypointDY:
                updateField(m_historyTimestamps[RestTime], turn, !isCurrent, m_currentData.waypointDY, iv->getValue());
                break;

             case gp::mi_ShipEngineType:
                // Engine type goes into RestTime slot.
                // PCC1 is inconsistent here:
                //   ccmain.ParseVCRShip: RestTurn
                //   ccinit.ProcessShipResult: MilitaryTime
                updateField(m_historyTimestamps[RestTime], turn, !isCurrent, m_currentData.engineType, iv->getValue());
                break;

             case gp::mi_ShipHull:
                if (iv->getValue() != 0) {
                    updateField(m_historyTimestamps[RestTime], turn, !isCurrent, m_currentData.hullType, iv->getValue());
                }
                break;

             case gp::mi_ShipBeamType:
                updateField(m_historyTimestamps[MilitaryTime], turn, !isCurrent, m_currentData.beamType, iv->getValue());
                break;

             case gp::mi_ShipNumBeams:
                updateField(m_historyTimestamps[MilitaryTime], turn, !isCurrent, m_currentData.numBeams, iv->getValue());
                break;

             case gp::mi_ShipNumBays:
                updateField(m_historyTimestamps[MilitaryTime], turn, !isCurrent, m_currentData.numBays, iv->getValue());
                break;

             case gp::mi_ShipTorpedoType:
                updateField(m_historyTimestamps[MilitaryTime], turn, !isCurrent, m_currentData.torpedoType, iv->getValue());
                break;

             case gp::mi_ShipAmmo:
                updateField(m_historyTimestamps[MilitaryTime], turn, !isCurrent, m_currentData.ammo, iv->getValue());
                break;

             case gp::mi_ShipNumLaunchers:
                updateField(m_historyTimestamps[MilitaryTime], turn, !isCurrent, m_currentData.numLaunchers, iv->getValue());
                break;

             case gp::mi_ShipMission:
                updateField(m_historyTimestamps[RestTime], turn, !isCurrent, m_currentData.mission, iv->getValue());
                break;

             case gp::mi_ShipEnemy:
                updateField(m_historyTimestamps[RestTime], turn, !isCurrent, m_currentData.primaryEnemy, iv->getValue());
                break;

             case gp::mi_ShipTow:
                updateField(m_historyTimestamps[RestTime], turn, !isCurrent, m_currentData.missionTowParameter, iv->getValue());
                break;

             case gp::mi_Damage:
                updateField(m_historyTimestamps[MilitaryTime], turn, !isCurrent, m_currentData.damage, iv->getValue());
                break;

             case gp::mi_ShipCrew:
                updateField(m_historyTimestamps[MilitaryTime], turn, !isCurrent, m_currentData.crew, iv->getValue());
                break;

             case gp::mi_ShipColonists:
                updateField(m_historyTimestamps[RestTime], turn, !isCurrent, m_currentData.colonists, iv->getValue());
                break;

             case gp::mi_ShipFuel:
                updateField(m_historyTimestamps[RestTime], turn, !isCurrent, m_currentData.neutronium, iv->getValue());
                break;

             case gp::mi_ShipCargoT:
                updateField(m_historyTimestamps[RestTime], turn, !isCurrent, m_currentData.tritanium, iv->getValue());
                break;

             case gp::mi_ShipCargoD:
                updateField(m_historyTimestamps[RestTime], turn, !isCurrent, m_currentData.duranium, iv->getValue());
                break;

             case gp::mi_ShipCargoM:
                updateField(m_historyTimestamps[RestTime], turn, !isCurrent, m_currentData.molybdenum, iv->getValue());
                break;

             case gp::mi_ShipSupplies:
                updateField(m_historyTimestamps[RestTime], turn, !isCurrent, m_currentData.supplies, iv->getValue());
                break;

             case gp::mi_ShipIntercept:
                updateField(m_historyTimestamps[RestTime], turn, !isCurrent, m_currentData.missionInterceptParameter, iv->getValue());
                break;

             case gp::mi_ShipMoney:
                updateField(m_historyTimestamps[RestTime], turn, !isCurrent, m_currentData.money, iv->getValue());
                break;

                /*
                 *  Ship Track fields
                 *
                 *  If source is empty (=history, untrusted scan), add only to track.
                 *  If source is nonempty (=trusted scan), add to ship proper.
                 */
                // ex GShip::addShipTrackEntry (sort-of)

             case gp::mi_Speed:
                if (ShipHistoryData::Track* p = adjustShipHistory(m_historyData, turn)) {
                    p->speed = iv->getValue();
                }
                if (!source.empty()) {
                    m_currentData.warpFactor = iv->getValue();
                }
                break;

             case gp::mi_X:
                if (ShipHistoryData::Track* p = adjustShipHistory(m_historyData, turn)) {
                    p->x = iv->getValue();
                }
                if (!source.empty()) {
                    m_currentData.x = iv->getValue();
                }
                break;

             case gp::mi_Y:
                if (ShipHistoryData::Track* p = adjustShipHistory(m_historyData, turn)) {
                    p->y = iv->getValue();
                }
                if (!source.empty()) {
                    m_currentData.y = iv->getValue();
                }
                break;

             case gp::mi_Heading:
                if (ShipHistoryData::Track* p = adjustShipHistory(m_historyData, turn)) {
                    p->heading = iv->getValue();
                }
                if (!source.empty()) {
                    m_scannedHeading = iv->getValue();
                }
                break;

             case gp::mi_Mass:
                if (ShipHistoryData::Track* p = adjustShipHistory(m_historyData, turn)) {
                    p->mass = iv->getValue();
                }
                if (!source.empty()) {
                    m_scannedMass = iv->getValue();
                }
                break;

             case gp::mi_ShipRemoteFlag:
                // ex GShip::addRCEntry
                m_remoteControlFlag = iv->getValue();
                break;

             default:
                break;
            }
        } else if (const gp::MessageStringValue_t* sv = dynamic_cast<gp::MessageStringValue_t*>(*i)) {
            switch (sv->getIndex()) {
             case gp::ms_FriendlyCode:
                updateField(m_historyTimestamps[RestTime], turn, !isCurrent, m_currentData.friendlyCode, sv->getValue());
                break;
             case gp::ms_Name:
                updateField(m_historyTimestamps[RestTime], turn, !isCurrent, m_currentData.name, sv->getValue());
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

void
game::map::Ship::internalCheck(PlayerSet_t availablePlayers, int turnNumber)
{
    // ex GShip::internalCheck, GShip::combinedCheck1
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

    // Database sanitisation: make sure owner is known, nonzero for everything but NoShip.
    if (m_currentData.owner.isSame(0)) {
        // Ships without owner are generated from explosion records (Util1Bang).
        // If anything else reports an unowned ship, that's an error in the data files.
        // Reset ship type
        m_kind = NoShip;

        // Avoid that anyone trusts this data
        m_shipSource = m_targetSource = m_xySource = PlayerSet_t();

        // Remove known data. If this data leaks into chartX.cc files, PCC2 < 2.0.7 will crash.
        m_currentData = ShipData();
    }

    // Update ages
    if (hasFullShipData()) {
        m_historyTimestamps[MilitaryTime] = m_historyTimestamps[RestTime] = turnNumber;
    }

    // If we see the ship, it must exist even if history data says otherwise.
    // The next condition might otherwise delete it.
    if (!m_shipSource.empty() || !m_targetSource.empty() || !m_xySource.empty()) {
        if (m_currentData.damage.orElse(0) > 150) {
            m_currentData.damage = 0;
        }
    }

    // If ship claims to exists, but we don't have current data, it's destroyed. Remove it.
    // (But don't upgrade a non-existant ship, e.g. explosion-only, to HistoryShip.)
    int owner;
    if (getOwner().get(owner)
        && owner != 0
        && ((availablePlayers.contains(owner) && m_shipSource.empty())
            || m_currentData.damage.orElse(0) > 150))
    {
        // Clear current data
        m_currentData.x = IntegerProperty_t();
        m_currentData.y = IntegerProperty_t();
        m_currentData.warpFactor = IntegerProperty_t();
        m_scannedHeading = IntegerProperty_t();
        m_scannedMass = IntegerProperty_t();

        // Clear current turn's history data, we know it does not exist this turn
        clearShipHistory(m_historyData, turnNumber);

        m_kind = HistoryShip;
    }

    // Update ship track.
    // The simplest way is to generate a whole new record and have the regular code assimilate it.
    if (isVisible()) {
        if (ShipHistoryData::Track* p = adjustShipHistory(m_historyData, turnNumber)) {
            p->x = m_currentData.x;
            p->y = m_currentData.y;
            p->speed = m_currentData.warpFactor;
            p->heading = getHeading();
            // FIXME: using the scanned mass here is wrong for own ships for which we should compute the mass.
            // Right now this is not a problem, but must be dealt with when the ship track is shown or saved again.
            p->mass = m_scannedMass;
        }
    }

    // If ship-track has current info, we can transform this into a guessed ship.
    if (const ShipHistoryData::Track* p = getShipHistory(m_historyData, turnNumber)) {
        // Warp factor
        if (!m_currentData.warpFactor.isValid()) {
            m_currentData.warpFactor = p->speed;
        }

        // Location
        if (m_kind == HistoryShip && p->x.isValid() && p->y.isValid()) {
            m_kind = GuessedShip;
            m_currentData.x = p->x;
            m_currentData.y = p->y;
            m_scannedMass = p->mass;
        }
    }
}


String_t
game::map::Ship::getName(ObjectName which, afl::string::Translator& tx, InterpreterInterface& iface) const
{
    // ex GShip::getName, global.pas:ShipNameRaw (PlainName), global.pas:Shipname (LongName)
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
        result = afl::string::Format(tx.translateString("Ship #%d").c_str(), getId());
    } else {
        result = afl::string::Format(tx.translateString("Ship #%d: %s").c_str(), getId(), plainName);
    }

    if (which == DetailedName) {
        String_t comment = iface.getComment(InterpreterInterface::Ship, getId());
        if (!comment.empty()) {
            result += ": ";
            result += comment;
        } else {
            int owner;
            String_t ownerName;
            if (getOwner().get(owner) && iface.getPlayerAdjective(owner, ownerName)) {
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

afl::base::Optional<int>
game::map::Ship::getOwner() const
{
    // ex GShip::getOwner
    return m_currentData.owner;
}

afl::base::Optional<game::map::Point>
game::map::Ship::getPosition() const
{
    // ex GShip::getPos
    int x, y;
    if (m_currentData.x.get(x) && m_currentData.y.get(y)) {
        return Point(x, y);
    } else {
        return afl::base::Nothing;
    }
}


/*
 *  Status inquiry
 */

bool
game::map::Ship::isVisible() const
{
    // ex GShip::isVisible
    return m_kind == CurrentShip
        || m_kind == CurrentTarget
        || m_kind == CurrentUnknown
        || m_kind == GuessedShip;
}

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

game::map::Ship::Kind
game::map::Ship::getShipKind() const
{
    // ex GShip::getShipKind
    return m_kind;
}

bool
game::map::Ship::hasAnyShipData() const
{
    // ex GShip::hasAnyShipData
    // Note that this is implemented differently than in PCC2!
    // PCC2: check whether any historic data is available
    // c2ng: check for known owner
    return m_currentData.owner.isValid();
}

bool
game::map::Ship::hasFullShipData() const
{
    // ex GShip::hasFullShipData
    return !m_shipSource.empty();
}


/*
 *  History accessors
 */

int
game::map::Ship::getHistoryTimestamp(Timestamp kind) const
{
    return m_historyTimestamps[kind];
}

int
game::map::Ship::getHistoryNewestLocationTurn() const
{
    // ex GShip::getHistoryNewestTurn
    return m_historyData.trackTurn;
}

const game::map::ShipHistoryData::Track*
game::map::Ship::getHistoryLocation(int turnNr) const
{
    // ex GShip::getHistoryEntry
    // FIXME: if turnNr==current, we want to report the computed mass, speed, heading here! See db::Loader.
    return getShipHistory(m_historyData, turnNr);
}


/*
 *  Test access
 */

void
game::map::Ship::setOwner(int owner)
{
    m_currentData.owner = owner;
    markDirty();
}

void
game::map::Ship::setPosition(Point pos)
{
    m_currentData.x = pos.getX();
    m_currentData.y = pos.getY();
    markDirty();
}


/*
 *  Type Accessors
 */

game::IntegerProperty_t
game::map::Ship::getMass(const game::spec::ShipList& shipList) const
{
    // ex GShip::getMass() const, global.pas:ShipMass
    if (m_kind == CurrentShip) {
        return getShipMass(m_currentData, shipList);
    } else {
        return m_scannedMass;
    }
}

game::IntegerProperty_t
game::map::Ship::getHull() const
{
    // ex GShip::getHullId() const
    return m_currentData.hullType;
}

void
game::map::Ship::setHull(IntegerProperty_t h)
{
    // ex GShip::setHullId, GShip::ShipInfo::init, global.pas:SetHistoryShipHull
    // Sanitize. Hull=0 is in some databases.
    int checkValue = 0;
    if (h.get(checkValue) && checkValue == 0) {
        h = IntegerProperty_t();
    }

    int oldValue = 0, newValue = 0;
    if (getHull().get(oldValue) && h.get(newValue) && oldValue != newValue) {
        // It's a hull change. Everything you know is wrong.
        clearShipHistory(m_historyData);
        m_historyTimestamps[MilitaryTime] = 0;
        m_historyTimestamps[RestTime] = 0;
        m_scannedHeading = NegativeProperty_t();
        m_currentData = ShipData();
        m_unitScores = UnitScoreList();
    }
    m_currentData.hullType = h;
    markDirty();
}


/*
 *  Owner accessors
 */

game::IntegerProperty_t
game::map::Ship::getRealOwner() const
{
    // ex GShip::getRealOwner
    // ex shipacc.pas:RealShipOwner
    int n;
    if (m_remoteControlFlag > 0) {
        return m_remoteControlFlag;
    } else if (getOwner().get(n)) {
        return n;
    } else {
        return IntegerProperty_t();
    }
}

int
game::map::Ship::getRemoteControlFlag() const
{
    // ex GShip::getRemoteControlFlag
    return m_remoteControlFlag;
}


/*
 *  Course accessors
 */

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

game::NegativeProperty_t
game::map::Ship::getWaypointDX() const
{
    // ex GShip::getWaypointDX
    return m_currentData.waypointDX;
}

game::NegativeProperty_t
game::map::Ship::getWaypointDY() const
{
    // ex GShip::getWaypointDY
    return m_currentData.waypointDY;
}

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

game::IntegerProperty_t
game::map::Ship::getWarpFactor() const
{
    // ex GShip::getWarp
    return m_currentData.warpFactor;
}

void
game::map::Ship::setWarpFactor(IntegerProperty_t warp)
{
    // ex GShip::setWarp
    if (!warp.isSame(m_currentData.warpFactor)) {
        m_currentData.warpFactor = warp;
        markDirty();
    }
}

bool
game::map::Ship::isHyperdriving(const UnitScoreDefinitionList& scoreDefinitions,
                                const game::spec::ShipList& shipList,
                                const game::config::HostConfiguration& config) const
{
    // ex GShip::isHyperdriving
    int warp;
    String_t fc;
    return getShipKind() == CurrentShip
        && hasSpecialFunction(game::spec::BasicHullFunction::Hyperdrive, scoreDefinitions, shipList, config)
        && getWarpFactor().get(warp)
        && warp > 0
        && getFriendlyCode().get(fc)
        && fc == "HYP";
}


/*
 *  Equipment accessors
 */

game::IntegerProperty_t
game::map::Ship::getEngineType() const
{
    // ex GShip::getEngineType
    return m_currentData.engineType;
}

void
game::map::Ship::setEngineType(IntegerProperty_t engineType)
{
    // ex GShip::setEngineType
    m_currentData.engineType = engineType;
    markDirty();
}

game::IntegerProperty_t
game::map::Ship::getBeamType() const
{
    // ex GShip::getBeamType
    return m_currentData.beamType;
}

void
game::map::Ship::setBeamType(IntegerProperty_t type)
{
    // ex GShip::setBeamType
    m_currentData.beamType = type;
    markDirty();
}

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

void
game::map::Ship::setNumBeams(IntegerProperty_t count)
{
    // ex GShip::setNumBeams
    m_currentData.numBeams = count;
    markDirty();
}

game::IntegerProperty_t
game::map::Ship::getNumBays() const
{
    // ex GShip::getNumBays
    return m_currentData.numBays;
}

void
game::map::Ship::setNumBays(IntegerProperty_t count)
{
    // ex GShip::setNumBays
    m_currentData.numBays = count;
    markDirty();
}

game::IntegerProperty_t
game::map::Ship::getTorpedoType() const
{
    // ex GShip::getTorpType
    return m_currentData.torpedoType;
}

void
game::map::Ship::setTorpedoType(IntegerProperty_t type)
{
    // ex GShip::setTorpType
    m_currentData.torpedoType = type;
    markDirty();
}

game::IntegerProperty_t
game::map::Ship::getNumLaunchers() const
{
    // ex GShip::getNumTorpLaunchers
    if (m_currentData.torpedoType.isSame(0)) {
        return 0;
    } else {
        return m_currentData.numLaunchers;
    }
}

void
game::map::Ship::setNumLaunchers(IntegerProperty_t count)
{
    // ex GShip::setNumTorpLaunchers
    m_currentData.numLaunchers = count;
    markDirty();
}

bool
game::map::Ship::hasWeapons() const
{
    return getNumBeams().orElse(0) > 0
        || getNumLaunchers().orElse(0) > 0
        || getNumBays().orElse(0) > 0;
}


/*
 *  Mission accessors
 */

String_t
game::map::Ship::getName() const
{
    String_t plainName;
    m_currentData.name.get(plainName);
    return plainName;
}

void
game::map::Ship::setName(const String_t& str)
{
    // ex GShip::setName
    m_currentData.name = str;
    markDirty();
}

game::IntegerProperty_t
game::map::Ship::getMission() const
{
    // ex GShip::getMission
    return m_currentData.mission;
}

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

game::IntegerProperty_t
game::map::Ship::getMissionParameter(MissionParameter which) const
{
    // ex GShip::getTowId, GShip::getInterceptId
    return which == InterceptParameter
        ? m_currentData.missionInterceptParameter
        : m_currentData.missionTowParameter;
}

game::IntegerProperty_t
game::map::Ship::getPrimaryEnemy() const
{
    // ex GShip::getPrimaryEnemy
    return m_currentData.primaryEnemy;
}

void
game::map::Ship::setPrimaryEnemy(IntegerProperty_t pe)
{
    // ex GShip::setPrimaryEnemy
    m_currentData.primaryEnemy = pe;
    markDirty();
}

game::IntegerProperty_t
game::map::Ship::getDamage() const
{
    // ex GShip::getDamage
    return m_currentData.damage;
}

void
game::map::Ship::setDamage(IntegerProperty_t damage)
{
    // ex GShip::setDamage
    m_currentData.damage = damage;
    markDirty();
}

game::IntegerProperty_t
game::map::Ship::getCrew() const
{
    // ex GShip::getCrew
    return m_currentData.crew;
}

void
game::map::Ship::setCrew(IntegerProperty_t crew)
{
    // ex GShip::setCrew
    m_currentData.crew = crew;
    markDirty();
}

game::StringProperty_t
game::map::Ship::getFriendlyCode() const
{
    // ex GShip::getFCode, GShip::isFCodeKnown
    return m_currentData.friendlyCode;
}

void
game::map::Ship::setFriendlyCode(StringProperty_t fc)
{
    // ex GShip::setFCode
    m_currentData.friendlyCode = fc;
    markDirty();
}


/*
 *  Cargo accessors
 */

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

game::IntegerProperty_t
game::map::Ship::getCargo(Element::Type type) const
{
    // ex GShip::getCargoRaw, GShip::getCargo
    int numBays, expectedType, numLaunchers, torpedoType;
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
            if (getTorpedoType().get(torpedoType)) {
                if (torpedoType == expectedType) {
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
    int numBays, expectedType, torpedoType;
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
            && getTorpedoType().get(torpedoType)
            && torpedoType == expectedType)
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


/*
 *  Transporter accesssors
 */

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


/*
 *  Fleet accessors
 */

void
game::map::Ship::setFleetNumber(int fno)
{
    // ex GShip::setFleetNumber
    m_fleetNumber = fno;
    markDirty();
}

int
game::map::Ship::getFleetNumber() const
{
    // ex GShip::getFleetNumber
    return m_fleetNumber;
}

void
game::map::Ship::setFleetName(String_t name)
{
    // ex GShip::setFleetName
    m_fleetName = name;
    markDirty();
}

const String_t&
game::map::Ship::getFleetName() const
{
    // ex GShip::getFleetName
    return m_fleetName;
}

bool
game::map::Ship::isFleetLeader() const
{
    // ex GShip::isFleetLeader
    return m_fleetNumber == getId();
}

bool
game::map::Ship::isFleetMember() const
{
    // ex GShip::isFleetMember
    return m_fleetNumber != 0 && m_fleetNumber != getId();
}


/*
 *  Function accessors
 */

void
game::map::Ship::addShipSpecialFunction(game::spec::ModifiedHullFunctionList::Function_t function)
{
    // ex GShip::addShipFunction, hullfunc.pas:AddFunctionToShip
    // FIXME: avoid duplicates
    m_specialFunctions.push_back(function);
    markDirty();
}

bool
game::map::Ship::hasSpecialFunction(int basicFunction,
                                    const UnitScoreDefinitionList& scoreDefinitions,
                                    const game::spec::ShipList& shipList,
                                    const game::config::HostConfiguration& config) const
{
    // ex GShip::canDoSpecial, hullfunc.pas:ShipOrHullDoes
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

void
game::map::Ship::enumerateShipFunctions(game::spec::HullFunctionList& list,
                                        const game::spec::ShipList& shipList) const
{
    // ex GShip::enumShipSpecificSpecials, hullfunc.pas:EnumHullfuncsForShip
    const game::spec::ModifiedHullFunctionList& mhf = shipList.modifiedHullFunctions();
    for (size_t i = 0, n = m_specialFunctions.size(); i < n; ++i) {
        game::spec::HullFunction f;
        if (mhf.getFunctionDefinition(mhf.getFunctionIdFromHostId(m_specialFunctions[i]), f)) {
            list.add(f);
        }
    }
}

bool
game::map::Ship::hasAnyShipSpecialFunctions() const
{
    // ex GShip::hasAnyShipSpecificSpecials
    return !m_specialFunctions.empty();
}

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

game::NegativeProperty_t
game::map::Ship::getScore(int16_t scoreId, const UnitScoreDefinitionList& scoreDefinitions) const
{
    // ex phost.pas:GetExperienceLevel (sort-of)
    UnitScoreList::Index_t index;
    int16_t value, turn;
    if (scoreDefinitions.lookup(scoreId, index) && m_unitScores.get(index, value, turn)) {
        return value;
    } else {
        return afl::base::Nothing;
    }
}


/*
 *  MessageLink
 */

game::map::MessageLink&
game::map::Ship::messages()
{
    // ex GShip::getAssociatedMessages
    return m_messages;
}

const game::map::MessageLink&
game::map::Ship::messages() const
{
    return m_messages;
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
