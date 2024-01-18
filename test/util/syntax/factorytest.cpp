/**
  *  \file test/util/syntax/factorytest.cpp
  *  \brief Test for util::syntax::Factory
  */

#include "util/syntax/factory.hpp"

#include "afl/test/testrunner.hpp"
#include "util/syntax/chighlighter.hpp"
#include "util/syntax/inihighlighter.hpp"
#include "util/syntax/keywordtable.hpp"
#include "util/syntax/lisphighlighter.hpp"
#include "util/syntax/nullhighlighter.hpp"
#include "util/syntax/pascalhighlighter.hpp"
#include "util/syntax/scripthighlighter.hpp"

/** Simple test. */
AFL_TEST("util.syntax.Factory", a)
{
    util::syntax::KeywordTable tab;
    util::syntax::Factory testee(tab);
    afl::base::Deleter del;

    // Test all names
    a.checkNonNull("01", dynamic_cast<util::syntax::IniHighlighter*>(&testee.create("pconfig.src", del)));
    a.checkNonNull("02", dynamic_cast<util::syntax::IniHighlighter*>(&testee.create("shiplist.txt", del)));
    a.checkNonNull("03", dynamic_cast<util::syntax::IniHighlighter*>(&testee.create("hullfunc.txt", del)));
    a.checkNonNull("04", dynamic_cast<util::syntax::IniHighlighter*>(&testee.create("amaster.src", del)));
    a.checkNonNull("05", dynamic_cast<util::syntax::IniHighlighter*>(&testee.create("pmaster.cfg", del)));
    a.checkNonNull("06", dynamic_cast<util::syntax::IniHighlighter*>(&testee.create("explmap.cfg", del)));
    a.checkNonNull("07", dynamic_cast<util::syntax::IniHighlighter*>(&testee.create("map.ini", del)));
    a.checkNonNull("08", dynamic_cast<util::syntax::IniHighlighter*>(&testee.create("whatever.ini", del)));
    a.checkNonNull("09", dynamic_cast<util::syntax::IniHighlighter*>(&testee.create("whatever.cfg", del)));
    a.checkNonNull("10", dynamic_cast<util::syntax::IniHighlighter*>(&testee.create("ini", del)));
    a.checkNonNull("11", dynamic_cast<util::syntax::ScriptHighlighter*>(&testee.create("file.q", del)));
    a.checkNonNull("12", dynamic_cast<util::syntax::ScriptHighlighter*>(&testee.create("file.ccscript", del)));
    a.checkNonNull("13", dynamic_cast<util::syntax::ScriptHighlighter*>(&testee.create("ccscript", del)));
    a.checkNonNull("14", dynamic_cast<util::syntax::CHighlighter*>(&testee.create("c", del)));
    a.checkNonNull("15", dynamic_cast<util::syntax::CHighlighter*>(&testee.create("c++", del)));
    a.checkNonNull("16", dynamic_cast<util::syntax::CHighlighter*>(&testee.create("file.c++", del)));
    a.checkNonNull("17", dynamic_cast<util::syntax::CHighlighter*>(&testee.create("file.cxx", del)));
    a.checkNonNull("18", dynamic_cast<util::syntax::CHighlighter*>(&testee.create("file.cc", del)));
    a.checkNonNull("19", dynamic_cast<util::syntax::CHighlighter*>(&testee.create("file.cpp", del)));
    a.checkNonNull("20", dynamic_cast<util::syntax::CHighlighter*>(&testee.create("file.h++", del)));
    a.checkNonNull("21", dynamic_cast<util::syntax::CHighlighter*>(&testee.create("file.hxx", del)));
    a.checkNonNull("22", dynamic_cast<util::syntax::CHighlighter*>(&testee.create("file.hh", del)));
    a.checkNonNull("23", dynamic_cast<util::syntax::CHighlighter*>(&testee.create("file.hpp", del)));
    a.checkNonNull("24", dynamic_cast<util::syntax::CHighlighter*>(&testee.create("file.h", del)));
    a.checkNonNull("25", dynamic_cast<util::syntax::CHighlighter*>(&testee.create("java", del)));
    a.checkNonNull("26", dynamic_cast<util::syntax::CHighlighter*>(&testee.create("file.java", del)));
    a.checkNonNull("27", dynamic_cast<util::syntax::CHighlighter*>(&testee.create("foo.js", del)));
    a.checkNonNull("28", dynamic_cast<util::syntax::CHighlighter*>(&testee.create("foo.as", del)));
    a.checkNonNull("29", dynamic_cast<util::syntax::CHighlighter*>(&testee.create("javascript", del)));
    a.checkNonNull("30", dynamic_cast<util::syntax::CHighlighter*>(&testee.create("jscript", del)));
    a.checkNonNull("31", dynamic_cast<util::syntax::PascalHighlighter*>(&testee.create("foo.pas", del)));
    a.checkNonNull("32", dynamic_cast<util::syntax::LispHighlighter*>(&testee.create("foo.el", del)));
    a.checkNonNull("33", dynamic_cast<util::syntax::NullHighlighter*>(&testee.create("x.bas", del)));
    a.checkNonNull("34", dynamic_cast<util::syntax::NullHighlighter*>(&testee.create("x.xls", del)));

    // Variations
    a.checkNonNull("41", dynamic_cast<util::syntax::IniHighlighter*>(&testee.create("pmaster.cfg.frag", del)));
    a.checkNonNull("42", dynamic_cast<util::syntax::IniHighlighter*>(&testee.create("PMASTER.CFG", del)));
}
