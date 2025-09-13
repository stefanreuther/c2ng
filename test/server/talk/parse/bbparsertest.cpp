/**
  *  \file test/server/talk/parse/bbparsertest.cpp
  *  \brief Test for server::talk::parse::BBParser
  */

#include "server/talk/parse/bbparser.hpp"

#include "afl/string/format.hpp"
#include "afl/test/testrunner.hpp"
#include "server/talk/inlinerecognizer.hpp"
#include "server/talk/nulllinkparser.hpp"
#include <memory>

using server::talk::TextNode;
using server::talk::parse::BBParser;

namespace {
    String_t getNodeName(const TextNode& n)
    {
        switch (TextNode::MajorKind(n.major)) {
         case TextNode::maPlain: return "plain";
         case TextNode::maInline:
            switch (TextNode::InlineFormat(n.minor)) {
             case TextNode::miInBold: return "inline-bold";
             case TextNode::miInItalic: return "inline-italic";
             case TextNode::miInStrikeThrough: return "inline-strike";
             case TextNode::miInUnderline: return "inline-under";
             case TextNode::miInMonospace: return "inline-tt";
            }
            return "inline-?";
         case TextNode::maInlineAttr:
            switch (TextNode::InlineAttrFormat(n.minor)) {
             case TextNode::miIAColor: return "ia-color";
             case TextNode::miIASize: return "ia-size";
             case TextNode::miIAFont: return "ia-font";
            }
            return "ia-?";
         case TextNode::maLink:
            switch (TextNode::LinkFormat(n.minor)) {
             case TextNode::miLinkUrl: return "link-url";
             case TextNode::miLinkEmail: return "link-email";
             case TextNode::miLinkThread: return "link-thread";
             case TextNode::miLinkPost: return "link-post";
             case TextNode::miLinkGame: return "link-game";
             case TextNode::miLinkUser: return "link-user";
             case TextNode::miLinkForum: return "link-forum";
            }
            return "link-?";
         case TextNode::maParagraph:
            switch (TextNode::ParagraphFormat(n.minor)) {
             case TextNode::miParNormal: return "paragraph";
             case TextNode::miParCode: return "code";
             case TextNode::miParCentered: return "centered";
             case TextNode::miParBreak: return "break";
             case TextNode::miParFragment: return "fragment";
            }
            return "par-?";
         case TextNode::maGroup:
            switch (TextNode::GroupFormat(n.minor)) {
             case TextNode::miGroupRoot: return "root";
             case TextNode::miGroupQuote: return "quote";
             case TextNode::miGroupListItem: return "list-item";
             case TextNode::miGroupList: return "list";
            }
            return "group-?";
         case TextNode::maSpecial:
            switch (TextNode::SpecialFormat(n.minor)) {
             case TextNode::miSpecialBreak: return "br";
             case TextNode::miSpecialImage: return "image";
             case TextNode::miSpecialSmiley: return "smiley";
            }
            return "special-?";
        }
        return "?";
    }

    String_t toString(TextNode* n)
    {
        if (n == 0) {
            return "nil";
        } else {
            String_t result = "[";
            result += getNodeName(*n);
            if (!n->text.empty()) {
                result += ",'";
                result += n->text;
                result += "'";
            }
            for (size_t i = 0; i < n->children.size(); ++i) {
                result += ",";
                result += toString(n->children[i]);
            }
            result += "]";
            return result;
        }
    }

    TextNode* doParse(server::talk::InlineRecognizer& recog,
                      server::talk::InlineRecognizer::Kinds_t options,
                      String_t text)
    {
        server::talk::NullLinkParser lp;
        server::talk::parse::BBLexer lex(text);
        BBParser parser(lex, recog, options, lp);
        return parser.parse();
    }

    const char* toString(BBParser::WarningType t)
    {
        switch (t) {
         case BBParser::SuspiciousText: return "SuspiciousText";
         case BBParser::MissingClose:   return "MissingClose";
         case BBParser::TagNotOpen:     return "TagNotOpen";
         case BBParser::BadLink:        return "BadLink";
         case BBParser::NoOwnText:      return "NoOwnText";
        }
        return "?";
    }

    String_t toString(const BBParser::Warning& w)
    {
        return afl::string::Format("%s,%s,%s,%d", toString(w.type), w.token, w.extra, w.pos);
    }

