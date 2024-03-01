/**
  *  \file game/proxy/flakvcrplayerproxy.cpp
  *  \brief Class game::proxy::FlakVcrPlayerProxy
  */

#include "game/proxy/flakvcrplayerproxy.hpp"
#include "game/vcr/flak/algorithm.hpp"
#include "game/vcr/flak/battle.hpp"
#include "game/vcr/flak/eventrecorder.hpp"
#include "game/vcr/flak/gameenvironment.hpp"
#include "game/vcr/flak/nullvisualizer.hpp"

namespace gvf = game::vcr::flak;

namespace {
    /* Logger name for this module */
    const char* LOG_NAME = "game.vcr.flak";

    /* Time to compute per request */
    /* Memory consumption estimation, per tick:
         500 ships x standard update                   x 32 bytes =  16k
                   x 20 weapons = 10000 weapon updates x 16 bytes = 160k
                   x 20 objects = 10000 object updates x 20 bytes = 200k
         >> Total                                                 = 376k  per tick */
    const int TIME_PER_REQUEST = 20;
}

class game::proxy::FlakVcrPlayerProxy::Trampoline {
 public:
    Trampoline(util::RequestSender<FlakVcrPlayerProxy> reply, VcrDatabaseAdaptor& adaptor);

    void initRequest(size_t index);
    void eventRequest();
    void jumpRequest(int32_t time);

    void saveTick(gvf::EventRecorder& r);
    void sendResponse(bool finish);

 private:
    util::RequestSender<FlakVcrPlayerProxy> m_reply;
    VcrDatabaseAdaptor& m_adaptor;

    Result_t m_result;
    std::auto_ptr<gvf::Algorithm> m_algorithm;
    size_t m_index;
};

game::proxy::FlakVcrPlayerProxy::Trampoline::Trampoline(util::RequestSender<FlakVcrPlayerProxy> reply, VcrDatabaseAdaptor& adaptor)
    : m_reply(reply),
      m_adaptor(adaptor),
      m_result(),
      m_algorithm(),
      m_index(0)
{ }

void
game::proxy::FlakVcrPlayerProxy::Trampoline::initRequest(size_t index)
{
    // Clear state
    m_index = index;
    m_result.clear();
    m_algorithm.reset();

    // Obtain battle
    gvf::Battle* battle = dynamic_cast<gvf::Battle*>(m_adaptor.getBattles()->getBattle(index));
    afl::sys::LogListener& log = m_adaptor.log();
    if (battle != 0) {
        // Create algorithm
        gvf::GameEnvironment env(m_adaptor.getRoot()->hostConfiguration(), m_adaptor.getShipList()->beams(), m_adaptor.getShipList()->launchers());
        m_algorithm.reset(new gvf::Algorithm(battle->setup(), env));

        // Initialize
        gvf::EventRecorder r;
        m_algorithm->init(env, r);
        saveTick(r);
        sendResponse(false);
    } else {
        // Report failure
        log.write(afl::sys::LogListener::Error, LOG_NAME, m_adaptor.translator()("Failed to access game data"));
        sendResponse(true);
    }
}

void
game::proxy::FlakVcrPlayerProxy::Trampoline::eventRequest()
{
    if (m_algorithm.get() != 0) {
        // Environment
        gvf::GameEnvironment env(m_adaptor.getRoot()->hostConfiguration(), m_adaptor.getShipList()->beams(), m_adaptor.getShipList()->launchers());
        gvf::EventRecorder r;

        // Play
        bool done = false;
        for (int i = 0; i < TIME_PER_REQUEST && !done; ++i) {
            done = !m_algorithm->playCycle(env, r);
            saveTick(r);
        }
        sendResponse(done);
    } else {
        sendResponse(true);
    }
}

void
game::proxy::FlakVcrPlayerProxy::Trampoline::jumpRequest(int32_t time)
{
    if (m_algorithm.get() != 0) {
        // Environment
        gvf::GameEnvironment env(m_adaptor.getRoot()->hostConfiguration(), m_adaptor.getShipList()->beams(), m_adaptor.getShipList()->launchers());
        gvf::NullVisualizer null;
        bool done = false;
        if (time == 0) {
            // Special case: rewind to beginning
            gvf::EventRecorder r;
            m_algorithm->init(env, r);
            saveTick(r);
        } else {
            // We want to report one tick's real time: go to time-1 and play silently, then play one tick visibly
            --time;

            // Go to a starting point.
            // FIXME: use checkpoints
            if (time <= m_algorithm->getTime()) {
                m_algorithm->init(env, null);
            }

            // Play, silently, until just before the time
            while (!done && m_algorithm->getTime() < time) {
                done = !m_algorithm->playCycle(env, null);
            }

            // Send some data
            if (!done) {
                gvf::EventRecorder r;
                done = !m_algorithm->playCycle(env, r);
                saveTick(r);
            }
        }
        sendResponse(done);
    } else {
        // Report failure
        sendResponse(true);
    }
}

void
game::proxy::FlakVcrPlayerProxy::Trampoline::saveTick(gvf::EventRecorder& r)
{
    util::StringInstructionList* p = m_result.pushBackNew(new util::StringInstructionList());
    r.swapContent(*p);
}

void
game::proxy::FlakVcrPlayerProxy::Trampoline::sendResponse(bool finish)
{
    class Task : public util::Request<FlakVcrPlayerProxy> {
     public:
        Task(Trampoline& tpl, bool finish)
            : m_result(), m_finish(finish)
            { tpl.m_result.swap(m_result); }
        virtual void handle(FlakVcrPlayerProxy& proxy)
            { proxy.sig_event.raise(m_result, m_finish); }
     private:
        Result_t m_result;
        bool m_finish;
    };
    m_reply.postNewRequest(new Task(*this, finish));
}


/*
 *  TrampolineFromAdaptor
 */

class game::proxy::FlakVcrPlayerProxy::TrampolineFromAdaptor : public afl::base::Closure<Trampoline*(VcrDatabaseAdaptor&)> {
 public:
    TrampolineFromAdaptor(util::RequestSender<FlakVcrPlayerProxy> reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(VcrDatabaseAdaptor& adaptor)
        { return new Trampoline(m_reply, adaptor); }
 private:
    util::RequestSender<FlakVcrPlayerProxy> m_reply;
};


game::proxy::FlakVcrPlayerProxy::FlakVcrPlayerProxy(util::RequestSender<VcrDatabaseAdaptor> sender, util::RequestDispatcher& recv)
    : m_reply(recv, *this),
      m_request(sender.makeTemporary(new TrampolineFromAdaptor(m_reply.getSender())))
{ }

game::proxy::FlakVcrPlayerProxy::~FlakVcrPlayerProxy()
{ }

void
game::proxy::FlakVcrPlayerProxy::initRequest(size_t index)
{
    m_request.postRequest(&Trampoline::initRequest, index);
}

void
game::proxy::FlakVcrPlayerProxy::eventRequest()
{
    m_request.postRequest(&Trampoline::eventRequest);
}

void
game::proxy::FlakVcrPlayerProxy::jumpRequest(int32_t time)
{
    m_request.postRequest(&Trampoline::jumpRequest, time);
}
