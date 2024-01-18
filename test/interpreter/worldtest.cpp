/**
  *  \file test/interpreter/worldtest.cpp
  *  \brief Test for interpreter::World
  */

#include "interpreter/world.hpp"

#include "afl/io/constmemorystream.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/specialcommand.hpp"
#include "interpreter/values.hpp"

/** Simple tests. */
AFL_TEST("interpreter.World:basics", a)
{
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    afl::sys::Log log;

    // Create world
    interpreter::World w(log, tx, fs);
    const interpreter::World& cw = w;

    // Verify sub-object accessors
    a.checkEqual("01. globalPropertyNames", &w.globalPropertyNames(), &cw.globalPropertyNames());
    a.checkEqual("02. shipPropertyNames",   &w.shipPropertyNames(),   &cw.shipPropertyNames());
    a.checkEqual("03. planetPropertyNames", &w.planetPropertyNames(), &cw.planetPropertyNames());
    a.checkEqual("04. globalValues",        &w.globalValues(),        &cw.globalValues());
    a.checkEqual("05. shipProperties",      &w.shipProperties(),      &cw.shipProperties());
    a.checkEqual("06. planetProperties",    &w.planetProperties(),    &cw.planetProperties());
    a.checkEqual("07. keymaps",             &w.keymaps(),             &cw.keymaps());
    a.checkEqual("08. atomTable",           &w.atomTable(),           &cw.atomTable());
    a.checkEqual("09. mutexList",           &w.mutexList(),           &cw.mutexList());
    a.checkEqual("10. fileTable",           &w.fileTable(),           &cw.fileTable());

    a.checkDifferent("11. planetPropertyNames", &w.planetPropertyNames(), &w.shipPropertyNames());
    a.checkDifferent("12. planetPropertyNames", &w.planetPropertyNames(), &w.globalPropertyNames());
    a.checkDifferent("13. shipPropertyNames",   &w.shipPropertyNames(),   &w.globalPropertyNames());
    a.checkDifferent("14. shipProperties",      &w.shipProperties(),      &w.planetProperties());

    a.checkEqual("21. fileSystem",  &w.fileSystem(),  &fs);
    a.checkEqual("22. logListener", &w.logListener(), &log);
    a.checkEqual("23. translator",  &w.translator(),  &tx);
    a.checkEqual("24. .translator", &cw.translator(), &tx);

    // Global values
    afl::data::NameMap::Index_t ix = w.globalPropertyNames().getIndexByName("A");
    a.checkDifferent("31. index A", ix, afl::data::NameMap::nil);
    a.checkNull("32. value A", w.globalValues()[ix]);

    ix = w.globalPropertyNames().getIndexByName("NEW_VALUE");
    a.checkEqual("41. undef index", ix, afl::data::NameMap::nil);

    w.setNewGlobalValue("NEW_VALUE", interpreter::makeStringValue("hi"));
    ix = w.globalPropertyNames().getIndexByName("NEW_VALUE");
    a.checkDifferent("51. new index", ix, afl::data::NameMap::nil);
    a.checkNonNull("52. new value", w.globalValues()[ix]);
    a.checkEqual("53. new value", interpreter::toString(w.globalValues()[ix], false), "hi");

    a.check("61. getGlobalValue", w.globalValues()[ix] == w.getGlobalValue("NEW_VALUE"));
    a.check("62. getGlobalValue", w.globalValues()[ix] == cw.getGlobalValue("NEW_VALUE"));
}

/** Test special command handling. */
AFL_TEST("interpreter.World:special", a)
{
    // A special command implementation
    class MySpecial : public interpreter::SpecialCommand {
     public:
        MySpecial(int nr)
            : m_number(nr)
            { }
        virtual void compileCommand(interpreter::Tokenizer& /*line*/, interpreter::BytecodeObject& /*bco*/, const interpreter::StatementCompilationContext& /*scc*/)
            { }
        int get() const
            { return m_number; }
     private:
        const int m_number;
    };

    // Create world
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    interpreter::World w(log, tx, fs);

    // Initial state
    a.checkNull("01. lookupSpecialCommand", w.lookupSpecialCommand("SC"));

    // Add commands
    w.addNewSpecialCommand("SC", new MySpecial(1));
    w.addNewSpecialCommand("SC2", new MySpecial(2));

    MySpecial* p = dynamic_cast<MySpecial*>(w.lookupSpecialCommand("SC"));
    a.check("11. lookupSpecialCommand", p != 0);
    a.checkEqual("12. get", p->get(), 1);

    p = dynamic_cast<MySpecial*>(w.lookupSpecialCommand("SC2"));
    a.check("21. lookupSpecialCommand", p != 0);
    a.checkEqual("22. get", p->get(), 2);

    // Overwrite one
    w.addNewSpecialCommand("SC", new MySpecial(99));

    p = dynamic_cast<MySpecial*>(w.lookupSpecialCommand("SC"));
    a.check("31. lookupSpecialCommand", p != 0);
    a.checkEqual("32. get", p->get(), 99);

    p = dynamic_cast<MySpecial*>(w.lookupSpecialCommand("SC2"));
    a.check("41. lookupSpecialCommand", p != 0);
    a.checkEqual("42. get", p->get(), 2);
}

/** Test load directory handling. */
AFL_TEST("interpreter.World:load-path", a)
{
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    interpreter::World w(log, tx, fs);

    // Verify
    a.checkEqual("01. fileSystem", &w.fileSystem(), &fs);
    a.checkNull("02. getLocalLoadDirectory", w.getLocalLoadDirectory().get());
    a.checkNull("03. getSystemLoadDirectory", w.getSystemLoadDirectory().get());

    // Load with no directories set
    afl::base::Ptr<afl::io::Stream> s = w.openLoadFile("x");
    a.checkNull("11. openLoadFile", s.get());

    // Make a local load directory
    afl::base::Ref<afl::io::InternalDirectory> localDir = afl::io::InternalDirectory::create("i");
    localDir->addStream("x", *new afl::io::ConstMemoryStream(afl::string::toBytes("1")));
    localDir->addStream("y", *new afl::io::ConstMemoryStream(afl::string::toBytes("11")));
    w.setLocalLoadDirectory(localDir.asPtr());

    // Load
    s = w.openLoadFile("x");
    a.checkNonNull("21. openLoadFile", s.get());
    a.checkEqual("22. getSize", s->getSize(), 1U);

    s = w.openLoadFile("y");
    a.checkNonNull("31. openLoadFile", s.get());
    a.checkEqual("32. getSize", s->getSize(), 2U);

    s = w.openLoadFile("z");
    a.checkNull("41. openLoadFile", s.get());

    // Make a system load directory
    afl::base::Ref<afl::io::InternalDirectory> sysDir = afl::io::InternalDirectory::create("s");
    sysDir->addStream("y", *new afl::io::ConstMemoryStream(afl::string::toBytes("111")));
    sysDir->addStream("z", *new afl::io::ConstMemoryStream(afl::string::toBytes("1111")));
    w.setSystemLoadDirectory(sysDir.asPtr());

    // Load
    s = w.openLoadFile("x");
    a.checkNonNull("51. openLoadFile", s.get());
    a.checkEqual("52. getSize", s->getSize(), 1U);

    s = w.openLoadFile("y");
    a.checkNonNull("61. openLoadFile", s.get());
    a.checkEqual("62. getSize", s->getSize(), 2U);

    s = w.openLoadFile("z");
    a.checkNonNull("71. openLoadFile", s.get());
    a.checkEqual("72. getSize", s->getSize(), 4U);
}
