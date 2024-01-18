/**
  *  \file test/server/talk/inlinerecognizertest.cpp
  *  \brief Test for server::talk::InlineRecognizer
  */

#include "server/talk/inlinerecognizer.hpp"
#include "afl/test/testrunner.hpp"

/** Test recognition of URLs. */
AFL_TEST("server.talk.InlineRecognizer:find:url", a)
{
    using server::talk::InlineRecognizer;
    InlineRecognizer testee;
    InlineRecognizer::Info result;
    const InlineRecognizer::Kinds_t KINDS(InlineRecognizer::Link);
    bool ok;

    /*
     *  Simple tests
     */

    // URL that fills the whole string
    ok = testee.find("http://foo/", 0, KINDS, result);
    a.check("01. ok", ok);
    a.checkEqual("02. kind",   result.kind, InlineRecognizer::Link);
    a.checkEqual("03. start",  result.start, 0U);
    a.checkEqual("04. length", result.length, 11U);
    a.checkEqual("05. text",   result.text, "http://foo/");

    // Email address that fills the whole string
    ok = testee.find("mailto:me@here.example", 0, KINDS, result);
    a.check("11. ok", ok);
    a.checkEqual("12. kind",   result.kind, InlineRecognizer::Link);
    a.checkEqual("13. start",  result.start, 0U);
    a.checkEqual("14. length", result.length, 22U);
    a.checkEqual("15. text",   result.text, "mailto:me@here.example");

    // URL with stuff before and after
    ok = testee.find("see http://foo/ for more", 0, KINDS, result);
    a.check("21. ok", ok);
    a.checkEqual("22. kind",   result.kind, InlineRecognizer::Link);
    a.checkEqual("23. start",  result.start, 4U);
    a.checkEqual("24. length", result.length, 11U);
    a.checkEqual("25. text",   result.text, "http://foo/");

    /*
     *  Specific tests
     */

    // Unrecognized protocol
    ok = testee.find("see foo://foo/ for more", 0, KINDS, result);
    a.check("31. ok", !ok);

    // Protocol preceded by letter
    ok = testee.find("see thttp://foo/ for more", 0, KINDS, result);
    a.check("41. ok", !ok);

    // Angle bracket
    ok = testee.find("see <http://foo/That Page> for more", 0, KINDS, result);
    a.check("51. ok", ok);
    a.checkEqual("52. kind",   result.kind, InlineRecognizer::Link);
    a.checkEqual("53. start",  result.start, 5U);
    a.checkEqual("54. length", result.length, 20U);
    a.checkEqual("55. text",   result.text, "http://foo/That Page");

    // Missing angle bracket
    ok = testee.find("see <http://foo/That Page\nfor more", 0, KINDS, result);
    a.check("61. ok", ok);
    a.checkEqual("62. kind",   result.kind, InlineRecognizer::Link);
    a.checkEqual("63. start",  result.start, 5U);
    a.checkEqual("64. length", result.length, 15U);
    a.checkEqual("65. text",   result.text, "http://foo/That");

    // Missing angle bracket
    ok = testee.find("see <http://foo/That Page", 0, KINDS, result);
    a.check("71. ok", ok);
    a.checkEqual("72. kind",   result.kind, InlineRecognizer::Link);
    a.checkEqual("73. start",  result.start, 5U);
    a.checkEqual("74. length", result.length, 15U);
    a.checkEqual("75. text",   result.text, "http://foo/That");

    // Strange character after protocol name
    ok = testee.find("see http:@xy maybe", 0, KINDS, result);
    a.check("81. ok", !ok);

    // Regular URL in parens
    ok = testee.find("see page (http://foo/bar/baz) for more", 0, KINDS, result);
    a.check("91. ok", ok);
    a.checkEqual("92. kind",   result.kind, InlineRecognizer::Link);
    a.checkEqual("93. start",  result.start, 10U);
    a.checkEqual("94. length", result.length, 18U);
    a.checkEqual("95. text",   result.text, "http://foo/bar/baz");

    // Wiki URL in parens
    ok = testee.find("see page (http://foo/wiki/Foo_(Bar)) for more", 0, KINDS, result);
    a.check("101. ok", ok);
    a.checkEqual("102. kind",   result.kind, InlineRecognizer::Link);
    a.checkEqual("103. start",  result.start, 10U);
    a.checkEqual("104. length", result.length, 25U);
    a.checkEqual("105. text",   result.text, "http://foo/wiki/Foo_(Bar)");

    // Wiki URL without parens
    ok = testee.find("see page http://foo/wiki/Foo_(Baz) for more", 0, KINDS, result);
    a.check("111. ok", ok);
    a.checkEqual("112. kind",   result.kind, InlineRecognizer::Link);
    a.checkEqual("113. start",  result.start, 9U);
    a.checkEqual("114. length", result.length, 25U);
    a.checkEqual("115. text",   result.text, "http://foo/wiki/Foo_(Baz)");

    // MSDN URL in parens
    ok = testee.find("see page (http://foo/bla(4.2).aspx) for more", 0, KINDS, result);
    a.check("121. ok", ok);
    a.checkEqual("122. kind",   result.kind, InlineRecognizer::Link);
    a.checkEqual("123. start",  result.start, 10U);
    a.checkEqual("124. length", result.length, 24U);
    a.checkEqual("125. text",   result.text, "http://foo/bla(4.2).aspx");

    // MSDN URL without parens
    ok = testee.find("see page http://foo/bla(5.1).aspx for more", 0, KINDS, result);
    a.check("131. ok", ok);
    a.checkEqual("132. kind",   result.kind, InlineRecognizer::Link);
    a.checkEqual("133. start",  result.start, 9U);
    a.checkEqual("134. length", result.length, 24U);
    a.checkEqual("135. text",   result.text, "http://foo/bla(5.1).aspx");

    // URL in quotes
    ok = testee.find("url = \"http://host/path\";", 0, KINDS, result);
    a.check("141. ok", ok);
    a.checkEqual("142. kind",   result.kind, InlineRecognizer::Link);
    a.checkEqual("143. start",  result.start, 7U);
    a.checkEqual("144. length", result.length, 16U);
    a.checkEqual("145. text",   result.text, "http://host/path");

    // URL with parens in quotes
    ok = testee.find("url = \"http://host/path/(what\";", 0, KINDS, result);
    a.check("151. ok", ok);
    a.checkEqual("152. kind",   result.kind, InlineRecognizer::Link);
    a.checkEqual("153. start",  result.start, 7U);
    a.checkEqual("154. length", result.length, 22U);
    a.checkEqual("155. text",   result.text, "http://host/path/(what");

    // URL with parens ending in '>'
    ok = testee.find("<url = http://host/path/(what>;", 0, KINDS, result);
    a.check("161. ok", ok);
    a.checkEqual("162. kind",   result.kind, InlineRecognizer::Link);
    a.checkEqual("163. start",  result.start, 7U);
    a.checkEqual("164. length", result.length, 22U);
    a.checkEqual("165. text",   result.text, "http://host/path/(what");

    // URL with dot and '>'
    ok = testee.find("<look here http://host/path.>", 0, KINDS, result);
    a.check("171. ok", ok);
    a.checkEqual("172. kind",   result.kind, InlineRecognizer::Link);
    a.checkEqual("173. start",  result.start, 11U);
    a.checkEqual("174. length", result.length, 17U);
    a.checkEqual("175. text",   result.text, "http://host/path.");

    // URL with dot
    ok = testee.find("look here http://host/path.", 0, KINDS, result);
    a.check("181. ok", ok);
    a.checkEqual("182. kind",   result.kind, InlineRecognizer::Link);
    a.checkEqual("183. start",  result.start, 10U);
    a.checkEqual("184. length", result.length, 16U);
    a.checkEqual("185. text",   result.text, "http://host/path");

    // URL preceded by word is not recognized
    a.check("191", !testee.find("see nothttp://foo/ for more", 0, InlineRecognizer::Kinds_t(InlineRecognizer::Link), result));
    // FIXME: should this be rejected? Right now it is recognized.
    // It makes no difference in practical use because no search leaves off at the given place.
    // a.check("192", !testee.find("see nothttp://foo/ for more", 7, InlineRecognizer::Kinds_t(InlineRecognizer::Link), result));
}

