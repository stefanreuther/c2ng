/**
  *  \file game/map/chunnelmission.cpp
  *  \brief Class game::map::ChunnelMission and related functions
  */

#include "game/map/chunnelmission.hpp"
#include "afl/base/countof.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "game/actions/cargotransfer.hpp"
#include "game/actions/cargotransfersetup.hpp"
#include "game/map/fleetmember.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/spec/basichullfunction.hpp"
#include "game/spec/mission.hpp"
#include "util/math.hpp"
#include "util/translation.hpp"

using game::spec::BasicHullFunction;
using game::config::HostConfiguration;
using game::map::ChunnelMission;
using game::actions::CargoTransfer;
using game::actions::CargoTransferSetup;

namespace {
    bool canReceiveChunnel(const game::map::Ship& ship, const game::UnitScoreDefinitionList& shipScores, const game::spec::ShipList& shipList, const game::Root& root)
    {
        return ship.hasSpecialFunction(BasicHullFunction::FirecloudChunnel, shipScores, shipList, root.hostConfiguration())
            || ship.hasSpecialFunction(BasicHullFunction::ChunnelTarget, shipScores, shipList, root.hostConfiguration());
    }

    int getInitiatorCapabilities(const game::map::Ship& ship, const game::UnitScoreDefinitionList& shipScores, const game::spec::ShipList& shipList, const game::Root& root)
    {
        int result;
        if (ship.hasSpecialFunction(BasicHullFunction::FirecloudChunnel, shipScores, shipList, root.hostConfiguration())) {
            // Ship can do everything
            result = ChunnelMission::chk_Self | ChunnelMission::chk_Others;
        } else {
            // Check both of the lesser abilities, ship may have both
            result = 0;
            if (ship.hasSpecialFunction(BasicHullFunction::ChunnelSelf, shipScores, shipList, root.hostConfiguration())) {
                result |= ChunnelMission::chk_Self;
            }
            if (ship.hasSpecialFunction(BasicHullFunction::ChunnelOthers, shipScores, shipList, root.hostConfiguration())) {
                result |= ChunnelMission::chk_Others;
            }
        }
        return result;
    }
}


// Constructor.
game::map::ChunnelMission::ChunnelMission()
    : m_target(0), m_failure(0), m_kind(0)
{ }

