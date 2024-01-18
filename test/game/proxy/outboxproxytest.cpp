/**
  *  \file test/game/proxy/outboxproxytest.cpp
  *  \brief Test for game::proxy::OutboxProxy
  */

#include "game/proxy/outboxproxy.hpp"

#include <stdexcept>
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/io/textfile.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/msg/outbox.hpp"
#include "game/proxy/mailboxproxy.hpp"
#include "game/root.hpp"
#include "game/stringverifier.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/specificationloader.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"

/** Test behaviour on empty session. */
AFL_TEST("game.proxy.OutboxProxy:empty", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::OutboxProxy testee(t.gameSender());
    game::proxy::OutboxProxy::Info info;

    // Defaults
    a.checkEqual("01. getHeadersForDisplay", testee.getHeadersForDisplay(ind, 1, game::PlayerSet_t(2)), "");
    a.checkEqual("02. getMessage", testee.getMessage(ind, 1, info), false);

    std::auto_ptr<game::StringVerifier> v(testee.createStringVerifier(ind));
    a.checkNull("11. createStringVerifier", v.get());

    // Doesn't crash
    testee.addMessage(1, "x", game::PlayerSet_t(2));
    testee.setMessageText(3, "foo");
    testee.setMessageReceivers(4, game::PlayerSet_t(5));
    testee.deleteMessage(6);

    t.sync();
    ind.processQueue();
}

/** Test behaviour on normal populated session. */
AFL_TEST("game.proxy.OutboxProxy:normal", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::OutboxProxy testee(t.gameSender());

    // Prepare content
    // - players must be defined to determine what a universal message is (getHeadersForDisplay())
    t.session().setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    for (int i = 1; i <= 11; ++i) {
        t.session().getRoot()->playerList().create(i);
    }
    // - preload some messages
    t.session().setGame(new game::Game());
    game::msg::Outbox& mbx = t.session().getGame()->currentTurn().outbox();
    game::Id_t id1 = mbx.addMessage(1, "first", game::PlayerSet_t(10));
    game::Id_t id2 = mbx.addMessage(1, "second", game::PlayerSet_t(11));
    game::Id_t id3 = mbx.addMessage(3, "third", game::PlayerSet_t(5));
    a.checkEqual("01. getNumMessages", mbx.getNumMessages(), 3U);

    // Check getHeadersForDisplay
    a.checkEqual("11. getHeadersForDisplay", testee.getHeadersForDisplay(ind, 1, game::PlayerSet_t(2)),
                 "<<< Sub Space Message >>>\n"
                 "FROM: Player 1\n"
                 "TO: Player 2\n");

    // Check createStringVerifier
    std::auto_ptr<game::StringVerifier> v(testee.createStringVerifier(ind));
    a.checkNonNull("21. createStringVerifier", v.get());

    // Check getMessage - error case using a guaranteed-nonexistant Id
    {
        game::proxy::OutboxProxy::Info info;
        a.checkEqual("31. getMessage", testee.getMessage(ind, (id1|id2|id3)+1, info), false);
    }

    // Check getMessage - success case
    {
        game::proxy::OutboxProxy::Info info;
        a.checkEqual("41. getMessage", testee.getMessage(ind, id2, info), true);
        a.checkEqual("42. receivers", info.receivers, game::PlayerSet_t(11));
        a.checkEqual("43. text", info.text, "second");
        a.checkEqual("44. sender", info.sender, 1);
    }

    // Add messages, verify result
    testee.addMessage(5, "four", game::PlayerSet_t(4));
    testee.addMessage(5, "five", game::PlayerSet_t(6));
    t.sync();
    a.checkEqual("51. getNumMessages", mbx.getNumMessages(), 5U);

    // Modify, verify result
    testee.setMessageText(id2, "modified");
    testee.setMessageReceivers(id2, game::PlayerSet_t(9));
    t.sync();
    a.checkEqual("61. getMessageRawText", mbx.getMessageRawText(1), "modified");
    a.checkEqual("62. getMessageReceivers", mbx.getMessageReceivers(1), game::PlayerSet_t(9));

    // Delete
    testee.deleteMessage(id3);
    t.sync();
    a.checkEqual("71. getNumMessages", mbx.getNumMessages(), 4U);
    a.checkEqual("72. getMessageRawText", mbx.getMessageRawText(2), "four");
}

