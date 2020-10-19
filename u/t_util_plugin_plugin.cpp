/**
  *  \file u/t_util_plugin_plugin.cpp
  *  \brief Test for util::plugin::Plugin
  */

#include "util/plugin/plugin.hpp"

#include "t_util_plugin.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"

namespace {
    String_t savePluginFile(const util::plugin::Plugin& p)
    {
        afl::io::InternalStream s;
        p.savePluginFile(s);

        // The text file will be written in system format. Read it again and convert to a fixed format.
        String_t result;
        String_t line;
        s.setPos(0);
        afl::io::TextFile tf(s);
        while (tf.readLine(line)) {
            result += line;
            result += "\n";
        }

        return result;
    }

    void loadPluginFile(util::plugin::Plugin& p, String_t f)
    {
        afl::sys::Log log;
        afl::io::ConstMemoryStream ms(afl::string::toBytes(f));
        p.initFromPluginFile("dir", "name", ms, log);
    }
}


/** Test compareVersions(). */
void
TestUtilPluginPlugin::testVersion()
{
    // ex UtilPluginTestSuite::testVersion
    using util::plugin::compareVersions;

    TS_ASSERT( compareVersions("1.0", "1.0.1"));
    TS_ASSERT(!compareVersions("1.0.1", "1.0"));

    TS_ASSERT(!compareVersions("1.0", "1.0"));

    TS_ASSERT( compareVersions("1.0", "1.0a"));
    TS_ASSERT(!compareVersions("1.0a", "1.0"));

    TS_ASSERT( compareVersions("a", "b"));
    TS_ASSERT(!compareVersions("b", "a"));

    TS_ASSERT( compareVersions("a", "1"));
    TS_ASSERT(!compareVersions("1", "a"));

    TS_ASSERT( compareVersions("99", "100"));
    TS_ASSERT(!compareVersions("100", "99"));
}

/** Test constructor, initial values, setters, getters. */
void
TestUtilPluginPlugin::testInit()
{
    using util::plugin::Plugin;

    // Default initialisation
    Plugin testee("ID");
    TS_ASSERT_EQUALS(testee.getId(), "ID");
    TS_ASSERT_EQUALS(testee.getName(), "ID");
    TS_ASSERT_EQUALS(testee.getDescription(), "");
    TS_ASSERT_EQUALS(testee.getBaseDirectory(), "");
    TS_ASSERT_EQUALS(testee.getDefinitionFileName(), "");
    TS_ASSERT_EQUALS(testee.isLoaded(), false);
    TS_ASSERT(testee.getItems().empty());

    // Manipulation
    testee.setBaseDirectory("/p");
    testee.addItem(Plugin::ResourceFile, "foo.res");
    testee.setLoaded(true);

    // Verify
    TS_ASSERT_EQUALS(testee.getBaseDirectory(), "/p");
    TS_ASSERT_EQUALS(testee.isLoaded(), true);
    TS_ASSERT_EQUALS(testee.getItems().size(), 1U);
    TS_ASSERT_EQUALS(testee.getItems()[0].type, Plugin::ResourceFile);
    TS_ASSERT_EQUALS(testee.getItems()[0].name, "foo.res");
}

