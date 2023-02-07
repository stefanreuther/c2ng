/**
  *  \file game/map/minefieldformula.cpp
  *  \brief Minefield Formulas
  */

#include "game/map/minefieldformula.hpp"
#include "game/map/configuration.hpp"
#include "game/map/minefieldmission.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "util/math.hpp"

using game::config::HostConfiguration;

namespace game { namespace map { namespace {

    bool hasPossibleEnemyShip(const Universe& univ, Point pt, int owner)
    {
        bool hasOwnShip = false;
        const ObjectVector<Ship>& ships = univ.ships();
        for (Id_t i = 1; i <= ships.size(); ++i) {
            if (const Ship* pShip = ships.get(i)) {
                Point shipPos;
                int shipOwner = 0;
                if (pShip->getPosition().get(shipPos)
                    && shipPos == pt
                    && pShip->getRealOwner().get(shipOwner))
                {
                    if (shipOwner == owner) {
                        hasOwnShip = true;
                    }
                    if (shipOwner != owner) {
                        return true;
                    }
                }
            }
        }

        // No enemy ship seen. If we have an own ship, assume we would have seen it.
        return !hasOwnShip;
    }

    void addMinefield(MinefieldEffects_t& result, const Minefield& field, int32_t radiusChange, int numTorps, const Universe& univ, const Configuration& mapConfig, const HostVersion& host, const HostConfiguration& config)
    {
        Point center;
        int owner;
        if (field.getPosition().get(center) && field.getOwner().get(owner)) {
            const int32_t radiusLimit = field.isWeb()
                ? config[HostConfiguration::MaximumWebMinefieldRadius](owner)
                : config[HostConfiguration::MaximumMinefieldRadius](owner);
            const int32_t unitLimit = radiusLimit*radiusLimit;
            result.push_back(MinefieldEffect(center, field.getId(), radiusChange, field.getUnits(), unitLimit, owner, numTorps, field.isWeb(),
                                             isMinefieldEndangered(field, univ, mapConfig, host, config)));
        }
    }

} } }

bool
game::map::isMinefieldEndangered(const Minefield& field, const Universe& univ, const Configuration& mapConfig, const HostVersion& host, const game::config::HostConfiguration& config)
{
    // ex ship.pas:IsMinefieldEndangered

    // Determine minefield owner and position
    int mfOwner = 0;
    Point mfCenter;
    if (!field.getOwner().get(mfOwner) || !field.getPosition().get(mfCenter)) {
        return false;
    }

    // Determine effective minefield radius
    double radius = Minefield::getRadiusFromUnits(field.getUnitsForLaying(host, config));

    // Determine configuration option
    const HostConfiguration::StandardOptionDescriptor_t& range = field.isWeb()
        ? HostConfiguration::WebMineSweepRange
        : HostConfiguration::MineSweepRange;

    // Determine maximum range
    int maxRange = config[range](1);
    for (int i = 2; i <= MAX_PLAYERS; ++i) {
        maxRange = std::max(maxRange, config[range](i));
    }

    // Check planets
    const ObjectVector<Planet>& planets = univ.planets();
    for (Id_t i = 1; i <= planets.size(); ++i) {
        if (const Planet* pPlanet = planets.get(i)) {
            Point pt;
            int plOwner = 0;
            if (pPlanet->isVisible()
                && pPlanet->getPosition().get(pt)
                && (!pPlanet->getOwner().get(plOwner) || plOwner != mfOwner)
                && hasPossibleEnemyShip(univ, pt, mfOwner)
                && mapConfig.getSquaredDistance(mfCenter, pt) <= util::squareFloat(radius + maxRange))
            {
                return true;
            }
        }
    }

    // FIXME: PCC1 checks the ship's type.
    // When we add that, add it to hasPossibleEnemyShip as well.
    const ObjectVector<Ship>& ships = univ.ships();
    for (Id_t i = 1; i <= ships.size(); ++i) {
        if (const Ship* pShip = ships.get(i)) {
            Point shipPos;
            int shipOwner = 0;
            if (pShip->getPosition().get(shipPos)
                && pShip->getRealOwner().get(shipOwner)
                && shipOwner != mfOwner
                && mapConfig.getSquaredDistance(mfCenter, shipPos) <= util::squareFloat(radius + config[range](shipOwner)))
            {
                return true;
            }
        }
    }

    return false;
}


