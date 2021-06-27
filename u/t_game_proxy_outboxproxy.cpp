/**
  *  \file u/t_game_proxy_outboxproxy.cpp
  *  \brief Test for game::proxy::OutboxProxy
  */

#include "game/proxy/outboxproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/test/root.hpp"
#include "game/game.hpp"
#include "game/turn.hpp"
#include "game/msg/outbox.hpp"
#include "game/proxy/mailboxproxy.hpp"

/** Test behaviour on empty session. */
void
TestGameProxyOutboxProxy::testEmpty()
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::OutboxProxy testee(t.gameSender());
    game::proxy::OutboxProxy::Info info;

    // Defaults
    TS_ASSERT_EQUALS(testee.getHeadersForDisplay(ind, 1, game::PlayerSet_t(2)), "");
    TS_ASSERT_EQUALS(testee.getMessage(ind, 1, info), false);

    std::auto_ptr<game::StringVerifier> v(testee.createStringVerifier(ind));
    TS_ASSERT(v.get() == 0);

    // Doesn't crash
    testee.addMessage(1, "x", game::PlayerSet_t(2));
    testee.setMessageText(3, "foo");
    testee.setMessageReceivers(4, game::PlayerSet_t(5));
    testee.deleteMessage(6);

    t.sync();
    ind.processQueue();
}

/** Test behaviour on normal populated session. */
void
TestGameProxyOutboxProxy::testNormal()
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::OutboxProxy testee(t.gameSender());

    // Prepare content
    // - players must be defined to determine what a universal message is (getHeadersForDisplay())
    t.session().setRoot(new game::test::Root(game::HostVersion()));
    for (int i = 1; i <= 11; ++i) {
        t.session().getRoot()->playerList().create(i);
    }
    // - preload some messages
    t.session().setGame(new game::Game());
    game::msg::Outbox& mbx = t.session().getGame()->currentTurn().outbox();
    game::Id_t id1 = mbx.addMessage(1, "first", game::PlayerSet_t(10));
    game::Id_t id2 = mbx.addMessage(1, "second", game::PlayerSet_t(11));
    game::Id_t id3 = mbx.addMessage(3, "third", game::PlayerSet_t(5));
    TS_ASSERT_EQUALS(mbx.getNumMessages(), 3U);

    // Check getHeadersForDisplay
    TS_ASSERT_EQUALS(testee.getHeadersForDisplay(ind, 1, game::PlayerSet_t(2)),
                     "<<< Sub Space Message >>>\n"
                     "FROM: Player 1\n"
                     "TO: Player 2\n");

    // Check createStringVerifier
    std::auto_ptr<game::StringVerifier> v(testee.createStringVerifier(ind));
    TS_ASSERT(v.get() != 0);

    // Check getMessage - error case using a guaranteed-nonexistant Id
    {
        game::proxy::OutboxProxy::Info info;
        TS_ASSERT_EQUALS(testee.getMessage(ind, (id1|id2|id3)+1, info), false);
    }

    // Check getMessage - success case
    {
        game::proxy::OutboxProxy::Info info;
        TS_ASSERT_EQUALS(testee.getMessage(ind, id2, info), true);
        TS_ASSERT_EQUALS(info.receivers, game::PlayerSet_t(11));
        TS_ASSERT_EQUALS(info.text, "second");
        TS_ASSERT_EQUALS(info.sender, 1);
    }

    // Add messages, verify result
    testee.addMessage(5, "four", game::PlayerSet_t(4));
    testee.addMessage(5, "five", game::PlayerSet_t(6));
    t.sync();
    TS_ASSERT_EQUALS(mbx.getNumMessages(), 5U);

    // Modify, verify result
    testee.setMessageText(id2, "modified");
    testee.setMessageReceivers(id2, game::PlayerSet_t(9));
    t.sync();
    TS_ASSERT_EQUALS(mbx.getMessageRawText(1), "modified");
    TS_ASSERT_EQUALS(mbx.getMessageReceivers(1), game::PlayerSet_t(9));

    // Delete
    testee.deleteMessage(id3);
    t.sync();
    TS_ASSERT_EQUALS(mbx.getNumMessages(), 4U);
    TS_ASSERT_EQUALS(mbx.getMessageRawText(2), "four");
}

