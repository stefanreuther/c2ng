/**
  *  \file test/util/plugin/plugintest.cpp
  *  \brief Test for util::plugin::Plugin
  */

#include "util/plugin/plugin.hpp"

#include "afl/io/constmemorystream.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"

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
        afl::string::NullTranslator tx;
        p.initFromPluginFile("dir", "name", ms, log, tx);
    }
}


/** Test compareVersions(). */
AFL_TEST("util.plugin.Plugin:compareVersions", a)
{
    // ex UtilPluginTestSuite::testVersion
    using util::plugin::compareVersions;

    a.check("01",  compareVersions("1.0", "1.0.1"));
    a.check("02", !compareVersions("1.0.1", "1.0"));

    a.check("11", !compareVersions("1.0", "1.0"));

    a.check("21",  compareVersions("1.0", "1.0a"));
    a.check("22", !compareVersions("1.0a", "1.0"));

    a.check("31",  compareVersions("a", "b"));
    a.check("32", !compareVersions("b", "a"));

    a.check("41",  compareVersions("a", "1"));
    a.check("42", !compareVersions("1", "a"));

    a.check("51",  compareVersions("99", "100"));
    a.check("52", !compareVersions("100", "99"));
}

/** Test constructor, initial values, setters, getters. */
AFL_TEST("util.plugin.Plugin:basics", a)
{
    using util::plugin::Plugin;

    // Default initialisation
    Plugin testee("ID");
    a.checkEqual("01. getId",                 testee.getId(), "ID");
    a.checkEqual("02. getName",               testee.getName(), "ID");
    a.checkEqual("03. getDescription",        testee.getDescription(), "");
    a.checkEqual("04. getBaseDirectory",      testee.getBaseDirectory(), "");
    a.checkEqual("05. getDefinitionFileName", testee.getDefinitionFileName(), "");
    a.checkEqual("06. isLoaded",              testee.isLoaded(), false);
    a.check     ("07. getItems",              testee.getItems().empty());

    // Manipulation
    testee.setBaseDirectory("/p");
    testee.addItem(Plugin::ResourceFile, "foo.res");
    testee.setLoaded(true);

    // Verify
    a.checkEqual("11. getBaseDirectory", testee.getBaseDirectory(), "/p");
    a.checkEqual("12. isLoaded",         testee.isLoaded(), true);
    a.checkEqual("13. getId",            testee.getItems().size(), 1U);
    a.checkEqual("14. getId",            testee.getItems()[0].type, Plugin::ResourceFile);
    a.checkEqual("15. getId",            testee.getItems()[0].name, "foo.res");
}

