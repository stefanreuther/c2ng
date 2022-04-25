/**
  *  \file u/t_client_map_location.cpp
  *  \brief Test for client::map::Location
  */

#include "client/map/location.hpp"

#include "t_client_map.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/callreceiver.hpp"

using afl::string::Format;
using game::Reference;
using game::map::Configuration;
using game::map::Object;
using game::map::Point;
using game::proxy::LockProxy;
using game::ref::UserList;
using util::SkinColor;

namespace {
    const int SHIP_ID = 10;
    const int PLANET_ID = 33;

    /* Listener mock */
    class Listener : public client::map::Location::Listener,
                     public afl::test::CallReceiver
    {
     public:
        Listener(afl::test::Assert loc)
            : client::map::Location::Listener(), CallReceiver(loc)
            { }

        virtual void requestObjectList(Point pos)
            {
                checkCall(Format("requestObjectList(%d,%d)", pos.getX(), pos.getY()));
            }
        virtual void requestLockObject(Point pos, LockProxy::Flags_t flags)
            {
                String_t flagStr;
                if (flags.contains(LockProxy::Left)) {
                    flagStr += ",left";
                }
                if (flags.contains(LockProxy::MarkedOnly)) {
                    flagStr += ",marked";
                }
                checkCall(Format("requestLockObject(%d,%d%s)", pos.getX(), pos.getY(), flagStr));
            }
    };

    /* Test harness */
    struct TestHarness {
        Listener listener;
        afl::sys::Log log;
        client::map::Location location;

        TestHarness(afl::test::Assert loc)
            : listener(loc), log(), location(listener, log)
            {
                // Interface requires this initialisation
                location.setConfiguration(Configuration());
                location.setFocusedObject(Reference());
            }
    };

    /* Make a three-element list consisting of a title, ship, planet*/
    UserList makeList()
    {
        UserList result;
        result.add(UserList::DividerItem,   "title",  Reference(),                             false, Object::NotPlayable, SkinColor::Static);
        result.add(UserList::ReferenceItem, "ship",   Reference(Reference::Ship, SHIP_ID),     false, Object::Playable,    SkinColor::Green);
        result.add(UserList::ReferenceItem, "planet", Reference(Reference::Planet, PLANET_ID), false, Object::Playable,    SkinColor::Green);
        return result;
    }
}


/** Test regular initialisation. */
void
TestClientMapLocation::testInit()
{
    TestHarness h("testInit");

    // Cannot jump, lock, move here
    Point initial = h.location.getPosition();
    TS_ASSERT(!h.location.startJump());
    h.location.moveRelative(3, 4);
    h.location.lockObject(LockProxy::Flags_t());
    TS_ASSERT_EQUALS(h.location.getPosition(), initial);

    // Set initial position. Location will request object list.
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();

    // Provide object list.
    h.location.setObjectList(makeList());
    TS_ASSERT_EQUALS(h.location.getPosition(), Point(1200, 2300));
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 3U);
}

/** Test BuildAgain flow. */
void
TestClientMapLocation::testBuildAgain()
{
    TestHarness h("testBuildAgain");

    // Set initial position. Location will request object list.
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();

    // Modify location
    h.location.moveRelative(1, 0);
    h.location.moveRelative(2, 0);
    h.location.moveRelative(3, 0);

    // Provide object list. Location will request updated list and suppress this one.
    h.listener.expectCall("requestObjectList(1206,2300)");
    h.location.setObjectList(makeList());
    h.listener.checkFinish();
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 0U);

    // Provide final object list.
    h.location.setObjectList(makeList());
    TS_ASSERT_EQUALS(h.location.getPosition(), Point(1206, 2300));
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 3U);
}

/** Test BuildAgain flow, absolute movement. */
void
TestClientMapLocation::testBuildAgainAbs()
{
    TestHarness h("testBuildAgainAbs");

    // Set initial position. Location will request object list.
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();

    // Modify location
    h.location.setPosition(Point(1200, 2222));
    h.location.setPosition(Point(1200, 2300));

    // Provide object list. Location will request updated list and suppress this one.
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setObjectList(makeList());
    h.listener.checkFinish();
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 0U);

    // Provide final object list.
    h.location.setObjectList(makeList());
    TS_ASSERT_EQUALS(h.location.getPosition(), Point(1200, 2300));
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 3U);
}

