/**
  *  \file u/t_game_proxy_mailboxproxy.cpp
  *  \brief Test for game::proxy::MailboxProxy
  */

#include "game/proxy/mailboxproxy.hpp"

#include "t_game_proxy.hpp"
#include "afl/string/format.hpp"
#include "game/test/root.hpp"
#include "game/game.hpp"
#include "game/test/waitindicator.hpp"
#include "util/requestreceiver.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/io/nullfilesystem.hpp"

namespace {
    /*
     *  Mailbox for testing
     *
     *  Takes a string to determine filtered messages.
     *  Messages contain 'text-XX' as text, and 'head-X' (groups of 10) as header.
     */
    class TestMailbox : public game::msg::Mailbox {
     public:
        TestMailbox(String_t pattern)
            : m_pattern(pattern), m_flags()
            { }

        virtual size_t getNumMessages() const
            { return m_pattern.size(); }
        virtual String_t getMessageText(size_t index, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            { return afl::string::Format("text-%d", index); }
        virtual String_t getMessageHeading(size_t index, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            { return afl::string::Format("head-%d", index / 10); }
        virtual int getMessageTurnNumber(size_t /*index*/) const
            { return 0; }
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
        Flags_t m_flags;
    };

    /*
     *  Environment
     */

    struct Environment {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session;
        TestMailbox mailbox;
        game::msg::Configuration config;
        size_t currentMessage;

        Environment(String_t pattern)
            : tx(), fs(), session(tx, fs),
              mailbox(pattern),
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
    Environment env("x...x.x.");

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
                    "xx");
    
    // Set up tasking
    // WaitIndicator's RequestDispatcher personality serves both sides
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
    Environment env(".....");

    // Set up tasking
    // WaitIndicator's RequestDispatcher personality serves both sides
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
    Environment env(".....");

    // Set up tasking
    // WaitIndicator's RequestDispatcher personality serves both sides
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