/** Test initFromPluginFile(). */
AFL_TEST("util.plugin.Plugin:initFromPluginFile", a)
{
    // Set up
    using util::plugin::Plugin;
    Plugin testee("P");
    afl::sys::Log log;
    afl::string::NullTranslator tx;
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
    testee.initFromPluginFile("/usr/doc", "phost4help.c2p", ms, log, tx);

    // Verify
    a.checkEqual("01. getBaseDirectory",      testee.getBaseDirectory(), "/usr/doc");
    a.checkEqual("02. getDefinitionFileName", testee.getDefinitionFileName(), "phost4help.c2p");
    a.checkEqual("03. getName",               testee.getName(), "PHost 4 Help");
    a.checkEqual("04. getDescription",        testee.getDescription(), "Provides the PHost 4 manual.\nThe PHost help pages...");
    a.checkEqual("05. getItems",              testee.getItems().size(), 6U);
    a.checkEqual("06. getItems",              testee.getItems()[0].type, Plugin::HelpFile);
    a.checkEqual("07. getItems",              testee.getItems()[0].name, "phost4help.xml");
    a.checkEqual("08. getItems",              testee.getItems()[1].type, Plugin::PlainFile);
    a.checkEqual("09. getItems",              testee.getItems()[1].name, "logo.gif");
    a.checkEqual("10. getItems",              testee.getItems()[2].type, Plugin::Command);
    a.checkEqual("11. getItems",              testee.getItems()[2].name, "Print 'hi'");
    a.checkEqual("12. getItems",              testee.getItems()[3].type, Plugin::ScriptFile);
    a.checkEqual("13. getItems",              testee.getItems()[3].name, "test.q");
    a.checkEqual("14. getItems",              testee.getItems()[4].type, Plugin::PlainFile);
    a.checkEqual("15. getItems",              testee.getItems()[4].name, "readme.txt");
    a.checkEqual("16. getItems",              testee.getItems()[5].type, Plugin::ResourceFile);
    a.checkEqual("17. getItems",              testee.getItems()[5].name, "data.res");

    // Verify file
    a.checkEqual("21. savePluginFile", savePluginFile(testee),
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
AFL_TEST("util.plugin.Plugin:initFromResourceFile", a)
{
    // Set up
    using util::plugin::Plugin;
    afl::string::NullTranslator tx;
    Plugin testee("R");
    testee.initFromResourceFile("/usr/lib", "Image.res", tx);

    // Verify
    a.checkEqual("01. getBaseDirectory", testee.getBaseDirectory(), "/usr/lib");
    a.checkEqual("02. getName",          testee.getName(), "image.res");
    a.check     ("03. getDescription",  !testee.getDescription().empty());
    a.checkEqual("04. getItems",         testee.getItems().size(), 1U);
    a.checkEqual("05. getItems",         testee.getItems()[0].type, Plugin::ResourceFile);
    a.checkEqual("06. getItems",         testee.getItems()[0].name, "Image.res");

    // Verify file
    a.checkEqual("11. savePluginFile", savePluginFile(testee),
                 "# Auto-generated plugin definition file\n"
                 "Name = image.res\n"
                 "Description = Resource file (artwork)\n"
                 "ResourceFile = Image.res\n");
}

/** Test initFromScriptFile(). */
AFL_TEST("util.plugin.Plugin:initFromScriptFile", a)
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
    a.checkEqual("01. getBaseDirectory", testee.getBaseDirectory(), "/usr/bin");
    a.checkEqual("02. getName",          testee.getName(), "My plugin");
    a.checkEqual("03. getDescription",   testee.getDescription(), "This plugin does things. And other things.");
    a.checkEqual("04. getItems",         testee.getItems().size(), 1U);
    a.checkEqual("05. getItems",         testee.getItems()[0].type, Plugin::ScriptFile);
    a.checkEqual("06. getItems",         testee.getItems()[0].name, "s.q");

    // Verify file
    a.checkEqual("11. savePluginFile", savePluginFile(testee),
                 "# Auto-generated plugin definition file\n"
                 "Name = My plugin\n"
                 "Description = This plugin does things. And other things.\n"
                 "ScriptFile = s.q\n");
}

/** Test initFromConfigFile(). */
AFL_TEST("util.plugin.Plugin:initFromConfigFile", a)
{
    using util::plugin::Plugin;
    afl::string::NullTranslator tx;
    Plugin testee("C");
    afl::io::ConstMemoryStream ms(afl::string::toBytes("; Resource configuration\n"
                                                       "fonts.res\n"
                                                       "cc256.res\n"));
    testee.initFromConfigFile("resdir", "cfg", ms, tx);

    // Verify
    a.checkEqual("01. getBaseDirectory", testee.getBaseDirectory(), "resdir");
    a.checkEqual("02. getName",          testee.getName(), "cfg");
    a.check     ("03. getDescription",  !testee.getDescription().empty());
    a.checkEqual("04. getItems",         testee.getItems().size(), 2U);
    a.checkEqual("05. getItems",         testee.getItems()[0].type, Plugin::ResourceFile);
    a.checkEqual("06. getItems",         testee.getItems()[0].name, "fonts.res");
    a.checkEqual("07. getItems",         testee.getItems()[1].type, Plugin::ResourceFile);
    a.checkEqual("08. getItems",         testee.getItems()[1].name, "cc256.res");
}

/** Test initFromScriptFile(), variation: script has no clearly defined headline. */
AFL_TEST("util.plugin.Plugin:initFromScriptFile:no-headline", a)
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
    a.checkEqual("01. getBaseDirectory", testee.getBaseDirectory(), "/usr/bin");
    a.checkEqual("02. getName",          testee.getName(), "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Duis sem velit,");
    a.checkEqual("03. getDescription",   testee.getDescription(), "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Duis sem velit, ultrices et, fermentum auctor, rhoncus ut, ligula.");
    a.checkEqual("04. getItems",         testee.getItems().size(), 1U);
    a.checkEqual("05. getItems",         testee.getItems()[0].type, Plugin::ScriptFile);
    a.checkEqual("06. getItems",         testee.getItems()[0].name, "ipsum.q");

    // Verify file
    a.checkEqual("11. savePluginFile", savePluginFile(testee),
                 "# Auto-generated plugin definition file\n"
                 "Name = Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Duis sem velit,\n"
                 "Description = Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Duis sem velit, ultrices et, fermentum auctor, rhoncus ut, ligula.\n"
                 "ScriptFile = ipsum.q\n");
}

/** Test initFromScriptFile(), variation: script has no comment. */
AFL_TEST("util.plugin.Plugin:initFromScriptFile:no-comment", a)
{
    using util::plugin::Plugin;
    afl::string::NullTranslator tx;
    Plugin testee("S");

    afl::io::ConstMemoryStream ms(afl::string::toBytes("Print 'hi'\n"));
    testee.initFromScriptFile("/usr/bin", "headless.q", ms, tx);

    // Verify
    a.checkEqual("01. getBaseDirectory", testee.getBaseDirectory(), "/usr/bin");
    a.checkEqual("02. getName",          testee.getName(), "headless.q");
    a.checkEqual("03. getDescription",   testee.getDescription(), "Script file");
    a.checkEqual("04. getItems",         testee.getItems().size(), 1U);
    a.checkEqual("05. getItems",         testee.getItems()[0].type, Plugin::ScriptFile);
    a.checkEqual("06. getItems",         testee.getItems()[0].name, "headless.q");

    // Verify file
    a.checkEqual("11. savePluginFile", savePluginFile(testee),
                 "# Auto-generated plugin definition file\n"
                 "Name = headless.q\n"
                 "Description = Script file\n"
                 "ScriptFile = headless.q\n");
}

