/**
  *  \file game/proxy/classicvcrplayerproxy.cpp
  *  \brief Class game::proxy::ClassicVcrPlayerProxy
  */

#include "game/proxy/classicvcrplayerproxy.hpp"
#include "game/vcr/classic/eventrecorder.hpp"
#include "game/vcr/classic/eventvisualizer.hpp"
#include "game/vcr/classic/algorithm.hpp"
#include "game/vcr/classic/database.hpp"
#include "game/vcr/classic/nullvisualizer.hpp"

namespace gvc = game::vcr::classic;

namespace {
    /* Logger name for this module */
    const char* LOG_NAME = "game.vcr.classic";

    /* Number of battle ticks to render per request.
       Each battle tick can generate roundabout 2 sides x 40 weapons x 10 events = 800 events;
       At 6 words/event, that is 19200 bytes/tick, leading to around 2 MB buffer for TIME_PER_REQUEST=100. */
    const int TIME_PER_REQUEST = 100;
}

class game::proxy::ClassicVcrPlayerProxy::Trampoline {
 public:
    Trampoline(util::RequestSender<ClassicVcrPlayerProxy> reply, VcrDatabaseAdaptor& adaptor);

    void initRequest(size_t index);
    void eventRequest();
    void jumpRequest(game::vcr::classic::Time_t time);

    void sendResponse(bool finish);

 private:
    util::RequestSender<ClassicVcrPlayerProxy> m_reply;
    VcrDatabaseAdaptor& m_adaptor;

    game::vcr::classic::EventRecorder m_recorder;
    game::vcr::classic::EventVisualizer m_visualizer;
    std::auto_ptr<game::vcr::classic::Algorithm> m_algorithm;
    size_t m_index;
};


game::proxy::ClassicVcrPlayerProxy::Trampoline::Trampoline(util::RequestSender<ClassicVcrPlayerProxy> reply, VcrDatabaseAdaptor& adaptor)
    : m_reply(reply), m_adaptor(adaptor),
      m_recorder(), m_visualizer(m_recorder), m_algorithm(), m_index(0)
{ }

void
game::proxy::ClassicVcrPlayerProxy::Trampoline::initRequest(size_t index)
{
    m_index = index;

    gvc::Database* db = dynamic_cast<gvc::Database*>(&m_adaptor.battles());
    gvc::Battle* b = db != 0 ? db->getBattle(index) : 0;
    const Root& root = m_adaptor.root();
    const game::spec::ShipList& shipList = m_adaptor.shipList();
    afl::sys::LogListener& log = m_adaptor.log();
    if (db != 0 && b != 0) {
        m_algorithm.reset(b->createAlgorithm(m_visualizer, root.hostConfiguration(), shipList));
        if (m_algorithm.get() == 0) {
            // FIXME: must tell the player
            log.write(afl::sys::LogListener::Error, LOG_NAME, m_adaptor.translator()("Failed to set up VCR algorithm"));
            sendResponse(true);
        } else {
            uint16_t seed = b->getSeed();
            game::vcr::Object leftCopy = b->left(), rightCopy = b->right();
            m_algorithm->setCapabilities(b->getCapabilities());
            if (m_algorithm->checkBattle(leftCopy, rightCopy, seed)) {
                log.write(afl::sys::LogListener::Error, LOG_NAME, m_adaptor.translator()("VCR algorithm does not accept"));
                sendResponse(true);
            } else {
                m_visualizer.init(*m_algorithm, *b, shipList, root.playerList(), m_adaptor.getTeamSettings(), root.hostConfiguration(), m_adaptor.translator());
                sendResponse(false);
            }
        }
    } else {
        // FIXME: must tell the player
        log.write(afl::sys::LogListener::Error, LOG_NAME, m_adaptor.translator()("Failed to access game data"));
        sendResponse(true);
    }
}

void
game::proxy::ClassicVcrPlayerProxy::Trampoline::eventRequest()
{
    if (m_algorithm.get() != 0) {
        bool done = false;
        for (int i = 0; i < TIME_PER_REQUEST && !done; ++i) {
            done = !m_visualizer.playCycle(*m_algorithm);
        }
        sendResponse(done);
    } else {
        sendResponse(true);
    }
}

void
game::proxy::ClassicVcrPlayerProxy::Trampoline::jumpRequest(game::vcr::classic::Time_t time)
{
    // ex VcrSpriteVisualizer::windTo [sort-of]
    gvc::Database* db = dynamic_cast<gvc::Database*>(&m_adaptor.battles());
    gvc::Battle* b = db != 0 ? db->getBattle(m_index) : 0;
    if (db != 0 && b != 0 && m_algorithm.get() != 0) {
        const gvc::Time_t now = m_algorithm->getTime();
        bool done = false;

        // Go to a starting point.
        // FIXME: use checkpoints
        if (time < now) {
            m_algorithm->initBattle(b->left(), b->right(), b->getSeed());
        }

        // Play, silently
        gvc::NullVisualizer nullVis;
        m_algorithm->setVisualizer(nullVis);
        while (!done && m_algorithm->getTime() < time) {
            done = !m_algorithm->playCycle();
        }
        if (done) {
            game::vcr::Object left, right;
            m_algorithm->doneBattle(left, right);
        }
        m_algorithm->setVisualizer(m_visualizer);

        // Send state
        m_visualizer.refresh(*m_algorithm, done);
        sendResponse(done);
    } else {
        sendResponse(true);
    }
}

void
game::proxy::ClassicVcrPlayerProxy::Trampoline::sendResponse(bool finish)
{
    class Response : public util::Request<ClassicVcrPlayerProxy> {
     public:
        Response(gvc::EventRecorder& rec, bool finish)
            : m_list(),
              m_finish(finish)
            { rec.swapContent(m_list); }
        virtual void handle(ClassicVcrPlayerProxy& s)
            { s.sig_event.raise(m_list, m_finish); }
     private:
        util::StringInstructionList m_list;
        bool m_finish;
    };
    m_reply.postNewRequest(new Response(m_recorder, finish));
}

/*
 *  TrampolineFromAdaptor
 */

class game::proxy::ClassicVcrPlayerProxy::TrampolineFromAdaptor : public afl::base::Closure<Trampoline*(VcrDatabaseAdaptor&)> {
 public:
    TrampolineFromAdaptor(util::RequestSender<ClassicVcrPlayerProxy> reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(VcrDatabaseAdaptor& adaptor)
        { return new Trampoline(m_reply, adaptor); }
 private:
    util::RequestSender<ClassicVcrPlayerProxy> m_reply;
};


/*
 *  ClassicVcrPlayerProxy
 */

game::proxy::ClassicVcrPlayerProxy::ClassicVcrPlayerProxy(util::RequestSender<VcrDatabaseAdaptor> sender, util::RequestDispatcher& recv)
    : m_reply(recv, *this),
      m_request(sender.makeTemporary(new TrampolineFromAdaptor(m_reply.getSender())))
{ }

game::proxy::ClassicVcrPlayerProxy::~ClassicVcrPlayerProxy()
{ }

void
game::proxy::ClassicVcrPlayerProxy::initRequest(size_t index)
{
    m_request.postRequest(&Trampoline::initRequest, index);
}

void
game::proxy::ClassicVcrPlayerProxy::eventRequest()
{
    m_request.postRequest(&Trampoline::eventRequest);
}

void
game::proxy::ClassicVcrPlayerProxy::jumpRequest(game::vcr::classic::Time_t time)
{
    m_request.postRequest(&Trampoline::jumpRequest, time);
}
