/**
  *  \file u/t_game_msg_browser.cpp
  *  \brief Test for game::msg::Browser
  */

#include "game/msg/browser.hpp"

#include "t_game_msg.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/msg/configuration.hpp"
#include "game/msg/mailbox.hpp"
#include "game/playerlist.hpp"

namespace {
    class TestMailbox : public game::msg::Mailbox {
     public:
        TestMailbox(String_t pattern)
            : m_pattern(pattern)
            { }

        virtual size_t getNumMessages() const
            { return m_pattern.size(); }
        virtual String_t getMessageHeaderText(size_t /*index*/, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            { return String_t(); }
        virtual String_t getMessageBodyText(size_t index, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            { return afl::string::Format("text-%d", index); }
        virtual String_t getMessageForwardText(size_t index, afl::string::Translator& tx, const game::PlayerList& players) const
            { return defaultGetMessageForwardText(index, tx, players); }
        virtual String_t getMessageReplyText(size_t index, afl::string::Translator& tx, const game::PlayerList& players) const
            { return defaultGetMessageReplyText(index, tx, players); }
        virtual util::rich::Text getMessageDisplayText(size_t index, afl::string::Translator& tx, const game::PlayerList& players) const
            { return getMessageText(index, tx, players); }
        virtual String_t getMessageHeading(size_t index, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            {
                TS_ASSERT(index < m_pattern.size());
                return afl::string::Format("head-%d%c", index / 10, m_pattern[index]);
            }
        virtual Metadata getMessageMetadata(size_t /*index*/, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            { return Metadata(); }
        virtual Actions_t getMessageActions(size_t /*index*/) const
            { return Actions_t(); }
        virtual void performMessageAction(size_t /*index*/, Action /*a*/)
            { }
        virtual void receiveMessageData(size_t /*index*/, game::parser::InformationConsumer& /*consumer*/, const game::TeamSettings& /*teamSettings*/, bool /*onRequest*/, afl::charset::Charset& /*cs*/)
            { }
     private:
        String_t m_pattern;
    };

    struct Environment {
        afl::string::NullTranslator tx;
        game::PlayerList players;
        game::msg::Configuration config;

        Environment()
            {
                // Filter "all" messages that have a 'x' in the pattern
                for (size_t i = 0; i < 100; ++i) {
                    config.setHeadingFiltered(afl::string::Format("head-%dx", i), true);
                }
            }
    };
}

/** Test behaviour on empty mailbox.
    Browsing functions must return 0 (although that is not a valid index), summary must be empty. */
void
TestGameMsgBrowser::testEmpty()
{
    Environment env;
    TestMailbox mbox("");

    game::msg::Browser testee(mbox, env.tx, env.players, &env.config);

    TS_ASSERT_EQUALS(testee.findFirstMessage(), 0U);
    TS_ASSERT_EQUALS(testee.findLastMessage(), 0U);

    TS_ASSERT_EQUALS(testee.browse(0, game::msg::Browser::First, 0), 0U);
    TS_ASSERT_EQUALS(testee.browse(0, game::msg::Browser::Last, 0), 0U);
    TS_ASSERT_EQUALS(testee.browse(0, game::msg::Browser::Next, 0), 0U);
    TS_ASSERT_EQUALS(testee.browse(0, game::msg::Browser::Previous, 0), 0U);

    game::msg::Browser::Summary_t sum;
    testee.buildSummary(sum);
    TS_ASSERT_EQUALS(sum.size(), 0U);
}

/** Test normal browsing behaviour. */
void
TestGameMsgBrowser::testNormal()
{
    Environment env;
    TestMailbox mbox("..xx...x");

    game::msg::Browser testee(mbox, env.tx, env.players, &env.config);

    // isMessageFiltered
    TS_ASSERT_EQUALS(testee.isMessageFiltered(0), false);
    TS_ASSERT_EQUALS(testee.isMessageFiltered(1), false);
    TS_ASSERT_EQUALS(testee.isMessageFiltered(2), true);
    TS_ASSERT_EQUALS(testee.isMessageFiltered(7), true);

    // findFirstMessage, findLastMessage
    TS_ASSERT_EQUALS(testee.findFirstMessage(), 0U);
    TS_ASSERT_EQUALS(testee.findLastMessage(), 6U);

    // browse first/last
    TS_ASSERT_EQUALS(testee.browse(0, game::msg::Browser::First, 0), 0U);
    TS_ASSERT_EQUALS(testee.browse(0, game::msg::Browser::Last, 0), 6U);

    // Forward from 0, skipping over filtered
    TS_ASSERT_EQUALS(testee.browse(0, game::msg::Browser::Next, 0), 1U);
    TS_ASSERT_EQUALS(testee.browse(0, game::msg::Browser::Next, 1), 1U);
    TS_ASSERT_EQUALS(testee.browse(0, game::msg::Browser::Next, 2), 4U);
    TS_ASSERT_EQUALS(testee.browse(0, game::msg::Browser::Next, 10), 6U);

    // Forward from non-0
    TS_ASSERT_EQUALS(testee.browse(2, game::msg::Browser::Next, 0), 4U);

    // Backward
    TS_ASSERT_EQUALS(testee.browse(7, game::msg::Browser::Previous, 0), 6U);
    TS_ASSERT_EQUALS(testee.browse(7, game::msg::Browser::Previous, 1), 6U);
    TS_ASSERT_EQUALS(testee.browse(7, game::msg::Browser::Previous, 2), 5U);
    TS_ASSERT_EQUALS(testee.browse(7, game::msg::Browser::Previous, 4), 1U);

    TS_ASSERT_EQUALS(testee.browse(4, game::msg::Browser::Previous, 0), 1U);
}

/** Test normal behaviour with "no filter" option.
    Messages that report being filtered must be handled normally. */
void
TestGameMsgBrowser::testUnfiltered()
{
    Environment env;
    TestMailbox mbox("..xx...x");

    game::msg::Browser testee(mbox, env.tx, env.players, 0 /* no filter */);

    // isMessageFiltered: always false
    TS_ASSERT_EQUALS(testee.isMessageFiltered(0), false);
    TS_ASSERT_EQUALS(testee.isMessageFiltered(1), false);
    TS_ASSERT_EQUALS(testee.isMessageFiltered(2), false);
    TS_ASSERT_EQUALS(testee.isMessageFiltered(7), false);

    // findFirstMessage, findLastMessage
    TS_ASSERT_EQUALS(testee.findFirstMessage(), 0U);
    TS_ASSERT_EQUALS(testee.findLastMessage(), 7U);

    // browse
    TS_ASSERT_EQUALS(testee.browse(0, game::msg::Browser::First, 0), 0U);
    TS_ASSERT_EQUALS(testee.browse(0, game::msg::Browser::Last, 0), 7U);
    TS_ASSERT_EQUALS(testee.browse(0, game::msg::Browser::Next, 2), 2U);
    TS_ASSERT_EQUALS(testee.browse(7, game::msg::Browser::Previous, 4), 3U);
}

/** Test behaviour with all messages filtered.
    findFirstMessage/findLastMessage must report first or last total because there is no unfiltered message. */
void
TestGameMsgBrowser::testAllFiltered()
{
    Environment env;
    TestMailbox mbox("xxx");

    game::msg::Browser testee(mbox, env.tx, env.players, &env.config);

    // isMessageFiltered
    TS_ASSERT_EQUALS(testee.isMessageFiltered(0), true);
    TS_ASSERT_EQUALS(testee.isMessageFiltered(1), true);
    TS_ASSERT_EQUALS(testee.isMessageFiltered(2), true);

    // findFirstMessage, findLastMessage
    TS_ASSERT_EQUALS(testee.findFirstMessage(), 0U);
    TS_ASSERT_EQUALS(testee.findLastMessage(), 2U);

    // browse first/last
    TS_ASSERT_EQUALS(testee.browse(0, game::msg::Browser::First, 0), 0U);
    TS_ASSERT_EQUALS(testee.browse(0, game::msg::Browser::Last, 0), 2U);

    // browse next/previous will not advance
    TS_ASSERT_EQUALS(testee.browse(0, game::msg::Browser::Next, 1), 0U);
    TS_ASSERT_EQUALS(testee.browse(2, game::msg::Browser::Previous, 1), 2U);
}

/** Test buildSummary(). */
void
TestGameMsgBrowser::testSummary()
{
    Environment env;
    TestMailbox mbox(".........."
                     "xxxxxxxxxx"
                     ".........."
                     ".........."
                     "xxxxxxx");

    game::msg::Browser testee(mbox, env.tx, env.players, &env.config);

    game::msg::Browser::Summary_t sum;
    testee.buildSummary(sum);

    TS_ASSERT_EQUALS(sum.size(), 5U);
    TS_ASSERT_EQUALS(sum[0].index, 0U);
    TS_ASSERT_EQUALS(sum[0].count, 10U);
    TS_ASSERT_EQUALS(sum[0].isFiltered, false);
    TS_ASSERT_EQUALS(sum[0].heading, "head-0.");

    TS_ASSERT_EQUALS(sum[1].index, 10U);
    TS_ASSERT_EQUALS(sum[1].count, 10U);
    TS_ASSERT_EQUALS(sum[1].isFiltered, true);
    TS_ASSERT_EQUALS(sum[1].heading, "head-1x");

    TS_ASSERT_EQUALS(sum[4].index, 40U);
    TS_ASSERT_EQUALS(sum[4].count, 7U);
    TS_ASSERT_EQUALS(sum[4].isFiltered, true);
    TS_ASSERT_EQUALS(sum[4].heading, "head-4x");
}

/** Test search(). */
void
TestGameMsgBrowser::testSearch()
{
    Environment env;
    TestMailbox mbox("..xx...x");

    game::msg::Browser testee(mbox, env.tx, env.players, &env.config);

    // Finding message 4
    // - normally
    TS_ASSERT_EQUALS(testee.search(0, game::msg::Browser::Next,     1, "text-4").index, 4U);
    TS_ASSERT_EQUALS(testee.search(0, game::msg::Browser::Next,     1, "text-4").found, true);

    // - case-blind
    TS_ASSERT_EQUALS(testee.search(0, game::msg::Browser::Next,     1, "TEXT-4").index, 4U);

    // - from end
    TS_ASSERT_EQUALS(testee.search(7, game::msg::Browser::Previous, 1, "text-4").index, 4U);

    // - first, last
    TS_ASSERT_EQUALS(testee.search(0, game::msg::Browser::First,    1, "text-4").index, 4U);
    TS_ASSERT_EQUALS(testee.search(0, game::msg::Browser::Last,     1, "text-4").index, 4U);

    // - with repeat (will settle at first found)
    TS_ASSERT_EQUALS(testee.search(0, game::msg::Browser::Next,     2, "text-4").index, 4U);

    // Repeat case (will find 4,5)
    TS_ASSERT_EQUALS(testee.search(1, game::msg::Browser::Next,     2, "text").index, 5U);

    // Failure case
    TS_ASSERT_EQUALS(testee.search(5, game::msg::Browser::Next,     1, "notfound").index, 5U);
    TS_ASSERT_EQUALS(testee.search(5, game::msg::Browser::Previous, 1, "notfound").index, 5U);
    TS_ASSERT_EQUALS(testee.search(5, game::msg::Browser::First,    1, "notfound").index, 0U);
    TS_ASSERT_EQUALS(testee.search(5, game::msg::Browser::Last,     1, "notfound").index, 7U);

    TS_ASSERT_EQUALS(testee.search(5, game::msg::Browser::Next,     1, "notfound").found, false);

    // Filtered case: not found because it's filtered
    TS_ASSERT_EQUALS(testee.search(0, game::msg::Browser::Next,     1, "text-2").index, 0U);
    TS_ASSERT_EQUALS(testee.search(0, game::msg::Browser::Next,     1, "text-2").found, false);
}

