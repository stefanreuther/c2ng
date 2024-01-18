/**
  *  \file test/game/msg/browsertest.cpp
  *  \brief Test for game::msg::Browser
  */

#include "game/msg/browser.hpp"

#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/msg/configuration.hpp"
#include "game/msg/mailbox.hpp"
#include "game/playerlist.hpp"

namespace {
    class TestMailbox : public game::msg::Mailbox {
     public:
        TestMailbox(afl::test::Assert a, String_t pattern)
            : m_assert(a), m_pattern(pattern)
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
                m_assert.check("getMessageHeading", index < m_pattern.size());
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
        afl::test::Assert m_assert;
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
AFL_TEST("game.msg.Browser:empty", a)
{
    Environment env;
    TestMailbox mbox(a, "");

    game::msg::Browser testee(mbox, env.tx, env.players, &env.config);

    a.checkEqual("01. findFirstMessage", testee.findFirstMessage(), 0U);
    a.checkEqual("02. findLastMessage", testee.findLastMessage(), 0U);

    a.checkEqual("11. browse", testee.browse(0, game::msg::Browser::First, 0), 0U);
    a.checkEqual("12. browse", testee.browse(0, game::msg::Browser::Last, 0), 0U);
    a.checkEqual("13. browse", testee.browse(0, game::msg::Browser::Next, 0), 0U);
    a.checkEqual("14. browse", testee.browse(0, game::msg::Browser::Previous, 0), 0U);

    game::msg::Browser::Summary_t sum;
    testee.buildSummary(sum);
    a.checkEqual("21. buildSummary", sum.size(), 0U);
}

/** Test normal browsing behaviour. */
AFL_TEST("game.msg.Browser:normal", a)
{
    Environment env;
    TestMailbox mbox(a, "..xx...x");

    game::msg::Browser testee(mbox, env.tx, env.players, &env.config);

    // isMessageFiltered
    a.checkEqual("01. isMessageFiltered", testee.isMessageFiltered(0), false);
    a.checkEqual("02. isMessageFiltered", testee.isMessageFiltered(1), false);
    a.checkEqual("03. isMessageFiltered", testee.isMessageFiltered(2), true);
    a.checkEqual("04. isMessageFiltered", testee.isMessageFiltered(7), true);

    // findFirstMessage, findLastMessage
    a.checkEqual("11. findFirstMessage", testee.findFirstMessage(), 0U);
    a.checkEqual("12. findLastMessage", testee.findLastMessage(), 6U);

    // browse first/last
    a.checkEqual("21. browse", testee.browse(0, game::msg::Browser::First, 0), 0U);
    a.checkEqual("22. browse", testee.browse(0, game::msg::Browser::Last, 0), 6U);

    // Forward from 0, skipping over filtered
    a.checkEqual("31. browse", testee.browse(0, game::msg::Browser::Next, 0), 1U);
    a.checkEqual("32. browse", testee.browse(0, game::msg::Browser::Next, 1), 1U);
    a.checkEqual("33. browse", testee.browse(0, game::msg::Browser::Next, 2), 4U);
    a.checkEqual("34. browse", testee.browse(0, game::msg::Browser::Next, 10), 6U);

    // Forward from non-0
    a.checkEqual("41. browse", testee.browse(2, game::msg::Browser::Next, 0), 4U);

    // Backward
    a.checkEqual("51. browse", testee.browse(7, game::msg::Browser::Previous, 0), 6U);
    a.checkEqual("52. browse", testee.browse(7, game::msg::Browser::Previous, 1), 6U);
    a.checkEqual("53. browse", testee.browse(7, game::msg::Browser::Previous, 2), 5U);
    a.checkEqual("54. browse", testee.browse(7, game::msg::Browser::Previous, 4), 1U);

    a.checkEqual("61. browse", testee.browse(4, game::msg::Browser::Previous, 0), 1U);
}

/** Test normal behaviour with "no filter" option.
    Messages that report being filtered must be handled normally. */
AFL_TEST("game.msg.Browser:no-filter-option", a)
{
    Environment env;
    TestMailbox mbox(a, "..xx...x");

    game::msg::Browser testee(mbox, env.tx, env.players, 0 /* no filter */);

    // isMessageFiltered: always false
    a.checkEqual("01. isMessageFiltered", testee.isMessageFiltered(0), false);
    a.checkEqual("02. isMessageFiltered", testee.isMessageFiltered(1), false);
    a.checkEqual("03. isMessageFiltered", testee.isMessageFiltered(2), false);
    a.checkEqual("04. isMessageFiltered", testee.isMessageFiltered(7), false);

    // findFirstMessage, findLastMessage
    a.checkEqual("11. findFirstMessage", testee.findFirstMessage(), 0U);
    a.checkEqual("12. findLastMessage", testee.findLastMessage(), 7U);

    // browse
    a.checkEqual("21. browse", testee.browse(0, game::msg::Browser::First, 0), 0U);
    a.checkEqual("22. browse", testee.browse(0, game::msg::Browser::Last, 0), 7U);
    a.checkEqual("23. browse", testee.browse(0, game::msg::Browser::Next, 2), 2U);
    a.checkEqual("24. browse", testee.browse(7, game::msg::Browser::Previous, 4), 3U);
}

/** Test behaviour with all messages filtered.
    findFirstMessage/findLastMessage must report first or last total because there is no unfiltered message. */
AFL_TEST("game.msg.Browser:all-filtered", a)
{
    Environment env;
    TestMailbox mbox(a, "xxx");

    game::msg::Browser testee(mbox, env.tx, env.players, &env.config);

    // isMessageFiltered
    a.checkEqual("01. isMessageFiltered", testee.isMessageFiltered(0), true);
    a.checkEqual("02. isMessageFiltered", testee.isMessageFiltered(1), true);
    a.checkEqual("03. isMessageFiltered", testee.isMessageFiltered(2), true);

    // findFirstMessage, findLastMessage
    a.checkEqual("11. findFirstMessage", testee.findFirstMessage(), 0U);
    a.checkEqual("12. findLastMessage", testee.findLastMessage(), 2U);

    // browse first/last
    a.checkEqual("21. browse", testee.browse(0, game::msg::Browser::First, 0), 0U);
    a.checkEqual("22. browse", testee.browse(0, game::msg::Browser::Last, 0), 2U);

    // browse next/previous will not advance
    a.checkEqual("31. browse", testee.browse(0, game::msg::Browser::Next, 1), 0U);
    a.checkEqual("32. browse", testee.browse(2, game::msg::Browser::Previous, 1), 2U);
}

/** Test buildSummary(). */
AFL_TEST("game.msg.Browser:buildSummary", a)
{
    Environment env;
    TestMailbox mbox(a, ".........."
                     "xxxxxxxxxx"
                     ".........."
                     ".........."
                     "xxxxxxx");

    game::msg::Browser testee(mbox, env.tx, env.players, &env.config);

    game::msg::Browser::Summary_t sum;
    testee.buildSummary(sum);

    a.checkEqual("01. size",       sum.size(), 5U);
    a.checkEqual("02. index",      sum[0].index, 0U);
    a.checkEqual("03. count",      sum[0].count, 10U);
    a.checkEqual("04. isFiltered", sum[0].isFiltered, false);
    a.checkEqual("05. heading",    sum[0].heading, "head-0.");

    a.checkEqual("11. index",      sum[1].index, 10U);
    a.checkEqual("12. count",      sum[1].count, 10U);
    a.checkEqual("13. isFiltered", sum[1].isFiltered, true);
    a.checkEqual("14. heading",    sum[1].heading, "head-1x");

    a.checkEqual("21. index",      sum[4].index, 40U);
    a.checkEqual("22. count",      sum[4].count, 7U);
    a.checkEqual("23. isFiltered", sum[4].isFiltered, true);
    a.checkEqual("24. heading",    sum[4].heading, "head-4x");
}

/** Test search(). */
AFL_TEST("game.msg.Browser:search", a)
{
    Environment env;
    TestMailbox mbox(a, "..xx...x");

    game::msg::Browser testee(mbox, env.tx, env.players, &env.config);

    // Finding message 4
    // - normally
    a.checkEqual("01. search", testee.search(0, game::msg::Browser::Next,     1, "text-4").index, 4U);
    a.checkEqual("02. search", testee.search(0, game::msg::Browser::Next,     1, "text-4").found, true);

    // - case-blind
    a.checkEqual("11. search", testee.search(0, game::msg::Browser::Next,     1, "TEXT-4").index, 4U);

    // - from end
    a.checkEqual("21. search", testee.search(7, game::msg::Browser::Previous, 1, "text-4").index, 4U);

    // - first, last
    a.checkEqual("31. search", testee.search(0, game::msg::Browser::First,    1, "text-4").index, 4U);
    a.checkEqual("32. search", testee.search(0, game::msg::Browser::Last,     1, "text-4").index, 4U);

    // - with repeat (will settle at first found)
    a.checkEqual("41. search", testee.search(0, game::msg::Browser::Next,     2, "text-4").index, 4U);

    // Repeat case (will find 4,5)
    a.checkEqual("51. search", testee.search(1, game::msg::Browser::Next,     2, "text").index, 5U);

    // Failure case
    a.checkEqual("61. search", testee.search(5, game::msg::Browser::Next,     1, "notfound").index, 5U);
    a.checkEqual("62. search", testee.search(5, game::msg::Browser::Previous, 1, "notfound").index, 5U);
    a.checkEqual("63. search", testee.search(5, game::msg::Browser::First,    1, "notfound").index, 0U);
    a.checkEqual("64. search", testee.search(5, game::msg::Browser::Last,     1, "notfound").index, 7U);

    a.checkEqual("71. search", testee.search(5, game::msg::Browser::Next,     1, "notfound").found, false);

    // Filtered case: not found because it's filtered
    a.checkEqual("81. search", testee.search(0, game::msg::Browser::Next,     1, "text-2").index, 0U);
    a.checkEqual("82. search", testee.search(0, game::msg::Browser::Next,     1, "text-2").found, false);
}
