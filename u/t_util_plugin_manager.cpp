/**
  *  \file u/t_util_plugin_manager.cpp
  *  \brief Test for util::plugin::Manager
  */

#include "util/plugin/manager.hpp"

#include "t_util_plugin.hpp"
#include "afl/base/ref.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"

using afl::base::Ref;
using afl::io::ConstMemoryStream;
using afl::io::InternalDirectory;
using afl::string::toBytes;
using util::plugin::Manager;
using util::plugin::Plugin;

/** Simple test sequence. */
void
TestUtilPluginManager::testIt()
{
    // Setup
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    Manager testee(tx, log);
//    TS_ASSERT_EQUALS(&testee.log(), &log);

    // Create a directory with some plugins in it
    Ref<InternalDirectory> dir = InternalDirectory::create("dir");
    dir->addStream("a.c2p", *new ConstMemoryStream(toBytes("provides = fa\n")));
    dir->addStream("b.c2p", *new ConstMemoryStream(toBytes("requires = c\n")));
    dir->addStream("c.c2p", *new ConstMemoryStream(toBytes("")));
    dir->addStream("readme.txt", *new ConstMemoryStream(toBytes("hi there")));

    // Read them
    testee.findPlugins(*dir);

    // Verify what we have
    TS_ASSERT(testee.getPluginById("A") != 0);
    TS_ASSERT(testee.getPluginById("B") != 0);
    TS_ASSERT(testee.getPluginById("C") != 0);
    TS_ASSERT(testee.getPluginById("FA") == 0);    // provided feature, but not a plugin

    // List them (alphabetic)
    {
        std::vector<Plugin*> alpha;
        testee.enumPlugins(alpha, false);
        TS_ASSERT_EQUALS(alpha.size(), 3U);
        TS_ASSERT_EQUALS(alpha[0]->getId(), "A");
        TS_ASSERT_EQUALS(alpha[1]->getId(), "B");
        TS_ASSERT_EQUALS(alpha[2]->getId(), "C");
    }

    // List them (ordered)
    {
        std::vector<Plugin*> order;
        testee.enumPlugins(order, true);
        TS_ASSERT_EQUALS(order.size(), 3U);
        TS_ASSERT_EQUALS(order[0]->getId(), "A");
        TS_ASSERT_EQUALS(order[1]->getId(), "C");
        TS_ASSERT_EQUALS(order[2]->getId(), "B");
    }

    // Provided features
    {
        Plugin::FeatureSet_t have;
        testee.enumProvidedFeatures(have);
        TS_ASSERT(have.find("A") != have.end());
        TS_ASSERT(have.find("B") != have.end());
        TS_ASSERT(have.find("C") != have.end());
        TS_ASSERT(have.find("FA") != have.end());
    }

    // Conflicts - named the same as a provided feature
    {
        Plugin tmp("FA");
        std::vector<Plugin*> conf;
        testee.enumConflictingPlugins(tmp, conf);
        TS_ASSERT_EQUALS(conf.size(), 1U);
        TS_ASSERT_EQUALS(conf[0]->getId(), "A");
    }

    // Conflicts - named the same as a known plugin but doesn't qualify as update
    {
        Plugin tmp("A");
        std::vector<Plugin*> conf;
        testee.enumConflictingPlugins(tmp, conf);
        TS_ASSERT_EQUALS(conf.size(), 1U);
        TS_ASSERT_EQUALS(conf[0]->getId(), "A");
    }

    // Conflicts - ok
    {
        Plugin tmp("B");
        std::vector<Plugin*> conf;
        testee.enumConflictingPlugins(tmp, conf);
        TS_ASSERT_EQUALS(conf.size(), 0U);
    }

    // Depending plugins: we cannot remove C because B depends on it
    {
        Plugin* c = testee.getPluginById("C");
        TS_ASSERT(c != 0);
        std::vector<Plugin*> deps;
        testee.enumDependingPlugins(*c, deps);
        TS_ASSERT_EQUALS(deps.size(), 1U);
        TS_ASSERT_EQUALS(deps[0]->getId(), "B");
    }

    // Remove B and recheck C
    {
        Plugin* b = testee.getPluginById("B");
        TS_ASSERT(b != 0);
        TS_ASSERT_EQUALS(b, testee.extractPlugin(b));
        delete b;

        Plugin* c = testee.getPluginById("C");
        TS_ASSERT(c != 0);
        std::vector<Plugin*> deps;
        testee.enumDependingPlugins(*c, deps);
        TS_ASSERT_EQUALS(deps.size(), 0U);
    }

    // Extract nonexistant
    {
        Plugin x("X");
        Plugin* p = testee.extractPlugin(&x);
        TS_ASSERT(p == 0);
    }

    // Add new plugin and check enumeration. New plugin always goes at end.
    {
        testee.addNewPlugin(new Plugin("1"));
        std::vector<Plugin*> alpha;
        testee.enumPlugins(alpha, false);
        TS_ASSERT_EQUALS(alpha.size(), 3U);
        TS_ASSERT_EQUALS(alpha[0]->getId(), "A");
        TS_ASSERT_EQUALS(alpha[1]->getId(), "C");
        TS_ASSERT_EQUALS(alpha[2]->getId(), "1");
    }
}

/** Test loading with cyclic or missing dependencies. */
void
TestUtilPluginManager::testCycle()
{
    // Setup
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    Manager testee(tx, log);

    // Create a directory with some plugins in it
    Ref<InternalDirectory> dir = InternalDirectory::create("dir");
    dir->addStream("a.c2p", *new ConstMemoryStream(toBytes("requires = x\n")));
    dir->addStream("b.c2p", *new ConstMemoryStream(toBytes("requires = c\n")));
    dir->addStream("c.c2p", *new ConstMemoryStream(toBytes("requires = b\n")));
    dir->addStream("d.c2p", *new ConstMemoryStream(toBytes("requires = b\n")));
    dir->addStream("e.c2p", *new ConstMemoryStream(toBytes("")));

    // Read them
    testee.findPlugins(*dir);

    // List them (alphabetic)
    {
        std::vector<Plugin*> alpha;
        testee.enumPlugins(alpha, false);
        TS_ASSERT_EQUALS(alpha.size(), 5U);
        TS_ASSERT_EQUALS(alpha[0]->getId(), "A");
        TS_ASSERT_EQUALS(alpha[1]->getId(), "B");
        TS_ASSERT_EQUALS(alpha[2]->getId(), "C");
        TS_ASSERT_EQUALS(alpha[3]->getId(), "D");
        TS_ASSERT_EQUALS(alpha[4]->getId(), "E");
    }

    // List them (ordered)
    {
        std::vector<Plugin*> order;
        testee.enumPlugins(order, true);
        TS_ASSERT_EQUALS(order.size(), 1U);
        TS_ASSERT_EQUALS(order[0]->getId(), "E");
    }
}

/** Test loading from NullFileSystem. */
void
TestUtilPluginManager::testNull()
{
    // Setup
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    Manager testee(tx, log);

    // Load null filesystem
    afl::io::NullFileSystem fs;
    TS_ASSERT_THROWS_NOTHING(testee.findPlugins(fs, "/"));

    // List them (alphabetic)
    {
        std::vector<Plugin*> alpha;
        testee.enumPlugins(alpha, false);
        TS_ASSERT_EQUALS(alpha.size(), 0U);
    }
}

