/**
  *  \file game/map/minefield.cpp
  *  \brief Class game::map::Minefield
  *
  *  FIXME: consider where we have to raise sig_change.
  */

#include <cmath>
#include "game/map/minefield.hpp"
#include "afl/string/format.hpp"
#include "util/math.hpp"

// Constructor and Destructor:
game::map::Minefield::Minefield(Id_t id)
    : CircularObject(id),
      m_position(),
      m_owner(0),
      m_isWeb(false),
      m_units(0),
      m_turn(0),
      m_reason(NoReason),
      m_previousTurn(0),
      m_previousUnits(0),
      m_currentTurn(0),
      m_currentRadius(0),
      m_currentUnits(0)
{
    // ex GMinefield::GMinefield
}

game::map::Minefield::Minefield(const Minefield& other)
    : CircularObject(other),
      m_position(other.m_position),
      m_owner(other.m_owner),
      m_isWeb(other.m_isWeb),
      m_units(other.m_units),
      m_turn(other.m_turn),
      m_reason(other.m_reason),
      m_previousTurn(other.m_previousTurn),
      m_previousUnits(other.m_previousUnits),
      m_currentTurn(other.m_currentTurn),
      m_currentRadius(other.m_currentRadius),
      m_currentUnits(other.m_currentUnits)
{ }

game::map::Minefield::Minefield(Id_t id, Point center, int owner, bool isWeb, int32_t units)
    : CircularObject(id),
      m_position(center),
      m_owner(owner),
      m_isWeb(isWeb),
      m_units(units),
      m_turn(0),
      m_reason(MinefieldScanned),
      m_previousTurn(0),
      m_previousUnits(units),
      m_currentTurn(0),
      m_currentRadius(getRadiusFromUnits(units)),
      m_currentUnits(units)
{ }

game::map::Minefield::~Minefield()
{ }

// Object:
String_t
game::map::Minefield::getName(ObjectName /*which*/, afl::string::Translator& tx, InterpreterInterface& iface) const
{
    // ex GMinefield::getName
    String_t result;
    if (!isValid()) {
        result = afl::string::Format(tx("Deleted Mine Field #%d"), getId());
    } else {
        if (m_isWeb) {
            result = afl::string::Format(tx("Web Mine Field #%d"), getId());
        } else {
            result = afl::string::Format(tx("Mine Field #%d"), getId());
        }

        String_t adj;
        if (iface.getPlayerAdjective(m_owner, adj)) {
            result += " (";
            result += adj;
            result += ")";
        }
    }
    return result;
}

afl::base::Optional<int>
game::map::Minefield::getOwner() const
{
    // ex GMinefield::getOwner
    if (isValid()) {
        return m_owner;
    } else {
        return afl::base::Nothing;
    }
}

afl::base::Optional<game::map::Point>
game::map::Minefield::getPosition() const
{
    // ex GMinefield::getPos
    if (isValid()) {
        return m_position;
    } else {
        return afl::base::Nothing;
    }
}

// CircularObject:
afl::base::Optional<int>
game::map::Minefield::getRadius() const
{
    // ex GMinefield::getRadius
    if (isValid()) {
        return m_currentRadius;
    } else {
        return afl::base::Nothing;
    }
}

afl::base::Optional<int32_t>
game::map::Minefield::getRadiusSquared() const
{
    // ex GMinefield::getRadiusSquared
    if (isValid()) {
        return m_currentUnits;
    } else {
        return afl::base::Nothing;
    }
}

void
game::map::Minefield::addReport(const Point pos,
                                const int owner,
                                const TypeReport type,
                                const SizeReport size,
                                const int32_t sizeValue,
                                const int turn,
                                const ReasonReport reason)
{
    // ex GMinefield::addMinefieldReport
    // Original version takes a TMinefieldReport which maps as follows:
    //     x,y -> pos
    //     owner
    //     web, type_known -> type
    //     units_known, radius_or_units -> size, sizeValue
    //     turn
    //     why -> reason

    // Is this information actually newer?
    if (turn < m_turn) {
        return;
    }

    // Is this the same field we already saw?
    const bool isSameField = (m_owner == owner && m_position == pos);

    // If we saw the minefield already with better reason, ignore this report.
    // For example, when laying and scooping a minefield in the same turn,
    // we get a Lay(size=X) report followed by a Sweep/Scoop(size=0) in util.dat.
    // Further Lay(size=X) reports, e.g. from messages, shall not override that.
    if (isSameField && turn == m_turn && reason < m_reason) {
        return;
    }

    // Turn change: move previous values into archive.
    if (turn > m_turn) {
        if (isSameField) {
            m_previousUnits = m_units;
            m_previousTurn = m_turn;
        } else {
            m_previousUnits = 0;
            m_previousTurn = 0;
        }
    }

    // Figure out unit count.
    if (size == UnitsKnown) {
        // Units known exactly.
        m_units = sizeValue;
    } else {
        // Units not known exactly. Check range.
        const int32_t newUnits = util::squareInteger(sizeValue);

        /* THost uses ERND(Sqrt(units)), PHost uses Trunc(Sqrt(units)).
           Therefore, actual radius is [r,r+1) in PHost, [r-.5,r+.5] for
           THost. Hence, possible unit ranges corresponding to this
           radius are (r-.5)² = r²-r-.25 and (r+1)². */
        const int32_t minUnits = newUnits - sizeValue - 1;
        const int32_t maxUnits = util::squareInteger(sizeValue + 1);

        if (isSameField
            && m_turn == turn
            && m_units >= minUnits
            && m_units <  maxUnits)
        {
            // The minefield was already seen this turn, with better information (exact unit count).
            // No change.
        } else {
            m_units = newUnits;
        }
    }

    // Update minefield. Avoid updating the type if it is not reliably known.
    m_position = pos;
    m_owner = owner;
    m_turn = turn;
    switch (type) {
     case UnknownType:
        if (!isSameField) {
            // We don't know what type it is, but it's different from what we have in the database, so reset the type.
            m_isWeb = false;
        }
        break;
     case IsMine:
        m_isWeb = false;
        break;
     case IsWeb:
        m_isWeb = true;
        break;
    }

    // Update cause.
    if (reason > m_reason || !isSameField) {
        m_reason = reason;
    }
}

