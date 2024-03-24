/**
  *  \file test/interpreter/mutexlisttest.cpp
  *  \brief Test for interpreter::MutexList
  */

#include "interpreter/mutexlist.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/mutexcontext.hpp"
#include "interpreter/process.hpp"
#include "interpreter/world.hpp"

/** Test destruction order. */
AFL_TEST_NOARG("interpreter.MutexList:destruction")
{
    // As of 20220801, this test has become pretty pointless as destruction no longer happens in the MutexContext object.
    {
        // Destroy MutexContext first, MutexList last.
        std::auto_ptr<interpreter::MutexContext> ctx;
        interpreter::MutexList testee;
        ctx.reset(new interpreter::MutexContext("FOO", "bar"));
        ctx.reset();
    }
    {
        // Destroy MutexList first, MutexContext last. This will abandon the mutex in the meantime.
        std::auto_ptr<interpreter::MutexContext> ctx;
        interpreter::MutexList testee;
        ctx.reset(new interpreter::MutexContext("FOO", "bar"));
    }
}

/** General usage test. */
AFL_TEST("interpreter.MutexList:basics", a)
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
    interpreter::MutexList::Mutex* m1 = testee.create("M1", "Note 1", &p1);
    a.checkNonNull("01. create", m1);

    // Create another mutex
    interpreter::MutexList::Mutex* m2 = testee.create("M2", "Note 2", &p2);
    a.checkNonNull("11. create", m2);
    a.checkDifferent("12", m1, m2);

    // Query
    a.checkEqual("21. M1",   testee.query("M1"), m1);
    a.checkEqual("22. M2",   testee.query("M2"), m2);
    a.checkNull("23. OTHER", testee.query("OTHER"));

    // Query mutex objects
    a.checkEqual("31. getName", m1->getName(), "M1");
    a.checkEqual("32. getNote", m1->getNote(), "Note 1");
    a.checkEqual("33. getOwner", m1->getOwner(), &p1);

    // Query mutexes by process
    {
        std::vector<interpreter::MutexList::Mutex*> list;
        testee.enumMutexes(list, &p1);
        a.checkEqual("41. size", list.size(), 1U);
        a.checkEqual("42. list", list[0], m1);
    }
    {
        std::vector<interpreter::MutexList::Mutex*> list;
        testee.enumMutexes(list, 0);
        a.checkEqual("43. size", list.size(), 2U);
    }

    // Collision
    AFL_CHECK_THROWS(a("51. create"), testee.create("M1", "Note 1a", &p1), interpreter::Error);
    AFL_CHECK_THROWS(a("52. create"), testee.create("M1", "Note 2a", &p2), interpreter::Error);
    AFL_CHECK_THROWS(a("53. load"),   testee.load("M1", "Note 2b", &p2),   interpreter::Error);

    // Not a collision, p1 left with refcount 2
    AFL_CHECK_SUCCEEDS(a("61. load"), testee.load("M1", "Note 1b", &p1));

    // Free the mutexes
    m2->removeReference();
    a.checkNull("71. M2", testee.query("M2"));
    a.check("72. M2", !testee.hasLock("M2"));

    m1->removeReference();
    a.checkEqual("81. M1", testee.query("M1"), m1);
    a.check("82. M2", testee.hasLock("M1"));
    m1->removeReference();
    a.checkNull("85. M1", testee.query("M1"));
    a.check("86. M2", !testee.hasLock("M1"));

    // Not a collision anymore
    m1 = 0;
    AFL_CHECK_SUCCEEDS(a("91. create"), m1 = testee.create("M1", "Note 2c", &p2));
    a.checkNonNull("92. create", m1);
    a.checkEqual("93. getName", m1->getName(), "M1");
    a.checkEqual("94. getNote", m1->getNote(), "Note 2c");
    a.checkEqual("95. getOwner", m1->getOwner(), &p2);
    m1->removeReference();
}

/** Test abandonment.
    If the MutexList dies while there are outstanding references, these must not yet be destroyed. */
AFL_TEST("interpreter.MutexList:abandon", a)
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
        AFL_CHECK_SUCCEEDS(a("01. create"), m1 = testee.create("M1", "Note 1", &p1));
        a.checkNonNull("02. create", m1);
        a.checkEqual("03. getName", m1->getName(), "M1");
    }

    // Mutex now abandoned
    a.checkEqual("11. getName", m1->getName(), "<dead>");
    m1->removeReference();
}

/** Test disowning. */
AFL_TEST("interpreter.MutexList:disown", a)
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
    interpreter::MutexList::Mutex* m1 = testee.create("M1", "Note 1", &p1);
    a.checkNonNull("01. create", m1);

    // Disown the mutex (pretend the process dies)
    testee.disownLocksByProcess(&p1);

    // Locking conflict!
    interpreter::MutexList::Mutex* m2 = 0;
    AFL_CHECK_THROWS(a("11. create"), m2 = testee.create("M1", "Note 1a", &p2), interpreter::Error);

    // Remove m1
    m1->removeReference();

    // Locking conflict now gone
    AFL_CHECK_SUCCEEDS(a("21. create"), m2 = testee.create("M1", "Note 1a", &p2));
    a.checkNonNull("22. create", m2);
    a.checkEqual("23. getOwner", m2->getOwner(), &p2);
    m2->removeReference();
}
