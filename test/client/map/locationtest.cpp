/**
  *  \file test/client/map/locationtest.cpp
  *  \brief Test for client::map::Location
  */

#include "client/map/location.hpp"

#include "afl/string/format.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"

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

                // Coverage...
                loc.checkEqual("getMode init", location.configuration().getMode(), Configuration::Flat);
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
AFL_TEST("client.map.Location:init", a)
{
    TestHarness h(a);

    // Cannot jump, lock, move here
    Point initial = h.location.getPosition();
    a.check("01. startJump", !h.location.startJump());
    h.location.moveRelative(3, 4);
    h.location.lockObject(LockProxy::Flags_t());
    a.checkEqual("02. getPosition", h.location.getPosition(), initial);

    // Set initial position. Location will request object list.
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();

    // Provide object list.
    h.location.setObjectList(makeList());
    a.checkEqual("11. getPosition", h.location.getPosition(), Point(1200, 2300));
    a.checkEqual("12. getNumObjects", h.location.getNumObjects(), 3U);
}

/** Test BuildAgain flow. */
AFL_TEST("client.map.Location:BuildAgain:relative", a)
{
    TestHarness h(a);

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
    a.checkEqual("01. getNumObjects", h.location.getNumObjects(), 0U);

    // Provide final object list.
    h.location.setObjectList(makeList());
    a.checkEqual("11. getPosition", h.location.getPosition(), Point(1206, 2300));
    a.checkEqual("12. getNumObjects", h.location.getNumObjects(), 3U);
}

/** Test BuildAgain flow, absolute movement. */
AFL_TEST("client.map.Location:BuildAgain:absolute", a)
{
    TestHarness h(a);

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
    a.checkEqual("01. getNumObjects", h.location.getNumObjects(), 0U);

    // Provide final object list.
    h.location.setObjectList(makeList());
    a.checkEqual("11. getPosition", h.location.getPosition(), Point(1200, 2300));
    a.checkEqual("12. getNumObjects", h.location.getNumObjects(), 3U);
}

/** Test Build, with null change. */
AFL_TEST("client.map.Location:setPosition:null-change", a)
{
    TestHarness h(a);

    // Set initial position. Location will request object list.
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();

    // Modify location
    h.location.setPosition(Point(1200, 2300));

    // Provide object list. Location will NOT request updated list because position did not actually change.
    h.location.setObjectList(makeList());
    h.listener.checkFinish();
    a.checkEqual("01. getNumObjects", h.location.getNumObjects(), 3U);
}

/** Test jump while building. */
AFL_TEST("client.map.Location:jump-while-building", a)
{
    TestHarness h(a);

    // Set initial position. Location will request object list.
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();

    // Trigger jump
    bool ok = h.location.startJump();
    a.check("01. startJump ok", ok);

    // Provide object list.
    h.location.setObjectList(makeList());
    a.checkEqual("11. getNumObjects", h.location.getNumObjects(), 0U);

    // Provide jump result
    h.listener.expectCall("requestObjectList(500,300)");
    h.location.setPosition(Point(500, 300));
    h.listener.checkFinish();

    // Provide updated object list
    h.location.setObjectList(makeList());
    a.checkEqual("21. getPosition", h.location.getPosition(), Point(500, 300));
    a.checkEqual("22. getNumObjects", h.location.getNumObjects(), 3U);
}

/** Test jump while building, jump completes first. */
AFL_TEST("client.map.Location:jump-completes-while-building", a)
{
    TestHarness h(a);

    // Set initial position. Location will request object list.
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();

    // Trigger jump
    bool ok = h.location.startJump();
    a.check("01", ok);

    // Cannot start another jump now
    a.check("11", !h.location.startJump());
    a.checkEqual("12. getNumObjects", h.location.getNumObjects(), 0U);

    // Provide jump result. Location will not yet request updated list.
    h.location.setPosition(Point(500, 300));

    // Provide object list.
    h.listener.expectCall("requestObjectList(500,300)");
    h.location.setObjectList(makeList());
    h.listener.checkFinish();

    // Provide updated object list
    h.location.setObjectList(makeList());
    a.checkEqual("21. getPosition", h.location.getPosition(), Point(500, 300));
    a.checkEqual("22. getNumObjects", h.location.getNumObjects(), 3U);
}

