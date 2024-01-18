/**
  *  \file test/util/configurationfiletest.cpp
  *  \brief Test for util::ConfigurationFile
  */

#include "util/configurationfile.hpp"

#include "afl/io/constmemorystream.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/textfile.hpp"
#include "afl/test/testrunner.hpp"

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
AFL_TEST("util.ConfigurationFile:load", a)
{
    // Test data
    afl::io::ConstMemoryStream in(afl::string::toBytes(TEST_FILE));
    afl::io::TextFile tf(in);

    // Parse it
    util::ConfigurationFile testee;
    testee.load(tf);

    // Verify
    a.checkEqual("01. getNumElements", testee.getNumElements(), 6U);

    // - first assignment
    const util::ConfigurationFile::Element* p = testee.getElementByIndex(0);
    a.checkNonNull("11. elem 0", p);
    a.checkEqual("12. type",   p->type, util::ConfigurationFile::Assignment);
    a.checkEqual("13. key",    p->key,  "PRE");
    a.checkEqual("14. prefix", p->prefix, " pre = ");
    a.checkEqual("15. value",  p->value, "1");

    // - section
    p = testee.getElementByIndex(1);
    a.checkNonNull("21. elem 1", p);
    a.checkEqual("22. type",   p->type, util::ConfigurationFile::Section);
    a.checkEqual("23. key",    p->key,  "SECTION");
    a.checkEqual("24. prefix", p->prefix, "\n; note\n% section");
    a.checkEqual("25. value",  p->value, "");

    // - assignment
    p = testee.getElementByIndex(2);
    a.checkNonNull("31. elem 2", p);
    a.checkEqual("32. type",   p->type, util::ConfigurationFile::Assignment);
    a.checkEqual("33. key",    p->key,  "SECTION.SEC");
    a.checkEqual("34. prefix", p->prefix, "  sec=");
    a.checkEqual("35. value",  p->value, "2");

    // - another section
    p = testee.getElementByIndex(3);
    a.checkNonNull("41. elem 3", p);
    a.checkEqual("42. type",   p->type, util::ConfigurationFile::Section);
    a.checkEqual("43. key",    p->key,  "MORE");
    a.checkEqual("44. prefix", p->prefix, "[more]");
    a.checkEqual("45. value",  p->value, "");

    // - another assignment
    p = testee.getElementByIndex(4);
    a.checkNonNull("51. elem 4", p);
    a.checkEqual("52. type",   p->type, util::ConfigurationFile::Assignment);
    a.checkEqual("53. key",    p->key,  "MORE.END");
    a.checkEqual("54. prefix", p->prefix, "# note\n  end=");
    a.checkEqual("55. value",  p->value, "4");

    // - unparsed
    p = testee.getElementByIndex(5);
    a.checkNonNull("61. elem 5", p);
    a.checkEqual("62. type",   p->type, util::ConfigurationFile::Generic);
    a.checkEqual("63. key",    p->key,  "");
    a.checkEqual("64. prefix", p->prefix, "wtf?");
    a.checkEqual("65. value",  p->value, "");

    // end
    p = testee.getElementByIndex(6);
    a.checkNull("71. end", p);
}

/** Test save(). */
AFL_TEST("util.ConfigurationFile:save", a)
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
    a.checkEqual("", afl::string::fromBytes(out.getContent()), TEST_FILE);
}

/** Test find(). */
AFL_TEST("util.ConfigurationFile:find", a)
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
    a.checkNonNull("01. PCONFIG section", p);

    p = testee.findElement(util::ConfigurationFile::Assignment, "PCONFIG.ALLOWSHIPNAMES");
    a.checkNonNull("11. assignment", p);
    a.checkEqual("12. value", p->value, "No");

    p = testee.findElement(util::ConfigurationFile::Assignment, "ALLOWSHIPNAMES");
    a.checkNull("21. assignment", p);

    p = testee.findElement(util::ConfigurationFile::Assignment, "PCONFIG");
    a.checkNull("31. assignment", p);

    p = testee.findElement(util::ConfigurationFile::Assignment, "pconfig.AllowShipNames");
    a.checkNonNull("41. assignment", p);
}

/** Test merge(). Merging into an empty object should exactly preserve the file (modulo invalid parts). */
AFL_TEST("util.ConfigurationFile:merge:into-empty", a)
{
    // Test data
    afl::io::ConstMemoryStream in(afl::string::toBytes(TEST_FILE));
    afl::io::TextFile tf(in);

    // Parse it
    util::ConfigurationFile fa;
    fa.load(tf);

    // Merge into new object
    util::ConfigurationFile fb;
    fb.merge(fa);

    // Save it
    afl::io::InternalStream out;
    afl::io::TextFile tf2(out);
    tf2.setSystemNewline(false);
    fb.save(tf2);
    tf2.flush();

    // Verify
    a.checkEqual("", afl::string::fromBytes(out.getContent()),
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
AFL_TEST("util.ConfigurationFile:merge:namespaced", a)
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

    a.checkEqual("", afl::string::fromBytes(out.getContent()),
                 "%NS\n"
                 "a=7\n"
                 "b=2\n"
                 "Q = 9\n");
}

/** Test remove(). */
AFL_TEST("util.ConfigurationFile:remove", a)
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
    a.checkNonNull("01. findElement", p);
    a.checkEqual("02. value", p->value, "No");

    a.check("11. remove", testee.remove("pconfig.allowshipnames"));

    p = testee.findElement(util::ConfigurationFile::Assignment, "pconfig.Allowshipnames");
    a.checkNonNull("21. findElement", p);
    a.checkEqual("22. value", p->value, "Yes");

    a.check("31. remove", testee.remove("PCONFIG.allowshipnames"));

    p = testee.findElement(util::ConfigurationFile::Assignment, "pCONFIG.Allowshipnames");
    a.checkNull("41. findElement", p);
}

/** Test add(). */
AFL_TEST("util.ConfigurationFile:add", a)
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

    a.checkEqual("", afl::string::fromBytes(out.getContent()),
                 "    FILTER=f1\n"
                 "    FILTER=f2\n"
                 "    filter = f3\n"
                 "    other = o\n"
                 "% sec\n"
                 "  filter = f4\n");
}

/** Test set(). */
AFL_TEST("util.ConfigurationFile:set", a)
{
    // Test data
    afl::io::ConstMemoryStream in(afl::string::toBytes(TEST_FILE));
    afl::io::TextFile tf(in);

    // Parse it
    util::ConfigurationFile testee;
    testee.load(tf);

    // Add stuff
    testee.set("pre", "one");
    testee.set("section.sec", "two");
    testee.set("more.end", "four");
    testee.set("newpre", "n1");
    testee.set("newsec.item", "n2");

    // Verify
    afl::io::InternalStream out;
    afl::io::TextFile tfo(out);
    tfo.setSystemNewline(false);
    testee.save(tfo);
    tfo.flush();

    a.checkEqual("", afl::string::fromBytes(out.getContent()),
                 " pre = one\n"
                 " newpre = n1\n"
                 "\n"
                 "; note\n"
                 "% section\n"
                 "  sec=two\n"
                 "[more]\n"
                 "# note\n"
                 "  end=four\n"
                 "wtf?\n"
                 "% newsec\n"
                 "  item = n2\n");
}
