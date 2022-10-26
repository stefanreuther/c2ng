/**
  *  \file u/t_game_proxy_mailboxproxy.cpp
  *  \brief Test for game::proxy::MailboxProxy
  */

#include "game/proxy/mailboxproxy.hpp"

#include "t_game_proxy.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/msg/inbox.hpp"
#include "game/proxy/inboxadaptor.hpp"
#include "game/test/counter.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"
#include "util/requestreceiver.hpp"

namespace {
    /*
     *  Mailbox for testing
     *
     *  Takes a string to determine filtered messages.
     *  Messages contain 'text-XX' as text, and 'head-X' (groups of 10) as header.
     */
    class TestMailbox : public game::msg::Mailbox {
     public:
        TestMailbox(String_t pattern, String_t prefix)
            : m_pattern(pattern), m_prefix(prefix), m_flags()
            { }

        virtual size_t getNumMessages() const
            { return m_pattern.size(); }
        virtual String_t getMessageText(size_t index, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            { return afl::string::Format("%stext-%d", m_prefix, index); }
        virtual String_t getMessageHeading(size_t index, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            { return afl::string::Format("head-%d", index / 10); }
        virtual int getMessageTurnNumber(size_t /*index*/) const
            { return 42; }
        virtual bool isMessageFiltered(size_t index, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/, const game::msg::Configuration& /*config*/) const
            {
                TS_ASSERT(index < m_pattern.size());
                return (m_pattern[index] == 'x');
            }
        virtual Flags_t getMessageFlags(size_t /*index*/) const
            { return m_flags; }
        virtual Actions_t getMessageActions(size_t /*index*/) const
            { return Actions_t(); }
        virtual void performMessageAction(size_t /*index*/, Action a)
            {
                if (a == ToggleConfirmed) {
                    m_flags ^= Confirmed;
                }
            }
     private:
        String_t m_pattern;
        String_t m_prefix;
        Flags_t m_flags;
    };

    /*
     *  Environment
     */

    struct Environment {
        afl::string::NullTranslator tx;
        afl::io::InternalFileSystem fs;
        game::Session session;
        TestMailbox mailbox;
        game::msg::Configuration config;
        size_t currentMessage;

        Environment(String_t pattern, String_t prefix)
            : tx(), fs(), session(tx, fs),
              mailbox(pattern, prefix),
              config(),
              currentMessage(0)
            {
                session.setRoot(new game::test::Root(game::HostVersion()));
                session.setGame(new game::Game());
            }
    };

    class TestAdaptor : public game::proxy::MailboxAdaptor {
     public:
        TestAdaptor(Environment& env)
            : m_env(env)
            { }
        virtual game::Session& session() const
            { return m_env.session; }
        virtual game::msg::Mailbox& mailbox() const
            { return m_env.mailbox; }
        virtual game::msg::Configuration* getConfiguration() const
            { return &m_env.config; }
        virtual size_t getCurrentMessage() const
            { return m_env.currentMessage; }
        virtual void setCurrentMessage(size_t n)
            { m_env.currentMessage = n; }
     private:
        Environment& m_env;
    };


    struct UpdateReceiver {
        UpdateReceiver()
            : m_index(999), m_data()
            { }

        void onUpdate(size_t index, const game::proxy::MailboxProxy::Message& d)
            { m_index = index; m_data = d; }

        size_t m_index;
        game::proxy::MailboxProxy::Message m_data;
    };

}

/** Test basic operations: getStatus(), browsing, returned attributes. */
void
TestGameProxyMailboxProxy::testIt()
{
    Environment env("x...x.x.", "");

    // Set up tasking
    // WaitIndicator's RequestDispatcher personality serves both sides
    game::test::WaitIndicator ind;
    TestAdaptor ad(env);
    util::RequestReceiver<game::proxy::MailboxAdaptor> recv(ind, ad);
    env.currentMessage = 3;

    // Testee
    game::proxy::MailboxProxy proxy(recv.getSender(), ind);

    // Verify initial status
    game::proxy::MailboxProxy::Status st;
    proxy.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.numMessages, 8U);
    TS_ASSERT_EQUALS(st.currentMessage, 3U);

    // Retrieve message
    UpdateReceiver u;
    proxy.sig_update.add(&u, &UpdateReceiver::onUpdate);
    proxy.setCurrentMessage(4);
    ind.processQueue();

    TS_ASSERT_EQUALS(u.m_index, 4U);
    TS_ASSERT_EQUALS(u.m_data.text.getText(), "text-4");
    TS_ASSERT_EQUALS(u.m_data.isFiltered, true);