/** Test recognition of smileys. */
AFL_TEST("server.talk.InlineRecognizer:find:smiley", a)
{
    using server::talk::InlineRecognizer;
    InlineRecognizer testee;
    InlineRecognizer::Info result;
    const InlineRecognizer::Kinds_t KINDS(InlineRecognizer::Smiley);
    bool ok;

    /*
     *  Simple tests
     */

    // Named smiley that fills the whole string
    ok = testee.find(":lol:", 0, KINDS, result);
    a.check("01. ok", ok);
    a.checkEqual("02. kind",   result.kind, InlineRecognizer::Smiley);
    a.checkEqual("03. start",  result.start, 0U);
    a.checkEqual("04. length", result.length, 5U);
    a.checkEqual("05. text",   result.text, "lol");

    // Named smiley in text
    ok = testee.find("haha :lol: haha", 0, KINDS, result);
    a.check("11. ok", ok);
    a.checkEqual("12. kind",   result.kind, InlineRecognizer::Smiley);
    a.checkEqual("13. start",  result.start, 5U);
    a.checkEqual("14. length", result.length, 5U);
    a.checkEqual("15. text",   result.text, "lol");

    // Regular smiley that fills the whole string
    ok = testee.find(":-(", 0, KINDS, result);
    a.check("21. ok", ok);
    a.checkEqual("22. kind",   result.kind, InlineRecognizer::Smiley);
    a.checkEqual("23. start",  result.start, 0U);
    a.checkEqual("24. length", result.length, 3U);
    a.checkEqual("25. text",   result.text, "sad");

    // Regular smiley in text
    ok = testee.find("boohoo :-( boohoo", 0, KINDS, result);
    a.check("31. ok", ok);
    a.checkEqual("32. kind",   result.kind, InlineRecognizer::Smiley);
    a.checkEqual("33. start",  result.start, 7U);
    a.checkEqual("34. length", result.length, 3U);
    a.checkEqual("35. text",   result.text, "sad");

    // Short smiley that fills the whole string
    ok = testee.find(":(", 0, KINDS, result);
    a.check("41. ok", ok);
    a.checkEqual("42. kind",   result.kind, InlineRecognizer::Smiley);
    a.checkEqual("43. start",  result.start, 0U);
    a.checkEqual("44. length", result.length, 2U);
    a.checkEqual("45. text",   result.text, "sad");

    // Short smiley in text
    ok = testee.find("bu :( bu", 0, KINDS, result);
    a.check("51. ok", ok);
    a.checkEqual("52. kind",   result.kind, InlineRecognizer::Smiley);
    a.checkEqual("53. start",  result.start, 3U);
    a.checkEqual("54. length", result.length, 2U);
    a.checkEqual("55. text",   result.text, "sad");

    /*
     *  Specific tests
     */

    // Unrecognized named smiley
    ok = testee.find(" :notasmiley: ", 0, KINDS, result);
    a.check("61. ok", !ok);

    // We're case-sensitive
    ok = testee.find(" :LOL: ", 0, KINDS, result);
    a.check("71. ok", !ok);

    // Symbol smiley starting with letter
    ok = testee.find("hey B-)", 0, KINDS, result);
    a.check("81. ok", ok);
    a.checkEqual("82. kind",   result.kind, InlineRecognizer::Smiley);
    a.checkEqual("83. start",  result.start, 4U);
    a.checkEqual("84. length", result.length, 3U);
    a.checkEqual("85. text",   result.text, "cool");

    // Symbol smiley starting with letter preceded by text
    ok = testee.find("heyB-)", 0, KINDS, result);
    a.check("91. ok", !ok);

    // Symbol smiley ending with letter
    ok = testee.find("hey :-P lol", 0, KINDS, result);
    a.check("101. ok", ok);
    a.checkEqual("102. kind",   result.kind, InlineRecognizer::Smiley);
    a.checkEqual("103. start",  result.start, 4U);
    a.checkEqual("104. length", result.length, 3U);
    a.checkEqual("105. text",   result.text, "tongue");

    // Symbol smiley ending with letter followed by text
    ok = testee.find("hey :-Plol", 0, KINDS, result);
    a.check("111. ok", !ok);
}

