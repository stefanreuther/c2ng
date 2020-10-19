/**
  *  \file u/t_util_syntax_factory.cpp
  *  \brief Test for util::syntax::Factory
  */

#include "util/syntax/factory.hpp"

#include "t_util_syntax.hpp"
#include "util/syntax/chighlighter.hpp"
#include "util/syntax/inihighlighter.hpp"
#include "util/syntax/keywordtable.hpp"
#include "util/syntax/lisphighlighter.hpp"
#include "util/syntax/nullhighlighter.hpp"
#include "util/syntax/pascalhighlighter.hpp"
#include "util/syntax/scripthighlighter.hpp"

/** Simple test. */
void
TestUtilSyntaxFactory::testIt()
{
    util::syntax::KeywordTable tab;
    util::syntax::Factory testee(tab);
    afl::base::Deleter del;

    // Test all names
    TS_ASSERT(dynamic_cast<util::syntax::IniHighlighter*>(&testee.create("pconfig.src", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::IniHighlighter*>(&testee.create("shiplist.txt", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::IniHighlighter*>(&testee.create("hullfunc.txt", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::IniHighlighter*>(&testee.create("amaster.src", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::IniHighlighter*>(&testee.create("pmaster.cfg", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::IniHighlighter*>(&testee.create("explmap.cfg", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::IniHighlighter*>(&testee.create("map.ini", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::IniHighlighter*>(&testee.create("whatever.ini", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::IniHighlighter*>(&testee.create("whatever.cfg", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::IniHighlighter*>(&testee.create("ini", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::ScriptHighlighter*>(&testee.create("file.q", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::ScriptHighlighter*>(&testee.create("file.ccscript", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::ScriptHighlighter*>(&testee.create("ccscript", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::CHighlighter*>(&testee.create("c", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::CHighlighter*>(&testee.create("c++", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::CHighlighter*>(&testee.create("file.c++", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::CHighlighter*>(&testee.create("file.cxx", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::CHighlighter*>(&testee.create("file.cc", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::CHighlighter*>(&testee.create("file.cpp", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::CHighlighter*>(&testee.create("file.h++", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::CHighlighter*>(&testee.create("file.hxx", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::CHighlighter*>(&testee.create("file.hh", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::CHighlighter*>(&testee.create("file.hpp", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::CHighlighter*>(&testee.create("file.h", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::CHighlighter*>(&testee.create("java", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::CHighlighter*>(&testee.create("file.java", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::CHighlighter*>(&testee.create("foo.js", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::CHighlighter*>(&testee.create("foo.as", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::CHighlighter*>(&testee.create("javascript", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::CHighlighter*>(&testee.create("jscript", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::PascalHighlighter*>(&testee.create("foo.pas", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::LispHighlighter*>(&testee.create("foo.el", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::NullHighlighter*>(&testee.create("x.bas", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::NullHighlighter*>(&testee.create("x.xls", del)) != 0);

    // Variations
    TS_ASSERT(dynamic_cast<util::syntax::IniHighlighter*>(&testee.create("pmaster.cfg.frag", del)) != 0);
    TS_ASSERT(dynamic_cast<util::syntax::IniHighlighter*>(&testee.create("PMASTER.CFG", del)) != 0);
}