/** Test initFromPluginFile(). */
void
TestUtilPluginPlugin::testInitPlugin()
{
    // Set up
    using util::plugin::Plugin;
    Plugin testee("P");
    afl::sys::Log log;
    afl::io::ConstMemoryStream ms(afl::string::toBytes(
                                      "# hi, I'm a plugin\n"
                                      "Name = PHost 4 Help\n"
                                      "Description = Provides the PHost 4 manual.\n"
                                      "Description = The PHost help pages...\n"
                                      "Requires = PCC 2.0.5\n"
                                      "HelpFile = phost4help.xml\n"
                                      "OtherFile=logo.gif\n"
                                      "Exec = Print 'hi'\n"
                                      "ScriptFile = test.q\n"
                                      "Provides = Text\n"
                                      "File = readme.txt\n"
                                      "resourceFile = data.res\n"
                                      ));
    testee.initFromPluginFile("/usr/doc", "phost4help.c2p", ms, log);

    // Verify
    TS_ASSERT_EQUALS(testee.getBaseDirectory(), "/usr/doc");
    TS_ASSERT_EQUALS(testee.getDefinitionFileName(), "phost4help.c2p");
    TS_ASSERT_EQUALS(testee.getName(), "PHost 4 Help");
    TS_ASSERT_EQUALS(testee.getDescription(), "Provides the PHost 4 manual.\nThe PHost help pages...");
    TS_ASSERT_EQUALS(testee.getItems().size(), 6U);
    TS_ASSERT_EQUALS(testee.getItems()[0].type, Plugin::HelpFile);
    TS_ASSERT_EQUALS(testee.getItems()[0].name, "phost4help.xml");
    TS_ASSERT_EQUALS(testee.getItems()[1].type, Plugin::PlainFile);
    TS_ASSERT_EQUALS(testee.getItems()[1].name, "logo.gif");
    TS_ASSERT_EQUALS(testee.getItems()[2].type, Plugin::Command);
    TS_ASSERT_EQUALS(testee.getItems()[2].name, "Print 'hi'");
    TS_ASSERT_EQUALS(testee.getItems()[3].type, Plugin::ScriptFile);
    TS_ASSERT_EQUALS(testee.getItems()[3].name, "test.q");
    TS_ASSERT_EQUALS(testee.getItems()[4].type, Plugin::PlainFile);
    TS_ASSERT_EQUALS(testee.getItems()[4].name, "readme.txt");
    TS_ASSERT_EQUALS(testee.getItems()[5].type, Plugin::ResourceFile);
    TS_ASSERT_EQUALS(testee.getItems()[5].name, "data.res");

    // Verify file
    TS_ASSERT_EQUALS(savePluginFile(testee),
                     "# Auto-generated plugin definition file\n"
                     "Name = PHost 4 Help\n"
                     "Description = Provides the PHost 4 manual.\n"
                     "Description = The PHost help pages...\n"
                     "Provides = TEXT\n"
                     "Requires = PCC 2.0.5\n"
                     "HelpFile = phost4help.xml\n"
                     "File = logo.gif\n"
                     "Exec = Print 'hi'\n"
                     "ScriptFile = test.q\n"
                     "File = readme.txt\n"
                     "ResourceFile = data.res\n");
}

/** Test initFromResourceFile(). */
void
TestUtilPluginPlugin::testInitResource()
{
    // Set up
    using util::plugin::Plugin;
    afl::string::NullTranslator tx;
    Plugin testee("R");
    testee.initFromResourceFile("/usr/lib", "Image.res", tx);

    // Verify
    TS_ASSERT_EQUALS(testee.getBaseDirectory(), "/usr/lib");
    TS_ASSERT_EQUALS(testee.getName(), "image.res");
    TS_ASSERT(!testee.getDescription().empty());
    TS_ASSERT_EQUALS(testee.getItems().size(), 1U);
    TS_ASSERT_EQUALS(testee.getItems()[0].type, Plugin::ResourceFile);
    TS_ASSERT_EQUALS(testee.getItems()[0].name, "Image.res");

    // Verify file
    TS_ASSERT_EQUALS(savePluginFile(testee),
                     "# Auto-generated plugin definition file\n"
                     "Name = image.res\n"
                     "Description = Resource file (artwork)\n"
                     "ResourceFile = Image.res\n");
}

/** Test initFromScriptFile(). */
void
TestUtilPluginPlugin::testInitScript()
{
    using util::plugin::Plugin;
    afl::string::NullTranslator tx;
    Plugin testee("S");

    afl::io::ConstMemoryStream ms(afl::string::toBytes("%\n"
                                                       "%  My plugin\n"
                                                       "%\n"
                                                       "%  This plugin does things. And other\n"
                                                       "%  things. And even more things. And stuff\n"
                                                       "%\n"
                                                       "Print 'hi'\n"));
    testee.initFromScriptFile("/usr/bin", "s.q", ms, tx);

    // Verify
    TS_ASSERT_EQUALS(testee.getBaseDirectory(), "/usr/bin");
    TS_ASSERT_EQUALS(testee.getName(), "My plugin");
    TS_ASSERT_EQUALS(testee.getDescription(), "This plugin does things. And other things.");
    TS_ASSERT_EQUALS(testee.getItems().size(), 1U);
    TS_ASSERT_EQUALS(testee.getItems()[0].type, Plugin::ScriptFile);
    TS_ASSERT_EQUALS(testee.getItems()[0].name, "s.q");

    // Verify file
    TS_ASSERT_EQUALS(savePluginFile(testee),
                     "# Auto-generated plugin definition file\n"
                     "Name = My plugin\n"
                     "Description = This plugin does things. And other things.\n"
                     "ScriptFile = s.q\n");
}

