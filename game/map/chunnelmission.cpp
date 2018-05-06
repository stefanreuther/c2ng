/**
  *  \file game/map/chunnelmission.cpp
  */

#include "game/map/chunnelmission.hpp"
#include "afl/string/parse.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/spec/hullfunction.hpp"
#include "game/spec/mission.hpp"
#include "util/math.hpp"

using game::spec::HullFunction;


game::map::ChunnelMission::ChunnelMission()
    : m_target(0), m_failure(0), m_kind(0)
{ }

// /** Check chunnel mission.
//     \param sh [in] Potential chunnel initiator
//     \param univ [in] Universe
//     \param cd [out] Chunnel info
//     \return true iff ship is chunneling

//     Postconditions:
//     - if ship does not chunnel, ch.target is 0
//     - if ship does chunnel, ch.target is ship id, and target ship does exist
//       . then, ch.failure contains bitfield of reasons why chunnel fails
//         (consequently, it's 0 to predict success)
//       . ch.kind is the kind of chunnel, bitfield of ship classes to take with
//         me (consequently, nonzero for a successful result)

//     Originally shipacc.pas::CheckChunnel. */
bool
game::map::ChunnelMission::check(const Ship& sh, const Universe& univ,
                                 const UnitScoreDefinitionList& scoreDefinitions,
                                 const game::spec::ShipList& shipList,
                                 const Root& root)
{
    // ex parseChunnelMission
    m_target  = 0;
    m_failure = 0;
    m_kind    = 0;

    /* For simplicity, check 3-place numeric FCs only */
    String_t fc;
    int sid = 0;
    const Ship* mate = 0;
    if (sh.getFriendlyCode().get(fc)
        && fc.size() == 3
        && afl::string::strToInteger(fc, sid)
        && sid != sh.getId()
        && (mate = univ.ships().get(sid)) != 0)
    {
        if (mate->isPlayable(Object::Playable)
            && (mate->hasSpecialFunction(HullFunction::FirecloudChunnel, scoreDefinitions, shipList, root.hostConfiguration())
                || mate->hasSpecialFunction(HullFunction::ChunnelTarget, scoreDefinitions, shipList, root.hostConfiguration())))
        {
            /* Target exists and can receive a chunnel. Can we start one?
               Note that we have rejected self-chunnel above. Hosts fail it
               implicitly for violating minimum distance. */
            if (sh.hasSpecialFunction(HullFunction::FirecloudChunnel, scoreDefinitions, shipList, root.hostConfiguration())) {
                m_kind = chk_Self | chk_Others;
            } else if (sh.hasSpecialFunction(HullFunction::ChunnelSelf, scoreDefinitions, shipList, root.hostConfiguration())) {
                m_kind = chk_Self;
            } else if (sh.hasSpecialFunction(HullFunction::ChunnelOthers, scoreDefinitions, shipList, root.hostConfiguration())) {
                m_kind = chk_Others;
            } else {
                // Not a chunneler
            }

            /* Found? */
            if (m_kind != 0) {
                m_target = sid;

                /* Now figure out failure modes
                   - mate is moving/being towed/fuelless/excessively damaged
                   - we have < 51 fuel
                   - we are moving/being towed/excessively damaged
                   - minimum distance violated */
                m_failure |= checkChunnelFailures(*mate, univ,  0, root);
                m_failure |= checkChunnelFailures(sh,    univ, 50, root) * (chf_Damaged/chf_MateDamaged);
                bool isPHost = root.hostVersion().getKind() == HostVersion::PHost; // FIXME: make accessors!
                const game::config::HostConfiguration& config = root.hostConfiguration();
                if (root.hostVersion().hasExtendedMissions(config) && sh.getMission().orElse(0) == game::spec::Mission::pmsn_Training + config[config.ExtMissionsStartAt]()) {
                    m_failure |= chf_Training;
                }

                /* Distance check:
                   HOST uses    ERND(Sqrt(....)) >= 100, i.e.   Sqrt(....) >= 99.5
                   PHost uses   Trunc(Sqrt(....)) >= MCD, i.e.  Sqrt(....) >= MCD */
                int32_t dist = isPHost ? util::squareInteger(config[config.MinimumChunnelDistance]()) : 9901;
                Point shipPosition, matePosition;
                if (!sh.getPosition(shipPosition)
                    || !mate->getPosition(matePosition)
                    || univ.config().getSquaredDistance(shipPosition, matePosition) < dist)
                {
                    m_failure |= chf_Distance;
                }
            }
        }
    }

    return m_kind != 0;
}

/** Check for possible chunnel failures.
    \param sh [in] Ship to check
    \param univ [in] Universe to check
    \param minFuel [in] Minimum fuel required on this ship */
int
game::map::ChunnelMission::checkChunnelFailures(const Ship& sh, const Universe& univ, const int minFuel, const Root& root)
{
    int result = 0;
    bool isPHost = root.hostVersion().getKind() == HostVersion::PHost; // FIXME: make accessors or solve this using appropriate default configuration
    if (isPHost && sh.getDamage().orElse(0) >= root.hostConfiguration()[game::config::HostConfiguration::DamageLevelForChunnelFail]()) {
        result |= chf_MateDamaged;
    }
    if (sh.getCargo(game::Element::Neutronium).orElse(0) <= minFuel) {
        result |= chf_MateFuel;
    }
    if (sh.getWarpFactor().orElse(0) > 0) {
        result |= chf_MateMoving;
    }
    if (univ.findShipTowing(sh.getId()) != 0) {
        result |= chf_MateTowed;
    }
    return result;
}
