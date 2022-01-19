/**
  *  \file u/t_game_browser_account.cpp
  *  \brief Test for game::browser::Account
  */

#include "game/browser/account.hpp"

#include "t_game_browser.hpp"
#include "afl/io/textfile.hpp"
#include "afl/io/internalstream.hpp"

/** Test basic operations (getter, setter). */
void
TestGameBrowserAccount::testBasic()
{
    game::browser::Account testee;
    TS_ASSERT(!testee.isValid());

    testee.setName("foo");
    TS_ASSERT_EQUALS(testee.getName(), "foo");
    TS_ASSERT(!testee.isValid());

    testee.setUser("bar");
    TS_ASSERT_EQUALS(testee.getUser(), "bar");
    TS_ASSERT(!testee.isValid());

    testee.setHost("baz");
    TS_ASSERT_EQUALS(testee.getHost(), "baz");
    TS_ASSERT(!testee.isValid());

    testee.setType("qux");
    TS_ASSERT_EQUALS(testee.getType(), "qux");
    TS_ASSERT(testee.isValid());

    const String_t* f = testee.get("fred");
    TS_ASSERT(f == 0);

    testee.set("fred", "flintstone", true);
    f = testee.get("fred");
    TS_ASSERT(f != 0);
    TS_ASSERT_EQUALS(*f, "flintstone");

    testee.setGameFolderName("1+1", "/home/games/1+1");
    f = testee.getGameFolderName("1+1");
    TS_ASSERT(f != 0);
    TS_ASSERT_EQUALS(*f, "/home/games/1+1");

    f = testee.get("game:1%2B1");
    TS_ASSERT(f != 0);
    TS_ASSERT_EQUALS(*f, "/home/games/1+1");

    testee.setGameFolderName("1+1", "");
    f = testee.get("game:1%2B1");
    TS_ASSERT(f == 0);
}

/** Test persistence. */
void
TestGameBrowserAccount::testPersistent()
{
    game::browser::Account testee;
    testee.setName("user @ host");
    testee.setUser("user");
    testee.setType("type");
    testee.setHost("host");
    testee.set("password", "secret!", false);

    afl::io::InternalStream ms;
    afl::io::TextFile tf(ms);
    tf.setSystemNewline(false);
    testee.write(tf);
    tf.flush();

    TS_ASSERT_EQUALS(afl::string::fromBytes(ms.getContent()),
                     "[user @ host]\n"
                     "host=host\n"
                     "type=type\n"
                     "user=user\n");
}

/** Test encoded storage. */
void
TestGameBrowserAccount::testEncode()
{
    game::browser::Account testee;
    testee.setEncoded("1", "", true);
    testee.setEncoded("2", "a", true);
    testee.setEncoded("3", "aa", true);
    testee.setEncoded("4", "aaa", true);
    testee.setEncoded("5", "aaaa", true);

    String_t result;
    const String_t* p;

    p = testee.get("0");
    TS_ASSERT(!testee.getEncoded("0", result));
    TS_ASSERT(p == 0);

    p = testee.get("1");
    TS_ASSERT(testee.getEncoded("1", result));
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(*p, "");
    TS_ASSERT_EQUALS(result, "");;

    p = testee.get("2");
    TS_ASSERT(testee.getEncoded("2", result));
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(*p, "YQ==");
    TS_ASSERT_EQUALS(result, "a");

    p = testee.get("3");
    TS_ASSERT(testee.getEncoded("3", result));
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(*p, "YWE=");
    TS_ASSERT_EQUALS(result, "aa");

    p = testee.get("4");
    TS_ASSERT(testee.getEncoded("4", result));
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(*p, "YWFh");
    TS_ASSERT_EQUALS(result, "aaa");

    p = testee.get("5");
    TS_ASSERT(testee.getEncoded("5", result));
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(*p, "YWFhYQ==");
    TS_ASSERT_EQUALS(result, "aaaa");;
}