/** General tests. */
AFL_TEST("server.talk.InlineRecognizer:find:general", a)
{
    using server::talk::InlineRecognizer;
    InlineRecognizer testee;
    InlineRecognizer::Info result;
    bool ok;

    // Test how startAt parameter affects result
    a.check("01",  testee.find("see http://foo/ for more", 0, InlineRecognizer::Kinds_t(InlineRecognizer::Link), result));
    a.check("02",  testee.find("see http://foo/ for more", 3, InlineRecognizer::Kinds_t(InlineRecognizer::Link), result));
    a.check("03",  testee.find("see http://foo/ for more", 4, InlineRecognizer::Kinds_t(InlineRecognizer::Link), result));
    a.check("04", !testee.find("see http://foo/ for more", 5, InlineRecognizer::Kinds_t(InlineRecognizer::Link), result));
    a.check("05", !testee.find("see http://foo/ for more", 8, InlineRecognizer::Kinds_t(InlineRecognizer::Link), result));
    a.check("06", !testee.find("see http://foo/ for more", 10, InlineRecognizer::Kinds_t(InlineRecognizer::Link), result));

    a.check("11",  testee.find("lol :-) lol", 0, InlineRecognizer::Kinds_t(InlineRecognizer::Smiley), result));
    a.check("12",  testee.find("lol :-) lol", 3, InlineRecognizer::Kinds_t(InlineRecognizer::Smiley), result));
    a.check("13",  testee.find("lol :-) lol", 4, InlineRecognizer::Kinds_t(InlineRecognizer::Smiley), result));
    a.check("14", !testee.find("lol :-) lol", 5, InlineRecognizer::Kinds_t(InlineRecognizer::Smiley), result));
    a.check("15", !testee.find("lol :-) lol", 8, InlineRecognizer::Kinds_t(InlineRecognizer::Smiley), result));

    // Test recognition of multiple kinds
    ok = testee.find("see http://foo/B-) for more", 0, InlineRecognizer::Kinds_t(InlineRecognizer::Link) + InlineRecognizer::Smiley, result);
    a.check("21. ok", ok);
    a.checkEqual("22. kind",   result.kind, InlineRecognizer::Link);
    a.checkEqual("23. start",  result.start, 4U);
    a.checkEqual("24. length", result.length, 13U);
    a.checkEqual("25. text",   result.text, "http://foo/B-");

    // Starting after the beginning of the URL will find the smiley
    ok = testee.find("see http://foo/B-) for more", 5, InlineRecognizer::Kinds_t(InlineRecognizer::Link) + InlineRecognizer::Smiley, result);
    a.check("31. ok", ok);
    a.checkEqual("32. kind",   result.kind, InlineRecognizer::Smiley);
    a.checkEqual("33. start",  result.start, 15U);
    a.checkEqual("34. length", result.length, 3U);
    a.checkEqual("35. text",   result.text, "cool");

    // Boundary case
    ok = testee.find("", 0, InlineRecognizer::Kinds_t(InlineRecognizer::Link) + InlineRecognizer::Smiley, result);
    a.check("41. ok", !ok);
}

/** Test getSmileyDefinitionByName. */
AFL_TEST("server.talk.InlineRecognizer:getSmileyDefinitionByName", a)
{
    server::talk::InlineRecognizer testee;
    const server::talk::InlineRecognizer::SmileyDefinition* p;

    // Border case
    p = testee.getSmileyDefinitionByName("");
    a.checkNull("01", p);

    // Find one
    p = testee.getSmileyDefinitionByName("lol");
    a.checkNonNull("11", p);
    a.checkEqual("12", String_t(p->name), "lol");

    // Find another one
    p = testee.getSmileyDefinitionByName("wink");
    a.checkNonNull("21", p);
    a.checkEqual("22", String_t(p->name), "wink");

    // Find yet another one
    p = testee.getSmileyDefinitionByName("cool");
    a.checkNonNull("31", p);
    a.checkEqual("32", String_t(p->name), "cool");

    // Mismatch: case sensitive
    p = testee.getSmileyDefinitionByName("LOL");
    a.checkNull("41", p);

    // Mismatch: prefix
    p = testee.getSmileyDefinitionByName("lolol");
    a.checkNull("51", p);

    // Mismatch: symbol
    p = testee.getSmileyDefinitionByName(":-)");
    a.checkNull("61", p);
}
