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
    Response(Session* pSession, Id_t id, IntegerProperty_t (&buildingOverride)[NUM_PLANETARY_BUILDING_TYPES], const game::map::UnloadInfo& unload)
        {
            if (pSession) {
                afl::string::Translator& tx = pSession->translator();
                Game* g = pSession->getGame().get();
                Root* r = pSession->getRoot().get();

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

                            if (game::spec::ShipList* sl = pSession->getShipList().get()) {
                                describePlanetDefenseEffects(m_defenseEffectsInfo, pp, *r, *sl, g->planetScores(), tx);
                            }

                            m_groundDefenseInfo = packGroundDefenseInfo(pp, *r);
                        }

                        // Unload info for reference by UI
                        m_unloadInfo = unload;
                    }
                }
            }
        }

    virtual void handle(PlanetInfoProxy& proxy)
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

/*
 *  Trampoline - Game-side state
 *
 *  FIXME: as of 20191220, we do NOT forward game-side changes.
 *  We would have to subscribe to...
 *  - session state
 *  - configuration changes
 *  - universe changes
 */

class game::proxy::PlanetInfoProxy::Trampoline : public util::SlaveObject<Session> {
 public:
    Trampoline(util::RequestSender<PlanetInfoProxy> reply)
        : m_reply(reply),
          m_planetId(0)
        { }

    ~Trampoline()
        { }

    void init(Session& /*session*/)
        { }

    void done(Session& /*session*/)
        { }

    void setPlanet(Session& session, Id_t id)
        {
            m_planetId = id;
            updateUnloadInfo(session);
            update(session);
        }

    void setAttackingClansOverride(Session& session, int32_t n)
        {
            m_unloadInfo.hostileUnload = n;
            m_unloadInfo.hostileUnloadIsAssumed = true;
            update(session);
        }

    void setBuildingOverride(Session& session, PlanetaryBuilding type, IntegerProperty_t amount)
        {
            m_buildingOverride[type] = amount;
            update(session);
        }

    void update(Session& session)
        {
            if (m_planetId != 0) {
                m_reply.postNewRequest(new Response(&session, m_planetId, m_buildingOverride, m_unloadInfo));
            }
        }

    void updateUnloadInfo(Session& session)
        {
            Game* g = session.getGame().get();
            Root* r = session.getRoot().get();
            game::spec::ShipList* sl = session.getShipList().get();
            if (g != 0 && r != 0 && sl != 0) {
                m_unloadInfo = prepareUnloadInfo(g->currentTurn().universe(),
                                                 m_planetId,
                                                 g->getViewpointPlayer(),
                                                 g->shipScores(),
                                                 *sl,
                                                 r->hostConfiguration());
            }
        }

 private:
    util::RequestSender<PlanetInfoProxy> m_reply;
    Id_t m_planetId;
    IntegerProperty_t m_buildingOverride[NUM_PLANETARY_BUILDING_TYPES];
    game::map::UnloadInfo m_unloadInfo;
};


/*
 *  PlanetInfoProxy
 */

// Constructor.
game::proxy::PlanetInfoProxy::PlanetInfoProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& receiver)
    : m_receiver(receiver, *this),
      m_sender(gameSender, new Trampoline(m_receiver.getSender()))
{ }

// Destructor.
game::proxy::PlanetInfoProxy::~PlanetInfoProxy()
{ }

// Set planet Id.
void
game::proxy::PlanetInfoProxy::setPlanet(Id_t id)
{
    class Task : public util::SlaveRequest<Session, Trampoline> {
     public:
        Task(Id_t id)
            : m_id(id)
            { }
        virtual void handle(Session& session, Trampoline& tpl)
            { tpl.setPlanet(session, m_id); }
     private:
        Id_t m_id;
    };
    m_sender.postNewRequest(new Task(id));
}

// Set number of buildings.
void
game::proxy::PlanetInfoProxy::setBuildingOverride(PlanetaryBuilding type, IntegerProperty_t amount)
{
    class Task : public util::SlaveRequest<Session, Trampoline> {
     public:
        Task(PlanetaryBuilding type, IntegerProperty_t amount)
            : m_type(type),
              m_amount(amount)
            { }
        virtual void handle(Session& session, Trampoline& tpl)
            { tpl.setBuildingOverride(session, m_type, m_amount); }
     private:
        PlanetaryBuilding m_type;
        IntegerProperty_t m_amount;
    };
    m_sender.postNewRequest(new Task(type, amount));
}

// Set number of attacking clans.
void
game::proxy::PlanetInfoProxy::setAttackingClansOverride(int32_t n)
{
    class Task : public util::SlaveRequest<Session, Trampoline> {
     public:
        Task(int32_t n)
            : m_n(n)
            { }
        virtual void handle(Session& session, Trampoline& tpl)
            { tpl.setAttackingClansOverride(session, m_n); }
     private:
        int32_t m_n;
    };
    m_sender.postNewRequest(new Task(n));
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
