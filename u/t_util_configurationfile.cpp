/**
  *  \file u/t_util_configurationfile.cpp
  *  \brief Test for util::ConfigurationFile
  */

#include "util/configurationfile.hpp"

#include "t_util.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/textfile.hpp"
#include "afl/io/internalstream.hpp"

namespace {
    const char*const TEST_FILE =
        " pre = 1\n"      // #0
        "\n"              // #1
        "; note\n"
        "% section\n"
        "  sec=2\n"       // #2
        "[more]\n"        // #3
        "# note\n"        // #4
        "  end=4\n"
        "wtf?\n";         // #5
}

/** Test load(). */
void
TestUtilConfigurationFile::testLoad()
{
    // Test data
    afl::io::ConstMemoryStream in(afl::string::toBytes(TEST_FILE));
    afl::io::TextFile tf(in);

    // Parse it
    util::ConfigurationFile testee;
    testee.load(tf);

    // Verify
    TS_ASSERT_EQUALS(testee.getNumElements(), 6U);

    // - first assignment
    const util::ConfigurationFile::Element* p = testee.getElementByIndex(0);
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->type, util::ConfigurationFile::Assignment);
    TS_ASSERT_EQUALS(p->key,  "PRE");
    TS_ASSERT_EQUALS(p->prefix, " pre = ");
    TS_ASSERT_EQUALS(p->value, "1");

    // - section
    p = testee.getElementByIndex(1);
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->type, util::ConfigurationFile::Section);
    TS_ASSERT_EQUALS(p->key,  "SECTION");
    TS_ASSERT_EQUALS(p->prefix, "\n; note\n% section");
    TS_ASSERT_EQUALS(p->value, "");

    // - assignment
    p = testee.getElementByIndex(2);
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->type, util::ConfigurationFile::Assignment);
    TS_ASSERT_EQUALS(p->key,  "SECTION.SEC");
    TS_ASSERT_EQUALS(p->prefix, "  sec=");
    TS_ASSERT_EQUALS(p->value, "2");

    // - another section
    p = testee.getElementByIndex(3);
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->type, util::ConfigurationFile::Section);
    TS_ASSERT_EQUALS(p->key,  "MORE");
    TS_ASSERT_EQUALS(p->prefix, "[more]");
    TS_ASSERT_EQUALS(p->value, "");

    // - another assignment
    p = testee.getElementByIndex(4);
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->type, util::ConfigurationFile::Assignment);
    TS_ASSERT_EQUALS(p->key,  "MORE.END");
    TS_ASSERT_EQUALS(p->prefix, "# note\n  end=");
    TS_ASSERT_EQUALS(p->value, "4");

    // - unparsed
    p = testee.getElementByIndex(5);
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->type, util::ConfigurationFile::Generic);
    TS_ASSERT_EQUALS(p->key,  "");
    TS_ASSERT_EQUALS(p->prefix, "wtf?");
    TS_ASSERT_EQUALS(p->value, "");

    // end
    p = testee.getElementByIndex(6);
    TS_ASSERT(p == 0);
}

/** Test save(). */
void
TestUtilConfigurationFile::testSave()
{
    // Test data
    afl::io::ConstMemoryStream in(afl::string::toBytes(TEST_FILE));
    afl::io::TextFile tf(in);

    // Parse it
    util::ConfigurationFile testee;
    testee.load(tf);

    // Save it
    afl::io::InternalStream out;
    afl::io::TextFile tf2(out);
    tf2.setSystemNewline(false);
    testee.save(tf2);
    tf2.flush();

    // Verify
    TS_ASSERT_EQUALS(afl::string::fromBytes(out.getContent()), TEST_FILE);
}

/** Test find(). */
void
TestUtilConfigurationFile::testFind()
{
    // Test data
    afl::io::ConstMemoryStream in(afl::string::toBytes("%pconfig\n"
                                                       "AllowShipNames = Yes\n"
                                                       "NumShips = 300\n"
                                                       "AllowShipNames = No\n"));
    afl::io::TextFile tf(in);

    // Parse it
    util::ConfigurationFile testee;
    testee.load(tf);

    // Verify
    const util::ConfigurationFile::Element* p = testee.findElement(util::ConfigurationFile::Section, "PCONFIG");
    TS_ASSERT(p != 0);

    p = testee.findElement(util::ConfigurationFile::Assignment, "PCONFIG.ALLOWSHIPNAMES");
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->value, "No");

    p = testee.findElement(util::ConfigurationFile::Assignment, "ALLOWSHIPNAMES");
    TS_ASSERT(p == 0);

    p = testee.findElement(util::ConfigurationFile::Assignment, "PCONFIG");
    TS_ASSERT(p == 0);

    p = testee.findElement(util::ConfigurationFile::Assignment, "pconfig.AllowShipNames");
    TS_ASSERT(p != 0);
}

