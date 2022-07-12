/**
  *  \file u/t_game_config_expressionlists.cpp
  *  \brief Test for game::config::ExpressionLists
  */

#include "game/config/expressionlists.hpp"

#include "t_game_config.hpp"
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

using afl::io::FileSystem;
using game::config::ExpressionLists;
using util::ExpressionList;

void
TestGameConfigExpressionLists::testAccess()
{
    ExpressionLists testee;
    const ExpressionLists& ct = testee;

    // Verify get()
    ExpressionList* p = testee.get(ExpressionLists::ShipLabels, ExpressionLists::Recent);
    const ExpressionList* pc = ct.get(ExpressionLists::ShipLabels, ExpressionLists::Recent);
    TS_ASSERT_EQUALS(p, pc);
    TS_ASSERT(p != 0);
    TS_ASSERT(p != testee.get(ExpressionLists::ShipLabels, ExpressionLists::Predefined));
    TS_ASSERT(p != testee.get(ExpressionLists::PlanetLabels, ExpressionLists::Recent));

    // Use it
    p->pushBackNew(new ExpressionList::Item("a", "[b]", "c"));
    TS_ASSERT(!p->empty());

    // Pack
    afl::string::NullTranslator tx;
    ExpressionLists::Items_t list;
    testee.pack(list, ExpressionLists::ShipLabels, tx);
    TS_ASSERT_EQUALS(list.size(), 1U);
    TS_ASSERT_EQUALS(list[0].name, "a");
    TS_ASSERT_EQUALS(list[0].flags, "[b]");
    TS_ASSERT_EQUALS(list[0].value, "c");
    TS_ASSERT_EQUALS(list[0].isHeading, false);

    // clear()
    testee.clear();

    // Note that we do not guarantee the pointers to be long-term stable!
    p = testee.get(ExpressionLists::ShipLabels, ExpressionLists::Recent);
    TS_ASSERT(p->empty());
}

void
TestGameConfigExpressionLists::testPackComplex()
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
    TS_ASSERT_EQUALS(list.size(), 6U);

    TS_ASSERT_EQUALS(list[0].name, "Last expressions");
    TS_ASSERT_EQUALS(list[0].isHeading, true);

    TS_ASSERT_EQUALS(list[1].name, "recent1");
    TS_ASSERT_EQUALS(list[1].flags, "[b]");
    TS_ASSERT_EQUALS(list[1].value, "value r1");
    TS_ASSERT_EQUALS(list[1].isHeading, false);

    TS_ASSERT_EQUALS(list[2].name, "recent2");
    TS_ASSERT_EQUALS(list[2].flags, "[c]");
    TS_ASSERT_EQUALS(list[2].value, "value r2");
    TS_ASSERT_EQUALS(list[2].isHeading, false);

    TS_ASSERT_EQUALS(list[3].name, "Predefined expressions");
    TS_ASSERT_EQUALS(list[3].isHeading, true);

    TS_ASSERT_EQUALS(list[4].name, "predef 1");
    TS_ASSERT_EQUALS(list[4].flags, "[x]");
    TS_ASSERT_EQUALS(list[4].value, "value p1");
    TS_ASSERT_EQUALS(list[4].isHeading, false);

    TS_ASSERT_EQUALS(list[5].name, "predef 2");
    TS_ASSERT_EQUALS(list[5].flags, "[y]");
    TS_ASSERT_EQUALS(list[5].value, "value p2");
    TS_ASSERT_EQUALS(list[5].isHeading, false);
}

void
TestGameConfigExpressionLists::testLoadRecent()
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

    util::ProfileDirectory profile(env, fs, tx, log);

    // Testee
    game::config::ExpressionLists testee;
    testee.loadRecentFiles(profile, log, tx);

    // Verify
    const ExpressionList* s = testee.get(ExpressionLists::ShipLabels, ExpressionLists::Recent);
    TS_ASSERT(s);
    TS_ASSERT_EQUALS(s->size(), 2U);
    TS_ASSERT_EQUALS(s->get(0)->name, "The Name");
    TS_ASSERT_EQUALS(s->get(0)->flags, "");
    TS_ASSERT_EQUALS(s->get(0)->value, "Name");
    TS_ASSERT_EQUALS(s->get(1)->name, "Not Id");
    TS_ASSERT_EQUALS(s->get(1)->flags, "[!]");
    TS_ASSERT_EQUALS(s->get(1)->value, "Id");

    const ExpressionList* p = testee.get(ExpressionLists::PlanetLabels, ExpressionLists::Recent);
    TS_ASSERT(p);
    TS_ASSERT_EQUALS(p->size(), 1U);
    TS_ASSERT_EQUALS(p->get(0)->name, "Planet Name");
    TS_ASSERT_EQUALS(p->get(0)->flags, "");
    TS_ASSERT_EQUALS(p->get(0)->value, "Name");
}