/** Test getMailboxAdaptor(). */
void
TestGameProxyOutboxProxy::testAdaptor()
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::OutboxProxy testee(t.gameSender());

    // Prepare content
    // - players must be defined to determine what a universal message is (getHeadersForDisplay())
    t.session().setRoot(new game::test::Root(game::HostVersion()));
    for (int i = 1; i <= 11; ++i) {
        t.session().getRoot()->playerList().create(i);
    }
    // - preload some messages
    t.session().setGame(new game::Game());
    game::msg::Outbox& mbx = t.session().getGame()->currentTurn().outbox();
    mbx.addMessage(1, "first", game::PlayerSet_t(10));
    mbx.addMessage(1, "second", game::PlayerSet_t(11));
    mbx.addMessage(3, "third", game::PlayerSet_t(5));
    TS_ASSERT_EQUALS(mbx.getNumMessages(), 3U);

    // Create adaptor
    util::RequestSender<game::proxy::MailboxAdaptor> ad(testee.getMailboxAdaptor());
    class Task : public util::Request<game::proxy::MailboxAdaptor> {
     public:
        void handle(game::proxy::MailboxAdaptor& ad)
            {
                // Objects must be present
                TS_ASSERT_THROWS_NOTHING(ad.session());
                TS_ASSERT_THROWS_NOTHING(ad.mailbox());

                // Object content
                TS_ASSERT_EQUALS(ad.mailbox().getNumMessages(), 3U);

                // Message storage
                TS_ASSERT_THROWS_NOTHING(ad.setCurrentMessage(2));
                TS_ASSERT_EQUALS(ad.getCurrentMessage(), 2U);
            }
    };
    ad.postNewRequest(new Task());
    t.sync();
}

/** Test cooperation of getMailboxAdaptor(), MailboxProxy. */
void
TestGameProxyOutboxProxy::testMailboxProxy()
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::OutboxProxy testee(t.gameSender());

    // Prepare content
    // - players must be defined to determine what a universal message is (getHeadersForDisplay())
    t.session().setRoot(new game::test::Root(game::HostVersion()));
    for (int i = 1; i <= 11; ++i) {
        t.session().getRoot()->playerList().create(i);
    }
    // - preload some messages
    t.session().setGame(new game::Game());
    game::msg::Outbox& mbx = t.session().getGame()->currentTurn().outbox();
    mbx.addMessage(1, "first",  game::PlayerSet_t(10));
    mbx.addMessage(1, "second", game::PlayerSet_t(10));
    mbx.addMessage(3, "third",  game::PlayerSet_t(5));
    TS_ASSERT_EQUALS(mbx.getNumMessages(), 3U);

    // Create MailboxProxy
    game::proxy::MailboxProxy proxy(testee.getMailboxAdaptor(), ind);

    // Get summary (for simplicity, use a synchronous call)
    game::msg::Browser::Summary_t summary;
    size_t index;
    proxy.getSummary(ind, summary, index);

    TS_ASSERT_EQUALS(index, 0U);
    TS_ASSERT_EQUALS(summary.size(), 2U);
    TS_ASSERT_EQUALS(summary[0].index, 0U);
    TS_ASSERT_EQUALS(summary[0].count, 2U);
    TS_ASSERT_EQUALS(summary[0].heading, "To: Player 10");
    TS_ASSERT_EQUALS(summary[1].index, 2U);
    TS_ASSERT_EQUALS(summary[1].count, 1U);
    TS_ASSERT_EQUALS(summary[1].heading, "To: Player 5");
}