    String_t doParseWarnings(String_t text)
    {
        class TestLinkParser : public server::talk::LinkParser {
         public:
            virtual afl::base::Optional<Result_t> parseGameLink(String_t text) const
                { if (text == "bad_game") { return afl::base::Nothing; } else { return Result_t(1, "g"); } }
            virtual afl::base::Optional<Result_t> parseForumLink(String_t text) const
                { if (text == "bad_forum") { return afl::base::Nothing; } else { return Result_t(1, "f"); } }
            virtual afl::base::Optional<Result_t> parseTopicLink(String_t text) const
                { if (text == "bad_topic") { return afl::base::Nothing; } else { return Result_t(1, "t"); } }
            virtual afl::base::Optional<Result_t> parseMessageLink(String_t text) const
                { if (text == "bad_message") { return afl::base::Nothing; } else { return Result_t(1, "m"); } }
            virtual afl::base::Optional<String_t> parseUserLink(String_t text) const
                { if (text == "bad_user") { return afl::base::Nothing; } else { return String_t("u"); } }
        };

        server::talk::InlineRecognizer recog;
        server::talk::InlineRecognizer::Kinds_t options;  // no options for now
        server::talk::parse::BBLexer lex(text);
        TestLinkParser lp;
        BBParser parser(lex, recog, options, lp);
        delete parser.parse();

        String_t result;
        for (size_t i = 0; i < parser.warnings().size(); ++i) {
            if (i != 0) {
                result += "|";
            }
            result += toString(parser.warnings()[i]);
        }
        return result;
    }
}

