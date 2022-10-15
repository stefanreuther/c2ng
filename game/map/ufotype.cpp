/**
  *  \file game/map/ufotype.cpp
  *  \brief Class game::map::UfoType
  */

#include <cassert>
#include <cmath>
#include <cstdlib>
#include "game/map/ufotype.hpp"
#include "afl/string/format.hpp"
#include "game/tables/wormholestabilityname.hpp"
#include "util/math.hpp"

using game::config::HostConfiguration;
using afl::string::Format;

namespace {
    const char*const LOG_NAME = "game.map.ufo";

    /** Offset between index parameters and array indexes. */
    const game::Id_t ID_OFFSET = 1;

    /** Type code to use for Wormholes.
        This is the same code as the one used by PHost. */
    const int WORMHOLE_TYPE = 1;

    /** Color for wormholes.
        Green, same which is used by PHost. */
    const int WORMHOLE_COLOR = 2;


    /** Estimate movement of a wormhole.
        \param pos_now [in] Current position (X or Y)
        \param pos_old [in] Old position (X or Y)
        \param vec     [in] Old guess
        \param time    [in] Time that has passed between pos_old and pos_now
        \return new guess */
    int estimateMovement(int pos_now, int pos_old, int vec, int time, const HostConfiguration& config)
    {
        // ex ccmain.pas:GuesstimateMovement
        const int disp = config[HostConfiguration::WrmDisplacement]();
        const int rand = config[HostConfiguration::WrmRandDisplacement]();
        if (disp == 0) {
            /* We know wormholes don't move (other than possible Brownian movement
               through WrmRandDisplacement) */
            return 0;
        } else {
            /* Wormhole moves by n*WrmDisplacement, plus r*WrmRandDisplacement,
               where n, r are from [-1,+1], n is deterministic, r is random.
               We want to know n.

               |WrmRandDisplacement|
               |WrmRandDisplacement|           |WrmRandDisplacement|
               |WrmDisplacement|WrmDisplacement| */
            // FIXME: adjust /dif/ for wraparound
            // FIXME: PCC1 uses slightly different formulas
            int dif = pos_now - pos_old;
            if (dif > 0 && dif > time * rand) {
                /* We moved to the right by more than WrmRandDisplacement,
                   so that must be n>0 */
                return disp;
            } else if (dif < 0 && dif < -time * rand) {
                /* We moved to the left by more then WrmRandDisplacement,
                   so that must be n<0 */
                return -disp;
            } else if (std::abs(dif) <= time * rand
                       && ((disp > 2*rand)
                           || std::abs(dif) < time * (disp - rand)))
            {
                /* We moved within the bounds of WrmRandDisplacement, and
                   that position cannot be achieved by WrmDisplacement. */
                return 0;
            } else {
                /* No usable information derivable, return old guess */
                return vec;
            }
        }
    }
}


game::map::UfoType::UfoType()
    : m_ufos(),
      m_wormholes()
{
    // ex GUfoType::GUfoType
}

game::map::UfoType::~UfoType()
{
    // ex GUfoType::~GUfoType
}

game::map::Ufo*
game::map::UfoType::addUfo(int id, int type, int color)
{
    // ex GUfoType::getUfoById (sort-of)

    // Ufo color cannot be 0
    if (color == 0) {
        return 0;
    }

    // Find index. findUfoIndexById returns 1-based index.
    Id_t index = findUfoIndexById(id);
    Ufo* result = getUfoByIndex(index);
    if (result != 0 && result->getId() == id) {
        // Accept existing Ufo
    } else {
        // Create new Ufo
        size_t i = static_cast<size_t>(index - ID_OFFSET);
        assert(i <= m_ufos.size());
        result = m_ufos.insertNew(m_ufos.begin() + i, new Ufo(id));
    }

    if (result != 0) {
        result->setTypeCode(type);
        result->setColorCode(color);
    }
    return result;
}

