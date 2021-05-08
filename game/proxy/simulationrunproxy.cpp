/**
  *  \file game/proxy/simulationrunproxy.cpp
  *  \brief Class game::proxy::SimulationRunProxy
  */

#include "game/proxy/simulationrunproxy.hpp"
#include "afl/except/assertionfailedexception.hpp"
#include "afl/string/format.hpp"
#include "game/game.hpp"
#include "game/proxy/simulationsetupproxy.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "game/sim/run.hpp"
#include "game/sim/runner.hpp"
#include "game/sim/session.hpp"
#include "game/sim/sessionextra.hpp"
#include "game/sim/simplerunner.hpp"
#include "util/randomnumbergenerator.hpp"

namespace {
    const char*const LOG_NAME = "game.proxy.sim.run";
    const afl::sys::LogListener::Level LOG_LEVEL = afl::sys::LogListener::Trace;
    using afl::string::Format;
}

/*
 *  Trampoline
 *
 *  Every run request comes with its own fresh StopSignal.
 *  UI side will signal the StopSignal and discard it, but game side will still have a reference to its copy.
 *  This saves a bunch of headaches when UI side does things quickly,
 *  i.e. in a run/stop/run sequence, the stop is never accidentally applied to the second run.
 */

class game::proxy::SimulationRunProxy::Trampoline {
    friend class Adaptor;
 public:
    Trampoline(util::RequestSender<SimulationRunProxy> reply, Session& session);

    void runFinite(size_t n, afl::base::Ptr<util::StopSignal> stopper);
    void runInfinite(afl::base::Ptr<util::StopSignal> stopper);
    void runSeries(afl::base::Ptr<util::StopSignal> stopper);

    VcrDatabaseAdaptor* makeClassResultBattleAdaptor(size_t index);
    VcrDatabaseAdaptor* makeUnitResultBattleAdaptor(size_t index, UnitInfo_t::Type type, bool max);

 private:
    util::RequestSender<SimulationRunProxy> m_reply;

    afl::base::Ref<game::sim::Session> m_sim;
    afl::sys::LogListener& m_log;
    afl::string::Translator& m_translator;
    afl::base::Ptr<game::spec::ShipList> m_shipList;
    afl::base::Ptr<Root> m_root;
    afl::base::Ptr<Game> m_game;
    util::RandomNumberGenerator m_rng;

    std::auto_ptr<game::sim::Runner> m_runner;

    void reportUpdate();
    void reportStop();
};

class game::proxy::SimulationRunProxy::Adaptor : public VcrDatabaseAdaptor {
 public:
    Adaptor(Trampoline& tpl, game::sim::Database_t b)
        : m_trampoline(tpl),
          m_battles(b)
        { }
    virtual const Root& root() const
        {
            afl::except::checkAssertion(m_trampoline.m_root.get() != 0, "<SimulationRunProxy.Adaptor.Root>");
            return *m_trampoline.m_root;
        }
    virtual const game::spec::ShipList& shipList() const
        {
            afl::except::checkAssertion(m_trampoline.m_shipList.get() != 0, "<SimulationRunProxy.Adaptor.ShipList>");
            return *m_trampoline.m_shipList;
        }
    virtual const TeamSettings* getTeamSettings() const
        { return 0; /* FIXME */ }
    virtual game::vcr::Database& battles()
        { return *m_battles; }
    virtual afl::sys::LogListener& log()
        { return m_trampoline.m_log; }
    virtual afl::string::Translator& translator()
        { return m_trampoline.m_translator; }
    virtual size_t getCurrentBattle() const
        { return 0; }
    virtual void setCurrentBattle(size_t /*n*/)
        { }
    virtual game::sim::Setup* getSimulationSetup() const
        { return &m_trampoline.m_sim->setup(); }
    virtual bool isGameObject(const game::vcr::Object& obj) const
        {
            Game* g = m_trampoline.m_game.get();
            return g != 0 && g->isGameObject(obj, shipList().hulls());
        }
 private:
    Trampoline& m_trampoline;
    game::sim::Database_t m_battles;
};

game::proxy::SimulationRunProxy::Trampoline::Trampoline(util::RequestSender<SimulationRunProxy> reply, Session& session)
    : m_reply(reply),
      m_sim(game::sim::getSimulatorSession(session)),
      m_log(session.log()),
      m_translator(session.translator()),
      m_shipList(session.getShipList()),
      m_root(session.getRoot()),
      m_game(session.getGame()),
      m_rng(session.rng()),
      m_runner()
{
    // Advance session's RNG so next invocation will be different
    session.rng()();

    // Create runner
    if (m_shipList.get() != 0 && m_root.get() != 0) {
        // Assign random friendly codes; those are shown to users
        game::sim::prepareSimulation(m_sim->setup(), m_sim->configuration(), m_rng);
        m_sim->setup().notifyListeners();

        // Build runner
        // FIXME: configure number of threads
        m_runner.reset(new game::sim::SimpleRunner(m_sim->setup(), m_sim->configuration(), *m_shipList, m_root->hostConfiguration(), m_rng));
        m_runner->sig_update.add(this, &Trampoline::reportUpdate);
    }
}

