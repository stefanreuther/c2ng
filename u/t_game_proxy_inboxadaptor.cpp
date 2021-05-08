/**
  *  \file u/t_game_proxy_inboxadaptor.cpp
  *  \brief Test for game::proxy::InboxAdaptor
  */

#include "game/proxy/inboxadaptor.hpp"

#include "t_game_proxy.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/msg/inbox.hpp"
#include "game/proxy/mailboxproxy.hpp"
#include "game/test/root.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/values.hpp"

using game::proxy::MailboxProxy;

namespace {
    const char*const PLAYER_MESSAGE =
        "(-r1000)<<< Sub Space Message >>>\n"
        "Hi there.\n";

    const int PLANET_ID = 333;
    const char*const PLANET_MESSAGE =
        "(-p0333)<<< Planet >>>\n"
        "It's a planet.\n";

    const int SHIP_ID = 222;
    const char*const SHIP_MESSAGE =
        "(-s0222)<<< Planet >>>\n"
        "Oh, a ship.\n";

    struct Environment {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session;
        game::test::WaitIndicator ind;
        util::RequestReceiver<game::Session> recv;

        Environment()
            : tx(), fs(), session(tx, fs), ind(), recv(ind, session)
            {
                // Create empty root
                session.setRoot(new game::test::Root(game::HostVersion()));

                // Create empty game
                session.setGame(new game::Game());

                // Add some messages
                game::msg::Inbox& inbox = session.getGame()->currentTurn().inbox();
                inbox.addMessage(PLAYER_MESSAGE, 0);  // 0
                inbox.addMessage(PLAYER_MESSAGE, 0);  // 1
                inbox.addMessage(PLANET_MESSAGE, 0);  // 2
                inbox.addMessage(SHIP_MESSAGE,   0);  // 3
                inbox.addMessage(PLAYER_MESSAGE, 0);  // 4
                inbox.addMessage(PLANET_MESSAGE, 0);  // 5
                inbox.addMessage(PLAYER_MESSAGE, 0);  // 6

                // Create some associations
                game::map::Planet& pl = *session.getGame()->currentTurn().universe().planets().create(PLANET_ID);
                pl.messages().add(2);
                pl.messages().add(5);

                game::map::Ship& sh = *session.getGame()->currentTurn().universe().ships().create(SHIP_ID);
                sh.messages().add(3);
            }
    };

    struct UpdateReceiver {
        UpdateReceiver()
            : m_index(999), m_data()
            { }

        void onUpdate(size_t index, const MailboxProxy::Message& d)
            { m_index = index; m_data = d; }

        size_t m_index;
        MailboxProxy::Message m_data;
    };

    void verifyStatus(Environment& env, MailboxProxy& proxy, size_t numMessages, size_t currentMessage)
    {
        MailboxProxy::Status st;
        proxy.getStatus(env.ind, st);
        TS_ASSERT_EQUALS(st.numMessages, numMessages);
        TS_ASSERT_EQUALS(st.currentMessage, currentMessage);
    }

    void verifyMessageText(Environment& env, MailboxProxy& proxy, size_t num, String_t text)
    {
        UpdateReceiver u;
        afl::base::SignalConnection conn(proxy.sig_update.add(&u, &UpdateReceiver::onUpdate));
        proxy.setCurrentMessage(num);
        env.ind.processQueue();

        TS_ASSERT_EQUALS(u.m_index, num);
        TS_ASSERT_EQUALS(u.m_data.text.getText(), text);
    }
}

/** Test makeInboxAdaptor() (global inbox). */
void
TestGameProxyInboxAdaptor::testInboxAdaptor()
{
    // Environment
    Environment env;
    MailboxProxy proxy(env.recv.getSender().makeTemporary(game::proxy::makeInboxAdaptor()), env.ind);

    // Verify
    verifyStatus(env, proxy, 7, 0);
    verifyMessageText(env, proxy, 0, PLAYER_MESSAGE);
}

/** Test makePlanetInboxAdaptor(). */
void
TestGameProxyInboxAdaptor::testPlanet()
{
    // Environment
    Environment env;
    MailboxProxy proxy(env.recv.getSender().makeTemporary(game::proxy::makePlanetInboxAdaptor(PLANET_ID)), env.ind);

    // Verify
    verifyStatus(env, proxy, 2, 0);
    verifyMessageText(env, proxy, 0, PLANET_MESSAGE);
}

/** Test makeShipInboxAdaptor(). */
void
TestGameProxyInboxAdaptor::testShip()
{
    // Environment
    Environment env;
    MailboxProxy proxy(env.recv.getSender().makeTemporary(game::proxy::makeShipInboxAdaptor(SHIP_ID)), env.ind);

    // Verify
    verifyStatus(env, proxy, 1, 0);
    verifyMessageText(env, proxy, 0, SHIP_MESSAGE);
}

/** Test index handling. */
void
TestGameProxyInboxAdaptor::testIndex()
{
    // Environment: Nr. 5 is the second planet message, causing initial position to begin at 1
    Environment env;
    env.session.world().setNewGlobalValue("CCUI$CURRENTINMSG", interpreter::makeIntegerValue(5));
    MailboxProxy proxy(env.recv.getSender().makeTemporary(game::proxy::makePlanetInboxAdaptor(PLANET_ID)), env.ind);

    // Verify initial state
    verifyStatus(env, proxy, 2, 1);
    verifyMessageText(env, proxy, 1, PLANET_MESSAGE);

    // Select message 0 in filtered set; should set outer cursor to 2
    proxy.setCurrentMessage(0);
    env.ind.processQueue();

    int32_t result = -1;
    TS_ASSERT(interpreter::checkIntegerArg(result, env.session.world().getGlobalValue("CCUI$CURRENTINMSG")));
    TS_ASSERT_EQUALS(result, 2);
}

/** Test filter handling.
    Initial position is chosen as a not filtered message. */
void
TestGameProxyInboxAdaptor::testFilter()
{
    // Environment: add PLAYER_MESSAGE to filter
    Environment env;

    const char* HEADING = "(r) Sub Space Message";
    TS_ASSERT_EQUALS(env.session.getGame()->currentTurn().inbox().getMessageHeading(0, env.tx, env.session.getRoot()->playerList()), HEADING);

    env.session.getGame()->messageConfiguration().setHeadingFiltered(HEADING, true);
    MailboxProxy proxy(env.recv.getSender().makeTemporary(game::proxy::makeInboxAdaptor()), env.ind);

    // Verify initial state
    verifyStatus(env, proxy, 7, 2);

    // Browse backwards with acceptFiltered=false; this will not change the position
    proxy.browse(game::msg::Browser::Previous, 1, false);
    verifyStatus(env, proxy, 7, 2);

    // Same thing with acceptFiltered=true; now it will change
    proxy.browse(game::msg::Browser::Previous, 1, true);
    verifyStatus(env, proxy, 7, 1);
}

