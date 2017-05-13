/**
  *  \file u/t_util_syntax_scripthighlighter.cpp
  *  \brief Test for util::syntax::ScriptHighlighter
  */

#include "util/syntax/scripthighlighter.hpp"

#include "t_util_syntax.hpp"
#include "util/syntax/keywordtable.hpp"
#include "util/syntax/segment.hpp"

namespace {
    /** Parse a continuation segment.
        The highlighter makes no guarantee about the size of individual segments and may spit out many small segments of the same format.
        This function collects continuation segments.
        Structure of a test therefore is:
        - perform initial "scan" invocation
        - repeatedly,
          - verify the segment format
          - call parseContinuation() and verify the result text. This will leave the next segment in seg. */
    String_t parseContinuation(util::syntax::Highlighter& hl, util::syntax::Segment& seg)
    {
        String_t result = afl::string::fromMemory(seg.getText());
        util::syntax::Format fmt = seg.getFormat();
        while (hl.scan(seg) && seg.getFormat() == fmt) {
            result += afl::string::fromMemory(seg.getText());
        }
        return result;
    }
}

/** Test behaviour with strings. */
void
TestUtilSyntaxScriptHighlighter::testString()
{
    util::syntax::KeywordTable table;
    util::syntax::ScriptHighlighter testee(table);
    util::syntax::Segment r;

    // x := "foo\"mod" % xy"
    testee.init(afl::string::toMemory("x := \"foo\\\"mod\" % xy\""));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "x := ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::StringFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "\"foo\\\"mod\"");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::CommentFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "% xy\"");
    TS_ASSERT(!testee.scan(r));

    // y := 'foo\'mod' % xy'
    testee.init(afl::string::toMemory("y := 'foo\\'mod' % xy'"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "y := ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::StringFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "'foo\\'");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "mod");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::StringFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "' % xy'");
    TS_ASSERT(!testee.scan(r));
}

/** Test declaration commands. */
void
TestUtilSyntaxScriptHighlighter::testDeclarations()
{
    util::syntax::KeywordTable table;
    util::syntax::ScriptHighlighter testee(table);
    util::syntax::Segment r;

    // sub foo(bar, optional baz)
    testee.init(afl::string::toMemory("sub foo(bar, optional baz)"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "sub");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::NameFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "foo");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "(");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::NameFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "bar");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), ", ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "optional");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::NameFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "baz");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), ")");
    TS_ASSERT(!testee.scan(r));

    // local sub hurz()
    testee.init(afl::string::toMemory("local sub hurz()"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "local");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "sub");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::NameFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "hurz");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "()");
    TS_ASSERT(!testee.scan(r));

    // endsub
    testee.init(afl::string::toMemory("endsub"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "endsub");
    TS_ASSERT(!testee.scan(r));

    // dim local i
    testee.init(afl::string::toMemory("dim local i"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "dim");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "local");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::NameFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "i");
    TS_ASSERT(!testee.scan(r));

    // sub foo(bar(baz)) - the "baz" is not a name
    testee.init(afl::string::toMemory("sub foo(bar(baz))"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "sub");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::NameFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "foo");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "(");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::NameFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "bar");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "(baz))");
    TS_ASSERT(!testee.scan(r));
}

/** Test commands. */
void
TestUtilSyntaxScriptHighlighter::testCommands()
{
    util::syntax::KeywordTable table;
    util::syntax::ScriptHighlighter testee(table);
    util::syntax::Segment r;

    // if this then that
    testee.init(afl::string::toMemory("if this then that"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "if");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " this ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "then");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " that");
    TS_ASSERT(!testee.scan(r));

    // for i:=a to b do c
    testee.init(afl::string::toMemory("for i:=a to b do c"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "for");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " i:=a ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "to");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " b ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "do");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " c");
    TS_ASSERT(!testee.scan(r));

    // case is > 3
    testee.init(afl::string::toMemory("case is > 3"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "case");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "is");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " > 3");
    TS_ASSERT(!testee.scan(r));

    // a := b xor c
    testee.init(afl::string::toMemory("a := b xor c"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "a := b ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "xor");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " c");
    TS_ASSERT(!testee.scan(r));

    // what is love? baby dont hurt me -- "is" is not a keyword here, and the "?" should not confuse us
    testee.init(afl::string::toMemory("what is love? baby dont hurt me"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "what is love? baby dont hurt me");
    TS_ASSERT(!testee.scan(r));
}
