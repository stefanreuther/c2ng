/**
  *  \file game/proxy/fictivestarbaseadaptor.cpp
  *  \brief Class game::proxy::FictiveStarbaseAdaptor
  */

#include "game/proxy/fictivestarbaseadaptor.hpp"
#include "game/game.hpp"
#include "game/map/universe.hpp"
#include "game/turn.hpp"

using game::Element;
using game::map::Planet;

namespace {
    /* Populate planet. Updates all classic "planet" properties with default values,
       and also marks the planet as a current planet.
       Returns owner. */
    int populatePlanet(Planet& pl, game::Session& session)
    {
        // ex fillUnknownFields
        game::map::PlanetData pd;

        // Owner
        int owner;
        if (!pl.getOwner(owner) || owner == 0) {
            if (const game::Game* g = session.getGame().get()) {
                owner = g->getViewpointPlayer();
            } else {
                owner = 1;
            }
        }
        pd.owner = owner;

        // Friendly Code
        pd.friendlyCode = pl.getFriendlyCode().orElse("hhg");

        // Buildings
        pd.numMines        = pl.getNumBuildings(game::MineBuilding).orElse(10);
        pd.numFactories    = pl.getNumBuildings(game::FactoryBuilding).orElse(10);
        pd.numDefensePosts = pl.getNumBuildings(game::DefenseBuilding).orElse(10);

        // Ore
        pd.minedNeutronium   = pl.getCargo(Element::Neutronium).orElse(1000);
        pd.minedTritanium    = pl.getCargo(Element::Tritanium).orElse(1000);
        pd.minedDuranium     = pl.getCargo(Element::Duranium).orElse(1000);
        pd.minedMolybdenum   = pl.getCargo(Element::Molybdenum).orElse(1000);
        pd.groundNeutronium  = pl.getOreGround(Element::Neutronium).orElse(10000);
        pd.groundTritanium   = pl.getOreGround(Element::Tritanium).orElse(10000);
        pd.groundDuranium    = pl.getOreGround(Element::Duranium).orElse(10000);
        pd.groundMolybdenum  = pl.getOreGround(Element::Molybdenum).orElse(10000);
        pd.densityNeutronium = pl.getOreDensity(Element::Neutronium).orElse(50);
        pd.densityTritanium  = pl.getOreDensity(Element::Tritanium).orElse(50);
        pd.densityDuranium   = pl.getOreDensity(Element::Duranium).orElse(50);
        pd.densityMolybdenum = pl.getOreDensity(Element::Molybdenum).orElse(50);

        // Colony
        int32_t colonistClans = pl.getCargo(Element::Colonists).orElse(100);
        if (colonistClans == 0) {
            colonistClans = 100;
        }
        pd.colonistClans     = colonistClans;
        pd.supplies          = pl.getCargo(Element::Supplies).orElse(10000);
        pd.money             = pl.getCargo(Element::Money).orElse(10000);
        pd.colonistTax       = pl.getColonistTax().orElse(0);
        pd.colonistHappiness = pl.getColonistHappiness().orElse(100);

        // Natives
        int32_t nativeClans = pl.getNatives().orElse(100);
        int nativeRace = pl.getNativeRace().orElse(0);
        if (nativeClans != 0 && nativeRace != 0) {
            pd.nativeTax        = pl.getNativeTax().orElse(0);
            pd.nativeHappiness  = pl.getNativeHappiness().orElse(100);
            pd.nativeGovernment = pl.getNativeGovernment().orElse(5);
            pd.nativeClans      = nativeClans;
            pd.nativeRace       = nativeRace;
        } else {
            pd.nativeTax        = 0;
            pd.nativeHappiness  = 100;
            pd.nativeGovernment = 0;
            pd.nativeClans      = 0;
            pd.nativeRace       = 0;
        }

        // Temperature
        pd.temperature = pl.getTemperature().orElse(50);

        // Build base? No.
        pd.baseFlag = 0;

        // Add it
        pl.addCurrentPlanetData(pd, game::PlayerSet_t(owner));

        return owner;
    }

