/**
  *  \file test/game/config/expressionliststest.cpp
  *  \brief Test for game::config::ExpressionLists
  */

#include "game/config/expressionlists.hpp"

#include "afl/io/constmemorystream.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "util/io.hpp"

using afl::io::FileSystem;
using game::config::ExpressionLists;
using util::ExpressionList;

AFL_TEST("game.config.ExpressionLists:access", a)
{
    ExpressionLists testee;
    const ExpressionLists& ct = testee;

    // Verify get()
    ExpressionList* p = testee.get(ExpressionLists::ShipLabels, ExpressionLists::Recent);
    const ExpressionList* pc = ct.get(ExpressionLists::ShipLabels, ExpressionLists::Recent);
    a.checkEqual("01. get", p, pc);
    a.checkNonNull("02. get", p);
    a.check("03. unique", p != testee.get(ExpressionLists::ShipLabels, ExpressionLists::Predefined));
    a.check("04. unique", p != testee.get(ExpressionLists::PlanetLabels, ExpressionLists::Recent));

    // Use it
    p->pushBackNew(new ExpressionList::Item("a", "[b]", "c"));
    a.check("11. empty", !p->empty());

    // Pack
    afl::string::NullTranslator tx;
    ExpressionLists::Items_t list;
    testee.pack(list, ExpressionLists::ShipLabels, tx);
    a.checkEqual("21. size", list.size(), 1U);
    a.checkEqual("22. name", list[0].name, "a");
    a.checkEqual("23. flags", list[0].flags, "[b]");
    a.checkEqual("24. value", list[0].value, "c");
    a.checkEqual("25. isHeading", list[0].isHeading, false);

    // clear()
    testee.clear();

    // Note that we do not guarantee the pointers to be long-term stable!
    p = testee.get(ExpressionLists::ShipLabels, ExpressionLists::Recent);
    a.check("31. empty", p->empty());
}

AFL_TEST("game.config.ExpressionLists:pack", a)
{
    ExpressionLists testee;

    // Verify get()
    ExpressionList* r = testee.get(ExpressionLists::ShipLabels, ExpressionLists::Recent);
    r->pushBackNew(new ExpressionList::Item("recent1", "[b]", "value r1"));
    r->pushBackNew(new ExpressionList::Item("recent2", "[c]", "value r2"));

    ExpressionList* p = testee.get(ExpressionLists::ShipLabels, ExpressionLists::Predefined);
    p->pushBackNew(new ExpressionList::Item("predef 1", "[x]", "value p1"));
    p->pushBackNew(new ExpressionList::Item("predef 2", "[y]", "value p2"));

    // Pack
    afl::string::NullTranslator tx;
    ExpressionLists::Items_t list;
    testee.pack(list, ExpressionLists::ShipLabels, tx);
    a.checkEqual("01", list.size(), 6U);

    a.checkEqual("11. name", list[0].name, "Last expressions");
    a.checkEqual("12. isHeading", list[0].isHeading, true);

    a.checkEqual("21. name", list[1].name, "recent1");
    a.checkEqual("22. flags", list[1].flags, "[b]");
    a.checkEqual("23. value", list[1].value, "value r1");
    a.checkEqual("24. isHeading", list[1].isHeading, false);

    a.checkEqual("31. name", list[2].name, "recent2");
    a.checkEqual("32. flags", list[2].flags, "[c]");
    a.checkEqual("33. value", list[2].value, "value r2");
    a.checkEqual("34. isHeading", list[2].isHeading, false);

    a.checkEqual("41. name", list[3].name, "Predefined expressions");
    a.checkEqual("42. isHeading", list[3].isHeading, true);

    a.checkEqual("51. name", list[4].name, "predef 1");
    a.checkEqual("52. flags", list[4].flags, "[x]");
    a.checkEqual("53. value", list[4].value, "value p1");
    a.checkEqual("54. isHeading", list[4].isHeading, false);

    a.checkEqual("61. name", list[5].name, "predef 2");
    a.checkEqual("62. flags", list[5].flags, "[y]");
    a.checkEqual("63. value", list[5].value, "value p2");
    a.checkEqual("64. isHeading", list[5].isHeading, false);
}

AFL_TEST("game.config.ExpressionLists:loadRecentFiles", a)
{
    // Test data
    const char*const LRU_INI =
        "[shiplabels]\n"
        "; ignore   me\n"
        "The Name  Name\n"
        "Not Id  [!] Id\n"
        "[other]\n"
        "....\n"
        "[PlanetLabels]\n"
        "Planet Name    Name\n";

    // Environment
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    afl::io::InternalFileSystem fs;
    afl::sys::InternalEnvironment env;
    fs.createDirectory("/profile");
    fs.openFile("/profile/lru.ini", FileSystem::Create)->fullWrite(afl::string::toBytes(LRU_INI));
    env.setSettingsDirectoryName("/profile");

    util::ProfileDirectory profile(env, fs);

    // Testee
    game::config::ExpressionLists testee;
    testee.loadRecentFiles(profile, log, tx);

    // Verify
    const ExpressionList* s = testee.get(ExpressionLists::ShipLabels, ExpressionLists::Recent);
    a.checkNonNull("01. get", s);
    a.checkEqual("02. size", s->size(), 2U);
    a.checkEqual("03. name", s->get(0)->name, "The Name");
    a.checkEqual("04. flags", s->get(0)->flags, "");
    a.checkEqual("05. value", s->get(0)->value, "Name");
    a.checkEqual("06. name", s->get(1)->name, "Not Id");
    a.checkEqual("07. flags", s->get(1)->flags, "[!]");
    a.checkEqual("08. value", s->get(1)->value, "Id");

    const ExpressionList* p = testee.get(ExpressionLists::PlanetLabels, ExpressionLists::Recent);
    a.checkNonNull("11. get", p);
    a.checkEqual("12. size", p->size(), 1U);
    a.checkEqual("13. name", p->get(0)->name, "Planet Name");
    a.checkEqual("14. flags", p->get(0)->flags, "");
    a.checkEqual("15. value", p->get(0)->value, "Name");
}