void
game::map::UfoType::addMessageInformation(const game::parser::MessageInformation& info)
{
    namespace gp = game::parser;

    if (info.getObjectType() == gp::MessageInformation::Ufo) {
        // Try to obtain Ufo object
        Ufo* existing = getUfoById(info.getObjectId());
        if (existing == 0) {
            // Does not exist. Do we have the essential information to create it?
            // type/color are essential for addUfo().
            int32_t type, color, x, y;
            if (info.getValue(gp::mi_Type, type) && info.getValue(gp::mi_Color, color) && info.getValue(gp::mi_X, x) && info.getValue(gp::mi_Y, y)) {
                existing = addUfo(info.getObjectId(), type, color);
            }
        }

        // Assimilate data
        if (existing != 0) {
            existing->addMessageInformation(info);
        }
    } else if (info.getObjectType() == gp::MessageInformation::Wormhole) {
        // ex GUfoType::addWormholeData (part)
        Wormhole& h = addWormhole(info.getObjectId());
        for (gp::MessageInformation::Iterator_t i = info.begin(); i != info.end(); ++i) {
            if (gp::MessageIntegerValue_t* iv = dynamic_cast<gp::MessageIntegerValue_t*>(*i)) {
                switch (iv->getIndex()) {
                 case gp::mi_X:                     h.pos.setX(iv->getValue());       break;
                 case gp::mi_Y:                     h.pos.setY(iv->getValue());       break;
                 case gp::mi_Mass:                  h.mass = iv->getValue();          break;
                 case gp::mi_WormholeStabilityCode: h.stabilityCode = iv->getValue(); break;
                 case gp::mi_UfoRealId:             h.ufoId = iv->getValue();         break;
                 case gp::mi_WormholeBidirFlag:     h.bidirFlag = iv->getValue();     break;
                 default: break;
                }
            }
        }
    } else {
        // Ignore
    }
}

// Postprocess after load.
void
game::map::UfoType::postprocess(int turn, const Configuration& mapConfig, const game::config::HostConfiguration& config, afl::string::Translator& tx, afl::sys::LogListener& log)
{
    // ex GUfoType::postprocess
    const int firstId = config[HostConfiguration::WormholeUFOsStartAt]();
    const int lastId = firstId + 200;

    /* Convert util.dat wormholes into Ufos */
    if (!m_wormholes.empty()) {
        /* There is no reproducible 1:1 correspondence between wormhole Ids
           and Ufo Ids. PHost allocates two wormhole slots for each type of
           wormhole, but unidirectional wormholes get just one Ufo slot. Hence,
           we assume that the sequence of Ufos matches the sequence of wormholes
           if there are some Ufos inside the reserved range. If there are no
           Ufos (because the player is using Dosplan), we generate the sequence
           internally.

           This should work if we have multiple RSTs with identical status
           (i.e. all with Ufos, or all without), as we first merge all results'
           Ufo and wormhole sequences, before merging those sequences. */

        // First, find first wormhole Ufo we saw this turn.
        Id_t index = findUfoIndexById(firstId);
        Ufo* pu = getUfoByIndex(index);
        while (pu != 0 && !pu->isSeenThisTurn()) {
            ++index;
            pu = getUfoByIndex(index);
        }

        // Regular merging
        while (!m_wormholes.empty() && pu != 0 && pu->getId() < lastId) {
            // Merge
            const int wormholeId = m_wormholes.begin()->first;
            Wormhole& h = *m_wormholes.begin()->second;
            mergeWormhole(*pu, wormholeId, h, false, turn, config, tx, log);

            // Advance to next Ufo; skip those that we didn't see this turn
            do {
                ++index;
                pu = getUfoByIndex(index);
            } while (pu != 0 && !pu->isSeenThisTurn());

            // Drop wormhole
            m_wormholes.erase(m_wormholes.begin());
        }

        // Generate new Ufos for unconsumed wormholes
        while (!m_wormholes.empty()) {
            const int wormholeId = m_wormholes.begin()->first;
            Wormhole& h = *m_wormholes.begin()->second;
            if (Ufo* newUfo = addUfo(firstId + wormholeId, WORMHOLE_TYPE, WORMHOLE_COLOR)) {
                mergeWormhole(*newUfo, wormholeId, h, true, turn, config, tx, log);
            }
            m_wormholes.erase(m_wormholes.begin());
        }
    }

    // Postprocessing. This updates guessed positions.
    for (UfoList_t::iterator i = m_ufos.begin(); i != m_ufos.end(); ++i) {
        (*i)->postprocess(turn, mapConfig);
    }

    // Connect wormhole Ufos
    Id_t index = findUfoIndexById(firstId);
    while (Ufo* pu = getUfoByIndex(index)) {
        if (pu->getId() >= lastId) {
            break;
        }
        ++index;

        /* We can connect this Ufo with the next one if it has an even
           real_id, and the next one exists and has a one-higher Id */
        const int32_t thisId = pu->getRealId();
        if (thisId != 0 && (thisId & 1) == 0) {
            Ufo* next = getUfoByIndex(index);
            if (next != 0 && next->getRealId() == thisId+1) {
                pu->connectWith(*next);
                ++index;
            }
        }
    }
}

game::map::Ufo*
game::map::UfoType::getObjectByIndex(Id_t index)
{
    // ex GUfoType::getObjectByIndex, GUfoType::getUfoByIndex
    Ufo* p = getUfoByIndex(index);
    if (p != 0 && !p->isValid()) {
        p = 0;
    }
    return p;
}

