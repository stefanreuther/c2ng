/**
  *  \file client/proxy/buildstructuresproxy.cpp
  */

#include "client/proxy/buildstructuresproxy.hpp"
#include "afl/string/format.hpp"
#include "game/actions/buildstructures.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/map/planetstorage.hpp"
#include "game/root.hpp"
#include "game/tables/temperaturename.hpp"
#include "game/turn.hpp"

using game::actions::BuildStructures;

class client::proxy::BuildStructuresProxy::Trampoline : public util::SlaveObject<game::Session> {
 public:
    Trampoline(util::RequestSender<BuildStructuresProxy> reply)
        : m_reply(reply)
        { }

    virtual void init(game::Session&)
        { }

    virtual void done(game::Session&)
        {
            m_action.reset();
            m_container.reset();
            conn_change.disconnect();
        }

     void init(game::Session& session, game::Id_t id, HeaderInfo& info, Status& status)
        {
            afl::string::Translator& tx = session.translator();
            try {
                // Preconditions
                game::Root& root = game::actions::mustHaveRoot(session);
                game::Game& game = game::actions::mustHaveGame(session);

                // Fetch planet
                game::map::Planet& planet = game::actions::mustExist(game.currentTurn().universe().planets().get(id));
                m_container.reset(new game::map::PlanetStorage(planet, session.interface(), root.hostConfiguration()));
                m_action.reset(new BuildStructures(planet, *m_container, root.hostConfiguration()));
                m_action->setUndoInformation(game.currentTurn().universe());

                // Produce output
                const int temp = planet.getTemperature().orElse(0);
                info.ok = true;
                info.hasBase = planet.hasBase();
                info.planetName = planet.getName(game::PlainName, tx, session.interface());
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
                for (size_t i = 0; i < game::NUM_PLANETARY_BUILDING_TYPES; ++i) {
                    game::PlanetaryBuilding ii = static_cast<game::PlanetaryBuilding>(i);
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

 private:
    util::RequestSender<BuildStructuresProxy> m_reply;
    std::auto_ptr<game::CargoContainer> m_container;
    std::auto_ptr<BuildStructures> m_action;
    afl::base::SignalConnection conn_change;
};

client::proxy::BuildStructuresProxy::BuildStructuresProxy(util::RequestSender<game::Session> gameSender, util::RequestDispatcher& receiver)
    : m_receiver(receiver, *this),
      m_sender(gameSender, new Trampoline(m_receiver.getSender()))
{ }

client::proxy::BuildStructuresProxy::~BuildStructuresProxy()
{ }

void
client::proxy::BuildStructuresProxy::init(Downlink& link, game::Id_t id, HeaderInfo& info)
{
    class Task : public util::SlaveRequest<game::Session, Trampoline> {
     public:
        Task(game::Id_t id, HeaderInfo& info)
            : m_id(id), m_info(info), m_status()
            { }
        virtual void handle(game::Session& session, Trampoline& tpl)
            { tpl.init(session, m_id, m_info, m_status); }
        const Status& status() const
            { return m_status; }
     private:
        game::Id_t m_id;
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
client::proxy::BuildStructuresProxy::update()
{
    class Task : public util::SlaveRequest<game::Session, Trampoline> {
     public:
        virtual void handle(game::Session&, Trampoline& tpl)
            { tpl.onChange(); }
    };
    m_sender.postNewRequest(new Task());
}

void
client::proxy::BuildStructuresProxy::addLimitCash(game::PlanetaryBuilding type, int count)
{
    class Task : public util::SlaveRequest<game::Session, Trampoline> {
     public:
        Task(game::PlanetaryBuilding type, int count)
            : m_type(type), m_count(count)
            { }
        virtual void handle(game::Session&, Trampoline& tpl)
            {
                if (BuildStructures* a = tpl.get()) {
                    a->addLimitCash(m_type, m_count);
                }
            }
     private:
        game::PlanetaryBuilding m_type;
        int m_count;
    };
    m_sender.postNewRequest(new Task(type, count));
}

void
client::proxy::BuildStructuresProxy::doStandardAutoBuild()
{
    class Task : public util::SlaveRequest<game::Session, Trampoline> {
     public:
        virtual void handle(game::Session&, Trampoline& tpl)
            {
                if (BuildStructures* a = tpl.get()) {
                    a->doStandardAutoBuild();
                }
            }
    };
    m_sender.postNewRequest(new Task());
}

void
client::proxy::BuildStructuresProxy::applyAutobuildSettings(const game::map::Planet::AutobuildSettings& settings)
{
    class Task : public util::SlaveRequest<game::Session, Trampoline> {
     public:
        Task(const game::map::Planet::AutobuildSettings& settings)
            : m_settings(settings)
            { }
        virtual void handle(game::Session& s, Trampoline& tpl)
            {
                if (BuildStructures* a = tpl.get()) {
                    const_cast<game::map::Planet&>(a->planet()).applyAutobuildSettings(m_settings);
                    s.notifyListeners();
                }
            }
     private:
        game::map::Planet::AutobuildSettings m_settings;
    };
    m_sender.postNewRequest(new Task(settings));
}

void
client::proxy::BuildStructuresProxy::commit()
{
    class Task : public util::SlaveRequest<game::Session, Trampoline> {
     public:
        virtual void handle(game::Session&, Trampoline& tpl)
            {
                if (BuildStructures* a = tpl.get()) {
                    a->commit();
                }
            }
    };
    m_sender.postNewRequest(new Task());
}
