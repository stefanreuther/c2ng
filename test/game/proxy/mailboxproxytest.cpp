/**
  *  \file test/game/proxy/mailboxproxytest.cpp
  *  \brief Test for game::proxy::MailboxProxy
  */

#include "game/proxy/mailboxproxy.hpp"

#include "afl/io/internalfilesystem.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
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
        TestMailbox(afl::test::Assert a, String_t pattern, String_t prefix)
            : m_assert(a), m_pattern(pattern), m_prefix(prefix), m_flags()
            { }

        virtual size_t getNumMessages() const
            { return m_pattern.size(); }
        virtual String_t getMessageHeaderText(size_t /*index*/, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            { return String_t(); }
        virtual String_t getMessageBodyText(size_t index, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            { return afl::string::Format("%stext-%d", m_prefix, index); }
        virtual String_t getMessageForwardText(size_t index, afl::string::Translator& tx, const game::PlayerList& players) const
            { return defaultGetMessageForwardText(index, tx, players); }
        virtual String_t getMessageReplyText(size_t index, afl::string::Translator& tx, const game::PlayerList& players) const
            { return defaultGetMessageReplyText(index, tx, players); }
        virtual util::rich::Text getMessageDisplayText(size_t index, afl::string::Translator& tx, const game::PlayerList& players) const
            { return getMessageText(index, tx, players); }
        virtual String_t getMessageHeading(size_t index, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            {
                m_assert.check("getMessageHeading: valid index", index < m_pattern.size());
                return afl::string::Format("head-%d%c", index / 10, m_pattern[index]);
            }
        virtual Metadata getMessageMetadata(size_t /*index*/, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            {
                Metadata md;
                md.turnNumber = 42;
                md.flags = m_flags;
                return md;
            }
        virtual Actions_t getMessageActions(size_t /*index*/) const
            { return Actions_t(); }
        virtual void performMessageAction(size_t /*index*/, Action a)
            {
                if (a == ToggleConfirmed) {
                    m_flags ^= Confirmed;
                }
            }
        virtual void receiveMessageData(size_t /*index*/, game::parser::InformationConsumer& /*consumer*/, const game::TeamSettings& /*teamSettings*/, bool /*onRequest*/, afl::charset::Charset& /*cs*/)
            { }
     private:
        afl::test::Assert m_assert;
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

        Environment(afl::test::Assert a, String_t pattern, String_t prefix)
            : tx(), fs(), session(tx, fs),
              mailbox(a, pattern, prefix),
              config(),
              currentMessage(0)
            {
                session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
                session.setGame(new game::Game());

                // Filter "all" messages that have a 'x' in the pattern
                for (size_t i = 0; i < 100; ++i) {
                    config.setHeadingFiltered(afl::string::Format("head-%dx", i), true);
                }
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
AFL_TEST("game.proxy.MailboxProxy:normal", a)
{
    Environment env(a, "x...x.x.", "");

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
    a.checkEqual("01. numMessages", st.numMessages, 8U);
    a.checkEqual("02. currentMessage", st.currentMessage, 3U);

    // Retrieve message
    UpdateReceiver u;
    proxy.sig_update.add(&u, &UpdateReceiver::onUpdate);
    proxy.setCurrentMessage(4);
    ind.processQueue();

    a.checkEqual("11. m_index", u.m_index, 4U);
    a.checkEqual("12. text", u.m_data.text.getText(), "text-4");
    a.checkEqual("13. isFiltered", u.m_data.isFiltered, true);

    // Browsing
    proxy.browse(game::msg::Browser::Last,     0, false);
    proxy.browse(game::msg::Browser::Previous, 1, false);
    ind.processQueue();

    a.checkEqual("21. m_index", u.m_index, 5U);
    a.checkEqual("22. text", u.m_data.text.getText(), "text-5");
    a.checkEqual("23. isFiltered", u.m_data.isFiltered, false);
}

/** Test getSummary(). */
AFL_TEST("game.proxy.MailboxProxy:getSummary", a)
{
    Environment env(a,
                    ".........."
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
    a.checkEqual("01", index, 1U);
    a.checkEqual("02. size",         sum.size(), 3U);
    a.checkEqual("03. heading 0",    sum[0].heading, "head-0.");
    a.checkEqual("04. isFiltered 0", sum[0].isFiltered, false);
    a.checkEqual("05. heading 1",    sum[1].heading, "head-1.");
    a.checkEqual("06. isFiltered 1", sum[1].isFiltered, false);
    a.checkEqual("07. heading 2",    sum[2].heading, "head-2x");
    a.checkEqual("08. isFiltered 2", sum[2].isFiltered, true);
}

/** Test toggleHeadingFiltered(). */
AFL_TEST("game.proxy.MailboxProxy:toggleHeadingFiltered", a)
{
    Environment env(a, ".....", "");

    // Set up tasking
    game::test::WaitIndicator ind;
    TestAdaptor ad(env);
    util::RequestReceiver<game::proxy::MailboxAdaptor> recv(ind, ad);

    // Testee
    game::proxy::MailboxProxy proxy(recv.getSender(), ind);
    proxy.toggleHeadingFiltered("hi");
    ind.processQueue();

    // Verify
    a.checkEqual("01. isHeadingFiltered", env.config.isHeadingFiltered("hi"), true);
}

/** Test performMessageAction(). */
AFL_TEST("game.proxy.MailboxProxy:performMessageAction", a)
{
    Environment env(a, ".....", "");

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
    a.check("01. flags", u.m_data.flags.contains(game::msg::Mailbox::Confirmed));
}

/** Test search. */
AFL_TEST("game.proxy.MailboxProxy:search", a)
{
    Environment env(a, ".......", "");

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

    a.checkEqual("01. m_index", u.m_index, 3U);
    a.checkEqual("02. text", u.m_data.text.getText(), "text-3");
    a.checkEqual("03. isFiltered", u.m_data.isFiltered, false);
    a.checkEqual("04. get", ctr.get(), 0);

    // Failure
    proxy.search(game::msg::Browser::Next, 1, false, "nope");
    ind.processQueue();

    a.checkEqual("11. m_index", u.m_index, 3U);
    a.checkEqual("12. text", u.m_data.text.getText(), "text-3");
    a.checkEqual("13. isFiltered", u.m_data.isFiltered, false);
    a.checkEqual("14. get", ctr.get(), 1);

    // Browsing must work
    proxy.browse(game::msg::Browser::Next, 1, false);
    ind.processQueue();

    a.checkEqual("21. m_index", u.m_index, 4U);
    a.checkEqual("22. text", u.m_data.text.getText(), "text-4");
    a.checkEqual("23. isFiltered", u.m_data.isFiltered, false);
}

/** Test write(), single message case. */
AFL_TEST("game.proxy.MailboxProxy:write", a)
{
    Environment env(a, ".......", "");

    // Set up tasking
    game::test::WaitIndicator ind;
    TestAdaptor ad(env);
    util::RequestReceiver<game::proxy::MailboxAdaptor> recv(ind, ad);

    // Test: write two single messages (exercises creation and append)
    game::proxy::MailboxProxy proxy(recv.getSender(), ind);
    String_t err;
    a.checkEqual("01. write", proxy.write(ind, "/test.txt", 1, 2, err), true);
    a.checkEqual("02. write", proxy.write(ind, "/test.txt", 3, 4, err), true);

    // Verify
    afl::base::Ref<afl::io::Stream> in(env.fs.openFile("/test.txt", afl::io::FileSystem::OpenRead));
    afl::io::TextFile tf(*in);
    String_t line;
    a.check("11. file content", tf.readLine(line)); a.checkEqual("11", line, "=== Turn 42 ===");
    a.check("12. file content", tf.readLine(line)); a.checkEqual("12", line, "--- Message 2 ---");
    a.check("13. file content", tf.readLine(line)); a.checkEqual("13", line, "text-1");
    a.check("14. file content", tf.readLine(line)); a.checkEqual("14", line, "=== Turn 42 ===");
    a.check("15. file content", tf.readLine(line)); a.checkEqual("15", line, "--- Message 4 ---");
    a.check("16. file content", tf.readLine(line)); a.checkEqual("16", line, "text-3");
    a.check("17. file content", !tf.readLine(line));
}

/** Test write(), multiple messages case. */
AFL_TEST("game.proxy.MailboxProxy:write:multiple", a)
{
    Environment env(a, ".......", "");

    // Set up tasking
    game::test::WaitIndicator ind;
    TestAdaptor ad(env);
    util::RequestReceiver<game::proxy::MailboxAdaptor> recv(ind, ad);

    // Test: write multiple messages in one go
    game::proxy::MailboxProxy proxy(recv.getSender(), ind);
    String_t err;
    a.checkEqual("01. write", proxy.write(ind, "/test.txt", 2, 5, err), true);

    // Verify
    afl::base::Ref<afl::io::Stream> in(env.fs.openFile("/test.txt", afl::io::FileSystem::OpenRead));
    afl::io::TextFile tf(*in);
    String_t line;
    a.check("11. file content", tf.readLine(line)); a.checkEqual("11", line, "=== Turn 42 ===");
    a.check("12. file content", tf.readLine(line)); a.checkEqual("12", line, "   3 message(s)");
    a.check("13. file content", tf.readLine(line)); a.checkEqual("13", line, "--- Message 3 ---");
    a.check("14. file content", tf.readLine(line)); a.checkEqual("14", line, "text-2");
    a.check("15. file content", tf.readLine(line)); a.checkEqual("15", line, "--- Message 4 ---");
    a.check("16. file content", tf.readLine(line)); a.checkEqual("16", line, "text-3");
    a.check("17. file content", tf.readLine(line)); a.checkEqual("17", line, "--- Message 5 ---");
    a.check("18. file content", tf.readLine(line)); a.checkEqual("18", line, "text-4");
    a.check("19. file content", !tf.readLine(line));
}

/** Test write(), error case. */
AFL_TEST("game.proxy.MailboxProxy:write:error", a)
{
    Environment env(a, ".......", "");

    // Set up tasking
    game::test::WaitIndicator ind;
    TestAdaptor ad(env);
    util::RequestReceiver<game::proxy::MailboxAdaptor> recv(ind, ad);

    // Test: write to a file that cannot be accessed
    game::proxy::MailboxProxy proxy(recv.getSender(), ind);
    String_t err;
    a.checkEqual("01. write", proxy.write(ind, "/bad/directory/test.txt", 2, 5, err), false);
    a.checkDifferent("02. error", err, "");
}

/** Test quoteMessage(). */
AFL_TEST("game.proxy.MailboxProxy:quoteMessage", a)
{
    Environment env(a, ".......", "(-r)<<< Message >>>\nFROM: me\n");

    // Set up tasking
    game::test::WaitIndicator ind;
    TestAdaptor ad(env);
    util::RequestReceiver<game::proxy::MailboxAdaptor> recv(ind, ad);

    game::proxy::MailboxProxy proxy(recv.getSender(), ind);

    // Reply
    game::proxy::MailboxProxy::QuoteResult r = proxy.quoteMessage(ind, 3, game::proxy::MailboxProxy::QuoteForReplying);
    a.checkEqual("01. text", r.text, "> text-3\n");

    // Forward
    game::proxy::MailboxProxy::QuoteResult f = proxy.quoteMessage(ind, 5, game::proxy::MailboxProxy::QuoteForForwarding);
    a.checkEqual("11. text", f.text, "--- Forwarded Message ---\n(-r)<<< Message >>>\nFROM: me\ntext-5\n--- End Forwarded Message ---");
}

/** Test quoteMessage(), special cases. */
AFL_TEST("game.proxy.MailboxProxy:quoteMessage:special-cases", a)
{
    Environment env(a, ".......", "(-r)<<< Message >>>\nFROM: me\n\n  <<< Universal Message >>>\n\nfirst\n\n\nsecond\n");

    // Set up tasking
    game::test::WaitIndicator ind;
    TestAdaptor ad(env);
    util::RequestReceiver<game::proxy::MailboxAdaptor> recv(ind, ad);

    game::proxy::MailboxProxy proxy(recv.getSender(), ind);

    // Reply
    game::proxy::MailboxProxy::QuoteResult r = proxy.quoteMessage(ind, 3, game::proxy::MailboxProxy::QuoteForReplying);
    a.checkEqual("01. text", r.text, "> first\n>\n> second\n> text-3\n");

    // Forward
    game::proxy::MailboxProxy::QuoteResult f = proxy.quoteMessage(ind, 5, game::proxy::MailboxProxy::QuoteForForwarding);
    a.checkEqual("11. text", f.text, "--- Forwarded Message ---\n(-r)<<< Message >>>\nFROM: me\n\n  <<< Universal Message >>>\n\nfirst\n\n\nsecond\ntext-5\n--- End Forwarded Message ---");
}

/** Test receiveData(); integration test against actual Inbox. */
AFL_TEST("game.proxy.MailboxProxy:receiveData", a)
{
    game::test::SessionThread t;
    t.session().setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
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

    // Scan message
    {
        class Consumer : public game::parser::InformationConsumer {
         public:
            virtual void addMessageInformation(const game::parser::MessageInformation&)
                { }
        };
        Consumer c;
        t.session().getGame()->currentTurn().inbox().receiveMessageData(0, c, t.session().getGame()->teamSettings(), false, t.session().getRoot()->charset());
    }

    // Set up tasking
    game::test::WaitIndicator ind;
    game::proxy::MailboxProxy proxy(t.gameSender().makeTemporary(game::proxy::makeInboxAdaptor()), ind);

    // Verify initial status
    game::proxy::MailboxProxy::Status st;
    proxy.getStatus(ind, st);
    a.checkEqual("01. numMessages", st.numMessages, 1U);
    a.checkEqual("02. currentMessage", st.currentMessage, 0U);

    // Retrieve message
    UpdateReceiver u;
    proxy.sig_update.add(&u, &UpdateReceiver::onUpdate);
    proxy.setCurrentMessage(0);
    t.sync();
    ind.processQueue();
    a.checkEqual("11. text", u.m_data.text.substr(0, 8).getText(), "(-r1000)");
    a.checkEqual("12. dataStatus", u.m_data.dataStatus, game::msg::Mailbox::DataReceivable);

    // Receive it
    proxy.receiveData();
    t.sync();
    ind.processQueue();
    a.checkEqual("21. dataStatus", u.m_data.dataStatus, game::msg::Mailbox::DataReceived);

    // Verify data actually got received
    game::map::Minefield* mf = t.session().getGame()->currentTurn().universe().minefields().get(61);
    a.checkNonNull("31. minefield", mf);

    game::map::Point pt;
    a.check("41. getPosition", mf->getPosition().get(pt));
    a.checkEqual("42. getX", pt.getX(), 2635);
    a.checkEqual("43. getY", pt.getY(), 1818);
    int r = 0;
    a.check("44. getRadius", mf->getRadius().get(r));
    a.checkEqual("45. radius", r, 104);
}
