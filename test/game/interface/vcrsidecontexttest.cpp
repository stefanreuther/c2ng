/**
  *  \file test/game/interface/vcrsidecontexttest.cpp
  *  \brief Test for game::interface::VcrSideContext
  */

#include "game/interface/vcrsidecontext.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/vcr/object.hpp"
#include "game/vcr/test/battle.hpp"
#include "game/vcr/test/database.hpp"
#include "interpreter/test/contextverifier.hpp"

using game::vcr::Database;
using afl::base::Ref;
using afl::base::Ptr;
using game::Root;
using game::spec::ShipList;

namespace {
    game::vcr::Object makeShip(game::Id_t id, int owner)
    {
        game::vcr::Object o;
        o.setId(id);
        o.setOwner(owner);
        o.setIsPlanet(false);
        o.setName("X");
        return o;
    }

    Ptr<Database> makeDefaultBattle()
    {
        Ptr<game::vcr::test::Database> db = new game::vcr::test::Database();
        game::vcr::test::Battle& b = db->addBattle();
        b.addObject(makeShip(10, 5), 0);
        b.addObject(makeShip(20, 6), 7);
        b.addObject(makeShip(30, 7), 7);
        return db;
    }
}

/** Test general operation. */
AFL_TEST("game.interface.VcrSideContext:basics", a)
{
    // Environment
    afl::string::NullTranslator tx;
    Ref<ShipList> shipList(*new ShipList());
    Ref<Root> root(game::test::makeRoot(game::HostVersion()));
    Ptr<Database> db = makeDefaultBattle();

    // Test
    game::interface::VcrSideContext testee(0, 1, tx, root, db, shipList);
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifyTypes();
    verif.verifyNotSerializable();
    a.checkNull("01. getObject", testee.getObject());

    // Verify some properties
    verif.verifyInteger("OWNER$", 6);
    verif.verifyInteger("ID", 20);
    verif.verifyInteger("INDEX", 2);
    a.check("11. next", testee.next());
    verif.verifyInteger("OWNER$", 7);
    verif.verifyInteger("ID", 30);
    verif.verifyInteger("INDEX", 3);
    a.check("12. next", !testee.next());
}

/** Test behaviour on null battle.
    Can normally not happen. */
AFL_TEST("game.interface.VcrSideContext:null", a)
{
    // Environment
    afl::string::NullTranslator tx;
    Ref<ShipList> sl(*new ShipList());
    Ref<Root> root(game::test::makeRoot(game::HostVersion()));
    Ptr<Database> db; // null

    // Test
    game::interface::VcrSideContext testee(20, 17, tx, root, db, sl);
    interpreter::test::ContextVerifier verif(testee, a);

    // Verify some properties
    verif.verifyNull("OWNER$");
    verif.verifyNull("ID");
    verif.verifyInteger("INDEX", 18);
}
