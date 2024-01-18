/**
  *  \file test/util/plugin/managertest.cpp
  *  \brief Test for util::plugin::Manager
  */

#include "util/plugin/manager.hpp"

#include "afl/base/ref.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"

using afl::base::Ref;
using afl::io::ConstMemoryStream;
using afl::io::InternalDirectory;
using afl::string::toBytes;
using util::plugin::Manager;
using util::plugin::Plugin;

/** Simple test sequence. */
AFL_TEST("util.plugin.Manager:basics", a)
{
    // Setup
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    Manager testee(tx, log);
    a.checkEqual("01. log", &testee.log(), &log);

    // Create a directory with some plugins in it
    Ref<InternalDirectory> dir = InternalDirectory::create("dir");
    dir->addStream("a.c2p", *new ConstMemoryStream(toBytes("provides = fa\n")));
    dir->addStream("b.c2p", *new ConstMemoryStream(toBytes("requires = c\n")));
    dir->addStream("c.c2p", *new ConstMemoryStream(toBytes("")));
    dir->addStream("readme.txt", *new ConstMemoryStream(toBytes("hi there")));

    // Read them
    testee.findPlugins(*dir);

    // Verify what we have
    a.checkNonNull("11. getPluginById", testee.getPluginById("A"));
    a.checkNonNull("12. getPluginById", testee.getPluginById("B"));
    a.checkNonNull("13. getPluginById", testee.getPluginById("C"));
    a.checkNull("14. getPluginById", testee.getPluginById("FA"));    // provided feature, but not a plugin

    // List them (alphabetic)
    {
        std::vector<Plugin*> alpha;
        testee.enumPlugins(alpha, false);
        a.checkEqual("21. size", alpha.size(), 3U);
        a.checkEqual("22. result", alpha[0]->getId(), "A");
        a.checkEqual("23. result", alpha[1]->getId(), "B");
        a.checkEqual("24. result", alpha[2]->getId(), "C");
    }

    // List them (alphabetic, textual)
    {
        Manager::Infos_t result;
        testee.enumPluginInfo(result);
        a.checkEqual("31. size", result.size(), 3U);
        a.checkEqual("32. result", result[0].id, "A");
        a.checkEqual("33. result", result[1].id, "B");
        a.checkEqual("34. result", result[2].id, "C");
    }

    // List them (ordered)
    {
        std::vector<Plugin*> order;
        testee.enumPlugins(order, true);
        a.checkEqual("41. size", order.size(), 3U);
        a.checkEqual("42. result", order[0]->getId(), "A");
        a.checkEqual("43. result", order[1]->getId(), "C");
        a.checkEqual("44. result", order[2]->getId(), "B");
    }

    // Provided features
    {
        Plugin::FeatureSet_t have;
        testee.enumProvidedFeatures(have);
        a.check("51. result", have.find("A") != have.end());
        a.check("52. result", have.find("B") != have.end());
        a.check("53. result", have.find("C") != have.end());
        a.check("54. result", have.find("FA") != have.end());
    }

    // Conflicts - named the same as a provided feature
    {
        Plugin tmp("FA");
        std::vector<Plugin*> conf;
        testee.enumConflictingPlugins(tmp, conf);
        a.checkEqual("61. size", conf.size(), 1U);
        a.checkEqual("62. result", conf[0]->getId(), "A");
    }

    // Conflicts - named the same as a known plugin but doesn't qualify as update
    {
        Plugin tmp("A");
        std::vector<Plugin*> conf;
        testee.enumConflictingPlugins(tmp, conf);
        a.checkEqual("71. size", conf.size(), 1U);
        a.checkEqual("72. result", conf[0]->getId(), "A");
    }

    // Conflicts - ok
    {
        Plugin tmp("B");
        std::vector<Plugin*> conf;
        testee.enumConflictingPlugins(tmp, conf);
        a.checkEqual("81. size", conf.size(), 0U);
    }

    // Depending plugins: we cannot remove C because B depends on it
    {
        Plugin* c = testee.getPluginById("C");
        a.checkNonNull("91", c);
        std::vector<Plugin*> deps;
        testee.enumDependingPlugins(*c, deps);
        a.checkEqual("92. size", deps.size(), 1U);
        a.checkEqual("93. result", deps[0]->getId(), "B");
    }

    // Remove B and recheck C
    {
        Plugin* b = testee.getPluginById("B");
        a.checkNonNull("101. getPluginById", b);
        a.checkEqual("102. extractPlugin", b, testee.extractPlugin(b));
        delete b;

        Plugin* c = testee.getPluginById("C");
        a.checkNonNull("111. getPluginById", c);
        std::vector<Plugin*> deps;
        testee.enumDependingPlugins(*c, deps);
        a.checkEqual("112. size", deps.size(), 0U);
    }

    // Extract nonexistant
    {
        Plugin x("X");
        Plugin* p = testee.extractPlugin(&x);
        a.checkNull("121. extractPlugin", p);
    }

    // Add new plugin and check enumeration. New plugin always goes at end.
    {
        testee.addNewPlugin(new Plugin("1"));
        std::vector<Plugin*> alpha;
        testee.enumPlugins(alpha, false);
        a.checkEqual("131. size", alpha.size(), 3U);
        a.checkEqual("132. result", alpha[0]->getId(), "A");
        a.checkEqual("133. result", alpha[1]->getId(), "C");
        a.checkEqual("134. result", alpha[2]->getId(), "1");
    }
}