/** Test merge(). Merging into an empty object should exactly preserve the file (modulo invalid parts). */
void
TestUtilConfigurationFile::testMergePreserve()
{
    // Test data
    afl::io::ConstMemoryStream in(afl::string::toBytes(TEST_FILE));
    afl::io::TextFile tf(in);

    // Parse it
    util::ConfigurationFile a;
    a.load(tf);

    // Merge into new object
    util::ConfigurationFile b;
    b.merge(a);

    // Save it
    afl::io::InternalStream out;
    afl::io::TextFile tf2(out);
    tf2.setSystemNewline(false);
    b.save(tf2);
    tf2.flush();

    // Verify
    TS_ASSERT_EQUALS(afl::string::fromBytes(out.getContent()),
                     " pre = 1\n"
                     "\n"
                     "; note\n"
                     "% section\n"
                     "  sec=2\n"
                     "[more]\n"
                     "# note\n"
                     "  end=4\n");
}

/** Test merge(). Merging a file with "NS.KEY" assignments should merge into section "%NS". */
void
TestUtilConfigurationFile::testMergeNamespaced()
{
    // Test data
    // - part 1
    afl::io::ConstMemoryStream in1(afl::string::toBytes("%NS\n"
                                                        "a=1\n"
                                                        "b=2\n"));
    afl::io::TextFile tf1(in1);
    util::ConfigurationFile c1;
    c1.load(tf1);

    // - part 2
    afl::io::ConstMemoryStream in2(afl::string::toBytes("ns.a=7\n"
                                                        "ns.q=9\n"));
    afl::io::TextFile tf2(in2);
    util::ConfigurationFile c2;
    c2.load(tf2);

    // Merge!
    c1.merge(c2);

    // Verify
    afl::io::InternalStream out;
    afl::io::TextFile tfo(out);
    tfo.setSystemNewline(false);
    c1.save(tfo);
    tfo.flush();
    
    TS_ASSERT_EQUALS(afl::string::fromBytes(out.getContent()),
                     "%NS\n"
                     "a=7\n"
                     "b=2\n"
                     "Q = 9\n");
}

/** Test remove(). */
void
TestUtilConfigurationFile::testRemove()
{
    // Test data
    afl::io::ConstMemoryStream in(afl::string::toBytes("%pconfig\n"
                                                       "AllowShipNames = Yes\n"
                                                       "NumShips = 300\n"
                                                       "AllowShipNames = No\n"));
    afl::io::TextFile tf(in);

    // Parse it
    util::ConfigurationFile testee;
    testee.load(tf);

    // Verify
    const util::ConfigurationFile::Element* p = testee.findElement(util::ConfigurationFile::Assignment, "PCONFIG.ALLOWSHIPNAMES");
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->value, "No");

    TS_ASSERT(testee.remove("pconfig.allowshipnames"));

    p = testee.findElement(util::ConfigurationFile::Assignment, "pconfig.Allowshipnames");
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->value, "Yes");

    TS_ASSERT(testee.remove("PCONFIG.allowshipnames"));
    
    p = testee.findElement(util::ConfigurationFile::Assignment, "pCONFIG.Allowshipnames");
    TS_ASSERT(p == 0);
}

/** Test add(). */
void
TestUtilConfigurationFile::testAdd()
{
    // Test data
    afl::io::ConstMemoryStream in(afl::string::toBytes("    FILTER=f1\n"
                                                       "    FILTER=f2\n"));
    afl::io::TextFile tf(in);

    // Parse it
    util::ConfigurationFile testee;
    testee.load(tf);

    // Add to it
    testee.add("other", "o");
    testee.add("filter", "f3");
    testee.add("sec", "filter", "f4");

    // Verify
    afl::io::InternalStream out;
    afl::io::TextFile tfo(out);
    tfo.setSystemNewline(false);
    testee.save(tfo);
    tfo.flush();
    
    TS_ASSERT_EQUALS(afl::string::fromBytes(out.getContent()),
                     "    FILTER=f1\n"
                     "    FILTER=f2\n"
                     "    filter = f3\n"
                     "    other = o\n"
                     "% sec\n"
                     "  filter = f4\n");
}

