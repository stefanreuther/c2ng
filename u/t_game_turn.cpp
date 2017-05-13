/**
  *  \file u/t_game_turn.cpp
  *  \brief Test for game::Turn
  */

#include "game/turn.hpp"

#include "t_game.hpp"
#include "game/vcr/database.hpp"
#include "game/map/ionstorm.hpp"
#include "helper/counter.hpp"

/** Test setters/getters. */
void
TestGameTurn::testIt()
{
    class NullDatabase : public game::vcr::Database {
     public:
        virtual size_t getNumBattles() const
            { return 0; }
        virtual game::vcr::Battle* getBattle(size_t /*nr*/)
            { return 0; }
    };

    // Test initial values
    game::Turn testee;
    TS_ASSERT_EQUALS(testee.getTurnNumber(), 0);
    TS_ASSERT_EQUALS(testee.getDatabaseTurnNumber(), 0);
    TS_ASSERT_EQUALS(testee.getTimestamp(), game::Timestamp());
    TS_ASSERT(testee.getBattles().get() == 0);

    // Modify
    afl::base::Ptr<game::vcr::Database> db(new NullDatabase());
    testee.setTurnNumber(77);
    testee.setDatabaseTurnNumber(76);
    testee.setTimestamp(game::Timestamp(1,2,3,4,5,6));
    testee.setBattles(db);

    // Verify
    TS_ASSERT_EQUALS(testee.getTurnNumber(), 77);
    TS_ASSERT_EQUALS(testee.getDatabaseTurnNumber(), 76);
    TS_ASSERT_EQUALS(testee.getTimestamp(), game::Timestamp(1,2,3,4,5,6));
    TS_ASSERT_EQUALS(testee.getBattles().get(), db.get());

    // Test subobject accessors
    const game::Turn& ct = testee;
    TS_ASSERT_EQUALS(&testee.universe(), &ct.universe());
    TS_ASSERT_EQUALS(&testee.inbox(), &ct.inbox());
    TS_ASSERT_EQUALS(&testee.extras(), &ct.extras());
}

/** Test notifyListeners. */
void
TestGameTurn::testNotify()
{
    // Set up a universe
    Counter c;
    game::Turn testee;

    // Create an object and make it visible.
    // Using an ion storm is convenient because it does not need a postprocessing step to become visible.
    game::map::IonStorm* obj = testee.universe().ionStorms().create(77);
    TS_ASSERT(obj != 0);
    obj->setPosition(game::map::Point(2000, 2000));
    obj->setVoltage(100);
    
    obj->sig_change.add(&c, &Counter::increment);
    TS_ASSERT_EQUALS(c.get(), 0);

    // Perform a change to the universe. Turn::notifyListeners must call listeners.
    obj->markDirty();
    TS_ASSERT(obj->isDirty());
    testee.notifyListeners();
    TS_ASSERT_EQUALS(c.get(), 1);

    // Status has been reset, no more signal.
    testee.notifyListeners();
    TS_ASSERT_EQUALS(c.get(), 1);
}

