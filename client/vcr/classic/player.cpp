/**
  *  \file client/vcr/classic/player.cpp
  */

#include "client/vcr/classic/player.hpp"
#include "game/vcr/classic/database.hpp"
#include "game/vcr/classic/battle.hpp"
#include "game/vcr/classic/utils.hpp"
#include "game/root.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/game.hpp"
#include "util/requestreceiver.hpp"
#include "game/vcr/classic/nullvisualizer.hpp"

namespace gvc = game::vcr::classic;

namespace {
    /* Logger name for this module */
    const char* LOG_NAME = "client.vcr.classic";

    /* Number of battle ticks to render per request.
       Each battle tick can generate roundabout 2 sides x 40 weapons x 10 events = 800 events;
       At 6 words/event, that is 19200 bytes/tick, leading to around 2 MB buffer for TIME_PER_REQUEST=100. */
    const int TIME_PER_REQUEST = 100;
}

client::vcr::classic::Player::Player(util::RequestSender<PlayerListener> reply)
    : m_reply(reply),
      m_recorder(),
      m_visualizer(m_recorder),
      m_algorithm(),
      m_index(0)
{ }

void
client::vcr::classic::Player::init(game::Session& /*session*/)
{ }

void
client::vcr::classic::Player::done(game::Session& /*session*/)
{ }

void
client::vcr::classic::Player::initRequest(game::Session& session, size_t index)
{
    m_index = index;
    gvc::Database* db = gvc::getDatabase(session);
    gvc::Battle* b = db != 0 ? db->getBattle(index) : 0;
    const game::Root* root = session.getRoot().get();
    const game::spec::ShipList* shipList = session.getShipList().get();
    if (db != 0 && root != 0 && shipList != 0 && b != 0) {
        m_algorithm.reset(b->createAlgorithm(m_visualizer, root->hostConfiguration(), *shipList));
        if (m_algorithm.get() == 0) {
            // FIXME: must tell the player
            session.world().logListener().write(afl::sys::LogListener::Error, LOG_NAME, "!Failed to set up VCR algorithm");
            sendResponse(session, true);
        } else {
            // FIXME: must normally check getGame(), but since db is non-null, it will probably be...
            {
                uint16_t seed = b->getSeed();
                game::vcr::Object leftCopy = b->left(), rightCopy = b->right();
                m_algorithm->setCapabilities(b->getCapabilities());
                if (m_algorithm->checkBattle(leftCopy, rightCopy, seed)) {
                    session.world().logListener().write(afl::sys::LogListener::Error, LOG_NAME, "!VCR algorithm does not accept");
                }
            }
            m_visualizer.init(*m_algorithm, *b, *shipList, root->playerList(), session.getGame()->teamSettings(), root->hostConfiguration());
            sendResponse(session, false);
        }
    } else {
        // FIXME: must tell the player
        session.world().logListener().write(afl::sys::LogListener::Error, LOG_NAME, "!Failed to access game data");
        sendResponse(session, true);
    }
}

void
client::vcr::classic::Player::eventRequest(game::Session& session)
{
    if (m_algorithm.get() != 0) {
        bool done = false;
        for (int i = 0; i < TIME_PER_REQUEST && !done; ++i) {
            done = !m_visualizer.playCycle(*m_algorithm);
        }
        sendResponse(session, done);
    }
}

void
client::vcr::classic::Player::jumpRequest(game::Session& session, game::vcr::classic::Time_t time)
{
    // ex VcrSpriteVisualizer::windTo [sort-of]
    gvc::Database* db = gvc::getDatabase(session);
    gvc::Battle* b = db != 0 ? db->getBattle(m_index) : 0;
    const game::Root* root = session.getRoot().get();
    const game::spec::ShipList* shipList = session.getShipList().get();
    if (db != 0 && root != 0 && shipList != 0 && b != 0 && m_algorithm.get() != 0) {
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
        sendResponse(session, done);
    }
}

void
client::vcr::classic::Player::sendInitRequest(util::SlaveRequestSender<game::Session, Player>& sender, size_t index)
{
    class InitRequest : public util::SlaveRequest<game::Session, Player> {
     public:
        InitRequest(size_t index)
            : m_index(index)
            { }
        virtual void handle(game::Session& session, Player& player)
            { player.initRequest(session, m_index); }
     private:
        size_t m_index;
    };
    sender.postNewRequest(new InitRequest(index));
}

void
client::vcr::classic::Player::sendEventRequest(util::SlaveRequestSender<game::Session,Player>& sender)
{
    class EventRequest : public util::SlaveRequest<game::Session, Player> {
     public:
        virtual void handle(game::Session& session, Player& player)
            { player.eventRequest(session); }
    };
    sender.postNewRequest(new EventRequest());
}

void
client::vcr::classic::Player::sendJumpRequest(util::SlaveRequestSender<game::Session,Player>& sender, game::vcr::classic::Time_t time)
{
    class JumpRequest : public util::SlaveRequest<game::Session, Player> {
     public:
        JumpRequest(gvc::Time_t time)
            : m_time(time)
            { }
        virtual void handle(game::Session& session, Player& player)
            { player.jumpRequest(session, m_time); }
     private:
        gvc::Time_t m_time;
    };
    sender.postNewRequest(new JumpRequest(time));
}

void
client::vcr::classic::Player::sendResponse(game::Session& /*session*/, bool finish)
{
    class Response : public util::Request<PlayerListener> {
     public:
        Response(gvc::EventRecorder& rec, bool finish)
            : m_list(),
              m_finish(finish)
            { rec.swapContent(m_list); }
        virtual void handle(PlayerListener& s)
            { s.handleEvents(m_list, m_finish); }
     private:
        util::StringInstructionList m_list;
        bool m_finish;
    };
    m_reply.postNewRequest(new Response(m_recorder, finish));
}