void
game::map::Minefield::internalCheck(int currentTurn, const game::HostVersion& host, const game::config::HostConfiguration& config)
{
    // ex GMinefield::internalCheck, ccmain.pas:CalcMineDecay
    int32_t u = m_units;
    for (int i = m_turn; i < currentTurn; ++i) {
        u = getUnitsAfterDecay(u, host, config);
    }
    m_currentTurn = currentTurn;
    m_currentUnits = u;
    m_currentRadius = getRadiusFromUnits(u);
}

void
game::map::Minefield::erase(afl::base::Signal<void(Id_t)>* sig)
{
    m_position = Point();
    m_owner = 0;
    m_isWeb = false;
    m_units = 0;
    m_turn = 0;
    m_reason = NoReason;
    m_previousTurn = 0;
    m_previousUnits = 0;
    m_currentTurn = 0;
    m_currentRadius = 0;
    m_currentUnits = 0;

    // We must raise the "set change" signal before the "object change" signal,
    // to give observers a chance to take their hands off this object.
    // Otherwise, they would briefly see a deleted object, which they do not expect
    // (cursors try to show only valid objects).
    if (sig != 0) {
        sig->raise(0);
    }
    sig_change.raise(getId());
}

void
game::map::Minefield::setUnits(int32_t units)
{
    if (units != m_currentUnits) {
        // Update units
        m_currentUnits = units;
        m_currentRadius = getRadiusFromUnits(units);
        m_units = units;

        // Update scan meta-information
        m_turn = m_currentTurn;
        m_reason = NoReason;

        sig_change.raise(getId());
    }
}

bool
game::map::Minefield::isValid() const
{
    // This method does not exist in PCC 2.0.
    // We cannot let objects die (bug #308), so a swept minefield must be able to stay around.
    // A minefield cannot ever have owner zero, so that is our test.
    return m_owner != 0;
}

bool
game::map::Minefield::isWeb() const
{
    return m_isWeb;
}

game::map::Minefield::ReasonReport
game::map::Minefield::getReason() const
{
    // ex GMinefield::getAction
    // PCC2 also had a setAction which was not referenced.
    return m_reason;
}

int32_t
game::map::Minefield::getUnits() const
{
    return m_currentUnits;
}

int32_t
game::map::Minefield::getUnitsAfterDecay(int32_t origUnits, const game::HostVersion& host, const game::config::HostConfiguration& config) const
{
    // ex GMinefield::getUnitsAfterDecay
    // ex accessor.pas:MinesAfterDecay
    int decayRate = m_isWeb ? config[config.WebMineDecayRate](m_owner) : config[config.MineDecayRate](m_owner);
    if (!host.isRoundingMineDecay()) {
        /* PHost formula */
        return origUnits * (100 - decayRate) / 100;
    } else {
        /* THost formula (3.22.040). Actual formula is
             ERND(origUnits - origUnits*decayRate/100) - 1
           which should yield the same results. */
        /* Note that THost 3.0 does not have MineDecayRate, and thus only does "origUnits-1". */
        return std::max(0, util::divideAndRoundToEven(origUnits * (100-decayRate), 100, 0) - 1);
    }
}

int32_t
game::map::Minefield::getUnitsForLaying(const game::HostVersion& host, const game::config::HostConfiguration& config) const
{
    // ex GMinefield::getUnitsForLaying, ship.pas:MinefieldUnitsForLaying
    if (host.isMineLayingAfterMineDecay()) {
        return getUnitsAfterDecay(getUnits(), host, config);
    } else {
        return getUnits();
    }
}

int
game::map::Minefield::getTurnLastSeen() const
{
    return m_turn;
}

int32_t
game::map::Minefield::getUnitsLastSeen() const
{
    return m_units;
}

double
game::map::Minefield::getPassRate(double distance, bool cloaked, int player, const game::config::HostConfiguration& config) const
{
    // ex GMinefield::getPassRate
    double rate = (isWeb()
                   ? config[config.WebMineHitOdds](player) * 0.01
                   : cloaked
                   ? config[config.MineHitOddsWhenCloakedX10](player) * 0.001
                   : config[config.MineHitOdds](player) * 0.01);
    if (rate <= 0) {
        /* Hit rate below zero -> pass rate is 1.0 */
        return 1.0;
    } else if (rate >= 1.0) {
        /* Hit rate is one -> pass rate is 0.0 */
        return 0.0;
    } else {
        /* Normal computation */
        return std::pow(1.0 - rate, distance);
    }
}

int32_t
game::map::Minefield::getRadiusFromUnits(int32_t units)
{
    // ex GMinefield::getRadiusFromUnits
    // FIXME: rounding?
    return int(std::sqrt(double(units)));
}