    // Browsing
    proxy.browse(game::msg::Browser::Last,     0, false);
    proxy.browse(game::msg::Browser::Previous, 1, false);
    ind.processQueue();

    TS_ASSERT_EQUALS(u.m_index, 5U);
    TS_ASSERT_EQUALS(u.m_data.text.getText(), "text-5");
    TS_ASSERT_EQUALS(u.m_data.isFiltered, false);
}

/** Test getSummary(). */
void
TestGameProxyMailboxProxy::testSummary()
{
    Environment env(".........."
                    ".........."
                    "xx", "");

    // Set up tasking
    game::test::WaitIndicator ind;
    TestAdaptor ad(env);
    util::RequestReceiver<game::proxy::MailboxAdaptor> recv(ind, ad);
    env.currentMessage = 12;

    // Testee
    game::proxy::MailboxProxy proxy(recv.getSender(), ind);

    // Fetch summary
    game::msg::Browser::Summary_t sum;
    size_t index = 0;
    proxy.getSummary(ind, sum, index);

    // Verify
    TS_ASSERT_EQUALS(index, 1U);
    TS_ASSERT_EQUALS(sum.size(), 3U);
    TS_ASSERT_EQUALS(sum[0].heading, "head-0");
    TS_ASSERT_EQUALS(sum[0].isFiltered, false);
    TS_ASSERT_EQUALS(sum[1].heading, "head-1");
    TS_ASSERT_EQUALS(sum[1].isFiltered, false);
    TS_ASSERT_EQUALS(sum[2].heading, "head-2");
    TS_ASSERT_EQUALS(sum[2].isFiltered, true);
}

/** Test toggleHeadingFiltered(). */
void
TestGameProxyMailboxProxy::testToggleFiltered()
{
    Environment env(".....", "");

    // Set up tasking
    game::test::WaitIndicator ind;
    TestAdaptor ad(env);
    util::RequestReceiver<game::proxy::MailboxAdaptor> recv(ind, ad);

    // Testee
    game::proxy::MailboxProxy proxy(recv.getSender(), ind);
    proxy.toggleHeadingFiltered("hi");
    ind.processQueue();

    // Verify
    TS_ASSERT_EQUALS(env.config.isHeadingFiltered("hi"), true);
}

/** Test performMessageAction(). */
void
TestGameProxyMailboxProxy::testAction()
{
    Environment env(".....", "");

    // Set up tasking
    game::test::WaitIndicator ind;
    TestAdaptor ad(env);
    util::RequestReceiver<game::proxy::MailboxAdaptor> recv(ind, ad);

    // Testee
    game::proxy::MailboxProxy proxy(recv.getSender(), ind);

    // Receive updates
    UpdateReceiver u;
    proxy.sig_update.add(&u, &UpdateReceiver::onUpdate);

    // Toggle message 0's Confirmed using the implemented performMessageAction.
    proxy.setCurrentMessage(0);
    proxy.performMessageAction(game::msg::Mailbox::ToggleConfirmed);
    ind.processQueue();

    // Verify
    TS_ASSERT(u.m_data.flags.contains(game::msg::Mailbox::Confirmed));
}

/** Test search. */
void
TestGameProxyMailboxProxy::testSearch()
{
    Environment env(".......", "");

    // Set up tasking
    game::test::WaitIndicator ind;
    TestAdaptor ad(env);
    util::RequestReceiver<game::proxy::MailboxAdaptor> recv(ind, ad);
    env.currentMessage = 0;

    // Testee
    game::proxy::MailboxProxy proxy(recv.getSender(), ind);

    // Search
    game::test::Counter ctr;
    UpdateReceiver u;
    proxy.sig_update.add(&u, &UpdateReceiver::onUpdate);
    proxy.sig_searchFailure.add(&ctr, &game::test::Counter::increment);
    proxy.search(game::msg::Browser::Next, 1, false, "text-3");
    ind.processQueue();

    TS_ASSERT_EQUALS(u.m_index, 3U);
    TS_ASSERT_EQUALS(u.m_data.text.getText(), "text-3");
    TS_ASSERT_EQUALS(u.m_data.isFiltered, false);
    TS_ASSERT_EQUALS(ctr.get(), 0);

    // Failure
    proxy.search(game::msg::Browser::Next, 1, false, "nope");
    ind.processQueue();

    TS_ASSERT_EQUALS(u.m_index, 3U);
    TS_ASSERT_EQUALS(u.m_data.text.getText(), "text-3");
    TS_ASSERT_EQUALS(u.m_data.isFiltered, false);
    TS_ASSERT_EQUALS(ctr.get(), 1);

    // Browsing must work
    proxy.browse(game::msg::Browser::Next, 1, false);
    ind.processQueue();

    TS_ASSERT_EQUALS(u.m_index, 4U);
    TS_ASSERT_EQUALS(u.m_data.text.getText(), "text-4");
    TS_ASSERT_EQUALS(u.m_data.isFiltered, false);
}

/** Test write(), single message case. */
void
TestGameProxyMailboxProxy::testWrite()
{
    Environment env(".......", "");

    // Set up tasking
    game::test::WaitIndicator ind;
    TestAdaptor ad(env);
    util::RequestReceiver<game::proxy::MailboxAdaptor> recv(ind, ad);

    // Test: write two single messages (exercises creation and append)
    game::proxy::MailboxProxy proxy(recv.getSender(), ind);
    String_t err;
    TS_ASSERT_EQUALS(proxy.write(ind, "/test.txt", 1, 2, err), true);
    TS_ASSERT_EQUALS(proxy.write(ind, "/test.txt", 3, 4, err), true);

    // Verify
    afl::base::Ref<afl::io::Stream> in(env.fs.openFile("/test.txt", afl::io::FileSystem::OpenRead));
    afl::io::TextFile tf(*in);
    String_t line;
    TS_ASSERT(tf.readLine(line)); TS_ASSERT_EQUALS(line, "=== Turn 42 ===");
    TS_ASSERT(tf.readLine(line)); TS_ASSERT_EQUALS(line, "--- Message 2 ---");
    TS_ASSERT(tf.readLine(line)); TS_ASSERT_EQUALS(line, "text-1");
    TS_ASSERT(tf.readLine(line)); TS_ASSERT_EQUALS(line, "=== Turn 42 ===");
    TS_ASSERT(tf.readLine(line)); TS_ASSERT_EQUALS(line, "--- Message 4 ---");
    TS_ASSERT(tf.readLine(line)); TS_ASSERT_EQUALS(line, "text-3");
    TS_ASSERT(!tf.readLine(line));
}

/** Test write(), multiple messages case. */
void
TestGameProxyMailboxProxy::testWriteMulti()
{
    Environment env(".......", "");

    // Set up tasking
    game::test::WaitIndicator ind;
    TestAdaptor ad(env);
    util::RequestReceiver<game::proxy::MailboxAdaptor> recv(ind, ad);

    // Test: write multiple messages in one go
    game::proxy::MailboxProxy proxy(recv.getSender(), ind);
    String_t err;
    TS_ASSERT_EQUALS(proxy.write(ind, "/test.txt", 2, 5, err), true);

    // Verify
    afl::base::Ref<afl::io::Stream> in(env.fs.openFile("/test.txt", afl::io::FileSystem::OpenRead));
    afl::io::TextFile tf(*in);
    String_t line;
    TS_ASSERT(tf.readLine(line)); TS_ASSERT_EQUALS(line, "=== Turn 42 ===");
    TS_ASSERT(tf.readLine(line)); TS_ASSERT_EQUALS(line, "   3 message(s)");
    TS_ASSERT(tf.readLine(line)); TS_ASSERT_EQUALS(line, "--- Message 3 ---");
    TS_ASSERT(tf.readLine(line)); TS_ASSERT_EQUALS(line, "text-2");
    TS_ASSERT(tf.readLine(line)); TS_ASSERT_EQUALS(line, "--- Message 4 ---");
    TS_ASSERT(tf.readLine(line)); TS_ASSERT_EQUALS(line, "text-3");
    TS_ASSERT(tf.readLine(line)); TS_ASSERT_EQUALS(line, "--- Message 5 ---");
    TS_ASSERT(tf.readLine(line)); TS_ASSERT_EQUALS(line, "text-4");
    TS_ASSERT(!tf.readLine(line));
}

/** Test write(), error case. */
void
TestGameProxyMailboxProxy::testWriteError()
{
    Environment env(".......", "");

    // Set up tasking
    game::test::WaitIndicator ind;
    TestAdaptor ad(env);
    util::RequestReceiver<game::proxy::MailboxAdaptor> recv(ind, ad);

    // Test: write to a file that cannot be accessed
    game::proxy::MailboxProxy proxy(recv.getSender(), ind);
    String_t err;
    TS_ASSERT_EQUALS(proxy.write(ind, "/bad/directory/test.txt", 2, 5, err), false);
    TS_ASSERT_DIFFERS(err, "");
}

/** Test quoteMessage(). */
void
TestGameProxyMailboxProxy::testQuote()
{
    Environment env(".......", "(-r)<<< Message >>>\nFROM: me\n");

    // Set up tasking
    game::test::WaitIndicator ind;
    TestAdaptor ad(env);
    util::RequestReceiver<game::proxy::MailboxAdaptor> recv(ind, ad);

    game::proxy::MailboxProxy proxy(recv.getSender(), ind);

    // Reply
    game::proxy::MailboxProxy::QuoteResult r = proxy.quoteMessage(ind, 3, game::proxy::MailboxProxy::QuoteForReplying);
    TS_ASSERT_EQUALS(r.text, "> text-3\n");

    // Forward
    game::proxy::MailboxProxy::QuoteResult f = proxy.quoteMessage(ind, 5, game::proxy::MailboxProxy::QuoteForForwarding);
    TS_ASSERT_EQUALS(f.text, "--- Forwarded Message ---\n(-r)<<< Message >>>\nFROM: me\ntext-5\n--- End Forwarded Message ---");
}

/** Test quoteMessage(), special cases. */
void
TestGameProxyMailboxProxy::testQuote2()
{
    Environment env(".......", "(-r)<<< Message >>>\nFROM: me\n\n  <<< Universal Message >>>\n\nfirst\n\n\nsecond\n");

    // Set up tasking
    game::test::WaitIndicator ind;
    TestAdaptor ad(env);
    util::RequestReceiver<game::proxy::MailboxAdaptor> recv(ind, ad);

    game::proxy::MailboxProxy proxy(recv.getSender(), ind);

    // Reply
    game::proxy::MailboxProxy::QuoteResult r = proxy.quoteMessage(ind, 3, game::proxy::MailboxProxy::QuoteForReplying);
    TS_ASSERT_EQUALS(r.text, "> first\n>\n> second\n> text-3\n");

    // Forward
    game::proxy::MailboxProxy::QuoteResult f = proxy.quoteMessage(ind, 5, game::proxy::MailboxProxy::QuoteForForwarding);
    TS_ASSERT_EQUALS(f.text, "--- Forwarded Message ---\n(-r)<<< Message >>>\nFROM: me\n\n  <<< Universal Message >>>\n\nfirst\n\n\nsecond\ntext-5\n--- End Forwarded Message ---");
}

/** Test receiveData(); integration test against actual Inbox. */
void
TestGameProxyMailboxProxy::testData()
{
    game::test::SessionThread t;
    t.session().setRoot(new game::test::Root(game::HostVersion()));
    t.session().setGame(new game::Game());
    t.session().getGame()->currentTurn().setTurnNumber(10);
    t.session().getGame()->currentTurn().inbox().addMessage("(-r1000)<<< Message >>>\n"
                                                            "FROM: Fed\n"
                                                            "TO: me\n"
                                                            "\n"
                                                            "<<< VPA Data Transmission >>>\n"
                                                            "\n"
                                                            "OBJECT: Mine field 61\n"
                                                            "DATA: 2094989326\n"
                                                            "ocaalekakbhadaaaijmcaaaaaaaa\n", 10);

    // Set up tasking
    game::test::WaitIndicator ind;
    game::proxy::MailboxProxy proxy(t.gameSender().makeTemporary(game::proxy::makeInboxAdaptor()), ind);

    // Verify initial status
    game::proxy::MailboxProxy::Status st;
    proxy.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.numMessages, 1U);
    TS_ASSERT_EQUALS(st.currentMessage, 0U);

    // Retrieve message
    UpdateReceiver u;
    proxy.sig_update.add(&u, &UpdateReceiver::onUpdate);
    proxy.setCurrentMessage(0);
    t.sync();
    ind.processQueue();
    TS_ASSERT_EQUALS(u.m_data.text.substr(0, 8).getText(), "(-r1000)");
    TS_ASSERT_EQUALS(u.m_data.dataStatus, game::proxy::MailboxProxy::DataReceivable);

    // Receive it
    proxy.receiveData();
    t.sync();
    ind.processQueue();
    TS_ASSERT_EQUALS(u.m_data.dataStatus, game::proxy::MailboxProxy::DataReceived);

    // Verify data actually got received
    game::map::Minefield* mf = t.session().getGame()->currentTurn().universe().minefields().get(61);
    TS_ASSERT(mf != 0);

    game::map::Point pt;
    TS_ASSERT(mf->getPosition(pt));
    TS_ASSERT_EQUALS(pt.getX(), 2635);
    TS_ASSERT_EQUALS(pt.getY(), 1818);
    int r = 0;
    TS_ASSERT(mf->getRadius(r));
    TS_ASSERT_EQUALS(r, 104);
}