/** Test locking, regular case. */
AFL_TEST("client.map.Location:lockObject", a)
{
    TestHarness h(a);

    // Set initial position. Location will request object list.
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();

    // Provide object list.
    h.location.setObjectList(makeList());
    a.checkEqual("01. getPosition", h.location.getPosition(), Point(1200, 2300));

    // Request to lock
    h.listener.expectCall("requestLockObject(1200,2300)");
    h.location.lockObject(LockProxy::Flags_t());
    h.listener.checkFinish();

    // Jumping not allowed now
    a.check("11", !h.location.startJump());
    a.checkEqual("12. getNumObjects", h.location.getNumObjects(), 0U);

    // Produce result. Location will request object list
    h.listener.expectCall("requestObjectList(1222,2333)");
    h.location.setPosition(Point(1222,2333));
    h.listener.checkFinish();

    // Provide updated object list
    h.location.setObjectList(makeList());
    a.checkEqual("21. getPosition", h.location.getPosition(), Point(1222, 2333));
    a.checkEqual("22. getNumObjects", h.location.getNumObjects(), 3U);
}

/** Test locking while building object list. */
AFL_TEST("client.map.Location:lockObject:while-building", a)
{
    TestHarness h(a);

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
    a.checkEqual("01. getNumObjects", h.location.getNumObjects(), 0U);

    // Produce result. Location will request object list
    h.listener.expectCall("requestObjectList(1222,2333)");
    h.location.setPosition(Point(1222,2333));
    h.listener.checkFinish();

    // Provide updated object list
    h.location.setObjectList(makeList());
    a.checkEqual("11. getPosition", h.location.getPosition(), Point(1222, 2333));
    a.checkEqual("12. getNumObjects", h.location.getNumObjects(), 3U);
}

/** Test locking while lock active. */
AFL_TEST("client.map.Location:lockObject:while-locking", a)
{
    TestHarness h(a);

    // Set initial position. Location will request object list; provide it.
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();
    h.location.setObjectList(makeList());
    a.checkEqual("01. getPosition", h.location.getPosition(), Point(1200, 2300));

    // Request to lock
    h.listener.expectCall("requestLockObject(1200,2300)");
    h.location.lockObject(LockProxy::Flags_t());
    h.listener.checkFinish();
    a.checkEqual("11. getNumObjects", h.location.getNumObjects(), 0U);

    // Move and request further locks.
    h.location.moveRelative(1, 0);
    h.location.lockObject(LockProxy::Flags_t());
    h.location.moveRelative(2, 0);
    h.location.lockObject(LockProxy::Flags_t());
    h.location.moveRelative(3, 0);
    h.location.lockObject(LockProxy::Flags_t());
    a.checkEqual("21. getNumObjects", h.location.getNumObjects(), 0U);

    // Cannot jump at this time
    a.check("31. startJump", !h.location.startJump());

    // Produce result. Location will request lock update for result + movement.
    h.listener.expectCall("requestLockObject(1506,2400)");
    h.location.setPosition(Point(1500, 2400));
    h.listener.checkFinish();

    // Further movement.
    h.location.moveRelative(4, 0);
    a.checkEqual("41. getNumObjects", h.location.getNumObjects(), 0U);

    // Produce final result. Location will request object list; further movement is ignored.
    // (Reconsider if LockState::onPositionChange is modified.)
    h.listener.expectCall("requestObjectList(1222,2333)");
    h.location.setPosition(Point(1222,2333));
    h.listener.checkFinish();

    // Provide updated object list
    h.location.setObjectList(makeList());
    a.checkEqual("51. getPosition", h.location.getPosition(), Point(1222, 2333));
    a.checkEqual("52. getNumObjects", h.location.getNumObjects(), 3U);
}

/** Test build/jump/lock combo. */
AFL_TEST("client.map.Location:build-jump-lock", a)
{
    TestHarness h(a);

    // Set initial position. Location will request object list; provide it.
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();

    // Trigger jump
    bool ok = h.location.startJump();
    a.check("01. startJump ok", ok);

    // Cannot jump again
    a.check("11. startJump !ok", !h.location.startJump());
    a.checkEqual("12. getNumObjects", h.location.getNumObjects(), 0U);

    // Trigger lock
    h.location.lockObject(LockProxy::Flags_t());

    // Still cannot jump
    a.check("21. startJump !ok", !h.location.startJump());

    // Provide object list; keep waiting for jump result
    h.location.setObjectList(makeList());
    a.checkEqual("31. getNumObjects", h.location.getNumObjects(), 0U);

    // Still cannot jump
    a.check("41. startJump !ok", !h.location.startJump());

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
    a.checkEqual("51. getPosition", h.location.getPosition(), Point(2500, 2600));
    a.checkEqual("52. getNumObjects", h.location.getNumObjects(), 3U);
}

