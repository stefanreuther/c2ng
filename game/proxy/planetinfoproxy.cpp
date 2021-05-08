/**
  *  \file game/proxy/planetinfoproxy.cpp
  *  \brief Class game::proxy::PlanetInfoProxy
  */

#include "game/proxy/planetinfoproxy.hpp"
#include "game/game.hpp"
#include "game/map/universe.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"

/*
 *  Response
 */

class game::proxy::PlanetInfoProxy::Response : public util::Request<PlanetInfoProxy> {
 public:
    Response(Session& session, Id_t id, IntegerProperty_t (&buildingOverride)[NUM_PLANETARY_BUILDING_TYPES], const game::map::UnloadInfo& unload);
    ~Response();

    virtual void handle(PlanetInfoProxy& proxy);

 private:
    game::map::PlanetMineralInfo m_mineralInfo[NUM_MINERALS];
    afl::io::xml::Nodes_t m_climateInfo;
    afl::io::xml::Nodes_t m_colonyInfo;
    afl::io::xml::Nodes_t m_nativeInfo;
    afl::io::xml::Nodes_t m_buildingEffectsInfo;
    game::map::DefenseEffectInfos_t m_defenseEffectsInfo;
    game::map::UnloadInfo m_unloadInfo;
    game::map::GroundDefenseInfo m_groundDefenseInfo;
};


game::proxy::PlanetInfoProxy::Response::Response(Session& session, Id_t id, IntegerProperty_t (&buildingOverride)[NUM_PLANETARY_BUILDING_TYPES], const game::map::UnloadInfo& unload)
{
    afl::string::Translator& tx = session.translator();
    Game* g = session.getGame().get();
    Root* r = session.getRoot().get();

    if (g != 0 && r != 0) {
        const int turnNr = g->currentTurn().getTurnNumber();
        const game::config::HostConfiguration& config = r->hostConfiguration();
        const HostVersion& host = r->hostVersion();

        if (const game::map::Planet* pl = g->currentTurn().universe().planets().get(id)) {
            // Mineral Info
            const IntegerProperty_t mineOverride = buildingOverride[MineBuilding];
            m_mineralInfo[Neutronium] = packPlanetMineralInfo(*pl, Element::Neutronium, turnNr, config, host, mineOverride, tx);
            m_mineralInfo[Tritanium]  = packPlanetMineralInfo(*pl, Element::Tritanium,  turnNr, config, host, mineOverride, tx);
            m_mineralInfo[Duranium]   = packPlanetMineralInfo(*pl, Element::Duranium,   turnNr, config, host, mineOverride, tx);
            m_mineralInfo[Molybdenum] = packPlanetMineralInfo(*pl, Element::Molybdenum, turnNr, config, host, mineOverride, tx);

            // Textual infos
            describePlanetClimate(m_climateInfo, *pl, turnNr, *r, g->getViewpointPlayer(), tx);
            describePlanetColony (m_colonyInfo,  *pl, turnNr, *r, g->getViewpointPlayer(), unload, tx);
            describePlanetNatives(m_nativeInfo,  *pl, turnNr, *r, g->getViewpointPlayer(), unload, tx);

            // Building effects; work on a copy of the planet
            {
                game::map::Planet pp(*pl);
                for (size_t i = 0; i < NUM_PLANETARY_BUILDING_TYPES; ++i) {
                    if (buildingOverride[i].isValid()) {
                        pp.setNumBuildings(PlanetaryBuilding(i), buildingOverride[i]);
                    }
                }

                describePlanetBuildingEffects(m_buildingEffectsInfo, pp, *r, tx);

                if (game::spec::ShipList* sl = session.getShipList().get()) {
                    describePlanetDefenseEffects(m_defenseEffectsInfo, pp, *r, *sl, g->planetScores(), tx);
                }
                m_groundDefenseInfo = packGroundDefenseInfo(pp, *r);
            }

            // Unload info for reference by UI
            m_unloadInfo = unload;
        }
    }
}

game::proxy::PlanetInfoProxy::Response::~Response()
{ }

void
game::proxy::PlanetInfoProxy::Response::handle(PlanetInfoProxy& proxy)
{
    for (size_t i = 0; i < NUM_MINERALS; ++i) {
        proxy.m_mineralInfo[i] = m_mineralInfo[i];
    }
    m_climateInfo.swap(proxy.m_climateInfo);
    m_colonyInfo.swap(proxy.m_colonyInfo);
    m_nativeInfo.swap(proxy.m_nativeInfo);
    m_buildingEffectsInfo.swap(proxy.m_buildingEffectsInfo);
    proxy.m_unloadInfo = m_unloadInfo;
    proxy.m_groundDefenseInfo = m_groundDefenseInfo;
    proxy.m_defenseEffectsInfo = m_defenseEffectsInfo;
    proxy.sig_change.raise();
}




/*
 *  Trampoline - Game-side state
 *
 *  FIXME: as of 20191220, we do NOT forward game-side changes.
 *  We would have to subscribe to...
 *  - session state
 *  - configuration changes
 *  - universe changes
 */