/** Some basic tests. */
AFL_TEST("server.talk.parse.BBParser:basics", a)
{
    std::auto_ptr<TextNode> t;
    server::talk::InlineRecognizer recog;
    server::talk::InlineRecognizer::Kinds_t options;  // no options for now

    // plain text
    t.reset(doParse(recog, options, "hello, world"));
    a.checkEqual("01", toString(t.get()), "[root,[paragraph,[plain,'hello, world']]]");

    // two paragraphs
    t.reset(doParse(recog, options, "hello, world\n\n\n\ngood bye"));
    a.checkEqual("11", toString(t.get()), "[root,[paragraph,[plain,'hello, world']],[paragraph,[plain,'good bye']]]");
    t.reset(doParse(recog, options, "hello, world[center]good bye[/center]"));
    a.checkEqual("12", toString(t.get()), "[root,[paragraph,[plain,'hello, world']],[centered,[plain,'good bye']]]");

    // regular inline markup
    t.reset(doParse(recog, options, "hello, [b]world[/b]"));
    a.checkEqual("21", toString(t.get()), "[root,[paragraph,[plain,'hello, '],[inline-bold,[plain,'world']]]]");

    // regular inline markup missing end
    t.reset(doParse(recog, options, "hello, [b]world"));
    a.checkEqual("31", toString(t.get()), "[root,[paragraph,[plain,'hello, '],[inline-bold,[plain,'world']]]]");

    // inline markup missing start
    t.reset(doParse(recog, options, "hello,[/b] world"));
    a.checkEqual("41", toString(t.get()), "[root,[paragraph,[inline-bold,[plain,'hello,']],[plain,' world']]]");

    // color (various formats)
    t.reset(doParse(recog, options, "hello, [color=red]world"));
    a.checkEqual("51", toString(t.get()), "[root,[paragraph,[plain,'hello, '],[ia-color,'#ff0000',[plain,'world']]]]");
    t.reset(doParse(recog, options, "hello, [color=#123]world"));
    a.checkEqual("52", toString(t.get()), "[root,[paragraph,[plain,'hello, '],[ia-color,'#112233',[plain,'world']]]]");
    t.reset(doParse(recog, options, "hello, [color=123]world"));
    a.checkEqual("53", toString(t.get()), "[root,[paragraph,[plain,'hello, '],[ia-color,'#112233',[plain,'world']]]]");
    t.reset(doParse(recog, options, "hello, [color=#ABCDEF]world"));
    a.checkEqual("54", toString(t.get()), "[root,[paragraph,[plain,'hello, '],[ia-color,'#abcdef',[plain,'world']]]]");
    t.reset(doParse(recog, options, "hello, [color=Abcdef]world"));
    a.checkEqual("55", toString(t.get()), "[root,[paragraph,[plain,'hello, '],[ia-color,'#abcdef',[plain,'world']]]]");

    // size (various formats)
    t.reset(doParse(recog, options, "hello, [size=3]world"));
    a.checkEqual("61", toString(t.get()), "[root,[paragraph,[plain,'hello, '],[ia-size,'-2',[plain,'world']]]]");
    t.reset(doParse(recog, options, "hello, [size=+1]world"));
    a.checkEqual("62", toString(t.get()), "[root,[paragraph,[plain,'hello, '],[ia-size,'+1',[plain,'world']]]]");
    t.reset(doParse(recog, options, "hello, [size=-1]world"));
    a.checkEqual("63", toString(t.get()), "[root,[paragraph,[plain,'hello, '],[ia-size,'-1',[plain,'world']]]]");

    // font
    t.reset(doParse(recog, options, "hello, [font=courier]world"));
    a.checkEqual("71", toString(t.get()), "[root,[paragraph,[plain,'hello, '],[ia-font,'courier',[plain,'world']]]]");
    t.reset(doParse(recog, options, "hello, [font=\"Times Roman\"]world"));
    a.checkEqual("72", toString(t.get()), "[root,[paragraph,[plain,'hello, '],[ia-font,'Times Roman',[plain,'world']]]]");

    // links
    t.reset(doParse(recog, options, "hello @user there"));
    a.checkEqual("81", toString(t.get()), "[root,[paragraph,[plain,'hello '],[link-user,'user'],[plain,' there']]]");
    t.reset(doParse(recog, options, "hello [user]jj[/user] there"));
    a.checkEqual("82", toString(t.get()), "[root,[paragraph,[plain,'hello '],[link-user,'jj'],[plain,' there']]]");
    t.reset(doParse(recog, options, "hello [user=jj][/user] there"));
    a.checkEqual("83", toString(t.get()), "[root,[paragraph,[plain,'hello '],[link-user,'jj'],[plain,' there']]]");
    t.reset(doParse(recog, options, "hello [user=jj]xx[/user] there"));
    a.checkEqual("84", toString(t.get()), "[root,[paragraph,[plain,'hello '],[link-user,'jj',[plain,'xx']],[plain,' there']]]");

    // link with markup
    t.reset(doParse(recog, options, "[user][b]f[/b]runo[/user]"));
    a.checkEqual("91", toString(t.get()), "[root,[paragraph,[link-user,'fruno',[inline-bold,[plain,'f']],[plain,'runo']]]]");

    // nested links
    t.reset(doParse(recog, options, "[game=1]a [thread=2]b[/thread] c[/game]"));
    a.checkEqual("101", toString(t.get()), "[root,[paragraph,[link-game,'1',[plain,'a ']],[link-thread,'2',[plain,'b']],[plain,' c[/game]']]]");
    t.reset(doParse(recog, options, "[game=1]a @user c[/game]"));
    a.checkEqual("102", toString(t.get()), "[root,[paragraph,[link-game,'1',[plain,'a ']],[link-user,'user'],[plain,' c[/game]']]]");

    // noparse
    t.reset(doParse(recog, options, "hello [noparse][b]hi[/noparse][b]ho"));
    a.checkEqual("111", toString(t.get()), "[root,[paragraph,[plain,'hello [b]hi'],[inline-bold,[plain,'ho']]]]");
    t.reset(doParse(recog, options, "a[noparse][/[/noparse]noparse]b"));
    a.checkEqual("112", toString(t.get()), "[root,[paragraph,[plain,'a[/noparse]b']]]");
    t.reset(doParse(recog, options, "a[noparse][noparse][/noparse]b"));
    a.checkEqual("113", toString(t.get()), "[root,[paragraph,[plain,'a[noparse]b']]]");

    // list
    t.reset(doParse(recog, options, "a[list][*]b[*]c[/list]d"));
    a.checkEqual("121", toString(t.get()), "[root,[paragraph,[plain,'a']],[list,[list-item,[paragraph,[plain,'b']]],[list-item,[paragraph,[plain,'c']]]],[paragraph,[plain,'d']]]");
    t.reset(doParse(recog, options, "a[list=1][*]b[*]c[/list]d"));
    a.checkEqual("122", toString(t.get()), "[root,[paragraph,[plain,'a']],[list,'1',[list-item,[paragraph,[plain,'b']]],[list-item,[paragraph,[plain,'c']]]],[paragraph,[plain,'d']]]");

    // smiley (with tag)
    t.reset(doParse(recog, options, "a [:smile:] b"));
    a.checkEqual("131", toString(t.get()), "[root,[paragraph,[plain,'a '],[smiley,'smile'],[plain,' b']]]");

    // code
    t.reset(doParse(recog, options, "hello [code=c]static int a[b];[/code][b]ho"));
    a.checkEqual("141", toString(t.get()), "[root,[paragraph,[plain,'hello ']],[code,'c',[plain,'static int a[b];']],[paragraph,[inline-bold,[plain,'ho']]]]");

    // breaks
    t.reset(doParse(recog, options, "hello[break]world"));
    a.checkEqual("151", toString(t.get()), "[root,[paragraph,[plain,'hello']],[break],[paragraph,[plain,'world']]]");
    t.reset(doParse(recog, options, "hello[nl]world"));
    a.checkEqual("152", toString(t.get()), "[root,[paragraph,[plain,'hello'],[br],[plain,'world']]]");

    // quote
    t.reset(doParse(recog, options, "hello[quote]world"));
    a.checkEqual("161", toString(t.get()), "[root,[paragraph,[plain,'hello']],[quote,[paragraph,[plain,'world']]]]");
    t.reset(doParse(recog, options, "hello[quote]new[/quote]world"));
    a.checkEqual("162", toString(t.get()), "[root,[paragraph,[plain,'hello']],[quote,[paragraph,[plain,'new']]],[paragraph,[plain,'world']]]");
    t.reset(doParse(recog, options, "hello[quote]new[*]world"));
    a.checkEqual("163", toString(t.get()), "[root,[paragraph,[plain,'hello']],[quote,[list,[list-item,[paragraph,[plain,'new']]],[list-item,[paragraph,[plain,'world']]]]]]");

    // invalid markup
    t.reset(doParse(recog, options, "hello, [color=notacolor]world"));
    a.checkEqual("171", toString(t.get()), "[root,[paragraph,[plain,'hello, [color=notacolor]world']]]");
    t.reset(doParse(recog, options, "hello, [frobnicate]world"));
    a.checkEqual("172", toString(t.get()), "[root,[paragraph,[plain,'hello, [frobnicate]world']]]");
    t.reset(doParse(recog, options, "hello, [*]world"));
    a.checkEqual("173", toString(t.get()), "[root,[paragraph,[plain,'hello, [*]world']]]");
    t.reset(doParse(recog, options, "hello, [size=ludicrous]world"));
    a.checkEqual("174", toString(t.get()), "[root,[paragraph,[plain,'hello, [size=ludicrous]world']]]");
    t.reset(doParse(recog, options, "hello, [size=99]world"));
    a.checkEqual("175", toString(t.get()), "[root,[paragraph,[plain,'hello, [size=99]world']]]");
    t.reset(doParse(recog, options, "hello, [font=\"a;b\"]world"));
    a.checkEqual("176", toString(t.get()), "[root,[paragraph,[plain,'hello, [font=\"a;b\"]world']]]");
    t.reset(doParse(recog, options, "hello[/quote]world"));
    a.checkEqual("177", toString(t.get()), "[root,[paragraph,[plain,'hello[/quote]world']]]");
}