/** Test getMailboxAdaptor(). */
AFL_TEST("game.proxy.OutboxProxy:getMailboxAdaptor", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::OutboxProxy testee(t.gameSender());

    // Prepare content
    // - players must be defined to determine what a universal message is (getHeadersForDisplay())
    t.session().setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    for (int i = 1; i <= 11; ++i) {
        t.session().getRoot()->playerList().create(i);
    }
    // - preload some messages
    t.session().setGame(new game::Game());
    game::msg::Outbox& mbx = t.session().getGame()->currentTurn().outbox();
    mbx.addMessage(1, "first", game::PlayerSet_t(10));
    mbx.addMessage(1, "second", game::PlayerSet_t(11));
    mbx.addMessage(3, "third", game::PlayerSet_t(5));
    a.checkEqual("01. getNumMessages", mbx.getNumMessages(), 3U);

    // Create adaptor
    util::RequestSender<game::proxy::MailboxAdaptor> ad(testee.getMailboxAdaptor());
    class Task : public util::Request<game::proxy::MailboxAdaptor> {
     public:
        Task(afl::test::Assert a)
            : m_assert(a)
            { }
        void handle(game::proxy::MailboxAdaptor& ad)
            {
                // Objects must be present
                AFL_CHECK_SUCCEEDS(m_assert("11. session"), ad.session());
                AFL_CHECK_SUCCEEDS(m_assert("12. mailbox"), ad.mailbox());

                // Object content
                m_assert.checkEqual("21. getNumMessages", ad.mailbox().getNumMessages(), 3U);

                // Message storage
                AFL_CHECK_SUCCEEDS(m_assert("31"), ad.setCurrentMessage(2));
                m_assert.checkEqual("32. getCurrentMessage", ad.getCurrentMessage(), 2U);
            }
     private:
        afl::test::Assert m_assert;
    };
    ad.postNewRequest(new Task(a));
    t.sync();
}

/** Test cooperation of getMailboxAdaptor(), MailboxProxy. */
AFL_TEST("game.proxy.OutboxProxy:MailboxProxy", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::OutboxProxy testee(t.gameSender());

    // Prepare content
    // - players must be defined to determine what a universal message is (getHeadersForDisplay())
    t.session().setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    for (int i = 1; i <= 11; ++i) {
        t.session().getRoot()->playerList().create(i);
    }
    // - preload some messages
    t.session().setGame(new game::Game());
    game::msg::Outbox& mbx = t.session().getGame()->currentTurn().outbox();
    mbx.addMessage(1, "first",  game::PlayerSet_t(10));
    mbx.addMessage(1, "second", game::PlayerSet_t(10));
    mbx.addMessage(3, "third",  game::PlayerSet_t(5));
    a.checkEqual("01. getNumMessages", mbx.getNumMessages(), 3U);

    // Create MailboxProxy
    game::proxy::MailboxProxy proxy(testee.getMailboxAdaptor(), ind);

    // Get summary (for simplicity, use a synchronous call)
    game::msg::Browser::Summary_t summary;
    size_t index;
    proxy.getSummary(ind, summary, index);

    a.checkEqual("11. index", index, 0U);
    a.checkEqual("12. size",    summary.size(), 2U);
    a.checkEqual("13. index",   summary[0].index, 0U);
    a.checkEqual("14. count",   summary[0].count, 2U);
    a.checkEqual("15. heading", summary[0].heading, "To: Player 10");
    a.checkEqual("16. index",   summary[1].index, 2U);
    a.checkEqual("17. count",   summary[1].count, 1U);
    a.checkEqual("18. heading", summary[1].heading, "To: Player 5");
}