    /* Populate starbase. Updates most classic "starbase" properties with default values,
       and also marks the starbase as current. */
    void populateBase(Planet& pl, int owner)
    {
        // ex createDefaultBase
        game::map::BaseData bd;

        // Equipment
        bd.numBaseDefensePosts = pl.getNumBuildings(game::BaseDefenseBuilding).orElse(0);
        bd.damage              = pl.getBaseDamage().orElse(0);
        bd.numFighters         = pl.getCargo(Element::Fighters).orElse(0);
        bd.shipyardId          = pl.getBaseShipyardId().orElse(0);
        bd.shipyardAction      = pl.getBaseShipyardAction().orElse(0);
        bd.mission             = pl.getBaseMission().orElse(0);

        // Tech
        for (size_t i = 0; i < game::NUM_TECH_AREAS; ++i) {
            int level = pl.getBaseTechLevel(game::TechLevel(i)).orElse(1);
            if (level <= 0) {
                level = 1;
            }
            bd.techLevels[i] = level;
        }
        switch (pl.getNativeRace().orElse(0)) {
         case game::HumanoidNatives:   bd.techLevels[game::HullTech]    = 10; break;
         case game::AmphibianNatives:  bd.techLevels[game::BeamTech]    = 10; break;
         case game::GhipsoldalNatives: bd.techLevels[game::EngineTech]  = 10; break;
         case game::SiliconoidNatives: bd.techLevels[game::TorpedoTech] = 10; break;
        }

        // Leave shipBuildOrder at default, which is: no build order

        // Leave storage at default, which is: nothing stored.
        // This is normally not desirable because it means it cannot be modified,
        // but for now we do not want to modify.

        pl.addCurrentBaseData(bd, game::PlayerSet_t(owner));
    }

    /* Finish planet by filling in metainformation */
    void finishPlanet(Planet& pl, game::Session& session)
    {
        // Force position
        game::map::Point pt;
        if (!pl.getPosition(pt)) {
            pl.setPosition(game::map::Point(1000, 1000));
        }
        pl.setKnownToNotExist(false);

        // Check against flat map, i.e. map will not refuse knowing this planet.
        // This call is required to correctly set the base flags
        game::map::Configuration config;
        pl.internalCheck(config, session.translator(), session.log());

        // Make it editable
        pl.setPlayability(Planet::Editable);
    }
}

game::proxy::FictiveStarbaseAdaptor::FictiveStarbaseAdaptor(Session& session, Id_t planetId)
    : m_session(session),
      m_planet()
{
    // Fetch template planet, if any
    if (Game* g = session.getGame().get()) {
        if (const Planet* pl = g->currentTurn().universe().planets().get(planetId)) {
            m_planet.reset(new Planet(*pl));
        }
    }

    // Create default planet if needed
    if (m_planet.get() == 0) {
        m_planet.reset(new Planet(planetId == 0 ? 42 : planetId));
        m_planet->setName("Magrathea");
    }

    // Populate it
    int owner = populatePlanet(*m_planet, session);
    if (!m_planet->hasFullBaseData()) {
        populateBase(*m_planet, owner);
    }
    finishPlanet(*m_planet, session);
}

game::proxy::FictiveStarbaseAdaptor::~FictiveStarbaseAdaptor()
{ }

game::map::Planet&
game::proxy::FictiveStarbaseAdaptor::planet()
{
    return *m_planet;
}

game::Session&
game::proxy::FictiveStarbaseAdaptor::session()
{
    return m_session;
}

bool
game::proxy::FictiveStarbaseAdaptor::findShipCloningHere(Id_t& /*id*/, String_t& /*name*/)
{
    return false;
}

void
game::proxy::FictiveStarbaseAdaptor::cancelAllCloneOrders()
{ }

void
game::proxy::FictiveStarbaseAdaptor::notifyListeners()
{ }


/*
 *  FictiveStarbaseAdaptorFromSession
 */

game::proxy::FictiveStarbaseAdaptor*
game::proxy::FictiveStarbaseAdaptorFromSession::call(Session& session)
{
    return new FictiveStarbaseAdaptor(session, m_planetId);
}