/** Test build/jump/lock combo, different order. */
AFL_TEST("client.map.Location:build-jump-lock2", a)
{
    TestHarness h(a);

    // Set initial position. Location will request object list; provide it.
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();

    // Trigger jump
    bool ok = h.location.startJump();
    a.check("01. startJump ok", ok);

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
    a.checkEqual("11. getNumObjects", h.location.getNumObjects(), 0U);

    // Provide lock result. Location will request object list.
    h.listener.expectCall("requestObjectList(2500,2600)");
    h.location.setPosition(Point(2500,2600));
    h.listener.checkFinish();

    // Provide updated object list
    h.location.setObjectList(makeList());
    a.checkEqual("21. getPosition", h.location.getPosition(), Point(2500, 2600));
    a.checkEqual("22. getNumObjects", h.location.getNumObjects(), 3U);
}

/** Test jump while building, from BuildAgain. */
AFL_TEST("client.map.Location:jump-while-building-again", a)
{
    TestHarness h(a);

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
    a.check("01. startJump ok", ok);

    // Cannot start another jump now
    a.check("11. startJump !ok", !h.location.startJump());

    // Provide object list.
    h.location.setObjectList(makeList());
    a.checkEqual("21. getNumObjects", h.location.getNumObjects(), 0U);

    // Provide jump result. Location will request object list.
    h.listener.expectCall("requestObjectList(1300,1400)");
    h.location.setPosition(Point(1300, 1400));
    h.listener.checkFinish();

    // Provide final object list.
    h.location.setObjectList(makeList());
    a.checkEqual("31. getNumObjects", h.location.getNumObjects(), 3U);
}

/** Test lock while building, from BuildAgain. */
AFL_TEST("client.map.Location:lock-while-building-again", a)
{
    TestHarness h(a);

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
    a.check("01", !h.location.startJump());

    // Provide object list, Location will initiate jump.
    h.listener.expectCall("requestLockObject(1200,2306)");
    h.location.setObjectList(makeList());
    h.listener.checkFinish();
    a.checkEqual("11. getNumObjects", h.location.getNumObjects(), 0U);

    // Provide jump result. Location will request object list.
    h.listener.expectCall("requestObjectList(1300,1400)");
    h.location.setPosition(Point(1300, 1400));
    h.listener.checkFinish();

    // Provide final object list.
    h.location.setObjectList(makeList());
    a.checkEqual("21. getNumObjects", h.location.getNumObjects(), 3U);
}

/** Test absolute movement. */
AFL_TEST("client.map.Location:setPosition", a)
{
    TestHarness h(a);

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
    a.checkEqual("01. getNumObjects", h.location.getNumObjects(), 3U);
}

/** Test relative movement. */
AFL_TEST("client.map.Location:moveRelative", a)
{
    TestHarness h(a);

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
    a.checkEqual("01. getNumObjects", h.location.getNumObjects(), 3U);
}

/** Test regular jump. */
AFL_TEST("client.map.Location:startJump", a)
{
    TestHarness h(a);

    // Regular startup
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();
    h.location.setObjectList(makeList());

    // Trigger jump
    bool ok = h.location.startJump();
    a.check("01. startJump ok", ok);

    // Cannot start another jump now
    a.check("11. startJump !ok", !h.location.startJump());
    a.checkEqual("12. getNumObjects", h.location.getNumObjects(), 0U);

    // Finish jump
    h.listener.expectCall("requestObjectList(2000,3000)");
    h.location.setPosition(Point(2000, 3000));
    h.listener.checkFinish();
    h.location.setObjectList(makeList());
    a.checkEqual("21. getNumObjects", h.location.getNumObjects(), 3U);
}

