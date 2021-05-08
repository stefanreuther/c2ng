/**
  *  \file game/proxy/simulationrunproxy.hpp
  *  \brief Class game::proxy::SimulationRunProxy
  */
#ifndef C2NG_GAME_PROXY_SIMULATIONRUNPROXY_HPP
#define C2NG_GAME_PROXY_SIMULATIONRUNPROXY_HPP

#include <vector>
#include "afl/base/ptr.hpp"
#include "afl/base/signal.hpp"
#include "game/proxy/vcrdatabaseadaptor.hpp"
#include "game/sim/resultlist.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"
#include "util/stopsignal.hpp"

namespace game { namespace proxy {

    class SimulationSetupProxy;
    class WaitIndicator;

    /** Simulation runner proxy.
        Proxies a game::sim::Runner instance.

        This proxy is special because it performs bulk computation (the simulation) on the game thread.
        From UI side, it behaves as a normal bidirectional asynchronous proxy.
        However, while it is running a computation, the game thread is busy AND WILL NOT ANSWER OTHER PROXIES' REQUESTS.
        Be careful.

        This proxy caches the information received from the game thread and can therefore be queried at any time.

        Usage:
        - construct SimulationRunProxy from the SimulationSetupProxy you want to simulate
        - call a run() method to run simulations.
        - wait for sig_stop signal before performing operations on other proxies
        - at any time, query current simulation result
        - simulation results update during computation; watch sig_update. */
    class SimulationRunProxy {
     public:
        typedef game::sim::ResultList::ClassInfo ClassInfo_t;
        typedef std::vector<ClassInfo_t> ClassInfos_t;

        typedef game::sim::ResultList::UnitInfo UnitInfo_t;
        typedef std::vector<UnitInfo_t> UnitInfos_t;

        /** Constructor.
            \param setup SimulationSetupProxy whose setup to simulate */
        explicit SimulationRunProxy(SimulationSetupProxy& setup, util::RequestDispatcher& reply);
        ~SimulationRunProxy();

        /** Run a finite number of iterations.
            Returns immediately but starts the computation on the game session.

            This method should only be called if the game session is idle.
            It will then compute simulations until the desired number is reached or you call stop().
            After that, it will emit sig_stop.
            Until that, the game session is considered busy.

            If this method is called when the session is busy, the previous run will be aborted as if stop() were called.
            That previous stop will generate a sig_stop callback, as will the completion of the new run.

            \param n Number of iterations to run
            \see game::sim::Runner::makeFiniteLimit() */
        void runFinite(size_t n);

        /** Run indefinitely.
            Produces simulations until stop() is called.
            See runFinite() for call ordering constraints.
            \see game::sim::Runner::makeNoLimit() */
        void runInfinite();

        /** Run series.
            See runFinite() for call ordering constraints.
            \see game::sim::Runner::makeSeriesLimit() */
        void runSeries();

        /** Stop.
            Causes the current run to be aborted asynchronously and eventually to emit sig_stop as confirmation.
            If no run is currently active, does nothing. */
        void stop();

        /** Get number of battles run so far.
            \return Number */
        size_t getNumBattles() const;

        /** Get number of result classes.
            \return Number of result classes
            \see game::sim::ResultList::getNumClassResults */
        size_t getNumClassResults() const;

        /** Get class result.
            \param index Index [0,getNumClassResults())
            \return Class result info; 0 if index out of range
            \see game::sim::ResultList::getClassResult */
        const ClassInfo_t* getClassInfo(size_t index) const;

        /** Get class results.
            \return Vector of class results */
        const ClassInfos_t& getClassResults() const;

        /** Get number of unit results.
            \return Number of unit results
            \see game::sim::ResultList::getNumUnitResults */
        size_t getNumUnitResults() const;

        /** Get unit result.
            \param index Index [0,getNumUnitResults())
            \return Unit result info; 0 if index out of range
            \see game::sim::ResultList::getUnitResult */
        const UnitInfo_t* getUnitInfo(size_t index) const;

        /** Get unit results.
            \return Vector of unit results */
        const UnitInfos_t& getUnitResults() const;

        /** Get access to class result battles.
            Creates an Adaptor for use with VcrDatabaseProxy.
            \param index Index [0,getNumClassResults)
            \return Adaptor; usable at least as long as the SimulationRunProxy is valid */
        util::RequestSender<VcrDatabaseAdaptor> makeClassResultBattleAdaptor(size_t index);

        /** Get access to unit result battles.
            Creates an Adaptor for use with VcrDatabaseProxy.
            \param index Index [0,getNumClassResults)
            \param type  Type of battle
            \param max   true to access maximum specimen, false to access minimum
            \return Adaptor; usable at least as long as the SimulationRunProxy is valid
            \see game::sim::ResultList::getUnitSampleBattle */
        util::RequestSender<VcrDatabaseAdaptor> makeUnitResultBattleAdaptor(size_t index, UnitInfo_t::Type type, bool max);

        /** Signal: data update.
            This signal is raised when new data is available for retrieval using member functions.
            \see game::sim::Runner::sig_update */
        afl::base::Signal<void()> sig_update;

        /** Signal: simulation stopped.
            This signal is raised when a simulation has stopped voluntarily (finite limit) or on request (stop()).
            A possible sig_update is emitted before sig_stop. */
        afl::base::Signal<void()> sig_stop;

     private:
        class Trampoline;
        class TrampolineFromSession;
        class Adaptor;

        afl::base::Ptr<util::StopSignal> m_stopper;
        util::RequestReceiver<SimulationRunProxy> m_reply;
        util::RequestSender<Trampoline> m_request;

        afl::base::Ptr<util::StopSignal> makeNewStopSignal();
        void reportStop();

        // Status
        size_t m_numBattles;
        ClassInfos_t m_classResults;
        UnitInfos_t m_unitResults;
    };

} }

#endif