/** Test Build, with null change. */
void
TestClientMapLocation::testBuildNull()
{
    TestHarness h("testBuildNull");

    // Set initial position. Location will request object list.
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();

    // Modify location
    h.location.setPosition(Point(1200, 2300));

    // Provide object list. Location will NOT request updated list because position did not actually change.
    h.location.setObjectList(makeList());
    h.listener.checkFinish();
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 3U);
}

/** Test jump while building. */
void
TestClientMapLocation::testBuildJump()
{
    TestHarness h("testBuildJump");

    // Set initial position. Location will request object list.
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();

    // Trigger jump
    bool ok = h.location.startJump();
    TS_ASSERT(ok);

    // Provide object list.
    h.location.setObjectList(makeList());
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 0U);

    // Provide jump result
    h.listener.expectCall("requestObjectList(500,300)");
    h.location.setPosition(Point(500, 300));
    h.listener.checkFinish();

    // Provide updated object list
    h.location.setObjectList(makeList());
    TS_ASSERT_EQUALS(h.location.getPosition(), Point(500, 300));
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 3U);
}

/** Test jump while building, jump completes first. */
void
TestClientMapLocation::testBuildJump2()
{
    TestHarness h("testBuildJump2");

    // Set initial position. Location will request object list.
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();

    // Trigger jump
    bool ok = h.location.startJump();
    TS_ASSERT(ok);

    // Cannot start another jump now
    TS_ASSERT(!h.location.startJump());
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 0U);

    // Provide jump result. Location will not yet request updated list.
    h.location.setPosition(Point(500, 300));

    // Provide object list.
    h.listener.expectCall("requestObjectList(500,300)");
    h.location.setObjectList(makeList());
    h.listener.checkFinish();

    // Provide updated object list
    h.location.setObjectList(makeList());
    TS_ASSERT_EQUALS(h.location.getPosition(), Point(500, 300));
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 3U);
}

/** Test locking, regular case. */
void
TestClientMapLocation::testLock()
{
    TestHarness h("testLock");

    // Set initial position. Location will request object list.
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();

    // Provide object list.
    h.location.setObjectList(makeList());
    TS_ASSERT_EQUALS(h.location.getPosition(), Point(1200, 2300));

    // Request to lock
    h.listener.expectCall("requestLockObject(1200,2300)");
    h.location.lockObject(LockProxy::Flags_t());
    h.listener.checkFinish();

    // Jumping not allowed now
    TS_ASSERT(!h.location.startJump());
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 0U);

    // Produce result. Location will request object list
    h.listener.expectCall("requestObjectList(1222,2333)");
    h.location.setPosition(Point(1222,2333));
    h.listener.checkFinish();

    // Provide updated object list
    h.location.setObjectList(makeList());
    TS_ASSERT_EQUALS(h.location.getPosition(), Point(1222, 2333));
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 3U);
}

/** Test locking while building object list. */
void
TestClientMapLocation::testBuildLock()
{
    TestHarness h("testBuildLock");

    // Set initial position. Location will request object list.
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();

    // Request to lock. Nothing happens yet.
    h.location.lockObject(LockProxy::Flags_t());

    // Provide object list. Location will request lock now
    h.listener.expectCall("requestLockObject(1200,2300)");
    h.location.setObjectList(makeList());
    h.listener.checkFinish();
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 0U);

    // Produce result. Location will request object list
    h.listener.expectCall("requestObjectList(1222,2333)");
    h.location.setPosition(Point(1222,2333));
    h.listener.checkFinish();

    // Provide updated object list
    h.location.setObjectList(makeList());
    TS_ASSERT_EQUALS(h.location.getPosition(), Point(1222, 2333));
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 3U);
}

