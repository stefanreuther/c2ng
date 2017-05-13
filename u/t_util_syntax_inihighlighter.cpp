/**
  *  \file u/t_util_syntax_inihighlighter.cpp
  *  \brief Test for util::syntax::IniHighlighter
  */

#include "util/syntax/inihighlighter.hpp"

#include "t_util_syntax.hpp"
#include "util/syntax/segment.hpp"
#include "util/syntax/keywordtable.hpp"

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

/** Test comments. */
void
TestUtilSyntaxIniHighlighter::testComments()
{
    util::syntax::KeywordTable tab;
    util::syntax::IniHighlighter testee(tab, "x");
    util::syntax::Segment r;

    // Single comment
    testee.init(afl::string::toMemory(" # x"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::CommentFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "# x");
    TS_ASSERT(!testee.scan(r));

    // Single comment + newline
    testee.init(afl::string::toMemory(" # x\n"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::CommentFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "# x");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "\n");
    TS_ASSERT(!testee.scan(r));

    // Section comment
    testee.init(afl::string::toMemory(" ## x"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::Comment2Format);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "## x");
    TS_ASSERT(!testee.scan(r));

    // Single comment with semicolon
    testee.init(afl::string::toMemory(" ; x"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::CommentFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "; x");
    TS_ASSERT(!testee.scan(r));

    // Section comment with semicolon
    testee.init(afl::string::toMemory(" ;; x"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::Comment2Format);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), ";; x");
    TS_ASSERT(!testee.scan(r));

    // Variants...
    testee.init(afl::string::toMemory(" ;# x"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::CommentFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), ";# x");
    TS_ASSERT(!testee.scan(r));

    testee.init(afl::string::toMemory("#x"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::CommentFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "#x");
    TS_ASSERT(!testee.scan(r));
}

/** Test sections. */
void
TestUtilSyntaxIniHighlighter::testSections()
{
    util::syntax::KeywordTable tab;
    util::syntax::IniHighlighter testee(tab, "x");
    util::syntax::Segment r;

    // Brackets
    testee.init(afl::string::toMemory("[foo]"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::SectionFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "[foo]");
    TS_ASSERT(!testee.scan(r));

    // ...with newline
    testee.init(afl::string::toMemory("[foo]\n"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::SectionFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "[foo]");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "\n");
    TS_ASSERT(!testee.scan(r));

    // ...indented
    testee.init(afl::string::toMemory("    [foo]"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "    ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::SectionFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "[foo]");
    TS_ASSERT(!testee.scan(r));

    // ...with comment
    testee.init(afl::string::toMemory("[foo]#bar"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::SectionFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "[foo]");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::CommentFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "#bar");
    TS_ASSERT(!testee.scan(r));

    // ...with space and comment
    testee.init(afl::string::toMemory("[foo]  #bar"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::SectionFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "[foo]");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "  ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::CommentFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "#bar");
    TS_ASSERT(!testee.scan(r));

    // ...with garbage
    testee.init(afl::string::toMemory("[foo] bar"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::SectionFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "[foo]");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " bar");
    TS_ASSERT(!testee.scan(r));

    // ...with more garbage
    testee.init(afl::string::toMemory("[foo] bar ; baz"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::SectionFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "[foo]");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " bar ; baz");
    TS_ASSERT(!testee.scan(r));

    // Percent
    testee.init(afl::string::toMemory("%foo"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::SectionFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "%foo");
    TS_ASSERT(!testee.scan(r));

    // ...with newline
    testee.init(afl::string::toMemory("%foo\n"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::SectionFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "%foo");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "\n");
    TS_ASSERT(!testee.scan(r));

    // ...indented
    testee.init(afl::string::toMemory("    %foo"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "    ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::SectionFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "%foo");
    TS_ASSERT(!testee.scan(r));

    // ...with comment
    testee.init(afl::string::toMemory("[foo]#bar"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::SectionFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "[foo]");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::CommentFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "#bar");
    TS_ASSERT(!testee.scan(r));

    // ...with space and comment
    testee.init(afl::string::toMemory("%foo  #bar"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::SectionFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "%foo");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "  ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::CommentFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "#bar");
    TS_ASSERT(!testee.scan(r));

    // ...with garbage
    testee.init(afl::string::toMemory("%foo bar"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::SectionFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "%foo");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " bar");
    TS_ASSERT(!testee.scan(r));

    // ...with more garbage
    testee.init(afl::string::toMemory("%foo bar ; baz"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::SectionFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "%foo");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " bar ; baz");
    TS_ASSERT(!testee.scan(r));
}

    //     } else if (skip(m_text, cOther | cLBracket | cRBracket)) {
    //         // Name. Accept [] as well for things like "foo[1] = ...", maybe someone uses that.
    //         // FIXME? If a word stands alone on a line, we do not highlight it, but if it has trailing space, we do.
    //         // PlanetsCentral does this, do we want to keep it?
    //         if (skip1(m_text, cNewline)) {
    //             result.finish(DefaultFormat, m_text);
    //         } else {
    //             result.finish(NameFormat, m_text);
    //             m_state = sAfterName;

    //             // Links
    //             String_t key = afl::string::strLTrim(afl::string::fromMemory(result.getText()));

    //             String_t pfx = "ini.";
    //             String_t::size_type dot = key.find('.');
    //             if (dot != String_t::npos
    //                 && (m_section.empty()
    //                     || (dot == m_section.size() && key.compare(0, dot, m_section) == 0)))
    //             {
    //                 pfx += key;
    //             } else {
    //                 pfx += m_section;
    //                 pfx += ".";
    //                 pfx += key;
    //             }
    //             if (const String_t* link = m_table.get(pfx + ".link")) {
    //                 result.setLink(*link);
    //             }
    //             if (const String_t* info = m_table.get(pfx + ".info")) {
    //                 result.setInfo(*info);
    //             }
    //         }
    //     } else {
    //         // Don't know what it is. Skip the whole line.
    //         skip(m_text, ~cNewline);
    //         skip1(m_text, cNewline);
    //         result.finish(DefaultFormat, m_text);
    //     }
    //     break;

/** Test assignments. */
void
TestUtilSyntaxIniHighlighter::testAssignment()
{
    util::syntax::KeywordTable tab;
    util::syntax::IniHighlighter testee(tab, "a");
    util::syntax::Segment r;

    // Preload the table
    tab.add("ini.foo.f1.link", "first link");
    tab.add("ini.foo.f2.link", "second link");
    tab.add("ini.foo.f2.info", "second info");
    tab.add("ini.a.x.info", "ex info");
    tab.add("ini.a.y[2].info", "array info");

    // Assignments in section a
    testee.init(afl::string::toMemory("x = hi"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::NameFormat);
    TS_ASSERT_EQUALS(r.getInfo(), "ex info");
    TS_ASSERT_EQUALS(r.getLink(), "");
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "x");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(r.getInfo(), "");
    TS_ASSERT_EQUALS(r.getLink(), "");
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " = hi");
    TS_ASSERT(!testee.scan(r));

    // ...with array
    testee.init(afl::string::toMemory("  y[2] = ho"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "  ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::NameFormat);
    TS_ASSERT_EQUALS(r.getInfo(), "array info");
    TS_ASSERT_EQUALS(r.getLink(), "");
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "y[2]");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " = ho");
    TS_ASSERT(!testee.scan(r));

    // ...with comment
    testee.init(afl::string::toMemory("x = hi # ok"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::NameFormat);
    TS_ASSERT_EQUALS(r.getInfo(), "ex info");
    TS_ASSERT_EQUALS(r.getLink(), "");
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "x");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(r.getInfo(), "");
    TS_ASSERT_EQUALS(r.getLink(), "");
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " = hi # ok");
    TS_ASSERT(!testee.scan(r));

    // ...unknown
    testee.init(afl::string::toMemory("yy = 3"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::NameFormat);
    TS_ASSERT_EQUALS(r.getInfo(), "");
    TS_ASSERT_EQUALS(r.getLink(), "");
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "yy");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " = 3");
    TS_ASSERT(!testee.scan(r));

    // ...namespaced
    testee.init(afl::string::toMemory("a.x = ax"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::NameFormat);
    TS_ASSERT_EQUALS(r.getInfo(), "ex info");
    TS_ASSERT_EQUALS(r.getLink(), "");
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "a.x");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(r.getInfo(), "");
    TS_ASSERT_EQUALS(r.getLink(), "");
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " = ax");
    TS_ASSERT(!testee.scan(r));

    // ...capitalized namespaced
    testee.init(afl::string::toMemory("A.x = ax"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::NameFormat);
    TS_ASSERT_EQUALS(r.getInfo(), "ex info");
    TS_ASSERT_EQUALS(r.getLink(), "");
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "A.x");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(r.getInfo(), "");
    TS_ASSERT_EQUALS(r.getLink(), "");
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " = ax");
    TS_ASSERT(!testee.scan(r));

    // Elsewhere
    testee.init(afl::string::toMemory("foo.f1 = fx"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::NameFormat);
    TS_ASSERT_EQUALS(r.getInfo(), "");                           // not found because we're in section a!
    TS_ASSERT_EQUALS(r.getLink(), "");
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "foo.f1");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " = fx");
    TS_ASSERT(!testee.scan(r));

    // Elsewhere with delimiter
    testee.init(afl::string::toMemory("%foo\nf1 = fx"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::SectionFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "%foo");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "\n");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::NameFormat);
    TS_ASSERT_EQUALS(r.getInfo(), "");
    TS_ASSERT_EQUALS(r.getLink(), "first link");
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "f1");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " = fx");
    TS_ASSERT(!testee.scan(r));

    // Elsewhere with delimiter + namespace
    testee.init(afl::string::toMemory("%foo\nfoo.f2 = fy"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::SectionFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "%foo");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "\n");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::NameFormat);
    TS_ASSERT_EQUALS(r.getInfo(), "second info");
    TS_ASSERT_EQUALS(r.getLink(), "second link");
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "foo.f2");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " = fy");
    TS_ASSERT(!testee.scan(r));
}
