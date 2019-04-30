/**
  *  \file u/t_server_talk_parse_bbparser.cpp
  *  \brief Test for server::talk::parse::BBParser
  */

#include <memory>
#include "server/talk/parse/bbparser.hpp"

#include "t_server_talk_parse.hpp"
#include "server/talk/inlinerecognizer.hpp"

using server::talk::TextNode;

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
        server::talk::parse::BBLexer lex(text);
        server::talk::parse::BBParser parser(lex, recog, options);
        return parser.parse();
    }
}

/** Some basic tests. */
void
TestServerTalkParseBBParser::testIt()
{
    std::auto_ptr<TextNode> t;
    server::talk::InlineRecognizer recog;
    server::talk::InlineRecognizer::Kinds_t options;  // no options for now

    // plain text
    t.reset(doParse(recog, options, "hello, world"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello, world']]]");

    // two paragraphs
    t.reset(doParse(recog, options, "hello, world\n\n\n\ngood bye"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello, world']],[paragraph,[plain,'good bye']]]");
    t.reset(doParse(recog, options, "hello, world[center]good bye[/center]"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello, world']],[centered,[plain,'good bye']]]");

    // regular inline markup
    t.reset(doParse(recog, options, "hello, [b]world[/b]"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello, '],[inline-bold,[plain,'world']]]]");

    // regular inline markup missing end
    t.reset(doParse(recog, options, "hello, [b]world"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello, '],[inline-bold,[plain,'world']]]]");

    // inline markup missing start
    t.reset(doParse(recog, options, "hello,[/b] world"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[inline-bold,[plain,'hello,']],[plain,' world']]]");

    // color (various formats)
    t.reset(doParse(recog, options, "hello, [color=red]world"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello, '],[ia-color,'#ff0000',[plain,'world']]]]");
    t.reset(doParse(recog, options, "hello, [color=#123]world"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello, '],[ia-color,'#112233',[plain,'world']]]]");
    t.reset(doParse(recog, options, "hello, [color=123]world"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello, '],[ia-color,'#112233',[plain,'world']]]]");
    t.reset(doParse(recog, options, "hello, [color=#ABCDEF]world"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello, '],[ia-color,'#abcdef',[plain,'world']]]]");
    t.reset(doParse(recog, options, "hello, [color=Abcdef]world"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello, '],[ia-color,'#abcdef',[plain,'world']]]]");

    // size (various formats)
    t.reset(doParse(recog, options, "hello, [size=3]world"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello, '],[ia-size,'-2',[plain,'world']]]]");
    t.reset(doParse(recog, options, "hello, [size=+1]world"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello, '],[ia-size,'+1',[plain,'world']]]]");
    t.reset(doParse(recog, options, "hello, [size=-1]world"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello, '],[ia-size,'-1',[plain,'world']]]]");

    // font
    t.reset(doParse(recog, options, "hello, [font=courier]world"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello, '],[ia-font,'courier',[plain,'world']]]]");
    t.reset(doParse(recog, options, "hello, [font=\"Times Roman\"]world"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello, '],[ia-font,'Times Roman',[plain,'world']]]]");

    // links
    t.reset(doParse(recog, options, "hello @user there"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello '],[link-user,'user'],[plain,' there']]]");
    t.reset(doParse(recog, options, "hello [user]jj[/user] there"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello '],[link-user,'jj'],[plain,' there']]]");
    t.reset(doParse(recog, options, "hello [user=jj][/user] there"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello '],[link-user,'jj'],[plain,' there']]]");
    t.reset(doParse(recog, options, "hello [user=jj]xx[/user] there"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello '],[link-user,'jj',[plain,'xx']],[plain,' there']]]");

    // link with markup
    t.reset(doParse(recog, options, "[user][b]f[/b]runo[/user]"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[link-user,'fruno',[inline-bold,[plain,'f']],[plain,'runo']]]]");

    // nested links
    t.reset(doParse(recog, options, "[game=1]a [thread=2]b[/thread] c[/game]"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[link-game,'1',[plain,'a ']],[link-thread,'2',[plain,'b']],[plain,' c[/game]']]]");
    t.reset(doParse(recog, options, "[game=1]a @user c[/game]"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[link-game,'1',[plain,'a ']],[link-user,'user'],[plain,' c[/game]']]]");

    // noparse
    t.reset(doParse(recog, options, "hello [noparse][b]hi[/noparse][b]ho"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello [b]hi'],[inline-bold,[plain,'ho']]]]");
    t.reset(doParse(recog, options, "a[noparse][/[/noparse]noparse]b"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'a[/noparse]b']]]");
    t.reset(doParse(recog, options, "a[noparse][noparse][/noparse]b"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'a[noparse]b']]]");

    // list
    t.reset(doParse(recog, options, "a[list][*]b[*]c[/list]d"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'a']],[list,[list-item,[paragraph,[plain,'b']]],[list-item,[paragraph,[plain,'c']]]],[paragraph,[plain,'d']]]");
    t.reset(doParse(recog, options, "a[list=1][*]b[*]c[/list]d"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'a']],[list,'1',[list-item,[paragraph,[plain,'b']]],[list-item,[paragraph,[plain,'c']]]],[paragraph,[plain,'d']]]");

    // smiley (with tag)
    t.reset(doParse(recog, options, "a [:smile:] b"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'a '],[smiley,'smile'],[plain,' b']]]");

    // code
    t.reset(doParse(recog, options, "hello [code=c]static int a[b];[/code][b]ho"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello ']],[code,'c',[plain,'static int a[b];']],[paragraph,[inline-bold,[plain,'ho']]]]");

    // breaks
    t.reset(doParse(recog, options, "hello[break]world"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello']],[break],[paragraph,[plain,'world']]]");
    t.reset(doParse(recog, options, "hello[nl]world"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello'],[br],[plain,'world']]]");

    // quote
    t.reset(doParse(recog, options, "hello[quote]world"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello']],[quote,[paragraph,[plain,'world']]]]");
    t.reset(doParse(recog, options, "hello[quote]new[/quote]world"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello']],[quote,[paragraph,[plain,'new']]],[paragraph,[plain,'world']]]");
    t.reset(doParse(recog, options, "hello[quote]new[*]world"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello']],[quote,[list,[list-item,[paragraph,[plain,'new']]],[list-item,[paragraph,[plain,'world']]]]]]");

    // invalid markup
    t.reset(doParse(recog, options, "hello, [color=notacolor]world"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello, [color=notacolor]world']]]");
    t.reset(doParse(recog, options, "hello, [frobnicate]world"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello, [frobnicate]world']]]");
    t.reset(doParse(recog, options, "hello, [*]world"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello, [*]world']]]");
    t.reset(doParse(recog, options, "hello, [size=ludicrous]world"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello, [size=ludicrous]world']]]");
    t.reset(doParse(recog, options, "hello, [size=99]world"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello, [size=99]world']]]");
    t.reset(doParse(recog, options, "hello, [font=\"a;b\"]world"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello, [font=\"a;b\"]world']]]");
    t.reset(doParse(recog, options, "hello[/quote]world"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'hello[/quote]world']]]");
}

/** Tests using InlineRecognizer. */
void
TestServerTalkParseBBParser::testRecog()
{
    std::auto_ptr<TextNode> t;
    server::talk::InlineRecognizer recog;
    server::talk::InlineRecognizer::Kinds_t options;
    options += recog.Link;
    options += recog.Smiley;

    // Links
    t.reset(doParse(recog, options, "see http://link for more"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'see '],[link-url,'http://link'],[plain,' for more']]]");
    t.reset(doParse(recog, options, "see [url]http://link[/url] for more"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'see '],[link-url,'http://link'],[plain,' for more']]]");
    t.reset(doParse(recog, options, "see [url=http://link]the site at http://link[/url] for more"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'see '],[link-url,'http://link',[plain,'the site at http://link']],[plain,' for more']]]");

    // Smileys
    t.reset(doParse(recog, options, "this :-( sucks"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'this '],[smiley,'sad'],[plain,' sucks']]]");
    t.reset(doParse(recog, options, "this :sad: sucks"));
    TS_ASSERT_EQUALS(toString(t.get()), "[root,[paragraph,[plain,'this '],[smiley,'sad'],[plain,' sucks']]]");
}

