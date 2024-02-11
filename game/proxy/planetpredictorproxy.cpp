/**
  *  \file game/proxy/planetpredictorproxy.cpp
  *  \brief Class game::proxy::PlanetPredictorProxy
  */

#include "game/proxy/planetpredictorproxy.hpp"
#include "game/game.hpp"
#include "game/map/planetinfo.hpp"
#include "game/map/planetpredictor.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"

using game::actions::TaxationAction;
using game::config::HostConfiguration;

namespace {
    /*
     *  Utility for packPrediction()
     */

    struct Set {
        bool useColonists;
        bool useNatives;
        bool useExperience;
    };

    void packStatus(game::proxy::PlanetPredictorProxy::Status& st,
                    const Set& set,
                    const game::map::Planet& planet,
                    const game::UnitScoreDefinitionList& planetScores,
                    const HostConfiguration& config)
    {
        if (set.useColonists) {
            st.colonistClans.push_back(planet.getCargo(game::Element::Colonists).orElse(0));
        }
        if (set.useNatives) {
            st.nativeClans.push_back(planet.getNatives().orElse(0));
        }
        if (set.useExperience) {
            st.experiencePoints.push_back(planet.getScore(game::ScoreId_ExpPoints, planetScores).orElse(0));
            st.experienceLevel.push_back(config.getExperienceLevelFromPoints(st.experiencePoints.back()));
        }
    }
}


class game::proxy::PlanetPredictorProxy::Trampoline {
 public:
    Trampoline(Session& session, Id_t planetId, util::RequestSender<PlanetPredictorProxy> reply)
        : m_numTurns(0),
          m_numMines(),
          m_numFactories(),
          m_nativeTax(),
          m_colonistTax(),
          m_effectors(),
          m_planetId(planetId),
          m_reply(reply),
          m_root(),
          m_game(),
          m_shipList(),
          m_turn(),
          m_translator(session.translator()),
          m_planet(),
          conn_planetChange()
        { init(session); }

    void init(Session& session);

    void onPlanetChange();

    template<typename Addr>
    void setProperty(Addr which, int32_t value);

    void sendUpdate();
    void packPrediction(Status& st);

    // Public properties (for setProperty):
    int m_numTurns;
    afl::base::Optional<int32_t> m_numMines;
    afl::base::Optional<int32_t> m_numFactories;
    afl::base::Optional<int32_t> m_nativeTax;
    afl::base::Optional<int32_t> m_colonistTax;
    game::map::PlanetEffectors m_effectors;

 private:
    const Id_t m_planetId;
    util::RequestSender<PlanetPredictorProxy> m_reply;
    afl::base::Ptr<Root> m_root;
    afl::base::Ptr<Game> m_game;
    afl::base::Ptr<game::spec::ShipList> m_shipList;
    afl::base::Ptr<Turn> m_turn;
    afl::string::Translator& m_translator;
    game::map::Planet* m_planet;
    afl::base::SignalConnection conn_planetChange;
};

void
game::proxy::PlanetPredictorProxy::Trampoline::init(Session& session)
{
    // Keep components alive
    m_game = session.getGame();
    m_root = session.getRoot();
    m_shipList = session.getShipList();
    if (m_game.get() != 0 && m_root.get() != 0 && m_shipList.get() != 0) {
        m_turn = &m_game->viewpointTurn();

        // Attach to planet
        m_planet = m_turn->universe().planets().get(m_planetId);
        if (m_planet != 0) {
            conn_planetChange = m_planet->sig_change.add(this, &Trampoline::onPlanetChange);
            m_effectors = preparePlanetEffectors(m_turn->universe(), m_planetId, m_game->shipScores(), *m_shipList, m_root->hostConfiguration());
        }
    }
}

void
game::proxy::PlanetPredictorProxy::Trampoline::onPlanetChange()
{
    sendUpdate();
}


template<typename Addr>
inline void
game::proxy::PlanetPredictorProxy::Trampoline::setProperty(Addr which, int32_t value)
{
    (this->*which) = value;
    sendUpdate();
}

void
game::proxy::PlanetPredictorProxy::Trampoline::sendUpdate()
{
    class Task : public util::Request<PlanetPredictorProxy> {
     public:
        Task(Trampoline& me)
            : m_status()
            { me.packPrediction(m_status); }
        virtual void handle(PlanetPredictorProxy& proxy)
            { proxy.sig_update.raise(m_status); }
     private:
        Status m_status;
    };
    m_reply.postNewRequest(new Task(*this));
}