/** Tests using InlineRecognizer. */
AFL_TEST("server.talk.parse.BBParser:inline", a)
{
    std::auto_ptr<TextNode> t;
    server::talk::InlineRecognizer recog;
    server::talk::InlineRecognizer::Kinds_t options;
    options += recog.Link;
    options += recog.Smiley;

    // Links
    t.reset(doParse(recog, options, "see http://link for more"));
    a.checkEqual("01", toString(t.get()), "[root,[paragraph,[plain,'see '],[link-url,'http://link'],[plain,' for more']]]");
    t.reset(doParse(recog, options, "see [url]http://link[/url] for more"));
    a.checkEqual("02", toString(t.get()), "[root,[paragraph,[plain,'see '],[link-url,'http://link'],[plain,' for more']]]");
    t.reset(doParse(recog, options, "see [url=http://link]the site at http://link[/url] for more"));
    a.checkEqual("03", toString(t.get()), "[root,[paragraph,[plain,'see '],[link-url,'http://link',[plain,'the site at http://link']],[plain,' for more']]]");

    // Smileys
    t.reset(doParse(recog, options, "this :-( sucks"));
    a.checkEqual("11", toString(t.get()), "[root,[paragraph,[plain,'this '],[smiley,'sad'],[plain,' sucks']]]");
    t.reset(doParse(recog, options, "this :sad: sucks"));
    a.checkEqual("12", toString(t.get()), "[root,[paragraph,[plain,'this '],[smiley,'sad'],[plain,' sucks']]]");
}