/** Test initFromConfigFile(). */
void
TestUtilPluginPlugin::testInitConfig()
{
    using util::plugin::Plugin;
    afl::string::NullTranslator tx;
    Plugin testee("C");
    afl::io::ConstMemoryStream ms(afl::string::toBytes("; Resource configuration\n"
                                                       "fonts.res\n"
                                                       "cc256.res\n"));
    testee.initFromConfigFile("resdir", "cfg", ms, tx);

    // Verify
    TS_ASSERT_EQUALS(testee.getBaseDirectory(), "resdir");
    TS_ASSERT_EQUALS(testee.getName(), "cfg");
    TS_ASSERT(!testee.getDescription().empty());
    TS_ASSERT_EQUALS(testee.getItems().size(), 2U);
    TS_ASSERT_EQUALS(testee.getItems()[0].type, Plugin::ResourceFile);
    TS_ASSERT_EQUALS(testee.getItems()[0].name, "fonts.res");
    TS_ASSERT_EQUALS(testee.getItems()[1].type, Plugin::ResourceFile);
    TS_ASSERT_EQUALS(testee.getItems()[1].name, "cc256.res");
}

/** Test initFromScriptFile(), variation: script has no clearly defined headline. */
void
TestUtilPluginPlugin::testInitScript2()
{
    using util::plugin::Plugin;
    afl::string::NullTranslator tx;
    Plugin testee("S");

    afl::io::ConstMemoryStream ms(afl::string::toBytes("%\n"
                                                       "%  Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Duis sem velit,\n"
                                                       "%  ultrices et, fermentum auctor, rhoncus ut, ligula. Phasellus at purus sed\n"
                                                       "%  purus cursus iaculis. Suspendisse fermentum.\n"
                                                       "Print 'hi'\n"));
    testee.initFromScriptFile("/usr/bin", "ipsum.q", ms, tx);

    // Verify
    TS_ASSERT_EQUALS(testee.getBaseDirectory(), "/usr/bin");
    TS_ASSERT_EQUALS(testee.getName(), "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Duis sem velit,");
    TS_ASSERT_EQUALS(testee.getDescription(), "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Duis sem velit, ultrices et, fermentum auctor, rhoncus ut, ligula.");
    TS_ASSERT_EQUALS(testee.getItems().size(), 1U);
    TS_ASSERT_EQUALS(testee.getItems()[0].type, Plugin::ScriptFile);
    TS_ASSERT_EQUALS(testee.getItems()[0].name, "ipsum.q");

    // Verify file
    TS_ASSERT_EQUALS(savePluginFile(testee),
                     "# Auto-generated plugin definition file\n"
                     "Name = Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Duis sem velit,\n"
                     "Description = Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Duis sem velit, ultrices et, fermentum auctor, rhoncus ut, ligula.\n"
                     "ScriptFile = ipsum.q\n");
}

/** Test initFromScriptFile(), variation: script has no comment. */
void
TestUtilPluginPlugin::testInitScript3()
{
    using util::plugin::Plugin;
    afl::string::NullTranslator tx;
    Plugin testee("S");

    afl::io::ConstMemoryStream ms(afl::string::toBytes("Print 'hi'\n"));
    testee.initFromScriptFile("/usr/bin", "headless.q", ms, tx);

    // Verify
    TS_ASSERT_EQUALS(testee.getBaseDirectory(), "/usr/bin");
    TS_ASSERT_EQUALS(testee.getName(), "headless.q");
    TS_ASSERT_EQUALS(testee.getDescription(), "Script file");
    TS_ASSERT_EQUALS(testee.getItems().size(), 1U);
    TS_ASSERT_EQUALS(testee.getItems()[0].type, Plugin::ScriptFile);
    TS_ASSERT_EQUALS(testee.getItems()[0].name, "headless.q");

    // Verify file
    TS_ASSERT_EQUALS(savePluginFile(testee),
                     "# Auto-generated plugin definition file\n"
                     "Name = headless.q\n"
                     "Description = Script file\n"
                     "ScriptFile = headless.q\n");
}