/** Test addMessageToFile, empty session (tests the fallback cases). */
AFL_TEST("game.proxy.OutboxProxy:addMessageToFile:empty", a)
{
    afl::io::InternalFileSystem fs;
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    game::proxy::OutboxProxy testee(t.gameSender());

    // Save messages
    String_t err;
    a.check("01. addMessageToFile", testee.addMessageToFile(ind, 3, "t1\n", "/file.txt", err));
    a.check("02. addMessageToFile", testee.addMessageToFile(ind, 4, "t2\n  \n", "/file.txt", err));

    // Verify result
    afl::base::Ref<afl::io::Stream> in(fs.openFile("/file.txt", afl::io::FileSystem::OpenRead));
    afl::io::TextFile tf(*in);
    String_t s;
    a.check("11. readLine", tf.readLine(s)); a.checkEqual("11", s, "--- Message ---");
    a.check("12. readLine", tf.readLine(s)); a.checkEqual("12", s, "(-r3000)<<< Data Transmission >>>");
    a.check("13. readLine", tf.readLine(s)); a.checkEqual("13", s, "t1");
    a.check("14. readLine", tf.readLine(s)); a.checkEqual("14", s, "--- Message ---");
    a.check("15. readLine", tf.readLine(s)); a.checkEqual("15", s, "(-r4000)<<< Data Transmission >>>");
    a.check("16. readLine", tf.readLine(s)); a.checkEqual("16", s, "t2");
}

/** Test addMessageToFile, normal case. */
AFL_TEST("game.proxy.OutboxProxy:addMessageToFile:normal", a)
{
    afl::io::InternalFileSystem fs;
    game::test::SessionThread t(fs);

    // Define a root and game
    t.session().setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    for (int i = 1; i <= 11; ++i) {
        t.session().getRoot()->playerList().create(i);
    }
    t.session().getRoot()->playerList().get(3)->setName(game::Player::LongName, "Trinity");
    t.session().getRoot()->playerList().get(4)->setName(game::Player::LongName, "Quattro");
    t.session().setGame(new game::Game());
    t.session().getGame()->currentTurn().setTurnNumber(42);

    game::test::WaitIndicator ind;
    game::proxy::OutboxProxy testee(t.gameSender());

    // Save messages
    String_t err;
    a.check("01. addMessageToFile", testee.addMessageToFile(ind, 3, "t1\n", "/file.txt", err));
    a.check("02. addMessageToFile", testee.addMessageToFile(ind, 4, "t2\n  \n", "/file.txt", err));

    // Verify result
    afl::base::Ref<afl::io::Stream> in(fs.openFile("/file.txt", afl::io::FileSystem::OpenRead));
    afl::io::TextFile tf(*in);
    String_t s;
    a.check("11. readLine", tf.readLine(s)); a.checkEqual("11", s, "--- Message ---");
    a.check("12. readLine", tf.readLine(s)); a.checkEqual("12", s, "(-r3000)<<< Data Transmission >>>");
    a.check("13. readLine", tf.readLine(s)); a.checkEqual("13", s, "FROM: Trinity");
    a.check("14. readLine", tf.readLine(s)); a.checkEqual("14", s, "TURN: 42");
    a.check("15. readLine", tf.readLine(s)); a.checkEqual("15", s, "t1");
    a.check("16. readLine", tf.readLine(s)); a.checkEqual("16", s, "--- Message ---");
    a.check("17. readLine", tf.readLine(s)); a.checkEqual("17", s, "(-r4000)<<< Data Transmission >>>");
    a.check("18. readLine", tf.readLine(s)); a.checkEqual("18", s, "FROM: Quattro");
    a.check("19. readLine", tf.readLine(s)); a.checkEqual("19", s, "TURN: 42");
    a.check("20. readLine", tf.readLine(s)); a.checkEqual("20", s, "t2");
}

/** Test addMessageToFile, error case. */
AFL_TEST("game.proxy.OutboxProxy:addMessageToFile:error", a)
{
    afl::io::InternalFileSystem fs;
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    game::proxy::OutboxProxy testee(t.gameSender());

    // Save to uncreatible file
    String_t err;
    a.check("01. addMessageToFile", !testee.addMessageToFile(ind, 3, "t1\n", "/nonex/file.txt", err));
    a.checkDifferent("02. error", err, "");
}