void
TestGameConfigExpressionLists::testLoadPredefined()
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

    util::ProfileDirectory profile(env, fs, tx, log);

    // Testee
    game::config::ExpressionLists testee;
    testee.loadRecentFiles(profile, log, tx);
    testee.loadPredefinedFiles(profile, *gameDir, log, tx);

    // Verify
    const ExpressionList* s = testee.get(ExpressionLists::ShipLabels, ExpressionLists::Predefined);
    TS_ASSERT(s);
    TS_ASSERT_EQUALS(s->size(), 3U);
    TS_ASSERT_EQUALS(s->get(0)->name, "label 1");
    TS_ASSERT_EQUALS(s->get(0)->flags, "");
    TS_ASSERT_EQUALS(s->get(0)->value, "expr 1");
    TS_ASSERT_EQUALS(s->get(1)->name, "label 2");
    TS_ASSERT_EQUALS(s->get(1)->flags, "");
    TS_ASSERT_EQUALS(s->get(1)->value, "expr 2");
    TS_ASSERT_EQUALS(s->get(2)->name, "user label");
    TS_ASSERT_EQUALS(s->get(2)->flags, "");
    TS_ASSERT_EQUALS(s->get(2)->value, "user");

    const ExpressionList* p = testee.get(ExpressionLists::PlanetLabels, ExpressionLists::Predefined);
    TS_ASSERT(p);
    TS_ASSERT_EQUALS(p->size(), 1U);
    TS_ASSERT_EQUALS(p->get(0)->name, "p");
    TS_ASSERT_EQUALS(p->get(0)->flags, "");
    TS_ASSERT_EQUALS(p->get(0)->value, "l");

    const ExpressionList* f = testee.get(ExpressionLists::Search, ExpressionLists::Predefined);
    TS_ASSERT(f);
    TS_ASSERT_EQUALS(f->size(), 3U);
    TS_ASSERT_EQUALS(f->get(0)->name, "find 1");
    TS_ASSERT_EQUALS(f->get(0)->flags, "[abc]");
    TS_ASSERT_EQUALS(f->get(0)->value, "find 1");
    TS_ASSERT_EQUALS(f->get(1)->name, "user find a");
    TS_ASSERT_EQUALS(f->get(1)->flags, "[xy]");
    TS_ASSERT_EQUALS(f->get(1)->value, "find a");
    TS_ASSERT_EQUALS(f->get(2)->name, "user find b");
    TS_ASSERT_EQUALS(f->get(2)->flags, "[]");
    TS_ASSERT_EQUALS(f->get(2)->value, "find b");
}

void
TestGameConfigExpressionLists::testSave()
{
    // Environment
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    afl::io::InternalFileSystem fs;
    afl::sys::InternalEnvironment env;
    env.setSettingsDirectoryName("/profile");     // Will be auto-created!

    util::ProfileDirectory profile(env, fs, tx, log);

    // Testee
    game::config::ExpressionLists testee;
    ExpressionList* s = testee.get(ExpressionLists::ShipLabels, ExpressionLists::Recent);
    s->pushBackNew(new ExpressionList::Item("a  b  c", "", "xyz"));
    s->pushBackNew(new ExpressionList::Item("a b c", "[!]", "123"));
    testee.saveRecentFiles(profile, log, tx);

    // Verify
    afl::base::Ref<afl::io::Directory> profileDir(fs.openDirectory("/profile"));
    afl::base::Ref<afl::io::Stream> file = profileDir->openFile("lru.ini", FileSystem::OpenRead);
    String_t content = afl::string::fromBytes(file->createVirtualMapping()->get());

    // Remove \r, for Windows
    String_t::size_type n;
    while ((n = content.find('\r')) != String_t::npos) {
        content.erase(n, 1);
    }

    TS_ASSERT_EQUALS(content,
                     "[SHIPLABELS]\n"
                     "a b c  xyz\n"
                     "a b c  [!]123\n"
                     "\n");
}