void
game::proxy::SimulationRunProxy::Trampoline::runFinite(size_t n, afl::base::Ptr<util::StopSignal> stopper)
{
    m_log.write(LOG_LEVEL, LOG_NAME, Format("=> runFinite(%d)", n));
    if (m_runner.get() != 0) {
        size_t target = m_runner->resultList().getNumBattles() + n;
        if (m_runner->init()) {
            size_t now = m_runner->resultList().getNumBattles();
            if (now < target) {
                m_runner->run(m_runner->makeFiniteLimit(target - now), *stopper);
            }
        }
    }
    reportUpdate();
    reportStop();
}

void
game::proxy::SimulationRunProxy::Trampoline::runInfinite(afl::base::Ptr<util::StopSignal> stopper)
{
    m_log.write(LOG_LEVEL, LOG_NAME, "=> runInfinite");
    if (m_runner.get() != 0) {
        if (m_runner->init()) {
            m_runner->run(m_runner->makeNoLimit(), *stopper);
        }
    }
    reportUpdate();
    reportStop();
}

void
game::proxy::SimulationRunProxy::Trampoline::runSeries(afl::base::Ptr<util::StopSignal> stopper)
{
    m_log.write(LOG_LEVEL, LOG_NAME, "=> runSeries");
    if (m_runner.get() != 0) {
        if (m_runner->init()) {
            m_runner->run(m_runner->makeSeriesLimit(), *stopper);
        }
    }
    reportUpdate();
    reportStop();
}

inline game::proxy::VcrDatabaseAdaptor*
game::proxy::SimulationRunProxy::Trampoline::makeClassResultBattleAdaptor(size_t index)
{
    // Must have runner
    afl::except::checkAssertion(m_runner.get() != 0, "<makeClassResultBattleAdaptor.Runner>");

    // Must have class result
    const game::sim::ClassResult* r = m_runner->resultList().getClassResult(index);
    afl::except::checkAssertion(r != 0, "<makeClassResultBattleAdaptor.ClassResult>");

    // Must have sample battle
    game::sim::Database_t b = r->getSampleBattle();
    afl::except::checkAssertion(b.get() != 0, "<makeClassResultBattleAdaptor.Database>");

    return new Adaptor(*this, b);
}

inline game::proxy::VcrDatabaseAdaptor*
game::proxy::SimulationRunProxy::Trampoline::makeUnitResultBattleAdaptor(size_t index, UnitInfo_t::Type type, bool max)
{
    // Must have runner
    afl::except::checkAssertion(m_runner.get() != 0, "<makeUnitResultBattleAdaptor.Runner>");

    // Must have sample battle
    game::sim::Database_t b = m_runner->resultList().getUnitSampleBattle(index, type, max);
    afl::except::checkAssertion(b.get() != 0, "<makeClassResultBattleAdaptor.Database>");

    return new Adaptor(*this, b);
}

void
game::proxy::SimulationRunProxy::Trampoline::reportUpdate()
{
    class Task : public util::Request<SimulationRunProxy> {
     public:
        Task(const game::sim::ResultList& r, const game::sim::Setup& setup, util::NumberFormatter fmt)
            : m_numBattles(r.getNumBattles()),
              m_classResults()
            {
                for (size_t i = 0, n = r.getNumClassResults(); i != n; ++i) {
                    m_classResults.push_back(r.describeClassResult(i, fmt));
                }
                for (size_t i = 0, n = r.getNumUnitResults(); i != n; ++i) {
                    m_unitResults.push_back(r.describeUnitResult(i, setup));
                }
            }
        virtual void handle(SimulationRunProxy& proxy)
            {
                proxy.m_numBattles = m_numBattles;
                proxy.m_classResults.swap(m_classResults);
                proxy.m_unitResults.swap(m_unitResults);
                proxy.sig_update.raise();
            }
     private:
        size_t m_numBattles;
        ClassInfos_t m_classResults;
        UnitInfos_t m_unitResults;
    };


    if (m_runner.get() != 0 && m_root.get() != 0) {
        const game::sim::ResultList& r = m_runner->resultList();
        m_log.write(LOG_LEVEL, LOG_NAME, Format("<= update: %d runs, %d classes", r.getNumBattles(), r.getNumClassResults()));
        m_reply.postNewRequest(new Task(r, m_sim->setup(), m_root->userConfiguration().getNumberFormatter()));
    }
}

