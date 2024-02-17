/**
  *  \file test/game/interface/vcrpropertytest.cpp
  *  \brief Test for game::interface::VcrProperty
  */

#include "game/interface/vcrproperty.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "game/vcr/test/battle.hpp"
#include "game/vcr/test/database.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/test/valueverifier.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using game::Root;
using game::spec::ShipList;
using game::vcr::test::Battle;
using game::vcr::test::Database;
using interpreter::test::verifyNewInteger;
using interpreter::test::verifyNewNull;
using interpreter::test::verifyNewString;

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

    Battle& addBattle(Database& db)
    {
        Battle& b = db.addBattle();
        b.addObject(makeShip(10, 5), 0);
        b.addObject(makeShip(20, 6), 7);
        b.addObject(makeShip(30, 7), 7);
        return b;
    }
}

/** General functionality test. */
AFL_TEST("game.interface.VcrProperty", a)
{
    // Environment
    afl::string::NullTranslator tx;
    Ref<Root> r(game::test::makeRoot(game::HostVersion()));
    Ref<ShipList> sl(*new game::spec::ShipList());
    Ptr<Database> db(new Database());
    Battle& b = addBattle(*db);

    b.setAuxiliaryInformation(Battle::aiSeed,    1111);
    b.setAuxiliaryInformation(Battle::aiMagic,   2222);
    b.setAuxiliaryInformation(Battle::aiType,    3333);
    b.setAuxiliaryInformation(Battle::aiFlags,   4444);
    b.setAuxiliaryInformation(Battle::aiAmbient, 5555);
    b.setAlgorithmName("Algo");
    b.setPosition(game::map::Point(1492, 1998));

    // Verify scalars
    verifyNewInteger(a("ivpSeed"),      getVcrProperty(0, game::interface::ivpSeed,      tx, r, db, sl), 1111);
    verifyNewInteger(a("ivpMagic"),     getVcrProperty(0, game::interface::ivpMagic,     tx, r, db, sl), 2222);
    verifyNewInteger(a("ivpType"),      getVcrProperty(0, game::interface::ivpType,      tx, r, db, sl), 3333);
    verifyNewString (a("ivpAlgorithm"), getVcrProperty(0, game::interface::ivpAlgorithm, tx, r, db, sl), "Algo");
    verifyNewInteger(a("ivpFlags"),     getVcrProperty(0, game::interface::ivpFlags,     tx, r, db, sl), 4444);
    verifyNewInteger(a("ivpNumUnits"),  getVcrProperty(0, game::interface::ivpNumUnits,  tx, r, db, sl), 3);
    verifyNewInteger(a("ivpLocX"),      getVcrProperty(0, game::interface::ivpLocX,      tx, r, db, sl), 1492);
    verifyNewInteger(a("ivpLocY"),      getVcrProperty(0, game::interface::ivpLocY,      tx, r, db, sl), 1998);
    verifyNewInteger(a("ivpAmbient"),   getVcrProperty(0, game::interface::ivpAmbient,   tx, r, db, sl), 5555);

    // Verify 'Units'. Must have correct dimension.
    std::auto_ptr<afl::data::Value> units(getVcrProperty(0, game::interface::ivpUnits,   tx, r, db, sl));
    interpreter::CallableValue* cv = dynamic_cast<interpreter::CallableValue*>(units.get());
    a.checkNonNull("ivpUnits callable", cv);
    a.checkEqual("ivpUnits getDimension", cv->getDimension(1), 4);

    // Out-of-range access
    verifyNewNull(a("ivpSeed range"),  getVcrProperty(1, game::interface::ivpSeed,  tx, r, db, sl));
    verifyNewNull(a("ivpUnits range"), getVcrProperty(1, game::interface::ivpUnits, tx, r, db, sl));
}
