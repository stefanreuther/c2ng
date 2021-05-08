/**
  *  \file u/t_interpreter_mutexlist.cpp
  *  \brief Test for interpreter::MutexList
  */

#include "interpreter/mutexlist.hpp"

#include "t_interpreter.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/mutexcontext.hpp"
#include "interpreter/process.hpp"
#include "interpreter/world.hpp"

/** Test destruction order. */
void
TestInterpreterMutexList::testDestruction()
{
    {
        // Destroy MutexContext first, MutexList last.
        std::auto_ptr<interpreter::MutexContext> ctx;
        interpreter::MutexList testee;
        ctx.reset(new interpreter::MutexContext(testee.create("foo", "bar", 0)));
        ctx.reset();
    }
    {
        // Destroy MutexList first, MutexContext last. This will abandon the mutex in the meantime.
        std::auto_ptr<interpreter::MutexContext> ctx;
        interpreter::MutexList testee;
        ctx.reset(new interpreter::MutexContext(testee.create("foo", "bar", 0)));
    }
}

/** General usage test. */
void
TestInterpreterMutexList::testIt()
{
    // Environment
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);
    interpreter::Process p1(world, "1", 1);
    interpreter::Process p2(world, "2", 2);

    // Testee
    interpreter::MutexList testee;

    // Create a mutex
    interpreter::MutexList::Mutex* m1 = 0;
    TS_ASSERT_THROWS_NOTHING(m1 = testee.create("M1", "Note 1", &p1));
    TS_ASSERT(m1 != 0);

    // Create another mutex
    interpreter::MutexList::Mutex* m2 = 0;
    TS_ASSERT_THROWS_NOTHING(m2 = testee.create("M2", "Note 2", &p2));
    TS_ASSERT(m2 != 0);
    TS_ASSERT(m1 != m2);

    // Query
    TS_ASSERT_EQUALS(testee.query("M1"), m1);
    TS_ASSERT_EQUALS(testee.query("M2"), m2);
    TS_ASSERT(testee.query("OTHER") == 0);

    // Query mutex objects
    TS_ASSERT_EQUALS(m1->getName(), "M1");
    TS_ASSERT_EQUALS(m1->getNote(), "Note 1");
    TS_ASSERT_EQUALS(m1->getOwner(), &p1);

    // Query mutexes by process
    {
        std::vector<interpreter::MutexList::Mutex*> list;
        testee.enumMutexes(list, &p1);
        TS_ASSERT_EQUALS(list.size(), 1U);
        TS_ASSERT_EQUALS(list[0], m1);
    }
    {
        std::vector<interpreter::MutexList::Mutex*> list;
        testee.enumMutexes(list, 0);
        TS_ASSERT_EQUALS(list.size(), 2U);
    }

    // Collision
    TS_ASSERT_THROWS(testee.create("M1", "Note 1a", &p1), interpreter::Error);
    TS_ASSERT_THROWS(testee.create("M1", "Note 2a", &p2), interpreter::Error);
    TS_ASSERT_THROWS(testee.load("M1", "Note 2b", &p2),   interpreter::Error);

    // Not a collision, p1 left with refcount 2
    TS_ASSERT_THROWS_NOTHING(testee.load("M1", "Note 1b", &p1));

    // Free the mutexes
    m2->removeReference();
    TS_ASSERT(testee.query("M2") == 0);

    m1->removeReference();
    TS_ASSERT_EQUALS(testee.query("M1"), m1);
    m1->removeReference();
    TS_ASSERT(testee.query("M1") == 0);

    // Not a collision anymore
    m1 = 0;
    TS_ASSERT_THROWS_NOTHING(m1 = testee.create("M1", "Note 2c", &p2));
    TS_ASSERT(m1 != 0);
    TS_ASSERT_EQUALS(m1->getName(), "M1");
    TS_ASSERT_EQUALS(m1->getNote(), "Note 2c");
    TS_ASSERT_EQUALS(m1->getOwner(), &p2);
    m1->removeReference();
}

/** Test abandonment.
    If the MutexList dies while there are outstanding references, these must not yet be destroyed. */
void
TestInterpreterMutexList::testAbandon()
{
    // Environment
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);
    interpreter::Process p1(world, "1", 1);

    interpreter::MutexList::Mutex* m1 = 0;

    // Testee
    {
        interpreter::MutexList testee;
        TS_ASSERT_THROWS_NOTHING(m1 = testee.create("M1", "Note 1", &p1));
        TS_ASSERT(m1 != 0);
        TS_ASSERT_EQUALS(m1->getName(), "M1");
    }

    // Mutex now abandoned
    TS_ASSERT_EQUALS(m1->getName(), "<dead>");
    m1->removeReference();
}

/** Test disowning. */
void
TestInterpreterMutexList::testDisown()
{
    // Environment
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);
    interpreter::Process p1(world, "1", 1);
    interpreter::Process p2(world, "2", 2);

    // Testee
    interpreter::MutexList testee;

    // Create a mutex
    interpreter::MutexList::Mutex* m1 = 0;
    TS_ASSERT_THROWS_NOTHING(m1 = testee.create("M1", "Note 1", &p1));
    TS_ASSERT(m1 != 0);

    // Disown the mutex (pretend the process dies)
    testee.disownLocksByProcess(&p1);

    // Locking conflict!
    interpreter::MutexList::Mutex* m2 = 0;
    TS_ASSERT_THROWS(m2 = testee.create("M1", "Note 1a", &p2), interpreter::Error);

    // Remove m1
    m1->removeReference();

    // Locking conflict now gone
    TS_ASSERT_THROWS_NOTHING(m2 = testee.create("M1", "Note 1a", &p2));
    TS_ASSERT(m2 != 0);
    TS_ASSERT_EQUALS(m2->getOwner(), &p2);
    m2->removeReference();
}