void
game::proxy::SimulationRunProxy::Trampoline::reportStop()
{
    m_log.write(LOG_LEVEL, LOG_NAME, "<= stop");
    m_reply.postRequest(&SimulationRunProxy::reportStop);
}


/*
 *  TrampolineFromSession
 */

class game::proxy::SimulationRunProxy::TrampolineFromSession : public afl::base::Closure<Trampoline* (Session&)> {
 public:
    TrampolineFromSession(util::RequestSender<SimulationRunProxy> reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(m_reply, session); }
 private:
    util::RequestSender<SimulationRunProxy> m_reply;
};


/*
 *  SimulationRunProxy
 */

game::proxy::SimulationRunProxy::SimulationRunProxy(SimulationSetupProxy& setup, util::RequestDispatcher& reply)
    : m_stopper(),
      m_reply(reply, *this),
      m_request(setup.gameSender().makeTemporary(new TrampolineFromSession(m_reply.getSender()))),
      m_numBattles(),
      m_classResults(),
      m_unitResults()
{ }

game::proxy::SimulationRunProxy::~SimulationRunProxy()
{
    stop();
}

void
game::proxy::SimulationRunProxy::runFinite(size_t n)
{
    stop();
    m_request.postRequest(&Trampoline::runFinite, n, makeNewStopSignal());
}

void
game::proxy::SimulationRunProxy::runInfinite()
{
    stop();
    m_request.postRequest(&Trampoline::runInfinite, makeNewStopSignal());
}

void
game::proxy::SimulationRunProxy::runSeries()
{
    stop();
    m_request.postRequest(&Trampoline::runSeries, makeNewStopSignal());
}

void
game::proxy::SimulationRunProxy::stop()
{
    if (m_stopper.get() != 0) {
        m_stopper->set();
        m_stopper = 0;
    }
}

size_t
game::proxy::SimulationRunProxy::getNumBattles() const
{
    return m_numBattles;
}

size_t
game::proxy::SimulationRunProxy::getNumClassResults() const
{
    return m_classResults.size();
}

const game::proxy::SimulationRunProxy::ClassInfo_t*
game::proxy::SimulationRunProxy::getClassInfo(size_t index) const
{
    return (index < m_classResults.size() ? &m_classResults[index] : 0);
}

const game::proxy::SimulationRunProxy::ClassInfos_t&
game::proxy::SimulationRunProxy::getClassResults() const
{
    return m_classResults;
}

size_t
game::proxy::SimulationRunProxy::getNumUnitResults() const
{
    return m_unitResults.size();
}

const game::proxy::SimulationRunProxy::UnitInfo_t*
game::proxy::SimulationRunProxy::getUnitInfo(size_t index) const
{
    return (index < m_unitResults.size() ? &m_unitResults[index] : 0);
}

const game::proxy::SimulationRunProxy::UnitInfos_t&
game::proxy::SimulationRunProxy::getUnitResults() const
{
    return m_unitResults;
}

util::RequestSender<game::proxy::VcrDatabaseAdaptor>
game::proxy::SimulationRunProxy::makeClassResultBattleAdaptor(size_t index)
{
    class AdaptorFromTrampoline : public afl::base::Closure<VcrDatabaseAdaptor*(Trampoline&)> {
     public:
        AdaptorFromTrampoline(size_t index)
            : m_index(index)
            { }
        virtual VcrDatabaseAdaptor* call(Trampoline& tpl)
            { return tpl.makeClassResultBattleAdaptor(m_index); }
     private:
        size_t m_index;
    };
    return m_request.makeTemporary(new AdaptorFromTrampoline(index));
}

util::RequestSender<game::proxy::VcrDatabaseAdaptor>
game::proxy::SimulationRunProxy::makeUnitResultBattleAdaptor(size_t index, UnitInfo_t::Type type, bool max)
{
    class AdaptorFromTrampoline : public afl::base::Closure<VcrDatabaseAdaptor*(Trampoline&)> {
     public:
        AdaptorFromTrampoline(size_t index, UnitInfo_t::Type type, bool max)
            : m_index(index), m_type(type), m_max(max)
            { }
        virtual VcrDatabaseAdaptor* call(Trampoline& tpl)
            { return tpl.makeUnitResultBattleAdaptor(m_index, m_type, m_max); }
     private:
        size_t m_index;
        UnitInfo_t::Type m_type;
        bool m_max;
    };
    return m_request.makeTemporary(new AdaptorFromTrampoline(index, type, max));
}

afl::base::Ptr<util::StopSignal>
game::proxy::SimulationRunProxy::makeNewStopSignal()
{
    stop();

    afl::base::Ptr<util::StopSignal> result(new util::StopSignal());
    m_stopper = result;
    return result;
}

void
game::proxy::SimulationRunProxy::reportStop()
{
    m_stopper = 0;
    sig_stop.raise();
}