void
game::map::computeMineLayEffect(MinefieldEffects_t& result,
                                const MinefieldMission& mission,
                                const Ship& ship,
                                const Universe& univ,
                                const Configuration& mapConfig,
                                const Root& root)
{
    // ex WShipScannerChartWidget::drawPost (part)
    // Ship owner
    const int shipOwner = ship.getRealOwner().orElse(0);

    // New minefield for testing
    std::auto_ptr<Minefield> mf;
    int32_t existingUnits;
    if (const Minefield* pField = univ.minefields().get(mission.getRequiredMinefieldId())) {
        mf.reset(new Minefield(*pField));
        existingUnits = pField->getUnitsForLaying(root.hostVersion(), root.hostConfiguration());
    } else {
        Point center = ship.getPosition().orElse(Point());
        mf.reset(new Minefield(0, center, shipOwner, mission.isWeb(), 0));
        existingUnits = 0;
    }

    // New units
    int32_t newUnits = existingUnits + mission.getNumUnits();

    // Radius change exclusively from new units, so laying always produces a positive number
    int radiusChange = Minefield::getRadiusFromUnits(newUnits) - Minefield::getRadiusFromUnits(existingUnits);

    // For PHost (isMineLayingAfterMineDecay()): existingUnits includes this turn's decay.
    // For THost (!isMineLayingAfterMineDecay()): decay happens after laying, so we need to compute it here for display of the result.
    if (!root.hostVersion().isMineLayingAfterMineDecay()) {
        newUnits = mf->getUnitsAfterDecay(newUnits, root.hostVersion(), root.hostConfiguration());
    }
    mf->setUnits(newUnits);

    // Add it
    addMinefield(result, *mf, radiusChange, mission.getNumTorpedoes(), univ, mapConfig, root.hostVersion(), root.hostConfiguration());
}

void
game::map::computeMineScoopEffect(MinefieldEffects_t& result, const MinefieldMission& mission, const Ship& ship, const Universe& univ, const Configuration& mapConfig, const Root& root,
                                  const game::spec::ShipList& shipList)
{
    // ex WShipScannerChartWidget::drawPost (part)
    const Point shipPos = ship.getPosition().orElse(Point());
    int room = ship.getFreeCargo(shipList).orElse(0);
    const MinefieldType& mfs = univ.minefields();
    for (Id_t mfId = 1; mfId <= mfs.size() && room > 0; ++mfId) {
        // Check whether we can scoop this field
        const Minefield* mf = mfs.get(mfId);
        int mfOwner;
        Point mfPos;
        if (mf != 0
            && (mission.getRequiredMinefieldId() == 0 || mission.getRequiredMinefieldId() == mf->getId())
            && mf->isValid()
            && mf->getOwner().get(mfOwner)
            && mfOwner == mission.getMinefieldOwner()
            && mf->getPosition().get(mfPos)
            && mapConfig.getSquaredDistance(mfPos, shipPos) <= mf->getUnitsForLaying(root.hostVersion(), root.hostConfiguration()))
        {
            // Okay, scoop it. First, figure out conversion rate.
            int rate = ((mf->isWeb() ? root.hostConfiguration()[HostConfiguration::UnitsPerWebRate](mfOwner) : root.hostConfiguration()[HostConfiguration::UnitsPerTorpRate](mfOwner))
                        * util::squareInteger(ship.getTorpedoType().orElse(0)))
                / 100;
            if (rate == 0) {
                rate = 1;
            }

            // How much do we clear?
            int32_t existingUnits = mf->getUnitsForLaying(root.hostVersion(), root.hostConfiguration());
            int32_t scoopedUnits;
            int scoopedTorps = existingUnits / rate;
            if (scoopedTorps > room) {
                // We cannot clear this field
                scoopedTorps = room;
                scoopedUnits = room * rate;
            } else {
                // We can clear this field
                scoopedUnits = existingUnits;
            }
            room -= scoopedTorps;

            // Check limit
            if (mission.getNumTorpedoes() > 0 && scoopedTorps > mission.getNumTorpedoes()) {
                scoopedTorps = mission.getNumTorpedoes();
                scoopedUnits = scoopedTorps * rate;
            }

            // Render it
            if (scoopedTorps > 0) {
                // Changes
                int32_t newUnits = existingUnits - scoopedUnits;
                int radiusChange = Minefield::getRadiusFromUnits(newUnits) - Minefield::getRadiusFromUnits(existingUnits);

                if (!root.hostVersion().isMineLayingAfterMineDecay()) {
                    newUnits = mf->getUnitsAfterDecay(newUnits, root.hostVersion(), root.hostConfiguration());
                }

                Minefield newField(*mf);
                newField.setUnits(newUnits);
                addMinefield(result, newField, radiusChange, scoopedTorps, univ, mapConfig, root.hostVersion(), root.hostConfiguration());
            }
        }
    }
}