/** Test locking while lock active. */
void
TestClientMapLocation::testLockAgain()
{
    TestHarness h("testLockAgain");

    // Set initial position. Location will request object list; provide it.
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();
    h.location.setObjectList(makeList());
    TS_ASSERT_EQUALS(h.location.getPosition(), Point(1200, 2300));

    // Request to lock
    h.listener.expectCall("requestLockObject(1200,2300)");
    h.location.lockObject(LockProxy::Flags_t());
    h.listener.checkFinish();
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 0U);

    // Move and request further locks.
    h.location.moveRelative(1, 0);
    h.location.lockObject(LockProxy::Flags_t());
    h.location.moveRelative(2, 0);
    h.location.lockObject(LockProxy::Flags_t());
    h.location.moveRelative(3, 0);
    h.location.lockObject(LockProxy::Flags_t());
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 0U);

    // Cannot jump at this time
    TS_ASSERT(!h.location.startJump());

    // Produce result. Location will request lock update for result + movement.
    h.listener.expectCall("requestLockObject(1506,2400)");
    h.location.setPosition(Point(1500, 2400));
    h.listener.checkFinish();

    // Further movement.
    h.location.moveRelative(4, 0);
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 0U);

    // Produce final result. Location will request object list; further movement is ignored.
    // (Reconsider if LockState::onPositionChange is modified.)
    h.listener.expectCall("requestObjectList(1222,2333)");
    h.location.setPosition(Point(1222,2333));
    h.listener.checkFinish();

    // Provide updated object list
    h.location.setObjectList(makeList());
    TS_ASSERT_EQUALS(h.location.getPosition(), Point(1222, 2333));
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 3U);
}

/** Test build/jump/lock combo. */
void
TestClientMapLocation::testBuildJumpLock()
{
    TestHarness h("testBuildJumpLock");

    // Set initial position. Location will request object list; provide it.
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();

    // Trigger jump
    bool ok = h.location.startJump();
    TS_ASSERT(ok);

    // Cannot jump again
    TS_ASSERT(!h.location.startJump());
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 0U);

    // Trigger lock
    h.location.lockObject(LockProxy::Flags_t());

    // Still cannot jump
    TS_ASSERT(!h.location.startJump());

    // Provide object list; keep waiting for jump result
    h.location.setObjectList(makeList());
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 0U);

    // Still cannot jump
    TS_ASSERT(!h.location.startJump());

    // Move, because why not
    h.location.moveRelative(0, 2);

    // Provide jump result; this triggers the lock
    h.listener.expectCall("requestLockObject(2000,3002)");
    h.location.setPosition(Point(2000, 3000));
    h.listener.checkFinish();

    // Provide lock result. Location will request object list.
    h.listener.expectCall("requestObjectList(2500,2600)");
    h.location.setPosition(Point(2500,2600));
    h.listener.checkFinish();

    // Provide updated object list
    h.location.setObjectList(makeList());
    TS_ASSERT_EQUALS(h.location.getPosition(), Point(2500, 2600));
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 3U);
}

/** Test build/jump/lock combo, different order. */
void
TestClientMapLocation::testBuildJumpLock2()
{
    TestHarness h("testBuildJumpLock");

    // Set initial position. Location will request object list; provide it.
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();

    // Trigger jump
    bool ok = h.location.startJump();
    TS_ASSERT(ok);

    // Trigger lock
    h.location.lockObject(LockProxy::Flags_t());

    // Provide jump result; keep waiting for jump result
    h.location.setPosition(Point(2000, 3000));

    // Move, because why not
    h.location.moveRelative(0, 2);

    // Provide object list; this triggers the lock
    h.listener.expectCall("requestLockObject(2000,3002)");
    h.location.setObjectList(makeList());
    h.listener.checkFinish();
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 0U);

    // Provide lock result. Location will request object list.
    h.listener.expectCall("requestObjectList(2500,2600)");
    h.location.setPosition(Point(2500,2600));
    h.listener.checkFinish();

    // Provide updated object list
    h.location.setObjectList(makeList());
    TS_ASSERT_EQUALS(h.location.getPosition(), Point(2500, 2600));
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 3U);
}