/** Test dependency management, basics. */
void
TestUtilPluginPlugin::testSelfDepend()
{
    util::plugin::Plugin testee("X");
    loadPluginFile(testee, "Provides = A, B 2");

    // Check provided capabilities
    TS_ASSERT(testee.isProvided("X"));    // implicit
    TS_ASSERT(testee.isProvided("A"));
    TS_ASSERT(testee.isProvided("B"));
    TS_ASSERT(!testee.isProvided("C"));

    // Each plugin conflicts with itself
    TS_ASSERT(testee.isConflict(testee));

    // Each plugin serves as update to itself
    TS_ASSERT(testee.isUpdateFor(testee));

    // Enumerate
    util::plugin::Plugin::FeatureSet_t fset;
    testee.enumProvidedFeatures(fset);
    TS_ASSERT_EQUALS(fset["B"], "2");
}

/** Test dependency management. */
void
TestUtilPluginPlugin::testDepend()
{
    util::plugin::Plugin a("A");
    loadPluginFile(a, "Provides = FA 2.0");

    util::plugin::Plugin b1("B1");
    loadPluginFile(b1, "Requires = FA 2.0");

    util::plugin::Plugin b2("B2");
    loadPluginFile(b2, "Requires = FA 3.0");

    util::plugin::Plugin c("C");
    loadPluginFile(c, "Requires = FA, FB");

    // All depend on a although A doesn't entirely satisfy them
    TS_ASSERT(b1.isDependingOn(a));
    TS_ASSERT(b2.isDependingOn(a));
    TS_ASSERT(c.isDependingOn(a));

    // Verify feature set
    util::plugin::Plugin::FeatureSet_t fset;
    a.enumProvidedFeatures(fset);
    TS_ASSERT(b1.isSatisfiedBy(fset));
    TS_ASSERT(!b2.isSatisfiedBy(fset));
    TS_ASSERT(!c.isSatisfiedBy(fset));

    // Missing features
    util::plugin::Plugin::FeatureSet_t missing;
    c.enumMissingFeatures(fset, missing);
    TS_ASSERT(missing.find("FB") != missing.end());

    // Add FB; this should now satisfy C
    fset["FB"] = "";
    TS_ASSERT(c.isSatisfiedBy(fset));
}

/** Test dependency management, updates. */
void
TestUtilPluginPlugin::testUpdate()
{
    util::plugin::Plugin a("A");
    loadPluginFile(a,
                   "Provides = FA 2.0, FB\n"
                   "Requires = FC 2.0, FD");

    // Plugin is update to itself
    TS_ASSERT(a.isUpdateFor(a));

    // Better provides
    {
        util::plugin::Plugin up("A");
        loadPluginFile(up,
                       "Provides = FA 2.1, FB, FX\n"
                       "Requires = FC 2.0, FD");
        TS_ASSERT(up.isUpdateFor(a));
        TS_ASSERT(!a.isUpdateFor(up));
    }

    // Fewer requirements
    {
        util::plugin::Plugin up("A");
        loadPluginFile(up,
                       "Provides = FA 2.1, FB, FX\n"
                       "Requires = FD");
        TS_ASSERT(up.isUpdateFor(a));
        TS_ASSERT(!a.isUpdateFor(up));
    }

    // Worse provides
    {
        util::plugin::Plugin up("A");
        loadPluginFile(up,
                       "Provides = FA 1.9, FB\n"
                       "Requires = FC 2.0, FD");
        TS_ASSERT(!up.isUpdateFor(a));
        TS_ASSERT(a.isUpdateFor(up));
    }

    // Worse provides
    {
        util::plugin::Plugin up("A");
        loadPluginFile(up,
                       "Provides = FB\n"
                       "Requires = FC 2.0, FD");
        TS_ASSERT(!up.isUpdateFor(a));
        TS_ASSERT(a.isUpdateFor(up));
    }

    // Stricter requirements
    {
        util::plugin::Plugin up("A");
        loadPluginFile(up,
                       "Provides = FA 2.1, FB\n"
                       "Requires = FC 3.0, FD");
        TS_ASSERT(!up.isUpdateFor(a));
        TS_ASSERT(!a.isUpdateFor(up));
    }
}

/** Test unrelated plugins. */
void
TestUtilPluginPlugin::testUnrelated()
{
    util::plugin::Plugin a("A"), b("B");
    TS_ASSERT(!a.isConflict(b));
    TS_ASSERT(!b.isConflict(a));

    TS_ASSERT(!a.isUpdateFor(b));
    TS_ASSERT(!b.isUpdateFor(a));

    TS_ASSERT(!a.isDependingOn(b));
    TS_ASSERT(!b.isDependingOn(a));
}