/** Test warnings. */
AFL_TEST("server.talk.parse.BBParser:warn", a)
{
    // Baseline
    a.checkEqual("01", doParseWarnings("[quote]hi[/quote]ho"), "");

    // Suspicious
    a.checkEqual("11", doParseWarnings("hi["), "SuspiciousText,[,,2");
    a.checkEqual("12", doParseWarnings("hi [whatever] ho"), "SuspiciousText,[whatever],,3");
    a.checkEqual("13", doParseWarnings("hi [/whatever] ho"), "SuspiciousText,[/whatever],,3");

    // MissingClose
    a.checkEqual("21", doParseWarnings("hello [b]world"), "MissingClose,,b,14");
    a.checkEqual("22", doParseWarnings("hello [b]world\n\n"), "MissingClose,\n\n,b,14");

    // TagNotOpen
    a.checkEqual("31", doParseWarnings("hello[/b] world"), "TagNotOpen,[/b],,5");

    // NoOwnText
    a.checkEqual("51", doParseWarnings("[quote]hi[/quote]"), "NoOwnText,,,17");
    a.checkEqual("52", doParseWarnings("[quote]hi"), "MissingClose,,quote,9|NoOwnText,,,9");
    a.checkEqual("53", doParseWarnings("[quote][b]hi"), "MissingClose,,b,12|NoOwnText,,,12");

    // BadLink - Game
    a.checkEqual("41", doParseWarnings("[game]5[/game]"), "");
    a.checkEqual("42", doParseWarnings("[game]bad_game[/game]"), "BadLink,[/game],bad_game,14");
    a.checkEqual("43", doParseWarnings("[game=bad_game]foo[/game]"), "BadLink,[/game],bad_game,18");

    // BadLink - Forum
    a.checkEqual("51", doParseWarnings("[forum]5[/forum]"), "");
    a.checkEqual("52", doParseWarnings("[forum]bad_forum[/forum]"), "BadLink,[/forum],bad_forum,16");
    a.checkEqual("53", doParseWarnings("[forum=bad_forum]foo[/forum]"), "BadLink,[/forum],bad_forum,20");

    // BadLink - Topic
    a.checkEqual("61", doParseWarnings("[thread]5[/thread]"), "");
    a.checkEqual("62", doParseWarnings("[thread]bad_topic[/thread]"), "BadLink,[/thread],bad_topic,17");
    a.checkEqual("63", doParseWarnings("[thread=bad_topic]foo[/thread]"), "BadLink,[/thread],bad_topic,21");

    // BadLink - Message
    a.checkEqual("71", doParseWarnings("[post]5[/post]"), "");
    a.checkEqual("72", doParseWarnings("[post]bad_message[/post]"), "BadLink,[/post],bad_message,17");
    a.checkEqual("73", doParseWarnings("[post=bad_message]foo[/post]"), "BadLink,[/post],bad_message,21");

    // BadLink - User
    a.checkEqual("81", doParseWarnings("[user]xx[/user]"), "");
    a.checkEqual("82", doParseWarnings("[user]bad_user[/user]"), "BadLink,[/user],bad_user,14");
    a.checkEqual("83", doParseWarnings("[user=bad_user]foo[/user]"), "BadLink,[/user],bad_user,18");
    a.checkEqual("84", doParseWarnings("hi @bad_user"), "BadLink,@bad_user,bad_user,3");
}