// Parse a ship's chunnel mission.
bool
game::map::ChunnelMission::check(const Ship& sh, const Universe& univ,
                                 const UnitScoreDefinitionList& scoreDefinitions,
                                 const game::spec::ShipList& shipList,
                                 const Root& root)
{
    // ex parseChunnelMission
    // ex shipacc.pas:CheckChunnel
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
        if (mate->isPlayable(Object::Playable) && canReceiveChunnel(*mate, scoreDefinitions, shipList, root)) {
            /* Target exists and can receive a chunnel. Can we start one?
               Note that we have rejected self-chunnel above. Hosts fail it
               implicitly for violating minimum distance. */
            m_kind = getInitiatorCapabilities(sh, scoreDefinitions, shipList, root);

            /* Found? */
            if (m_kind != 0) {
                m_target = sid;

                /* Now figure out failure modes
                   - mate is moving/being towed/fuelless/excessively damaged
                   - we have too little fuel
                   - we are moving/being towed/excessively damaged
                   - minimum distance violated */
                int minFuel = root.hostVersion().getMinimumFuelToInitiateChunnel() - 1;
                m_failure |= checkChunnelFailures(*mate, univ, 0, root);
                m_failure |= checkChunnelFailures(sh,    univ, minFuel, root) * (chf_Damaged/chf_MateDamaged);
                const HostConfiguration& config = root.hostConfiguration();
                if (root.hostVersion().hasExtendedMissions(config) && sh.getMission().orElse(0) == game::spec::Mission::pmsn_Training + config[HostConfiguration::ExtMissionsStartAt]()) {
                    m_failure |= chf_Training;
                }

                // Distance check
                Point shipPosition, matePosition;
                if (!sh.getPosition(shipPosition)
                    || !mate->getPosition(matePosition)
                    || !root.hostVersion().isValidChunnelDistance2(univ.config().getSquaredDistance(shipPosition, matePosition), config))
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
    // ex shipacc.pas:CheckChunnelFailures
    int result = 0;
    bool isPHost = root.hostVersion().getKind() == HostVersion::PHost; // FIXME: make accessors or solve this using appropriate default configuration
    if (isPHost && sh.getDamage().orElse(0) >= root.hostConfiguration()[HostConfiguration::DamageLevelForChunnelFail]()) {
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


afl::data::StringList_t
game::map::formatChunnelFailureReasons(int failures, afl::string::Translator& tx)
{
    // ex game/ship-form.cc:formatChunnelFailures
    static const char*const problems[] = {
        N_("Initiator damaged"),
        N_("Initiator needs fuel"),
        N_("Initiator moving"),
        N_("Initiator under tow"),
        N_("Initiator is training"),
        N_("Mate damaged"),
        N_("Mate needs fuel"),
        N_("Mate moving"),
        N_("Mate under tow"),
        N_("Distance too short"),
    };
    static const int problemFlags[] = {
        ChunnelMission::chf_Damaged,
        ChunnelMission::chf_Fuel,
        ChunnelMission::chf_Moving,
        ChunnelMission::chf_Towed,
        ChunnelMission::chf_Training,
        ChunnelMission::chf_MateDamaged,
        ChunnelMission::chf_MateFuel,
        ChunnelMission::chf_MateMoving,
        ChunnelMission::chf_MateTowed,
        ChunnelMission::chf_Distance,
    };

    afl::data::StringList_t result;
    for (size_t i = 0; i < countof(problemFlags); ++i) {
        if (failures & problemFlags[i]) {
            result.push_back(tx(problems[i]));
        }
    }
    return result;
}

// Check validity of a chunnel mate.
bool
game::map::isValidChunnelMate(const Ship& initiator,
                              const Ship& mate,
                              const Configuration& mapConfig,
                              const Root& root,
                              const UnitScoreDefinitionList& shipScores,
                              const game::spec::ShipList& shipList)
{
    // ex client/widgets/navwidget.cc:isValidChunnelMate
    int initOwner, mateOwner;
    Point initPos, matePos;

    return initiator.getId() != mate.getId()
        && initiator.getOwner(initOwner)
        && initiator.getPosition(initPos)
        && getInitiatorCapabilities(initiator, shipScores, shipList, root) != 0
        && mate.getOwner(mateOwner)
        && mate.getPosition(matePos)
        && mate.isPlayable(game::map::Object::ReadOnly)
        && mateOwner == initOwner
        && mate.getFleetNumber() == 0
        && canReceiveChunnel(mate, shipScores, shipList, root)
        && root.hostVersion().isValidChunnelDistance2(mapConfig.getSquaredDistance(initPos, matePos), root.hostConfiguration());
}

// Set up a chunnel.
void
game::map::setupChunnel(Ship& initiator, Ship& mate, Universe& univ,
                        const game::config::HostConfiguration& config,
                        const game::spec::ShipList& shipList)
{
    // Clear speed and waypoint, set FC
    {
        game::map::Point pt;
        initiator.getPosition(pt);

        FleetMember initFM(univ, initiator);
        initFM.setWaypoint(pt, config, shipList);
        initFM.setWarpFactor(0, config, shipList);
        initiator.setFriendlyCode(String_t(afl::string::Format("%03d", mate.getId())));
    }

    // Mate
    if (mate.isPlayable(game::map::Object::Playable)) {
        // For simplicity, use the fleet calls (although mates never are fleet members)
        game::map::Point pt;
        mate.getPosition(pt);

        FleetMember mateFM(univ, mate);
        mateFM.setWaypoint(pt, config, shipList);
        mateFM.setWarpFactor(0, config, shipList);

        // If mate has no fuel, try to correct
        if (mate.getCargo(Element::Neutronium).orElse(-1) == 0) {
            CargoTransferSetup setup = CargoTransferSetup::fromPlanetShip(univ, univ.findPlanetAt(pt), mate.getId());
            if (setup.isValid() && setup.isDirect()) {
                CargoTransfer tr;
                setup.buildDirect(tr, univ, config, shipList);
                tr.move(Element::Neutronium, /* amount: */ 1, /* from: */ 0, /* to: */ 1, /* partial: */ true, /* sellSupplies: */ false);
                tr.commit();
            }
        }
    }
}
