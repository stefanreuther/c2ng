/**
  *  \file u/t_interpreter_world.cpp
  *  \brief Test for interpreter::World
  */

#include "interpreter/world.hpp"

#include "t_interpreter.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/values.hpp"
#include "interpreter/specialcommand.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/constmemorystream.hpp"

/** Simple tests. */
void
TestInterpreterWorld::testIt()
{
    afl::io::NullFileSystem fs;
    afl::sys::Log log;

    // Create world
    interpreter::World w(log, fs);
    const interpreter::World& cw = w;

    // Verify sub-object accessors
    TS_ASSERT_EQUALS(&w.globalPropertyNames(), &cw.globalPropertyNames());
    TS_ASSERT_EQUALS(&w.shipPropertyNames(),   &cw.shipPropertyNames());
    TS_ASSERT_EQUALS(&w.planetPropertyNames(), &cw.planetPropertyNames());
    TS_ASSERT_EQUALS(&w.globalValues(),        &cw.globalValues());
    TS_ASSERT_EQUALS(&w.shipProperties(),      &cw.shipProperties());
    TS_ASSERT_EQUALS(&w.planetProperties(),    &cw.planetProperties());
    TS_ASSERT_EQUALS(&w.keymaps(),             &cw.keymaps());
    TS_ASSERT_EQUALS(&w.atomTable(),           &cw.atomTable());
    TS_ASSERT_EQUALS(&w.processList(),         &cw.processList());
    TS_ASSERT_EQUALS(&w.mutexList(),           &cw.mutexList());
    TS_ASSERT_EQUALS(&w.fileTable(),           &cw.fileTable());

    TS_ASSERT_DIFFERS(&w.planetPropertyNames(), &w.shipPropertyNames());
    TS_ASSERT_DIFFERS(&w.planetPropertyNames(), &w.globalPropertyNames());
    TS_ASSERT_DIFFERS(&w.shipPropertyNames(),   &w.globalPropertyNames());
    TS_ASSERT_DIFFERS(&w.shipProperties(),      &w.planetProperties());

    TS_ASSERT_EQUALS(&w.fileSystem(),  &fs);
    TS_ASSERT_EQUALS(&w.logListener(), &log);

    // Global values
    afl::data::NameMap::Index_t ix = w.globalPropertyNames().getIndexByName("A");
    TS_ASSERT_DIFFERS(ix, afl::data::NameMap::nil);
    TS_ASSERT(w.globalValues()[ix] == 0);

    ix = w.globalPropertyNames().getIndexByName("NEW_VALUE");
    TS_ASSERT_EQUALS(ix, afl::data::NameMap::nil);

    w.setNewGlobalValue("NEW_VALUE", interpreter::makeStringValue("hi"));
    ix = w.globalPropertyNames().getIndexByName("NEW_VALUE");
    TS_ASSERT_DIFFERS(ix, afl::data::NameMap::nil);
    TS_ASSERT(w.globalValues()[ix] != 0);
    TS_ASSERT_EQUALS(interpreter::toString(w.globalValues()[ix], false), "hi");
}

/** Test special command handling. */
void
TestInterpreterWorld::testSpecial()
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
    afl::sys::Log log;
    interpreter::World w(log, fs);

    // Initial state
    TS_ASSERT(w.lookupSpecialCommand("SC") == 0);

    // Add commands
    w.addNewSpecialCommand("SC", new MySpecial(1));
    w.addNewSpecialCommand("SC2", new MySpecial(2));

    MySpecial* p = dynamic_cast<MySpecial*>(w.lookupSpecialCommand("SC"));
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->get(), 1);

    p = dynamic_cast<MySpecial*>(w.lookupSpecialCommand("SC2"));
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->get(), 2);

    // Overwrite one
    w.addNewSpecialCommand("SC", new MySpecial(99));

    p = dynamic_cast<MySpecial*>(w.lookupSpecialCommand("SC"));
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->get(), 99);

    p = dynamic_cast<MySpecial*>(w.lookupSpecialCommand("SC2"));
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->get(), 2);
}

/** Test load directory handling. */
void
TestInterpreterWorld::testLoad()
{
    afl::io::NullFileSystem fs;
    afl::sys::Log log;
    interpreter::World w(log, fs);

    // Verify
    TS_ASSERT_EQUALS(&w.fileSystem(), &fs);
    TS_ASSERT(w.getLocalLoadDirectory().get() == 0);
    TS_ASSERT(w.getSystemLoadDirectory().get() == 0);

    // Load with no directories set
    afl::base::Ptr<afl::io::Stream> s = w.openLoadFile("x");
    TS_ASSERT(s.get() == 0);

    // Make a local load directory
    afl::base::Ref<afl::io::InternalDirectory> localDir = afl::io::InternalDirectory::create("i");
    localDir->addStream("x", *new afl::io::ConstMemoryStream(afl::string::toBytes("1")));
    localDir->addStream("y", *new afl::io::ConstMemoryStream(afl::string::toBytes("11")));
    w.setLocalLoadDirectory(localDir.asPtr());

    // Load
    s = w.openLoadFile("x");
    TS_ASSERT(s.get() != 0);
    TS_ASSERT_EQUALS(s->getSize(), 1U);

    s = w.openLoadFile("y");
    TS_ASSERT(s.get() != 0);
    TS_ASSERT_EQUALS(s->getSize(), 2U);

    s = w.openLoadFile("z");
    TS_ASSERT(s.get() == 0);

    // Make a system load directory
    afl::base::Ref<afl::io::InternalDirectory> sysDir = afl::io::InternalDirectory::create("s");
    sysDir->addStream("y", *new afl::io::ConstMemoryStream(afl::string::toBytes("111")));
    sysDir->addStream("z", *new afl::io::ConstMemoryStream(afl::string::toBytes("1111")));
    w.setSystemLoadDirectory(sysDir.asPtr());

    // Load
    s = w.openLoadFile("x");
    TS_ASSERT(s.get() != 0);
    TS_ASSERT_EQUALS(s->getSize(), 1U);

    s = w.openLoadFile("y");
    TS_ASSERT(s.get() != 0);
    TS_ASSERT_EQUALS(s->getSize(), 2U);

    s = w.openLoadFile("z");
    TS_ASSERT(s.get() != 0);
    TS_ASSERT_EQUALS(s->getSize(), 4U);
}