void
game::proxy::PlanetPredictorProxy::Trampoline::packPrediction(Status& st)
{
    if (m_planet != 0 && m_root.get() != 0 && m_game.get() != 0) {
        // Determine set of variables
        Set set;
        set.useColonists = true;
        set.useNatives = m_planet->getNativeRace().orElse(0) != 0;
        set.useExperience = m_root->hostConfiguration()[HostConfiguration::NumExperienceLevels]() > 0
            && m_planet->getScore(ScoreId_ExpPoints, m_game->planetScores()).isValid();

        // Remember first turn
        packStatus(st, set, *m_planet, m_game->planetScores(), m_root->hostConfiguration());

        // Prepare planet for prediction
        game::map::PlanetPredictor pred(*m_planet);
        if (int32_t* p = m_numMines.get()) {
            pred.planet().setNumBuildings(MineBuilding, *p);
        }
        if (int32_t* p = m_numFactories.get()) {
            pred.planet().setNumBuildings(FactoryBuilding, *p);
        }
        if (int32_t* p = m_nativeTax.get()) {
            pred.planet().setNativeTax(*p);
        }
        if (int32_t* p = m_colonistTax.get()) {
            pred.planet().setColonistTax(*p);
        }

        // Compute further turns
        for (int i = 0; i < m_numTurns; ++i) {
            pred.computeTurn(m_effectors, m_game->planetScores(), m_root->hostConfiguration(), m_root->hostVersion());
            packStatus(st, set, pred.planet(), m_game->planetScores(), m_root->hostConfiguration());
        }

        // Effectors
        int owner = m_planet->getOwner().orElse(0);
        st.effectorLabel = m_effectors.describe(m_translator, owner, m_root->hostConfiguration());
    }
}


class game::proxy::PlanetPredictorProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(Id_t planetId, const util::RequestSender<PlanetPredictorProxy>& reply)
        : m_planetId(planetId), m_reply(reply)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session, m_planetId, m_reply); }
 private:
    Id_t m_planetId;
    util::RequestSender<PlanetPredictorProxy> m_reply;
};



/*
 *  PlanetPredictorProxy
 */

game::proxy::PlanetPredictorProxy::PlanetPredictorProxy(util::RequestDispatcher& reply,
                                                        util::RequestSender<Session> gameSender,
                                                        Id_t planetId)
    : m_reply(reply, *this),
      m_trampoline(gameSender.makeTemporary(new TrampolineFromSession(planetId, m_reply.getSender())))
{ }

game::proxy::PlanetPredictorProxy::~PlanetPredictorProxy()
{
}

game::map::PlanetEffectors
game::proxy::PlanetPredictorProxy::getEffectors(WaitIndicator& ind)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(game::map::PlanetEffectors& eff)
            : m_effectors(eff)
            { }
        virtual void handle(Trampoline& tpl)
            { m_effectors = tpl.m_effectors; }
     private:
        game::map::PlanetEffectors& m_effectors;
    };

    game::map::PlanetEffectors result;
    Task t(result);
    ind.call(m_trampoline, t);
    return result;
}

void
game::proxy::PlanetPredictorProxy::getStatus(WaitIndicator& ind, Status& out)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(Status& out)
            : m_out(out)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.packPrediction(m_out); }
     private:
        Status& m_out;
    };

    Task t(out);
    ind.call(m_trampoline, t);
}

void
game::proxy::PlanetPredictorProxy::setEffectors(const game::map::PlanetEffectors& eff)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(const game::map::PlanetEffectors& eff)
            : m_effectors(eff)
            { }
        virtual void handle(Trampoline& tpl)
            {
                tpl.m_effectors = m_effectors;
                tpl.sendUpdate();
            }
     private:
        game::map::PlanetEffectors m_effectors;
    };
    m_trampoline.postNewRequest(new Task(eff));
}

void
game::proxy::PlanetPredictorProxy::setNumTurns(int n)
{
    setProperty(&Trampoline::m_numTurns, n);
}

void
game::proxy::PlanetPredictorProxy::setNumBuildings(PlanetaryBuilding which, int n)
{
    switch (which) {
     case MineBuilding:
        setProperty(&Trampoline::m_numMines, n);
        break;
     case FactoryBuilding:
        setProperty(&Trampoline::m_numFactories, n);
        break;
     case DefenseBuilding:
     case BaseDefenseBuilding:
        break;
    }
}

void
game::proxy::PlanetPredictorProxy::setTax(Area_t area, int rate)
{
    switch (area) {
     case TaxationAction::Colonists:
        setProperty(&Trampoline::m_colonistTax, rate);
        break;
     case TaxationAction::Natives:
        setProperty(&Trampoline::m_nativeTax, rate);
        break;
    }
}

template<typename Addr>
inline void
game::proxy::PlanetPredictorProxy::setProperty(Addr a, int32_t value)
{
    m_trampoline.postRequest(&Trampoline::setProperty, a, value);
}
