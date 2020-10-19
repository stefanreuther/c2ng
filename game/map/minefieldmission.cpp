/**
  *  \file game/map/minefieldmission.cpp
  *  \brief Class game::map::MinefieldMission
  */

#include "game/map/minefieldmission.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/hostversion.hpp"
#include "game/map/minefield.hpp"
#include "game/map/minefieldtype.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/registrationkey.hpp"
#include "game/spec/mission.hpp"
#include "util/math.hpp"
#include "util/string.hpp"

using game::spec::Mission;

game::map::MinefieldMission::MinefieldMission()
    : m_mineId(0),
      m_owner(0),
      m_isWeb(false),
      m_numTorpedoes(0),
      m_numUnits(0),
      m_usedMission(false),
      m_usedFriendlyCode(false)
{ }

bool
game::map::MinefieldMission::checkLayMission(const Ship& ship,
                                             const Universe& univ,
                                             const Root& root,
                                             const UnitScoreDefinitionList& shipScores,
                                             const game::spec::ShipList& shipList)
{
    return checkLayMission(ship, univ, root.hostVersion(), root.registrationKey(), root.hostConfiguration(), shipScores, shipList);
}

bool
game::map::MinefieldMission::checkLayMission(const Ship& ship, const Universe& univ,
                                             const HostVersion& hostVersion,
                                             const RegistrationKey& key,
                                             const game::config::HostConfiguration& config,
                                             const UnitScoreDefinitionList& shipScores,
                                             const game::spec::ShipList& shipList)
{
    // ex parseMineLayingMission
    // ex shipacc.pas:ComputeMineDrop (part)
    // Check whether all required values are known
    int mission = 0, towId = 0, interceptId = 0, torpedoType = 0, numLaunchers = 0, torps = 0, owner = 0;
    if (!ship.getMission().get(mission)
        || !ship.getMissionParameter(TowParameter).get(towId)
        || !ship.getMissionParameter(InterceptParameter).get(interceptId)
        || !ship.getTorpedoType().get(torpedoType)
        || !ship.getNumLaunchers().get(numLaunchers)
        || !ship.getAmmo().get(torps)
        || !ship.getRealOwner().get(owner))
    {
        return false;
    }

    // Check whether ship can lay mines
    if (torpedoType <= 0 || numLaunchers <= 0 || torps <= 0) {
        return false;
    }

    // Check mission
    bool usemix = false, usemdx = false;     // Use mix/mdx fcode?
    bool makeweb = false;                    // Make a web field?
    int race = 0;                            // Lay as this race?
    int reqid = 0;                           // Require this Id?
    int torplimit = 0;                       // Torpedo limit
    if (mission == Mission::msn_LayMines) {
        // Lay mines: honors mix, mdx, but no other parameters
        usemix = usemdx = true;
    } else if (mission == Mission::msn_Special) {
        // Lay web (race is checked later): honors, mix, mdx, but no other parameters
        usemix = usemdx = true;
        makeweb = true;
    } else if (!hostVersion.isPHost() || key.getStatus() != RegistrationKey::Registered) {
        // Following missions only for registered players in PHost
        // FIXME: as for fcodes, we could query mission.cc for presence of these missions
        return false;
    } else if (mission == config[config.ExtMissionsStartAt]() + Mission::pmsn_LayMines) {
        // Lay Minefield: honors no fcodes, registered-only
        torplimit = interceptId;
        race      = towId;
    } else if (mission == config[config.ExtMissionsStartAt]() + Mission::pmsn_LayWeb) {
        // Lay Web Minefield: honors no fcodes, registered-only
        torplimit = interceptId;
        race      = towId;
        makeweb   = true;
    } else if (mission == config[config.ExtMissionsStartAt]() + Mission::pmsn_LayMinesIn) {
        // Lay Mines In: honors miX
        usemix    = true;
        torplimit = interceptId;
        reqid     = towId;
    } else if (mission == config[config.ExtMissionsStartAt]() + Mission::pmsn_LayWebIn) {
        // Lay Web Mines In: honors miX
        usemix    = true;
        torplimit = interceptId;
        reqid     = towId;
        makeweb   = true;
    } else {
        // Not a mine laying mission
        return false;
    }

    // Postprocess fcodes
    bool used_fc = false;
    if (usemix || usemdx) {
        /* We accept a friendly code if it
               - is not listed in fcodes.cc (assuming that is blank, missing, unmaintained)
           OR  -     works on the ship
                 AND is not restricted to registered ships, or player is registered */
        const String_t fc  = ship.getFriendlyCode().orElse(String_t());
        const game::spec::FriendlyCodeList& fcList = shipList.friendlyCodes();
        const game::spec::FriendlyCodeList::Iterator_t fci = fcList.getCodeByName(fc);
        const bool validfc = (fci == fcList.end()
                              || (*fci != 0
                                  && (*fci)->worksOn(ship, shipScores, shipList, config)
                                  && (*fci)->isPermitted(key)));
        if (validfc && fc.size() == 3) {
            if (usemix && fc[0] == 'm' && fc[1] == 'i') {
                // miX
                used_fc = util::parsePlayerCharacter(fc[2], race)
                    && race > 0;
            } else if (usemdx && fc[0] == 'm' && fc[1] == 'd') {
                // mdX
                if (fc[2] == '0') {
                    torplimit = 100, used_fc = true;
                } else if (fc[2] >= '1' && fc[2] <= '9') {
                    torplimit = 10*(fc[2]-'0'), used_fc = true;
                } else if (fc[2] == 'h') {
                    torplimit = torps/2, used_fc = true;
                } else if (fc[2] == 'q') {
                    torplimit = torps/4, used_fc = true;
                } else {
                    // unrecognized
                }
            }
        }
    }

    // Postprocess race and limit
    if (race <= 0 || race > MAX_PLAYERS) {
        // FIXME: normally checks for NUM_PLAYERS=11
        race = owner;
    }
    if (torplimit > 0 && torplimit < torps) {
        torps = torplimit;
    }

    // Check whether this type is allowed
    if (makeweb) {
        if (config.getPlayerMissionNumber(owner) != 7 || !config[config.AllowWebMines]()) {
            return false;
        }
    } else {
        if (!config[config.AllowMinefields]()) {
            return false;
        }
    }

    // Postprocess race, reqid
    const MinefieldType& mfc = univ.minefields();
    if (reqid != 0) {
        // We're requiring a particular minefield, so check whether it exists and matches our parameters.
        // If it doesn't, we refuse to lay mines, in the same way PHost does.
        const Minefield* mf = mfc.get(reqid);
        if (!mf) {
            return false;
        }

        if (hostVersion.hasAutomaticMineIdentity()) {
            mf->getOwner(race);
        }

        int mfOwner = 0;
        Point mfPos, shipPos;
        mf->getOwner(mfOwner);
        mf->getPosition(mfPos);
        ship.getPosition(shipPos);
        if (mfOwner != race
            || mf->isWeb() != makeweb
            || univ.config().getSquaredDistance(mfPos, shipPos) > mf->getUnitsForLaying(hostVersion, config))
        {
            return false;
        }
    } else {
        // We're not requiring a particular minefield, so find one.
        int32_t closest = 0;
        Point shipPos;
        ship.getPosition(shipPos);
        if (hostVersion.hasMinefieldCenterBug()) {
            for (Id_t i = mfc.findNextIndex(0); i != 0; i = mfc.findNextIndex(i)) {
                if (const Minefield* mf = mfc.get(i)) {
                    int mfOwner;
                    if (mf->getOwner(mfOwner) && mfOwner == race && mf->isWeb() == makeweb) {
                        Point mfPos;
                        mf->getPosition(mfPos);
                        int32_t dist = univ.config().getSquaredDistance(mfPos, shipPos);
                        if (reqid == 0 || dist < closest) {
                            // Minefield matches type and is close.
                            // We note its Id only when we're inside;
                            // set it to zero if we're outside the field and make a new one
                            reqid = (dist <= mf->getUnitsForLaying(hostVersion, config) ? i : -1);
                            closest = dist;
                        }
                    }
                }
            }
            if (reqid < 0) {
                reqid = 0;
            }
        } else {
            for (Id_t i = mfc.findNextIndex(0); i != 0; i = mfc.findNextIndex(i)) {
                if (const Minefield* mf = mfc.get(i)) {
                    int mfOwner;
                    if (mf->getOwner(mfOwner) && mfOwner == race && mf->isWeb() == makeweb) {
                        Point mfPos;
                        mf->getPosition(mfPos);
                        int32_t dist = univ.config().getSquaredDistance(mfPos, shipPos);
                        if (dist <= mf->getUnitsForLaying(hostVersion, config)) {
                            // Minefield matches type and is close, and we're inside
                            reqid = i;
                            closest = dist;
                            break;
                        }
                    }
                }
            }
        }
    }

    // Find mine laying rate
    int rate, srate, mrate;
    if (makeweb) {
        srate = config[config.UnitsPerWebRate](owner);
        mrate = config[config.UnitsPerWebRate](race);
    } else {
        srate = config[config.UnitsPerTorpRate](owner);
        mrate = config[config.UnitsPerTorpRate](race);
    }
    rate = (srate < mrate ? srate : mrate);

    // We now know that we're laying mines, so fill in the structure
    m_mineId = reqid;
    m_owner = race;
    m_isWeb = makeweb;
    m_numTorpedoes = torps;
    m_numUnits = int32_t(torps) * rate * util::squareInteger(ship.getTorpedoType().orElse(1)) / 100;
    m_usedMission = true;
    m_usedFriendlyCode = used_fc;
    return true;
}