game::Id_t
game::map::UfoType::getNextIndex(Id_t index) const
{
    // ex GUfoType::getNextIndex
    if (index < Id_t(m_ufos.size())) {
        return index+1;
    } else {
        return 0;
    }
}

game::Id_t
game::map::UfoType::getPreviousIndex(Id_t index) const
{
    // ex GUfoType::getPreviousIndex
    if (index == 0) {
        return static_cast<Id_t>(m_ufos.size());
    } else {
        return index-1;
    }
}

game::Id_t
game::map::UfoType::findUfoIndexById(int id) const
{
    // ex GUfoType::getFirstAt, sort-of
    // FIXME: binary search?
    UfoList_t::iterator i = m_ufos.begin();
    while (i != m_ufos.end() && (*i)->getId() < id) {
        ++i;
    }
    return ID_OFFSET + Id_t(i - m_ufos.begin());
}

game::map::Ufo*
game::map::UfoType::getUfoByIndex(Id_t index)
{
    if (index > 0 && index <= Id_t(m_ufos.size())) {
        return m_ufos[index - ID_OFFSET];
    } else {
        return 0;
    }
}

game::map::Ufo*
game::map::UfoType::getUfoById(int id)
{
    Ufo* p = getUfoByIndex(findUfoIndexById(id));
    if (p && p->getId() != id) {
        p = 0;
    }
    return p;
}

// Get/add wormhole.
game::map::UfoType::Wormhole&
game::map::UfoType::addWormhole(int id)
{
    Wormhole* p = m_wormholes[id];
    if (p == 0) {
        p = m_wormholes.insertNew(id, new Wormhole());
    }
    return *p;
}

// Merge wormhole data.
void
game::map::UfoType::mergeWormhole(Ufo& ufo, int wormholeId, const Wormhole& data, bool isNew, int turnNumber, const game::config::HostConfiguration& config, afl::string::Translator& tx, afl::sys::LogListener& log)
{
    // ex GUfo::addWormholeData, ccmain.pas::ConsumeWormhole
    int mass = 0;
    bool massOK = data.mass.get(mass);
    if (isNew) {
        // Making a new wormhole Ufo, so fill in header
        ufo.setName(Format(tx("Wormhole #%d"), wormholeId));

        // Mass/directionality
        if (massOK) {
            int flag;
            String_t fmt = (data.bidirFlag.get(flag)
                            ? (flag != 0
                               ? tx("%d kt/Bidir.")
                               : tx("%d kt/Enter only"))
                            : tx("%d kt"));
            ufo.setInfo1(Format(fmt, mass));
        }
    } else {
        // Just updating an old Ufo
        Point oldPosition;
        if (ufo.getPosition(oldPosition) && oldPosition != data.pos) {
            log.write(afl::sys::LogListener::Warn, LOG_NAME,
                      Format(tx("Ufo #%d and wormhole #%d do not match."), ufo.getId(), wormholeId));
        }
    }

    ufo.setRealId(wormholeId);
    ufo.setColorCode(WORMHOLE_COLOR);
    ufo.setTypeCode(WORMHOLE_TYPE);

    int stabilityCode;
    if (data.stabilityCode.get(stabilityCode)) {
        // Our stability codes include a percentage and override what the host gave
        ufo.setInfo2(game::tables::WormholeStabilityName(tx)(stabilityCode));
    }
    ufo.setPosition(data.pos);
    ufo.setSpeed(0);
    ufo.setHeading(afl::base::Nothing);

    int range, radius;
    if (!massOK || mass <= 0) {
        // degenerate case
        range = 0;
        radius = 2;
    } else {
        // normal case
        range  = util::roundToInt(10 * std::exp(std::log(double(mass)) / 3));
        radius = util::roundToInt(std::exp(config[HostConfiguration::WrmEntryPowerX100]()*0.01 * std::log(double(mass))) / 2);
    }
    ufo.setPlanetRange(range);
    ufo.setShipRange(range);
    ufo.setRadius(radius);

    ufo.setIsSeenThisTurn(true);
    ufo.setIsStoredInHistory(true);

    // Estimate movement
    const int lastTurnNumber = ufo.getLastTurn();
    if (lastTurnNumber > 0 && lastTurnNumber < turnNumber) {
        const Point lastPos = ufo.getLastPosition();
        const Point vec     = ufo.getMovementVector();
        const int numTurns  = turnNumber - lastTurnNumber;
        const int newVecX   = estimateMovement(data.pos.getX(), lastPos.getX(), vec.getX(), numTurns, config);
        const int newVecY   = estimateMovement(data.pos.getY(), lastPos.getY(), vec.getY(), numTurns, config);

        ufo.setMovementVector(Point(newVecX, newVecY));
    }
}