class game::proxy::PlanetInfoProxy::Trampoline {
 public:
    Trampoline(Session& session, util::RequestSender<PlanetInfoProxy> reply)
        : m_session(session),
          m_reply(reply),
          m_planetId(0)
        { }

    ~Trampoline();

    void setPlanet(Id_t id);
    void setAttackingClansOverride(int32_t n);
    void setBuildingOverride(PlanetaryBuilding type, IntegerProperty_t amount);
    void update();
    void updateUnloadInfo();

 private:
    Session& m_session;
    util::RequestSender<PlanetInfoProxy> m_reply;
    Id_t m_planetId;
    IntegerProperty_t m_buildingOverride[NUM_PLANETARY_BUILDING_TYPES];
    game::map::UnloadInfo m_unloadInfo;
};


/*
 *  Trampoline
 */

game::proxy::PlanetInfoProxy::Trampoline::~Trampoline()
{ }

inline void
game::proxy::PlanetInfoProxy::Trampoline::setPlanet(Id_t id)
{
    m_planetId = id;
    updateUnloadInfo();
    update();
}

inline void
game::proxy::PlanetInfoProxy::Trampoline::setAttackingClansOverride(int32_t n)
{
    m_unloadInfo.hostileUnload = n;
    m_unloadInfo.hostileUnloadIsAssumed = true;
    update();
}

inline void
game::proxy::PlanetInfoProxy::Trampoline::setBuildingOverride(PlanetaryBuilding type, IntegerProperty_t amount)
{
    m_buildingOverride[type] = amount;
    update();
}

void
game::proxy::PlanetInfoProxy::Trampoline::update()
{
    if (m_planetId != 0) {
        m_reply.postNewRequest(new Response(m_session, m_planetId, m_buildingOverride, m_unloadInfo));
    }
}

inline void
game::proxy::PlanetInfoProxy::Trampoline::updateUnloadInfo()
{
    Game* g = m_session.getGame().get();
    Root* r = m_session.getRoot().get();
    game::spec::ShipList* sl = m_session.getShipList().get();
    if (g != 0 && r != 0 && sl != 0) {
        m_unloadInfo = prepareUnloadInfo(g->currentTurn().universe(), m_planetId, g->getViewpointPlayer(), g->shipScores(), *sl, r->hostConfiguration());
    }
}



class game::proxy::PlanetInfoProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(const util::RequestSender<PlanetInfoProxy>& reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session, m_reply); }
 private:
    util::RequestSender<PlanetInfoProxy> m_reply;
};



/*
 *  PlanetInfoProxy
 */

// Constructor.
game::proxy::PlanetInfoProxy::PlanetInfoProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& receiver)
    : m_receiver(receiver, *this),
      m_sender(gameSender.makeTemporary(new TrampolineFromSession(m_receiver.getSender())))
{ }

// Destructor.
game::proxy::PlanetInfoProxy::~PlanetInfoProxy()
{ }

// Set planet Id.
void
game::proxy::PlanetInfoProxy::setPlanet(Id_t id)
{
    m_sender.postRequest(&Trampoline::setPlanet, id);
}

// Set number of buildings.
void
game::proxy::PlanetInfoProxy::setBuildingOverride(PlanetaryBuilding type, IntegerProperty_t amount)
{
    m_sender.postRequest(&Trampoline::setBuildingOverride, type, amount);
}

// Set number of attacking clans.
void
game::proxy::PlanetInfoProxy::setAttackingClansOverride(int32_t n)
{
    m_sender.postRequest(&Trampoline::setAttackingClansOverride, n);
}

// Get mineral info.
const game::map::PlanetMineralInfo&
game::proxy::PlanetInfoProxy::getMineralInfo(Mineral m) const
{
    return m_mineralInfo[m];
}

// Get climate info.
const afl::io::xml::Nodes_t&
game::proxy::PlanetInfoProxy::getClimateInfo() const
{
    return m_climateInfo;
}

// Get colony info.
const afl::io::xml::Nodes_t&
game::proxy::PlanetInfoProxy::getColonyInfo() const
{
    return m_colonyInfo;
}

// Get natives info.
const afl::io::xml::Nodes_t&
game::proxy::PlanetInfoProxy::getNativeInfo() const
{
    return m_nativeInfo;
}

// Get building effects information.
const afl::io::xml::Nodes_t&
game::proxy::PlanetInfoProxy::getBuildingEffectsInfo() const
{
    return m_buildingEffectsInfo;
}

// Get defense effects information.
const game::map::DefenseEffectInfos_t&
game::proxy::PlanetInfoProxy::getDefenseEffectsInfo() const
{
    return m_defenseEffectsInfo;
}

// Get unload information.
const game::map::UnloadInfo&
game::proxy::PlanetInfoProxy::getUnloadInfo() const
{
    return m_unloadInfo;
}

// Get ground defense information.
const game::map::GroundDefenseInfo&
game::proxy::PlanetInfoProxy::getGroundDefenseInfo() const
{
    return m_groundDefenseInfo;
}
