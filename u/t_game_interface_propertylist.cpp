/**
  *  \file u/t_game_interface_propertylist.cpp
  *  \brief Test for game::interface::PropertyList
  */

#include "game/interface/propertylist.hpp"

#include "t_game_interface.hpp"
#include "afl/data/namemap.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/map/minefield.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "interpreter/values.hpp"

using game::interface::PropertyList;

namespace {
    struct TestHarness {
        afl::sys::Log log;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        interpreter::World world;

        TestHarness()
            : log(), tx(), fs(), world(log, tx, fs)
            { }
    };

    const PropertyList::Info* find(const PropertyList& pl, String_t name)
    {
        for (size_t i = 0; i < pl.infos.size(); ++i) {
            if (pl.infos[i].name == name) {
                return &pl.infos[i];
            }
        }
        return 0;
    }
}

/** Test buildPropertyList() for ships.
    Also tests multiple format usecases.
    A: create environment, some ship properties, and a ship.
    E: ship properties correctly reported */
void
TestGameInterfacePropertyList::testShip()
{
    TestHarness h;
    game::map::Ship ship(33);

    // The world starts with some unspecified names. Just overwrite it entirely.
    afl::data::NameMap names;
    names.add("ONE");
    names.add("TWO");
    names.add("ONE.MORE");
    h.world.shipPropertyNames().swap(names);

    // Property values
    h.world.shipProperties().create(33)->setNew(0, interpreter::makeStringValue("sv"));
    h.world.shipProperties().create(33)->setNew(2, interpreter::makeIntegerValue(2));

    // Test
    PropertyList testee;
    buildPropertyList(testee, &ship, h.world, h.tx);

    // Verify
    TS_ASSERT_EQUALS(testee.title, "Ship Properties");

    const PropertyList::Info* pi = find(testee, "One");
    TS_ASSERT(pi != 0);
    TS_ASSERT_EQUALS(pi->value, "\"sv\"");
    TS_ASSERT_EQUALS(pi->valueColor, util::SkinColor::Static);

    pi = find(testee, "Two");
    TS_ASSERT(pi != 0);
    TS_ASSERT_EQUALS(pi->value, "Empty");
    TS_ASSERT_EQUALS(pi->valueColor, util::SkinColor::Faded);

    pi = find(testee, "One.More");
    TS_ASSERT(pi != 0);
    TS_ASSERT_EQUALS(pi->value, "2");
    TS_ASSERT_EQUALS(pi->valueColor, util::SkinColor::Static);
}

/** Test buildPropertyList() for planets.
    A: create environment, a planet property, and a planet.
    E: planet property correctly */
void
TestGameInterfacePropertyList::testPlanet()
{
    TestHarness h;
    game::map::Planet planet(77);

    // One property for testing
    afl::data::NameMap::Index_t idx = h.world.planetPropertyNames().add("T");
    h.world.planetProperties().create(77)->setNew(idx, interpreter::makeIntegerValue(42));

    // Test
    PropertyList testee;
    buildPropertyList(testee, &planet, h.world, h.tx);

    // Verify
    TS_ASSERT_EQUALS(testee.title, "Planet Properties");

    const PropertyList::Info* pi = find(testee, "T");
    TS_ASSERT(pi != 0);
    TS_ASSERT_EQUALS(pi->value, "42");
    TS_ASSERT_EQUALS(pi->valueColor, util::SkinColor::Static);
}

/** Test buildPropertyList() for empty properties.
    Verifies that a property value is reported even when the storage slot doesn't physically exist.
    A: create environment, a planet property but no value, and a planet.
    E: planet property correctly */
void
TestGameInterfacePropertyList::testEmpty()
{
    TestHarness h;
    game::map::Planet planet(77);

    // One property for testing
    h.world.planetPropertyNames().add("T");

    // Test
    PropertyList testee;
    buildPropertyList(testee, &planet, h.world, h.tx);

    // Verify
    TS_ASSERT_EQUALS(testee.title, "Planet Properties");

    const PropertyList::Info* pi = find(testee, "T");
    TS_ASSERT(pi != 0);
    TS_ASSERT_EQUALS(pi->value, "Empty");
    TS_ASSERT_EQUALS(pi->valueColor, util::SkinColor::Faded);
}

/** Test buildPropertyList() for other objects.
    A: create environment, and an object other than ship or planet.
    E: empty result reported */
void
TestGameInterfacePropertyList::testOther()
{
    TestHarness h;
    game::map::Minefield mf(88);

    // Test
    PropertyList testee;
    buildPropertyList(testee, &mf, h.world, h.tx);

    // Verify
    TS_ASSERT_EQUALS(testee.title, "");
    TS_ASSERT(testee.infos.empty());
}