/** Test jump while building, from BuildAgain. */
void
TestClientMapLocation::testBuildAgainJump()
{
    TestHarness h("testBuildAgainJump");

    // Set initial position. Location will request object list.
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();

    // Modify location
    h.location.moveRelative(1, 0);
    h.location.moveRelative(2, 0);
    h.location.moveRelative(3, 0);

    // Request jump
    bool ok = h.location.startJump();
    TS_ASSERT(ok);

    // Cannot start another jump now
    TS_ASSERT(!h.location.startJump());

    // Provide object list.
    h.location.setObjectList(makeList());
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 0U);

    // Provide jump result. Location will request object list.
    h.listener.expectCall("requestObjectList(1300,1400)");
    h.location.setPosition(Point(1300, 1400));
    h.listener.checkFinish();

    // Provide final object list.
    h.location.setObjectList(makeList());
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 3U);
}

/** Test lock while building, from BuildAgain. */
void
TestClientMapLocation::testBuildAgainLock()
{
    TestHarness h("testBuildAgain");

    // Set initial position. Location will request object list.
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();

    // Modify location
    h.location.moveRelative(0, 1);
    h.location.moveRelative(0, 2);
    h.location.moveRelative(0, 3);

    // Request lock
    h.location.lockObject(LockProxy::Flags_t());

    // Cannot start a jump now
    TS_ASSERT(!h.location.startJump());

    // Provide object list, Location will initiate jump.
    h.listener.expectCall("requestLockObject(1200,2306)");
    h.location.setObjectList(makeList());
    h.listener.checkFinish();
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 0U);

    // Provide jump result. Location will request object list.
    h.listener.expectCall("requestObjectList(1300,1400)");
    h.location.setPosition(Point(1300, 1400));
    h.listener.checkFinish();

    // Provide final object list.
    h.location.setObjectList(makeList());
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 3U);
}

/** Test absolute movement. */
void
TestClientMapLocation::testMoveAbs()
{
    TestHarness h("testMoveAbs");

    // Regular startup
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();
    h.location.setObjectList(UserList());

    // Set new absolute position
    h.listener.expectCall("requestObjectList(2000,3000)");
    h.location.setPosition(Point(2000, 3000));
    h.listener.checkFinish();
    h.location.setObjectList(makeList());
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 3U);
}

/** Test relative movement. */
void
TestClientMapLocation::testMoveRel()
{
    TestHarness h("testMoveAbs");

    // Regular startup
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();
    h.location.setObjectList(UserList());

    // Move relative
    h.listener.expectCall("requestObjectList(1210,2320)");
    h.location.moveRelative(10, 20);
    h.listener.checkFinish();
    h.location.setObjectList(makeList());
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 3U);
}

/** Test regular jump. */
void
TestClientMapLocation::testJump()
{
    TestHarness h("testJump");

    // Regular startup
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();
    h.location.setObjectList(makeList());

    // Trigger jump
    bool ok = h.location.startJump();
    TS_ASSERT(ok);

    // Cannot start another jump now
    TS_ASSERT(!h.location.startJump());
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 0U);

    // Finish jump
    h.listener.expectCall("requestObjectList(2000,3000)");
    h.location.setPosition(Point(2000, 3000));
    h.listener.checkFinish();
    h.location.setObjectList(makeList());
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 3U);
}

/** Test lock while jumping. */
void
TestClientMapLocation::testJumpLock()
{
    TestHarness h("testJump");

    // Regular startup
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();
    h.location.setObjectList(makeList());

    // Trigger jump
    bool ok = h.location.startJump();
    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 0U);

    // Request lock
    h.location.lockObject(LockProxy::Flags_t());

    // Finish jump; this will cause the lock to be executed
    h.listener.expectCall("requestLockObject(2000,3000)");
    h.location.setPosition(Point(2000, 3000));
    h.listener.checkFinish();

    // Finish lock
    h.listener.expectCall("requestObjectList(2222,3333)");
    h.location.setPosition(Point(2222, 3333));
    h.listener.checkFinish();
    h.location.setObjectList(makeList());
    TS_ASSERT_EQUALS(h.location.getNumObjects(), 3U);
}

