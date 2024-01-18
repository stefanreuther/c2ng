/**
  *  \file test/game/turntest.cpp
  *  \brief Test for game::Turn
  */

#include "game/turn.hpp"

#include "afl/test/testrunner.hpp"
#include "game/map/ionstorm.hpp"
#include "game/test/counter.hpp"
#include "game/vcr/database.hpp"

/** Test setters/getters. */
AFL_TEST("game.Turn:basics", a)
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
    a.checkEqual("01. getTurnNumber", testee.getTurnNumber(), 0);
    a.checkEqual("02. getDatabaseTurnNumber", testee.getDatabaseTurnNumber(), 0);
    a.checkEqual("03. getTimestamp", testee.getTimestamp(), game::Timestamp());
    a.checkNull("04. getBattles", testee.getBattles().get());

    // Modify
    afl::base::Ptr<game::vcr::Database> db(new NullDatabase());
    testee.setTurnNumber(77);
    testee.setDatabaseTurnNumber(76);
    testee.setTimestamp(game::Timestamp(1,2,3,4,5,6));
    testee.setBattles(db);

    // Verify
    a.checkEqual("11. getTurnNumber", testee.getTurnNumber(), 77);
    a.checkEqual("12. getDatabaseTurnNumber", testee.getDatabaseTurnNumber(), 76);
    a.checkEqual("13. getTimestamp", testee.getTimestamp(), game::Timestamp(1,2,3,4,5,6));
    a.checkEqual("14. getBattles", testee.getBattles().get(), db.get());

    // Test subobject accessors
    const game::Turn& ct = testee;
    a.checkEqual("21. universe", &testee.universe(), &ct.universe());
    a.checkEqual("22. inbox", &testee.inbox(), &ct.inbox());
    a.checkEqual("23. extras", &testee.extras(), &ct.extras());
}

/** Test notifyListeners. */
AFL_TEST("game.Turn:notifyListeners", a)
{
    // Set up a universe
    game::test::Counter c;
    game::Turn testee;

    // Create an object and make it visible.
    // Using an ion storm is convenient because it does not need a postprocessing step to become visible.
    game::map::IonStorm* obj = testee.universe().ionStorms().create(77);
    a.checkNonNull("01. ion storm created", obj);
    obj->setPosition(game::map::Point(2000, 2000));
    obj->setVoltage(100);

    obj->sig_change.add(&c, &game::test::Counter::increment);
    a.checkEqual("11. counter", c.get(), 0);

    // Perform a change to the universe. Turn::notifyListeners must call listeners.
    obj->markDirty();
    a.check("21. isDirty", obj->isDirty());
    testee.notifyListeners();
    a.checkEqual("22. counter", c.get(), 1);

    // Status has been reset, no more signal.
    testee.notifyListeners();
    a.checkEqual("31. counter", c.get(), 1);
}
