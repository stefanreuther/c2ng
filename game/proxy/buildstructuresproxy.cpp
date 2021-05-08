/**
  *  \file game/proxy/buildstructuresproxy.cpp
  *  \brief Class game::proxy::BuildStructuresProxy
  */

#include "game/proxy/buildstructuresproxy.hpp"
#include "afl/string/format.hpp"
#include "game/actions/buildstructures.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/map/planetstorage.hpp"
#include "game/root.hpp"
#include "game/tables/temperaturename.hpp"
#include "game/turn.hpp"

using game::actions::BuildStructures;
using game::map::Planet;

class game::proxy::BuildStructuresProxy::Trampoline {
 public:
    Trampoline(Session& session, util::RequestSender<BuildStructuresProxy> reply)
        : m_session(session),
          m_reply(reply)
        { }

     void init(Id_t id, HeaderInfo& info, Status& status)
        {
            afl::string::Translator& tx = m_session.translator();
            try {
                // Preconditions
                Root& root = game::actions::mustHaveRoot(m_session);
                Game& game = game::actions::mustHaveGame(m_session);

                // Fetch planet
                Planet& planet = game::actions::mustExist(game.currentTurn().universe().planets().get(id), tx);
                m_container.reset(new game::map::PlanetStorage(planet, root.hostConfiguration(), tx));
                m_action.reset(new BuildStructures(planet, *m_container, root.hostConfiguration(), tx));
                m_action->setUndoInformation(game.currentTurn().universe());

                // Produce output
                const int temp = planet.getTemperature().orElse(0);
                info.ok = true;
                info.hasBase = planet.hasBase();
                info.planetName = planet.getName(tx);
                info.planetInfo = afl::string::Format(tx("(Id #%d, %s - %d" "\xC2\xB0" "\x46)"), planet.getId(), game::tables::TemperatureName(tx)(temp), temp);

                // Status
                describe(status);

                // Signal
                conn_change = m_action->sig_change.add(this, &Trampoline::onChange);
            }
            catch (std::exception& e) {
                info.ok = false;
            }
        }

    void describe(Status& out)
        {
            if (m_action.get() != 0) {
                for (size_t i = 0; i < NUM_PLANETARY_BUILDING_TYPES; ++i) {
                    PlanetaryBuilding ii = static_cast<PlanetaryBuilding>(i);
                    out.buildings[i].have = m_action->getNumBuildings(ii);
                    out.buildings[i].want = m_action->planet().getAutobuildGoal(ii);
                    out.buildings[i].max  = m_action->getMaxBuildingsRuleLimit(ii);
                    out.buildings[i].speed = m_action->planet().getAutobuildSpeed(ii);
                }
                out.available = m_action->costAction().getAvailableAmountAsCost();
                out.needed    = m_action->costAction().getCost();
                out.remaining = m_action->costAction().getRemainingAmountAsCost();
            }
        }

    void onChange()
        {
            class Task : public util::Request<BuildStructuresProxy> {
             public:
                Task(Trampoline& tpl)
                    : m_status()
                    { tpl.describe(m_status); }
                virtual void handle(BuildStructuresProxy& proxy)
                    { proxy.sig_statusChange.raise(m_status); }
             private:
                Status m_status;
            };
            m_reply.postNewRequest(new Task(*this));
        }

    BuildStructures* get()
        { return m_action.get(); }

    void notifyListeners()
        { m_session.notifyListeners(); }

 private:
    Session& m_session;
    util::RequestSender<BuildStructuresProxy> m_reply;
    std::auto_ptr<CargoContainer> m_container;
    std::auto_ptr<BuildStructures> m_action;
    afl::base::SignalConnection conn_change;
};

class game::proxy::BuildStructuresProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(const util::RequestSender<BuildStructuresProxy>& reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session, m_reply); }
 private:
    util::RequestSender<BuildStructuresProxy> m_reply;
};



game::proxy::BuildStructuresProxy::BuildStructuresProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& receiver)
    : m_receiver(receiver, *this),
      m_sender(gameSender.makeTemporary(new TrampolineFromSession(m_receiver.getSender())))
{ }

game::proxy::BuildStructuresProxy::~BuildStructuresProxy()
{ }

void
game::proxy::BuildStructuresProxy::init(WaitIndicator& link, Id_t id, HeaderInfo& info)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(Id_t id, HeaderInfo& info)
            : m_id(id), m_info(info), m_status()
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.init(m_id, m_info, m_status); }
        const Status& status() const
            { return m_status; }
     private:
        Id_t m_id;
        HeaderInfo& m_info;
        Status m_status;
    };
    Task t(id, info);
    link.call(m_sender, t);

    // FIXME: needed?
    // if (info.ok) {
    //     sig_statusChange.raise(t.status());
    // }
}

void
game::proxy::BuildStructuresProxy::update()
{
    class Task : public util::Request<Trampoline> {
     public:
        virtual void handle(Trampoline& tpl)
            { tpl.onChange(); }
    };
    m_sender.postNewRequest(new Task());
}

void
game::proxy::BuildStructuresProxy::addLimitCash(PlanetaryBuilding type, int count)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(PlanetaryBuilding type, int count)
            : m_type(type), m_count(count)
            { }
        virtual void handle(Trampoline& tpl)
            {
                if (BuildStructures* a = tpl.get()) {
                    a->addLimitCash(m_type, m_count);
                }
            }
     private:
        PlanetaryBuilding m_type;
        int m_count;
    };
    m_sender.postNewRequest(new Task(type, count));
}

void
game::proxy::BuildStructuresProxy::doStandardAutoBuild()
{
    class Task : public util::Request<Trampoline> {
     public:
        virtual void handle(Trampoline& tpl)
            {
                if (BuildStructures* a = tpl.get()) {
                    a->doStandardAutoBuild();
                }
            }
    };
    m_sender.postNewRequest(new Task());
}

void
game::proxy::BuildStructuresProxy::applyAutobuildSettings(const game::map::Planet::AutobuildSettings& settings)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(const Planet::AutobuildSettings& settings)
            : m_settings(settings)
            { }
        virtual void handle(Trampoline& tpl)
            {
                if (BuildStructures* a = tpl.get()) {
                    const_cast<Planet&>(a->planet()).applyAutobuildSettings(m_settings);
                    tpl.notifyListeners();
                }
            }
     private:
        Planet::AutobuildSettings m_settings;
    };
    m_sender.postNewRequest(new Task(settings));
}

void
game::proxy::BuildStructuresProxy::commit()
{
    class Task : public util::Request<Trampoline> {
     public:
        virtual void handle(Trampoline& tpl)
            {
                if (BuildStructures* a = tpl.get()) {
                    a->commit();
                }
            }
    };
    m_sender.postNewRequest(new Task());
}
