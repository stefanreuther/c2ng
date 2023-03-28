/**
  *  \file u/t_game_interface_cargomethod.cpp
  *  \brief Test for game::interface::CargoMethod
  */

#include "game/interface/cargomethod.hpp"

#include "t_game_interface.hpp"
#include "afl/data/segment.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/exception.hpp"
#include "game/map/configuration.hpp"
#include "game/map/planetdata.hpp"
#include "game/map/shipdata.hpp"
#include "game/map/universe.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "interpreter/test/valueverifier.hpp"

using game::Element;
using game::map::Object;
using game::map::Planet;
using game::map::Ship;

namespace {
    const int HULL_ID = 10;
    const int X = 1291;
    const int Y = 2823;

    struct Environment {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session;
        interpreter::Process proc;
        afl::base::Ref<game::Root> root;
        afl::base::Ref<game::Turn> turn;
        game::map::Configuration mapConfig;
        afl::base::Ref<game::spec::ShipList> shipList;

        Environment()
            : tx(), fs(), session(tx, fs),
              proc(session.world(), "tester", 777),
              root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0)))),
              turn(*new game::Turn()),
              shipList(*new game::spec::ShipList())
            {
                // Process: push a frame to be able to set CARGO.REMAINDER variable
                proc.pushFrame(interpreter::BytecodeObject::create(true), false)
                    .localNames.add("CARGO.REMAINDER");

                // Ship list: create a hull for a ship that can hold 200 cargo, 100 fuel
                game::spec::Hull& h = *shipList->hulls().create(HULL_ID);
                h.setMaxCargo(200);
                h.setMaxFuel(100);

                // Session: connect ship list (no need to connect root, game; they're not supposed to be taken from session!)
                session.setShipList(shipList.asPtr());
            }
    };

    Ship& addShip(Environment& env, int id, int owner, Object::Playability playability)
    {
        Ship& sh = *env.turn->universe().ships().create(id);
        game::map::ShipData sd;
        sd.x = X;
        sd.y = Y;
        sd.owner = owner;
        sd.hullType = HULL_ID;
        sd.beamType = 0;
        sd.numBeams = 0;
        sd.numBays = 0;
        sd.torpedoType = 0;
        sd.ammo = 0;
        sd.numLaunchers = 0;
        sd.colonists = 0;
        sd.neutronium = 10;
        sd.tritanium = 10;
        sd.duranium = 10;
        sd.molybdenum = 10;
        sd.supplies = 10;
        sd.money = 100;
        sd.unload.targetId = 0;
        sd.transfer.targetId = 0;
        sh.addCurrentShipData(sd, game::PlayerSet_t(owner));
        sh.internalCheck(game::PlayerSet_t(owner), 10);
        sh.setPlayability(playability);
        return sh;
    }

    Planet& addPlanet(Environment& env, int id, int owner, Object::Playability playability)
    {
        Planet& pl = *env.turn->universe().planets().create(id);
        game::map::PlanetData pd;
        pd.owner = owner;
        pd.minedNeutronium = 1000;
        pd.minedTritanium = 1000;
        pd.minedDuranium = 1000;
        pd.minedMolybdenum = 1000;
        pd.colonistClans = 1000;
        pd.supplies = 1000;
        pd.money = 5000;
        pd.baseFlag = 0;
        pl.addCurrentPlanetData(pd, game::PlayerSet_t(owner));
        pl.setPosition(game::map::Point(X, Y));
        pl.internalCheck(env.mapConfig, game::PlayerSet_t(owner), 10, env.tx, env.session.log());
        pl.setPlayability(playability);
        return pl;
    }
}