/** Test loading with cyclic or missing dependencies. */
AFL_TEST("util.plugin.Manager:cyclic-dependency", a)
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
        a.checkEqual("01. size", alpha.size(), 5U);
        a.checkEqual("02. result", alpha[0]->getId(), "A");
        a.checkEqual("03. result", alpha[1]->getId(), "B");
        a.checkEqual("04. result", alpha[2]->getId(), "C");
        a.checkEqual("05. result", alpha[3]->getId(), "D");
        a.checkEqual("06. result", alpha[4]->getId(), "E");
    }

    // List them (ordered)
    {
        std::vector<Plugin*> order;
        testee.enumPlugins(order, true);
        a.checkEqual("11. size", order.size(), 1U);
        a.checkEqual("12. result", order[0]->getId(), "E");
    }
}

/** Test loading from NullFileSystem. */
AFL_TEST("util.plugin.Manager:findPlugins:NullFileSystem", a)
{
    // Setup
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    Manager testee(tx, log);

    // Load null filesystem
    afl::io::NullFileSystem fs;
    AFL_CHECK_SUCCEEDS(a("01. findPlugins"), testee.findPlugins(fs, "/"));

    // List them (alphabetic)
    {
        std::vector<Plugin*> alpha;
        testee.enumPlugins(alpha, false);
        a.checkEqual("11. size", alpha.size(), 0U);
    }
}

/** Test describePlugin(). */
AFL_TEST("util.plugin.Manager:describePlugin", a)
{
    // Setup
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    Manager testee(tx, log);

    // Create a directory with some plugins in it
    Ref<InternalDirectory> dir = InternalDirectory::create("dir");
    dir->addStream("a.c2p", *new ConstMemoryStream(toBytes("description = first plugin\n"
                                                           "name = first\n"
                                                           "requires = x, b, q\n"
                                                           "provides = f\n"
                                                           "exec = print 'hi'\n"
                                                           "helpfile = foo.xml\n")));
    dir->addStream("b.c2p", *new ConstMemoryStream(toBytes("provides = q\n"
                                                           "name = second\n")));
    testee.findPlugins(*dir);

    // Verify
    Manager::Details da = testee.describePlugin(testee.getPluginById("A"));
    a.checkEqual("01. id",               da.id, "A");
    a.checkEqual("02. name",             da.name, "first");
    a.checkEqual("03. description",      da.description, "first plugin");
    a.checkEqual("04. usedFeatures",     da.usedFeatures.size(), 2U);
    a.checkEqual("05. usedFeatures",     da.usedFeatures[0], "B");
    a.checkEqual("06. usedFeatures",     da.usedFeatures[1], "Q");
    a.checkEqual("07. missingFeatures",  da.missingFeatures.size(), 1U);
    a.checkEqual("08. missingFeatures",  da.missingFeatures[0], "X");
    a.checkEqual("09. providedFeatures", da.providedFeatures.size(), 1U);
    a.checkEqual("10. providedFeatures", da.providedFeatures[0], "F");
    a.checkEqual("11. files",            da.files.size(), 1U);
    a.checkEqual("12. files",            da.files[0], "foo.xml");

    Manager::Details db = testee.describePlugin(testee.getPluginById("B"));
    a.checkEqual("21. id",               db.id, "B");
    a.checkEqual("22. name",             db.name, "second");
    a.checkEqual("23. description",      db.description, "");
    a.checkEqual("24. usedFeatures",     db.usedFeatures.size(), 0U);
    a.checkEqual("25. missingFeatures",  db.missingFeatures.size(), 0U);
    a.checkEqual("26. providedFeatures", db.providedFeatures.size(), 1U);
    a.checkEqual("27. providedFeatures", db.providedFeatures[0], "Q");
    a.checkEqual("28. files",            db.files.size(), 0U);
}

/** Test describePlugin(), null case. */
AFL_TEST("util.plugin.Manager:describePlugin:null", a)
{
    // Setup
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    Manager testee(tx, log);

    Manager::Details d = testee.describePlugin(0);
    a.checkEqual("01. id",          d.id, "");
    a.checkEqual("02. name",        d.name, "");
    a.checkEqual("03. status",      d.status, Manager::NotLoaded);
    a.checkEqual("04. description", d.description, "");
}