/** Test lock while jumping. */
AFL_TEST("client.map.Location:lock-while-jumping", a)
{
    TestHarness h(a);

    // Regular startup
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();
    h.location.setObjectList(makeList());

    // Trigger jump
    bool ok = h.location.startJump();
    a.check("01. startJump ok", ok);
    a.checkEqual("02. getNumObjects", h.location.getNumObjects(), 0U);
    a.checkNull("03. getObjectByIndex", h.location.getObjectByIndex(0));

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
    a.checkEqual("11. getNumObjects", h.location.getNumObjects(), 3U);
}

/** Test focused object, default case. */
AFL_TEST("client.map.Location:getFocusedObject", a)
{
    TestHarness h(a);

    // Regular startup
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();
    h.location.setObjectList(makeList());

    // Verify current focused object, must be the ship
    a.checkEqual("01. getCurrentObjectIndex", h.location.getCurrentObjectIndex(), 1U);
    a.checkEqual("02. getFocusedObject", h.location.getFocusedObject(), Reference(Reference::Ship, SHIP_ID));

    const UserList::Item* it = h.location.getObjectByIndex(1);
    a.checkNonNull("11. getObjectByIndex", it);
    a.checkEqual("12. name", it->name, "ship");
    a.checkEqual("13. reference", it->reference, Reference(Reference::Ship, SHIP_ID));

    // Object can be changed
    h.location.setFocusedObject(Reference(Reference::Planet, PLANET_ID));
    a.checkEqual("21. getFocusedObject", h.location.getFocusedObject(), Reference(Reference::Planet, PLANET_ID));

    a.checkEqual("31. getCurrentObjectIndex", h.location.getCurrentObjectIndex(), 2U);

    it = h.location.getObjectByIndex(2);
    a.checkNonNull("41. getObjectByIndex", it);
    a.checkEqual("42. name", it->name, "planet");
    a.checkEqual("43. reference", it->reference, Reference(Reference::Planet, PLANET_ID));

    // Range check
    it = h.location.getObjectByIndex(3);
    a.checkNull("51", it);
}

/** Test focused object, pre-set case. */
AFL_TEST("client.map.Location:setFocusedObject", a)
{
    TestHarness h(a);

    // Regular startup
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();
    h.location.setFocusedObject(Reference(Reference::Planet, PLANET_ID));
    h.location.setObjectList(makeList());

    // Verify current focused object, must be the planet
    a.checkEqual("01. getCurrentObjectIndex", h.location.getCurrentObjectIndex(), 2U);

    const UserList::Item* it = h.location.getObjectByIndex(2);
    a.checkNonNull("11. getObjectByIndex", it);
    a.checkEqual("12. name", it->name, "planet");
    a.checkEqual("13. reference", it->reference, Reference(Reference::Planet, PLANET_ID));
}

/** Test cycleFocusedObject(). */
AFL_TEST("client.map.Location:cycleFocusedObject", a)
{
    TestHarness h(a);

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
    a.checkEqual("01", h.location.getCurrentObjectIndex(), 0U);

    // Forward, not marked
    h.location.cycleFocusedObject(true, false);
    a.checkEqual("11", h.location.getCurrentObjectIndex(), 1U);

    // Forward, marked
    h.location.cycleFocusedObject(true, true);
    a.checkEqual("21", h.location.getCurrentObjectIndex(), 3U);

    // Backward, not marked
    h.location.cycleFocusedObject(false, false);
    a.checkEqual("31", h.location.getCurrentObjectIndex(), 2U);

    // Backward, marked
    h.location.cycleFocusedObject(false, true);
    a.checkEqual("41", h.location.getCurrentObjectIndex(), 1U);
    h.location.cycleFocusedObject(false, true);
    a.checkEqual("42", h.location.getCurrentObjectIndex(), 9U);

    // Forward again (wrap)
    h.location.cycleFocusedObject(true, false);
    a.checkEqual("51", h.location.getCurrentObjectIndex(), 0U);
}

/** Test cycleFocusedObject(), empty list, */
AFL_TEST("client.map.Location:cycleFocusedObject:empty", a)
{
    TestHarness h(a);

    // Regular startup
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();
    h.location.setObjectList(UserList());

    // Verify
    a.checkEqual("01", h.location.getCurrentObjectIndex(), 0U);

    // Cycling does not change anything
    h.location.cycleFocusedObject(true, false);
    a.checkEqual("11", h.location.getCurrentObjectIndex(), 0U);
    h.location.cycleFocusedObject(false, false);
    a.checkEqual("12", h.location.getCurrentObjectIndex(), 0U);
}