bool
game::map::MinefieldMission::checkScoopMission(const Ship& ship,
                                               const Root& root,
                                               const UnitScoreDefinitionList& shipScores,
                                               const game::spec::ShipList& shipList)
{
    // ex parseMineScoopMission
    // Check whether all required values are known
    int mission = 0, towId = 0, interceptId = 0, torpedoType = 0, numLaunchers = 0, beamType = 0, numBeams = 0, owner = 0;
    if (!ship.getMission().get(mission)
        || !ship.getMissionParameter(TowParameter).get(towId)
        || !ship.getMissionParameter(InterceptParameter).get(interceptId)
        || !ship.getTorpedoType().get(torpedoType)
        || !ship.getNumLaunchers().get(numLaunchers)
        || !ship.getBeamType().get(beamType)
        || !ship.getNumBeams().get(numBeams)
        || !ship.getRealOwner().get(owner))
    {
        return false;
    }

    // Check whether ship can scoop mines
    const game::config::HostConfiguration& config = root.hostConfiguration();
    const HostVersion& host = root.hostVersion();
    if (torpedoType <= 0
        || numLaunchers <= 0
        || (host.isBeamRequiredForMineScooping() && (beamType <= 0 || numBeams <= 0)))
    {
        return false;
    }

    // Do we want to scoop?
    if (mission == Mission::msn_MineSweep) {
        // Check for "msc" fcode
        const String_t shipFC = ship.getFriendlyCode().orElse(String_t());
        const String_t fc  = ship.getFriendlyCode().orElse(String_t());
        const game::spec::FriendlyCodeList& fcList = shipList.friendlyCodes();
        const game::spec::FriendlyCodeList::Iterator_t fci = fcList.getCodeByName(fc);

        if (fc == "msc"
            && (fci == fcList.end()
                || (*fci != 0
                    && (*fci)->worksOn(ship, shipScores, shipList, root.hostConfiguration())
                    && (*fci)->isPermitted(root.registrationKey()))))
        {
            // accept
            m_mineId           = 0;
            m_owner            = owner;
            m_isWeb            = false; // irrelevant for scooping
            m_numTorpedoes     = 0;     // means: no limit
            m_numUnits         = 0;     // irrelevant for scooping
            m_usedMission      = true;
            m_usedFriendlyCode = true;
            return true;
        } else {
            // reject
            return false;
        }
    } else if (host.isPHost()
               && root.registrationKey().getStatus() == RegistrationKey::Registered
               && mission == config[config.ExtMissionsStartAt]() + Mission::pmsn_ScoopTorps)
    {
        // PHost mission
        m_mineId           = towId;
        m_owner            = owner;
        m_isWeb            = false;
        m_numTorpedoes     = interceptId;
        m_numUnits         = 0;
        m_usedMission      = true;
        m_usedFriendlyCode = false;
        return true;
    } else {
        return false;
    }
}

// Mission ordering:
//   PHost:          THost:
//     Mine decay
//     Mine lay      Mine lay
//     MDM
//     Mine Sweep    Mine sweep
//                   Mine decay
//                   MDM
//     Web drain     Web drain
// --> lay/sweep is after decay in PHost, before decay in Host (getUnitsForLaying())