/** Test dependency management, basics. */
AFL_TEST("util.plugin.Plugin:dependencies:single", a)
{
    util::plugin::Plugin testee("X");
    loadPluginFile(testee, "Provides = A, B 2");

    // Check provided capabilities
    a.check("01. isProvided", testee.isProvided("X"));    // implicit
    a.check("02. isProvided", testee.isProvided("A"));
    a.check("03. isProvided", testee.isProvided("B"));
    a.check("04. isProvided", !testee.isProvided("C"));

    // Each plugin conflicts with itself
    a.check("11. isConflict", testee.isConflict(testee));

    // Each plugin serves as update to itself
    a.check("21. isUpdateFor", testee.isUpdateFor(testee));

    // Enumerate
    util::plugin::Plugin::FeatureSet_t fset;
    testee.enumProvidedFeatures(fset);
    a.checkEqual("31. enumProvidedFeatures", fset["B"], "2");
}

/** Test dependency management. */
AFL_TEST("util.plugin.Plugin:dependencies:multiple", a)
{
    util::plugin::Plugin pa("A");
    loadPluginFile(pa, "Provides = FA 2.0");

    util::plugin::Plugin pb1("B1");
    loadPluginFile(pb1, "Requires = FA 2.0");

    util::plugin::Plugin pb2("B2");
    loadPluginFile(pb2, "Requires = FA 3.0");

    util::plugin::Plugin pc("C");
    loadPluginFile(pc, "Requires = FA, FB");

    // All depend on a although A doesn't entirely satisfy them
    a.check("01. isDependingOn", pb1.isDependingOn(pa));
    a.check("02. isDependingOn", pb2.isDependingOn(pa));
    a.check("03. isDependingOn", pc.isDependingOn(pa));

    // Verify feature set
    util::plugin::Plugin::FeatureSet_t fset;
    pa.enumProvidedFeatures(fset);
    a.check("11. isSatisfiedBy", pb1.isSatisfiedBy(fset));
    a.check("12. isSatisfiedBy", !pb2.isSatisfiedBy(fset));
    a.check("13. isSatisfiedBy", !pc.isSatisfiedBy(fset));

    // Missing features
    util::plugin::Plugin::FeatureSet_t missing;
    pc.enumMissingFeatures(fset, missing);
    a.check("21. enumMissingFeatures", missing.find("FB") != missing.end());

    // Add FB; this should now satisfy C
    fset["FB"] = "";
    a.check("31. isSatisfiedBy", pc.isSatisfiedBy(fset));
}

/** Test dependency management, updates. */
AFL_TEST("util.plugin.Plugin:dependency:update", a)
{
    util::plugin::Plugin pa("A");
    loadPluginFile(pa,
                   "Provides = FA 2.0, FB\n"
                   "Requires = FC 2.0, FD");

    // Plugin is update to itself
    a.check("01", pa.isUpdateFor(pa));

    // Better provides
    {
        util::plugin::Plugin up("A");
        loadPluginFile(up,
                       "Provides = FA 2.1, FB, FX\n"
                       "Requires = FC 2.0, FD");
        a.check("11", up.isUpdateFor(pa));
        a.check("12", !pa.isUpdateFor(up));
    }

    // Fewer requirements
    {
        util::plugin::Plugin up("A");
        loadPluginFile(up,
                       "Provides = FA 2.1, FB, FX\n"
                       "Requires = FD");
        a.check("21", up.isUpdateFor(pa));
        a.check("22", !pa.isUpdateFor(up));
    }

    // Worse provides
    {
        util::plugin::Plugin up("A");
        loadPluginFile(up,
                       "Provides = FA 1.9, FB\n"
                       "Requires = FC 2.0, FD");
        a.check("31", !up.isUpdateFor(pa));
        a.check("32", pa.isUpdateFor(up));
    }

    // Worse provides
    {
        util::plugin::Plugin up("A");
        loadPluginFile(up,
                       "Provides = FB\n"
                       "Requires = FC 2.0, FD");
        a.check("41", !up.isUpdateFor(pa));
        a.check("42", pa.isUpdateFor(up));
    }

    // Stricter requirements
    {
        util::plugin::Plugin up("A");
        loadPluginFile(up,
                       "Provides = FA 2.1, FB\n"
                       "Requires = FC 3.0, FD");
        a.check("51", !up.isUpdateFor(pa));
        a.check("52", !pa.isUpdateFor(up));
    }
}

/** Test unrelated plugins. */
AFL_TEST("util.plugin.Plugin:dependency:unrelated", a)
{
    util::plugin::Plugin pa("A"), pb("B");
    a.check("01", !pa.isConflict(pb));
    a.check("02", !pb.isConflict(pa));

    a.check("11", !pa.isUpdateFor(pb));
    a.check("12", !pb.isUpdateFor(pa));

    a.check("21", !pa.isDependingOn(pb));
    a.check("22", !pb.isDependingOn(pa));
}