/** Test cycleFocusedObject(), with no marked objects. cycleFocusedObject(marked=true) must terminate. */
AFL_TEST("client.map.Location:cycleFocusedObject:no-marked", a)
{
    TestHarness h(a);

    // Regular startup
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();
    h.location.setObjectList(makeList());    // This list has no marked objects

    a.checkEqual("01", h.location.getCurrentObjectIndex(), 1U);

    // Cycling with marked=true does not change anything
    h.location.cycleFocusedObject(true, true);
    a.checkEqual("11", h.location.getCurrentObjectIndex(), 1U);
    h.location.cycleFocusedObject(false, true);
    a.checkEqual("12", h.location.getCurrentObjectIndex(), 1U);

    // Cycling with marked=false does browse. Also exercise skipping over title.
    h.location.cycleFocusedObject(true, false);
    a.checkEqual("21", h.location.getCurrentObjectIndex(), 2U);
    h.location.cycleFocusedObject(true, false);
    a.checkEqual("22", h.location.getCurrentObjectIndex(), 1U);
}

/** Test losing the focused object. */
AFL_TEST("client.map.Location:setFocusedObject:not-on-list", a)
{
    TestHarness h(a);

    // Regular startup
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();
    h.location.setFocusedObject(Reference(Reference::Planet, PLANET_ID+1));    // not on the list
    h.location.setObjectList(makeList());

    // Verify current focused object
    a.checkEqual("01. getCurrentObjectIndex", h.location.getCurrentObjectIndex(), 1U);
    a.checkEqual("02. getFocusedObject", h.location.getFocusedObject(), Reference(Reference::Ship, SHIP_ID));
}

/** Test keeping the focused object. */
AFL_TEST("client.map.Location:setFocusedObject:preserve-over-list-change", a)
{
    TestHarness h(a);

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
    a.checkEqual("01. getCurrentObjectIndex", h.location.getCurrentObjectIndex(), 1U);
    a.checkEqual("02. getFocusedObject", h.location.getFocusedObject(), Reference(Reference::Planet, 3));

    // Update object list
    UserList u2;
    u2.add(UserList::ReferenceItem, "m", Reference(Reference::Planet, 1), false, Object::Playable, SkinColor::Green);
    u2.add(UserList::ReferenceItem, "n", Reference(Reference::Planet, 2), false, Object::Playable, SkinColor::Green);
    u2.add(UserList::ReferenceItem, "o", Reference(Reference::Planet, 3), false, Object::Playable, SkinColor::Green);
    u2.add(UserList::ReferenceItem, "p", Reference(Reference::Planet, 4), false, Object::Playable, SkinColor::Green);
    u2.add(UserList::ReferenceItem, "q", Reference(Reference::Planet, 5), false, Object::Playable, SkinColor::Green);
    h.location.setObjectList(u2);

    // Focus updates with Id
    a.checkEqual("11. getCurrentObjectIndex", h.location.getCurrentObjectIndex(), 2U);
    a.checkEqual("12. getFocusedObject", h.location.getFocusedObject(), Reference(Reference::Planet, 3));
}

/** Test handling of preferred object. */
AFL_TEST("client.map.Location:setPreferredObject", a)
{
    TestHarness h(a);

    // Set a preferred object
    h.location.setPreferredObject(Reference(Reference::Planet, 5));
    a.checkEqual("01. getPreferredObject", h.location.getPreferredObject(), Reference(Reference::Planet, 5));

    // Regular startup
    h.listener.expectCall("requestObjectList(1200,2300)");
    h.location.setPosition(Point(1200, 2300));
    h.listener.checkFinish();

    // Set object list
    UserList u;
    u.add(UserList::ReferenceItem, "A", Reference(Reference::Planet, 1), false, Object::Playable, SkinColor::Green);
    u.add(UserList::ReferenceItem, "B", Reference(Reference::Planet, 3), false, Object::Playable, SkinColor::Green);
    u.add(UserList::ReferenceItem, "C", Reference(Reference::Planet, 5), false, Object::Playable, SkinColor::Green);
    h.location.setObjectList(u);

    // Verify current focused object
    a.checkEqual("11. getCurrentObjectIndex", h.location.getCurrentObjectIndex(), 2U);
    a.checkEqual("12. getFocusedObject", h.location.getFocusedObject(), Reference(Reference::Planet, 5));
}