/** Test doCargoTransfer(Planet). */
void
TestGameInterfaceCargoMethod::testCargoTransferPlanet()
{
    // Planet to ship: 'CargoTransfer "t20", 17'
    {
        Environment env;
        Ship& sh = addShip(env, 17, 1, Object::Playable);
        Planet& pl = addPlanet(env, 100, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackString("t20");
        seg.pushBackInteger(17);
        interpreter::Arguments args(seg, 0, 2);

        game::interface::doCargoTransfer(pl, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root);

        TS_ASSERT_EQUALS(pl.getCargo(Element::Tritanium).orElse(-1), 980);
        TS_ASSERT_EQUALS(sh.getCargo(Element::Tritanium).orElse(-1), 30);
    }

    // Planet to ship with supply sale: 'CargoTransfer "s20", 17, "s"'
    {
        Environment env;
        Ship& sh = addShip(env, 17, 1, Object::Playable);
        Planet& pl = addPlanet(env, 100, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackString("s20");
        seg.pushBackInteger(17);
        seg.pushBackString("s");
        interpreter::Arguments args(seg, 0, 3);

        game::interface::doCargoTransfer(pl, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root);

        TS_ASSERT_EQUALS(pl.getCargo(Element::Supplies).orElse(-1), 980);
        TS_ASSERT_EQUALS(sh.getCargo(Element::Money).orElse(-1), 120);
    }

    // Overload: 'CargoTransfer "t200", 17'
    {
        Environment env;
        Ship& sh = addShip(env, 17, 1, Object::Playable);
        Planet& pl = addPlanet(env, 100, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackString("t200");
        seg.pushBackInteger(17);
        interpreter::Arguments args(seg, 0, 2);

        TS_ASSERT_THROWS(game::interface::doCargoTransfer(pl, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root), game::Exception);
    }

    // Overload, with overload flag enabled: 'CargoTransfer "t200", 17, "o"'
    {
        Environment env;
        Ship& sh = addShip(env, 17, 1, Object::Playable);
        Planet& pl = addPlanet(env, 100, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackString("t200");
        seg.pushBackInteger(17);
        seg.pushBackString("o");
        interpreter::Arguments args(seg, 0, 3);

        game::interface::doCargoTransfer(pl, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root);

        TS_ASSERT_EQUALS(pl.getCargo(Element::Tritanium).orElse(-1), 800);
        TS_ASSERT_EQUALS(sh.getCargo(Element::Tritanium).orElse(-1), 210);
    }

    // Overload, with partial flag enabled: 'CargoTransfer "t200", 17, "n"'
    {
        Environment env;
        Ship& sh = addShip(env, 17, 1, Object::Playable);
        Planet& pl = addPlanet(env, 100, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackString("t200");
        seg.pushBackInteger(17);
        seg.pushBackString("n");
        interpreter::Arguments args(seg, 0, 3);

        game::interface::doCargoTransfer(pl, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root);

        TS_ASSERT_EQUALS(pl.getCargo(Element::Tritanium).orElse(-1), 840);
        TS_ASSERT_EQUALS(sh.getCargo(Element::Tritanium).orElse(-1), 170);
        interpreter::test::verifyNewString("remainder", env.proc.getVariable("CARGO.REMAINDER").release(), "40T");
    }

    // Planet to foreign ship with proxy: 'CargoTransfer "t20", 222, 17'
    {
        Environment env;
        Ship& proxy = addShip(env, 17, 1, Object::Playable);
        Ship& sh = addShip(env, 222, 2, Object::NotPlayable);
        Planet& pl = addPlanet(env, 100, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackString("t20");
        seg.pushBackInteger(222);
        seg.pushBackInteger(17);
        interpreter::Arguments args(seg, 0, 3);

        game::interface::doCargoTransfer(pl, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root);

        TS_ASSERT_EQUALS(pl.getCargo(Element::Tritanium).orElse(-1), 980);
        TS_ASSERT_EQUALS(proxy.getCargo(Element::Tritanium).orElse(-1), 10);
        TS_ASSERT_EQUALS(proxy.getTransporterTargetId(Ship::TransferTransporter).orElse(-1), 222);
        TS_ASSERT_EQUALS(proxy.getTransporterCargo(Ship::TransferTransporter, Element::Tritanium).orElse(-1), 20);
    }

    // Null amount
    {
        Environment env;
        /*Ship& sh =*/ addShip(env, 17, 1, Object::Playable);
        Planet& pl = addPlanet(env, 100, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackInteger(17);
        interpreter::Arguments args(seg, 0, 2);

        TS_ASSERT_THROWS_NOTHING(game::interface::doCargoTransfer(pl, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root));
    }

    // Null target
    {
        Environment env;
        Planet& pl = addPlanet(env, 100, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackString("T20");
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 2);

        TS_ASSERT_THROWS_NOTHING(game::interface::doCargoTransfer(pl, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root));
    }

    // Error: invalid cargospec
    {
        Environment env;
        /*Ship& sh =*/ addShip(env, 17, 1, Object::Playable);
        Planet& pl = addPlanet(env, 100, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackString("xyzzy");
        seg.pushBackInteger(17);
        interpreter::Arguments args(seg, 0, 2);

        TS_ASSERT_THROWS(game::interface::doCargoTransfer(pl, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root), game::Exception);
    }

    // Error: invalid target
    {
        Environment env;
        Planet& pl = addPlanet(env, 100, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackString("T20");
        seg.pushBackInteger(17);
        interpreter::Arguments args(seg, 0, 2);

        TS_ASSERT_THROWS(game::interface::doCargoTransfer(pl, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root), game::Exception);
    }

    // Arity errror
    {
        Environment env;
        Planet& pl = addPlanet(env, 100, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackString("T20");
        interpreter::Arguments args(seg, 0, 1);

        TS_ASSERT_THROWS(game::interface::doCargoTransfer(pl, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root), interpreter::Error);
    }

    // Type errror
    {
        Environment env;
        Planet& pl = addPlanet(env, 100, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackString("T20");
        seg.pushBackString("17");
        interpreter::Arguments args(seg, 0, 2);

        TS_ASSERT_THROWS(game::interface::doCargoTransfer(pl, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root), interpreter::Error);
    }
}

/** Test doCargoTransfer(Ship). */
void
TestGameInterfaceCargoMethod::testCargoTransferShip()
{
    // Ship to ship: 'CargoTransfer "t7", 34'
    {
        Environment env;
        Ship& from = addShip(env, 17, 1, Object::Playable);
        Ship& to   = addShip(env, 34, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackString("t7");
        seg.pushBackInteger(34);
        interpreter::Arguments args(seg, 0, 2);

        game::interface::doCargoTransfer(from, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root);

        TS_ASSERT_EQUALS(from.getCargo(Element::Tritanium).orElse(-1), 3);
        TS_ASSERT_EQUALS(to.getCargo(Element::Tritanium).orElse(-1), 17);
    }

    // Underflow, ship to ship: 'CargoTransfer "t50", 34'
    {
        Environment env;
        Ship& from = addShip(env, 17, 1, Object::Playable);
        Ship& to   = addShip(env, 34, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackString("t50");
        seg.pushBackInteger(34);
        interpreter::Arguments args(seg, 0, 2);

        TS_ASSERT_THROWS(game::interface::doCargoTransfer(from, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root), game::Exception);
    }

    // Underflow, with "n" flag: 'CargoTransfer "t7", 34, "n"'
    {
        Environment env;
        Ship& from = addShip(env, 17, 1, Object::Playable);
        Ship& to   = addShip(env, 34, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackString("t50");
        seg.pushBackInteger(34);
        seg.pushBackString("n");
        interpreter::Arguments args(seg, 0, 3);

        game::interface::doCargoTransfer(from, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root);

        TS_ASSERT_EQUALS(from.getCargo(Element::Tritanium).orElse(-1), 0);
        TS_ASSERT_EQUALS(to.getCargo(Element::Tritanium).orElse(-1), 20);
        interpreter::test::verifyNewString("remainder", env.proc.getVariable("CARGO.REMAINDER").release(), "40T");
    }

    // Overflow
    {
        Environment env;
        Ship& from = addShip(env, 17, 1, Object::Playable);
        Ship& to   = addShip(env, 34, 1, Object::Playable);
        from.setCargo(Element::Neutronium, 90);
        to.setCargo(Element::Neutronium, 90);

        afl::data::Segment seg;
        seg.pushBackString("n40");
        seg.pushBackInteger(34);
        interpreter::Arguments args(seg, 0, 2);

        TS_ASSERT_THROWS(game::interface::doCargoTransfer(from, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root), game::Exception);

        TS_ASSERT_EQUALS(from.getCargo(Element::Neutronium).orElse(-1), 90);
        TS_ASSERT_EQUALS(to.getCargo(Element::Neutronium).orElse(-1), 90);
    }

    // Overflow, with "o" option
    {
        Environment env;
        Ship& from = addShip(env, 17, 1, Object::Playable);
        Ship& to   = addShip(env, 34, 1, Object::Playable);
        from.setCargo(Element::Neutronium, 90);
        to.setCargo(Element::Neutronium, 90);

        afl::data::Segment seg;
        seg.pushBackString("n40");
        seg.pushBackInteger(34);
        seg.pushBackString("o");
        interpreter::Arguments args(seg, 0, 3);

        TS_ASSERT_THROWS_NOTHING(game::interface::doCargoTransfer(from, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root));

        TS_ASSERT_EQUALS(from.getCargo(Element::Neutronium).orElse(-1), 50);
        TS_ASSERT_EQUALS(to.getCargo(Element::Neutronium).orElse(-1), 130);
    }

    // Null amount: 'CargoTransfer EMPTY, 34'
    {
        Environment env;
        Ship& from = addShip(env, 17, 1, Object::Playable);
        Ship& to   = addShip(env, 34, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackInteger(34);
        interpreter::Arguments args(seg, 0, 2);

        TS_ASSERT_THROWS_NOTHING(game::interface::doCargoTransfer(from, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root));
    }

    // Null target: 'CargoTransfer "t7", EMPTY'
    {
        Environment env;
        Ship& from = addShip(env, 17, 1, Object::Playable);
        Ship& to   = addShip(env, 34, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackString("t7");
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 2);

        TS_ASSERT_THROWS_NOTHING(game::interface::doCargoTransfer(from, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root));

        TS_ASSERT_EQUALS(from.getCargo(Element::Tritanium).orElse(-1), 10);
    }

    // Error: invalid target
    {
        Environment env;
        Ship& from = addShip(env, 17, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackString("t7");
        seg.pushBackInteger(34);
        interpreter::Arguments args(seg, 0, 2);

        TS_ASSERT_THROWS(game::interface::doCargoTransfer(from, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root), game::Exception);
    }

    // Arity errror
    {
        Environment env;
        Ship& from = addShip(env, 17, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackString("T20");
        interpreter::Arguments args(seg, 0, 1);

        TS_ASSERT_THROWS(game::interface::doCargoTransfer(from, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root), interpreter::Error);
    }

    // Type errror
    {
        Environment env;
        Ship& from = addShip(env, 17, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackString("T20");
        seg.pushBackString("17");
        interpreter::Arguments args(seg, 0, 2);

        TS_ASSERT_THROWS(game::interface::doCargoTransfer(from, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root), interpreter::Error);
    }
}

/** Test doCargoUnload(). */
void
TestGameInterfaceCargoMethod::testCargoUnload()
{
    // Ship to planet: 'CargoUnload "t7"'
    {
        Environment env;
        Ship& sh = addShip(env, 17, 1, Object::Playable);
        Planet& pl = addPlanet(env, 100, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackString("t7");
        interpreter::Arguments args(seg, 0, 1);

        game::interface::doCargoUnload(sh, false, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root);

        TS_ASSERT_EQUALS(pl.getCargo(Element::Tritanium).orElse(-1), 1007);
        TS_ASSERT_EQUALS(sh.getCargo(Element::Tritanium).orElse(-1), 3);
    }

    // Planet to ship: 'CargoUpload "t7"'
    {
        Environment env;
        Ship& sh = addShip(env, 17, 1, Object::Playable);
        Planet& pl = addPlanet(env, 100, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackString("t7");
        interpreter::Arguments args(seg, 0, 1);

        game::interface::doCargoUnload(sh, true, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root);

        TS_ASSERT_EQUALS(pl.getCargo(Element::Tritanium).orElse(-1), 993);
        TS_ASSERT_EQUALS(sh.getCargo(Element::Tritanium).orElse(-1), 17);
    }

    // Upload with overflow: 'CargoUpload "500n"'
    {
        Environment env;
        Ship& sh = addShip(env, 17, 1, Object::Playable);
        Planet& pl = addPlanet(env, 100, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackString("500n");
        interpreter::Arguments args(seg, 0, 1);

        TS_ASSERT_THROWS(game::interface::doCargoUnload(sh, true, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root), game::Exception);
    }

    // Upload with overflow, overload permission: 'CargoUpload "500n", "o"'
    {
        Environment env;
        Ship& sh = addShip(env, 17, 1, Object::Playable);
        Planet& pl = addPlanet(env, 100, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackString("500n");
        seg.pushBackString("O");
        interpreter::Arguments args(seg, 0, 2);

        TS_ASSERT_THROWS_NOTHING(game::interface::doCargoUnload(sh, true, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root));

        TS_ASSERT_EQUALS(pl.getCargo(Element::Neutronium).orElse(-1), 500);
        TS_ASSERT_EQUALS(sh.getCargo(Element::Neutronium).orElse(-1), 510);
    }

    // Upload with overflow, partial: 'CargoUpload "500n", "n"'
    {
        Environment env;
        Ship& sh = addShip(env, 17, 1, Object::Playable);
        Planet& pl = addPlanet(env, 100, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackString("500n");
        seg.pushBackString("N");
        interpreter::Arguments args(seg, 0, 2);

        TS_ASSERT_THROWS_NOTHING(game::interface::doCargoUnload(sh, true, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root));

        TS_ASSERT_EQUALS(pl.getCargo(Element::Neutronium).orElse(-1), 910);
        TS_ASSERT_EQUALS(sh.getCargo(Element::Neutronium).orElse(-1), 100);
        interpreter::test::verifyNewString("remainder", env.proc.getVariable("CARGO.REMAINDER").release(), "410N");
    }

    // Unload in deep space: 'CargoUnload "t7"'
    {
        Environment env;
        Ship& sh = addShip(env, 17, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackString("t7");
        interpreter::Arguments args(seg, 0, 1);

        TS_ASSERT_THROWS(game::interface::doCargoUnload(sh, false, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root), game::Exception);
    }

    // Unload in deep space, with jettison clearance: 'CargoUnload "t7", "j"'
    {
        Environment env;
        Ship& sh = addShip(env, 17, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackString("t7");
        seg.pushBackString("j");
        interpreter::Arguments args(seg, 0, 2);

        game::interface::doCargoUnload(sh, false, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root);
        TS_ASSERT_EQUALS(sh.getCargo(Element::Tritanium).orElse(-1), 3);
        TS_ASSERT_EQUALS(sh.getTransporterCargo(Ship::UnloadTransporter, Element::Tritanium).orElse(-1), 7);
    }

    // Ship to planet with supply sale: 'CargoUnload "s7", "s"'
    {
        Environment env;
        Ship& sh = addShip(env, 17, 1, Object::Playable);
        Planet& pl = addPlanet(env, 100, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackString("s7");
        seg.pushBackString("s");
        interpreter::Arguments args(seg, 0, 2);

        game::interface::doCargoUnload(sh, false, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root);

        TS_ASSERT_EQUALS(pl.getCargo(Element::Supplies).orElse(-1), 1000);
        TS_ASSERT_EQUALS(pl.getCargo(Element::Money).orElse(-1), 5007);
        TS_ASSERT_EQUALS(sh.getCargo(Element::Supplies).orElse(-1), 3);
    }

    // Indirect: 'CargoUpload "t10", "20"' from foreign ship
    {
        Environment env;
        Ship& them = addShip(env, 17, 2, Object::NotPlayable);
        Ship& me = addShip(env, 20, 1, Object::Playable);
        Planet& pl = addPlanet(env, 100, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackString("t10");
        seg.pushBackString("20");
        interpreter::Arguments args(seg, 0, 2);

        game::interface::doCargoUnload(them, true, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root);

        TS_ASSERT_EQUALS(pl.getCargo(Element::Tritanium).orElse(-1), 990);
        TS_ASSERT_EQUALS(me.getTransporterTargetId(Ship::TransferTransporter).orElse(-1), 17);
        TS_ASSERT_EQUALS(me.getTransporterCargo(Ship::TransferTransporter, Element::Tritanium).orElse(-1), 10);
    }

    // Indirect required, but invalid: 'CargoUpload "t10", 20' from foreign ship
    {
        Environment env;
        Ship& them = addShip(env, 17, 2, Object::NotPlayable);
        Ship& me = addShip(env, 20, 3, Object::Playable);
        Planet& pl = addPlanet(env, 100, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackString("t10");
        seg.pushBackInteger(20);
        interpreter::Arguments args(seg, 0, 2);

        TS_ASSERT_THROWS(game::interface::doCargoUnload(them, true, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root), game::Exception);
    }

    // Null amount
    {
        Environment env;
        Ship& sh = addShip(env, 17, 1, Object::Playable);
        Planet& pl = addPlanet(env, 100, 1, Object::Playable);

        afl::data::Segment seg;
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 1);

        game::interface::doCargoUnload(sh, false, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root);

        TS_ASSERT_EQUALS(pl.getCargo(Element::Tritanium).orElse(-1), 1000);
        TS_ASSERT_EQUALS(sh.getCargo(Element::Tritanium).orElse(-1), 10);
    }

    // Arity errror
    {
        Environment env;
        Ship& sh = addShip(env, 17, 1, Object::Playable);

        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);

        TS_ASSERT_THROWS(game::interface::doCargoUnload(sh, false, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root), interpreter::Error);
    }

    // Unknown ship
    {
        Environment env;
        Ship& sh = *env.turn->universe().ships().create(17);
        TS_ASSERT(!sh.getPosition().isValid());

        afl::data::Segment seg;
        seg.pushBackString("t7");
        interpreter::Arguments args(seg, 0, 1);

        TS_ASSERT_THROWS(game::interface::doCargoUnload(sh, false, env.proc, args, env.session, env.mapConfig, *env.turn, *env.root), game::Exception);
    }
}