/** Test focused object, default case. */
void
TestClientMapLocation::testFocusedObject()
{
    TestHarness h("testFocusedObject");

    // Regular startup
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();
    h.location.setObjectList(makeList());

    // Verify current focused object, must be the ship
    TS_ASSERT_EQUALS(h.location.getCurrentObjectIndex(), 1U);
    TS_ASSERT_EQUALS(h.location.getFocusedObject(), Reference(Reference::Ship, SHIP_ID));

    const UserList::Item* it = h.location.getObjectByIndex(1);
    TS_ASSERT(it != 0);
    TS_ASSERT_EQUALS(it->name, "ship");
    TS_ASSERT_EQUALS(it->reference, Reference(Reference::Ship, SHIP_ID));

    // Object can be changed
    h.location.setFocusedObject(Reference(Reference::Planet, PLANET_ID));
    TS_ASSERT_EQUALS(h.location.getFocusedObject(), Reference(Reference::Planet, PLANET_ID));

    TS_ASSERT_EQUALS(h.location.getCurrentObjectIndex(), 2U);

    it = h.location.getObjectByIndex(2);
    TS_ASSERT(it != 0);
    TS_ASSERT_EQUALS(it->name, "planet");
    TS_ASSERT_EQUALS(it->reference, Reference(Reference::Planet, PLANET_ID));
}

/** Test focused object, pre-set case. */
void
TestClientMapLocation::testFocusedObjectPreset()
{
    TestHarness h("testFocusedObjectPreset");

    // Regular startup
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();
    h.location.setFocusedObject(Reference(Reference::Planet, PLANET_ID));
    h.location.setObjectList(makeList());

    // Verify current focused object, must be the planet
    TS_ASSERT_EQUALS(h.location.getCurrentObjectIndex(), 2U);

    const UserList::Item* it = h.location.getObjectByIndex(2);
    TS_ASSERT(it != 0);
    TS_ASSERT_EQUALS(it->name, "planet");
    TS_ASSERT_EQUALS(it->reference, Reference(Reference::Planet, PLANET_ID));
}

/** Test cycleFocusedObject(). */
void
TestClientMapLocation::testCycleFocus()
{
    TestHarness h("testCycleFocus");

    // Regular startup
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();

    // Define a list
    UserList u;
    for (int i = 1; i <= 10; ++i) {
        u.add(UserList::ReferenceItem, "planet", Reference(Reference::Planet, i), (i % 2) == 0, Object::Playable, SkinColor::Green);
    }
    h.location.setObjectList(u);

    // Verify
    TS_ASSERT_EQUALS(h.location.getCurrentObjectIndex(), 0U);

    // Forward, not marked
    h.location.cycleFocusedObject(true, false);
    TS_ASSERT_EQUALS(h.location.getCurrentObjectIndex(), 1U);

    // Forward, marked
    h.location.cycleFocusedObject(true, true);
    TS_ASSERT_EQUALS(h.location.getCurrentObjectIndex(), 3U);

    // Backward, not marked
    h.location.cycleFocusedObject(false, false);
    TS_ASSERT_EQUALS(h.location.getCurrentObjectIndex(), 2U);

    // Backward, marked
    h.location.cycleFocusedObject(false, true);
    TS_ASSERT_EQUALS(h.location.getCurrentObjectIndex(), 1U);
    h.location.cycleFocusedObject(false, true);
    TS_ASSERT_EQUALS(h.location.getCurrentObjectIndex(), 9U);

    // Forward again (wrap)
    h.location.cycleFocusedObject(true, false);
    TS_ASSERT_EQUALS(h.location.getCurrentObjectIndex(), 0U);
}