/** Test loadMessageTextFromFile(), success case. */
AFL_TEST("game.proxy.OutboxProxy:loadMessageTextFromFile", a)
{
    afl::io::InternalFileSystem fs;
    fs.openFile("/file", afl::io::FileSystem::Create)
        ->fullWrite(afl::string::toBytes("TURN: 30\n"
                                         "FROM: Me\n"
                                         "TO: You\n"
                                         "\n"
                                         "Hi there\n"));
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    game::proxy::OutboxProxy testee(t.gameSender());

    String_t text;
    String_t error;
    bool ok = testee.loadMessageTextFromFile(ind, text, "/file", error);
    a.check("01. loadMessageTextFromFile", ok);
    a.checkEqual("02. text", text, "Hi there");
}

/** Test loadMessageTextFromFile(), failure case. */
AFL_TEST("game.proxy.OutboxProxy:loadMessageTextFromFile:error", a)
{
    afl::io::InternalFileSystem fs;
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    game::proxy::OutboxProxy testee(t.gameSender());

    String_t text;
    String_t error;
    bool ok = testee.loadMessageTextFromFile(ind, text, "/file", error);
    a.check("01. loadMessageTextFromFile", !ok);
    a.checkDifferent("02. error", error, "");
}

/** Test loadMessageTextFromFile(), fully-populated root case.
    Exercises usage of game charset and StringVerifier. */
AFL_TEST("game.proxy.OutboxProxy:loadMessageTextFromFile:full", a)
{
    // String verifier for testing; accepts only lower-case and unicode
    class TestSV : public game::StringVerifier {
     public:
        virtual bool isValidString(Context /*ctx*/, const String_t& /*text*/) const
            {
                throw std::runtime_error("isValidString unexpected");
            }
        virtual bool isValidCharacter(Context ctx, afl::charset::Unichar_t ch) const
            {
                if (ctx != Message) {
                    throw std::runtime_error("isValidCharacter unexpected context");
                }
                return (ch >= 'a' && ch <= 'z') || ch >= 0x80;
            }
        virtual size_t getMaxStringLength(Context /*ctx*/) const
            {
                throw std::runtime_error("getMaxStringLength unexpected");
                return 0;
            }
        virtual TestSV* clone() const
            { return new TestSV(); }
    };

    afl::io::InternalFileSystem fs;
    fs.openFile("/file", afl::io::FileSystem::Create)
        ->fullWrite(afl::string::toBytes("TURN: 30\n"
                                         "FROM: Me\n"
                                         "TO: You\n"
                                         "\n"
                                         "Hi there\n"
                                         "G\x94od d\x84y\n"));
    game::test::SessionThread t(fs);
    t.session().setRoot(new game::Root(afl::io::InternalDirectory::create("<empty>"),
                                       *new game::test::SpecificationLoader(),
                                       game::HostVersion(),
                                       std::auto_ptr<game::RegistrationKey>(new game::test::RegistrationKey(game::RegistrationKey::Registered, 10)),
                                       std::auto_ptr<game::StringVerifier>(new TestSV()),
                                       std::auto_ptr<afl::charset::Charset>(new afl::charset::CodepageCharset(afl::charset::g_codepage437)),
                                       game::Root::Actions_t()));

    game::test::WaitIndicator ind;
    game::proxy::OutboxProxy testee(t.gameSender());

    String_t text;
    String_t error;
    bool ok = testee.loadMessageTextFromFile(ind, text, "/file", error);
    a.check("11. loadMessageTextFromFile", ok);

    // Capitals and spaces are eaten by StringVerifier; \n would be eaten as well but is passed through.
    // Non-ASCII is converted to UTF-8 by game charset and passed through by StringVerifier.
    a.checkEqual("21. text", text, "ithere\n\xC3\xB6odd\xC3\xA4y");
}
