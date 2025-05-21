/**
  *  \file test/game/browser/accounttest.cpp
  *  \brief Test for game::browser::Account
  */

#include "game/browser/account.hpp"

#include "afl/io/internalstream.hpp"
#include "afl/io/textfile.hpp"
#include "afl/test/testrunner.hpp"

/** Test basic operations (getter, setter). */
AFL_TEST("game.browser.Account:basic", a)
{
    game::browser::Account testee;
    a.check("01. isValid", !testee.isValid());

    testee.setName("foo");
    a.checkEqual("11. getName", testee.getName(), "foo");
    a.check("12. isValid", !testee.isValid());

    testee.setUser("bar");
    a.checkEqual("21. getUser", testee.getUser(), "bar");
    a.check("22. isValid", !testee.isValid());

    testee.setHost("baz");
    a.checkEqual("31. getHost", testee.getHost(), "baz");
    a.check("32. isValid", !testee.isValid());

    testee.setType("qux");
    a.checkEqual("41. getType", testee.getType(), "qux");
    a.check("42. isValid", testee.isValid());

    const String_t* f = testee.get("fred");
    a.checkNull("51. get", f);

    testee.set("fred", "flintstone", true);
    f = testee.get("fred");
    a.checkNonNull("61. get", f);
    a.checkEqual("62. get", *f, "flintstone");

    testee.setGameFolderName("1+1", "/home/games/1+1");
    f = testee.getGameFolderName("1+1");
    a.checkNonNull("71. getGameFolderName", f);
    a.checkEqual("72. getGameFolderName", *f, "/home/games/1+1");

    f = testee.get("game:1%2B1");
    a.checkNonNull("81. get", f);
    a.checkEqual("82. get", *f, "/home/games/1+1");

    testee.setGameFolderName("1+1", "");
    f = testee.get("game:1%2B1");
    a.checkNull("91. get", f);
}

/** Test persistence. */
AFL_TEST("game.browser.Account:write", a)
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

    a.checkEqual("write result", afl::string::fromBytes(ms.getContent()),
                 "[user @ host]\n"
                 "host=host\n"
                 "type=type\n"
                 "user=user\n");
}

/** Test encoded storage. */
AFL_TEST("game.browser.Account:getEncoded", a)
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
    a.check("01. getEncoded", !testee.getEncoded("0").isValid());
    a.checkNull("02. get", p);

    p = testee.get("1");
    a.check("11. getEncoded", testee.getEncoded("1").get(result));
    a.checkNonNull("12. get", p);
    a.checkEqual("13. get", *p, "");
    a.checkEqual("14. getEncoded", result, "");

    p = testee.get("2");
    a.check("21. getEncoded", testee.getEncoded("2").get(result));
    a.checkNonNull("22. get", p);
    a.checkEqual("23. get", *p, "YQ==");
    a.checkEqual("24. getEncoded", result, "a");

    p = testee.get("3");
    a.check("31. getEncoded", testee.getEncoded("3").get(result));
    a.checkNonNull("32. get", p);
    a.checkEqual("33. get", *p, "YWE=");
    a.checkEqual("34. getEncoded", result, "aa");

    p = testee.get("4");
    a.check("41. getEncoded", testee.getEncoded("4").get(result));
    a.checkNonNull("42. get", p);
    a.checkEqual("43. get", *p, "YWFh");
    a.checkEqual("44. getEncoded", result, "aaa");

    p = testee.get("5");
    a.check("51. getEncoded", testee.getEncoded("5").get(result));
    a.checkNonNull("52. get", p);
    a.checkEqual("53. get", *p, "YWFhYQ==");
    a.checkEqual("54. getEncoded", result, "aaaa");
}