AFL_TEST("game.config.ExpressionLists:loadPredefinedFiles", a)
{
    // Test data
    const char*const EXPR_INI =
        "[shiplabels]\n"
        "label 1    expr 1\n"
        "label 2    expr 2\n"
        "[find]\n"
        "find 1     [abc] find 1\n";

    const char*const EXPR_CC =
        "[shiplabels]\n"
        "user label   user\n"
        "[find]\n"
        "user find a   [xy] find a\n"
        "user find b   []find b\n"
        "[planetlabels]\n"
        "p   l\n";

    afl::base::Ref<afl::io::InternalDirectory> gameDir(afl::io::InternalDirectory::create("game"));
    gameDir->addStream("expr.cc", *new afl::io::ConstMemoryStream(afl::string::toBytes(EXPR_CC)));

    // Environment
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    afl::io::InternalFileSystem fs;
    afl::sys::InternalEnvironment env;
    fs.createDirectory("/profile");
    fs.openFile("/profile/expr.ini", FileSystem::Create)->fullWrite(afl::string::toBytes(EXPR_INI));
    env.setSettingsDirectoryName("/profile");

    util::ProfileDirectory profile(env, fs);

    // Testee
    game::config::ExpressionLists testee;
    testee.loadPredefinedFiles(profile, *gameDir, log, tx);

    // Verify
    const ExpressionList* s = testee.get(ExpressionLists::ShipLabels, ExpressionLists::Predefined);
    a.checkNonNull("01. get", s);
    a.checkEqual("02. size",  s->size(), 3U);
    a.checkEqual("03. name",  s->get(0)->name, "label 1");
    a.checkEqual("04. flags", s->get(0)->flags, "");
    a.checkEqual("05. value", s->get(0)->value, "expr 1");
    a.checkEqual("06. name",  s->get(1)->name, "label 2");
    a.checkEqual("07. flags", s->get(1)->flags, "");
    a.checkEqual("08. value", s->get(1)->value, "expr 2");
    a.checkEqual("09. name",  s->get(2)->name, "user label");
    a.checkEqual("10. flags", s->get(2)->flags, "");
    a.checkEqual("11. value", s->get(2)->value, "user");

    const ExpressionList* p = testee.get(ExpressionLists::PlanetLabels, ExpressionLists::Predefined);
    a.checkNonNull("21. get", p);
    a.checkEqual("22. size",  p->size(), 1U);
    a.checkEqual("23. name",  p->get(0)->name, "p");
    a.checkEqual("24. flags", p->get(0)->flags, "");
    a.checkEqual("25. value", p->get(0)->value, "l");

    const ExpressionList* f = testee.get(ExpressionLists::Search, ExpressionLists::Predefined);
    a.checkNonNull("31. get", f);
    a.checkEqual("32. size",  f->size(), 3U);
    a.checkEqual("33. name",  f->get(0)->name, "find 1");
    a.checkEqual("34. flags", f->get(0)->flags, "[abc]");
    a.checkEqual("35. value", f->get(0)->value, "find 1");
    a.checkEqual("36. name",  f->get(1)->name, "user find a");
    a.checkEqual("37. flags", f->get(1)->flags, "[xy]");
    a.checkEqual("38. value", f->get(1)->value, "find a");
    a.checkEqual("39. name",  f->get(2)->name, "user find b");
    a.checkEqual("40. flags", f->get(2)->flags, "[]");
    a.checkEqual("41. value", f->get(2)->value, "find b");
}

AFL_TEST("game.config.ExpressionLists:saveRecentFiles", a)
{
    // Environment
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    afl::io::InternalFileSystem fs;
    afl::sys::InternalEnvironment env;
    env.setSettingsDirectoryName("/profile");     // Will be auto-created!

    util::ProfileDirectory profile(env, fs);

    // Testee
    game::config::ExpressionLists testee;
    ExpressionList* s = testee.get(ExpressionLists::ShipLabels, ExpressionLists::Recent);
    s->pushBackNew(new ExpressionList::Item("a  b  c", "", "xyz"));
    s->pushBackNew(new ExpressionList::Item("a b c", "[!]", "123"));
    testee.saveRecentFiles(profile, log, tx);

    // Verify
    afl::base::Ref<afl::io::Directory> profileDir(fs.openDirectory("/profile"));
    afl::base::Ref<afl::io::Stream> file = profileDir->openFile("lru.ini", FileSystem::OpenRead);
    String_t content = util::normalizeLinefeeds(file->createVirtualMapping()->get());
    a.checkEqual("content", content,
                 "[SHIPLABELS]\n"
                 "a b c  xyz\n"
                 "a b c  [!]123\n"
                 "\n");
}