/** Test cycleFocusedObject(), empty list, */
void
TestClientMapLocation::testCycleFocusEmpty()
{
    TestHarness h("testCycleFocus");

    // Regular startup
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();
    h.location.setObjectList(UserList());

    // Verify
    TS_ASSERT_EQUALS(h.location.getCurrentObjectIndex(), 0U);

    // Cycling does not change anything
    h.location.cycleFocusedObject(true, false);
    TS_ASSERT_EQUALS(h.location.getCurrentObjectIndex(), 0U);
    h.location.cycleFocusedObject(false, false);
    TS_ASSERT_EQUALS(h.location.getCurrentObjectIndex(), 0U);
}

/** Test cycleFocusedObject(), with no marked objects. cycleFocusedObject(marked=true) must terminate. */
void
TestClientMapLocation::testCycleFocusUnmarked()
{
    TestHarness h("testCycleFocusUnmarked");

    // Regular startup
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();
    h.location.setObjectList(makeList());    // This list has no marked objects

    TS_ASSERT_EQUALS(h.location.getCurrentObjectIndex(), 1U);

    // Cycling with marked=true does not change anything
    h.location.cycleFocusedObject(true, true);
    TS_ASSERT_EQUALS(h.location.getCurrentObjectIndex(), 1U);
    h.location.cycleFocusedObject(false, true);
    TS_ASSERT_EQUALS(h.location.getCurrentObjectIndex(), 1U);

    // Cycling with marked=false does browse. Also exercise skipping over title.
    h.location.cycleFocusedObject(true, false);
    TS_ASSERT_EQUALS(h.location.getCurrentObjectIndex(), 2U);
    h.location.cycleFocusedObject(true, false);
    TS_ASSERT_EQUALS(h.location.getCurrentObjectIndex(), 1U);
}

/** Test losing the focused object. */
void
TestClientMapLocation::testLoseFocusedObject()
{
    TestHarness h("testLoseFocusedObject");

    // Regular startup
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();
    h.location.setFocusedObject(Reference(Reference::Planet, PLANET_ID+1));    // not on the list
    h.location.setObjectList(makeList());

    // Verify current focused object
    TS_ASSERT_EQUALS(h.location.getCurrentObjectIndex(), 1U);
    TS_ASSERT_EQUALS(h.location.getFocusedObject(), Reference(Reference::Ship, SHIP_ID));
}

/** Interface test. */
void
TestClientMapLocation::testKeepFocusedObject()
{
    TestHarness h("testLoseFocusedObject");

    // Regular startup
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();

    // Set object list
    UserList u;
    u.add(UserList::ReferenceItem, "A", Reference(Reference::Planet, 1), false, Object::Playable, SkinColor::Green);
    u.add(UserList::ReferenceItem, "B", Reference(Reference::Planet, 3), false, Object::Playable, SkinColor::Green);
    u.add(UserList::ReferenceItem, "C", Reference(Reference::Planet, 5), false, Object::Playable, SkinColor::Green);
    h.location.setFocusedObject(Reference(Reference::Planet, 3));
    h.location.setObjectList(u);

    // Verify current focused object
    TS_ASSERT_EQUALS(h.location.getCurrentObjectIndex(), 1U);
    TS_ASSERT_EQUALS(h.location.getFocusedObject(), Reference(Reference::Planet, 3));

    // Update object list
    UserList u2;
    u2.add(UserList::ReferenceItem, "m", Reference(Reference::Planet, 1), false, Object::Playable, SkinColor::Green);
    u2.add(UserList::ReferenceItem, "n", Reference(Reference::Planet, 2), false, Object::Playable, SkinColor::Green);
    u2.add(UserList::ReferenceItem, "o", Reference(Reference::Planet, 3), false, Object::Playable, SkinColor::Green);
    u2.add(UserList::ReferenceItem, "p", Reference(Reference::Planet, 4), false, Object::Playable, SkinColor::Green);
    u2.add(UserList::ReferenceItem, "q", Reference(Reference::Planet, 5), false, Object::Playable, SkinColor::Green);
    h.location.setObjectList(u2);

    // Focus updates with Id
    TS_ASSERT_EQUALS(h.location.getCurrentObjectIndex(), 2U);
    TS_ASSERT_EQUALS(h.location.getFocusedObject(), Reference(Reference::Planet, 3));
}

