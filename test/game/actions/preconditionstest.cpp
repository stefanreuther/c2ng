/**
  *  \file test/game/actions/preconditionstest.cpp
  *  \brief Test for game::actions::Preconditions
  */

#include "game/actions/preconditions.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/exception.hpp"
#include "game/game.hpp"
#include "game/map/configuration.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/registrationkey.hpp"
#include "game/root.hpp"
#include "game/specificationloader.hpp"
#include "game/stringverifier.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/specificationloader.hpp"
#include "game/test/stringverifier.hpp"
#include "game/turn.hpp"

namespace {
    void addBase(game::map::Planet& planet)
    {
        game::map::BaseData data;
        data.numBaseDefensePosts = 9;
        data.damage = 0;
        planet.addCurrentBaseData(data, game::PlayerSet_t(1));
        planet.addCurrentPlanetData(game::map::PlanetData(), game::PlayerSet_t(1));

        game::map::Configuration config;
        afl::string::NullTranslator tx;
        afl::sys::Log log;
        planet.internalCheck(config, game::PlayerSet_t(1), 15, tx, log);
    }
}

/*
 *  Ship
 */

// Uninitialized object throws
AFL_TEST("game.actions.Preconditions:mustBePlayed(Ship):uninit", a)
{
    game::map::Ship ship(42);
    AFL_CHECK_THROWS(a, game::actions::mustBePlayed(ship), game::Exception);
}

// ReadOnly is not sufficient
AFL_TEST("game.actions.Preconditions:mustBePlayed(Ship):ReadOnly", a)
{
    game::map::Ship ship(42);
    ship.setPlayability(ship.ReadOnly);
    AFL_CHECK_THROWS(a, game::actions::mustBePlayed(ship), game::Exception);
}

// Playable is sufficient
AFL_TEST("game.actions.Preconditions:mustBePlayed(Ship):Playable", a)
{
    game::map::Ship ship(42);
    ship.setPlayability(ship.Playable);
    AFL_CHECK_SUCCEEDS(a, game::actions::mustBePlayed(ship));
}

/*
 *  Planet
 */

// Uninitialized object throws
AFL_TEST("game.actions.Preconditions:mustBePlayed(Planet):uninit", a)
{
    game::map::Planet planet(42);
    AFL_CHECK_THROWS(a, game::actions::mustBePlayed(planet), game::Exception);
}

// ReadOnly is not sufficient
AFL_TEST("game.actions.Preconditions:mustBePlayed(Planet):ReadOnly", a)
{
    game::map::Planet planet(42);
    planet.setPlayability(planet.ReadOnly);
    AFL_CHECK_THROWS(a, game::actions::mustBePlayed(planet), game::Exception);
}

// Playable is sufficient
AFL_TEST("game.actions.Preconditions:mustBePlayed(Planet):Playable", a)
{
    game::map::Planet planet(42);
    planet.setPlayability(planet.Playable);
    AFL_CHECK_SUCCEEDS(a, game::actions::mustBePlayed(planet));
}

/*
 *  Base
 */

// Uninitialized object throws
AFL_TEST("game.actions.Preconditions:mustHavePlayedBase:uninit", a)
{
    game::map::Planet planet(42);
    AFL_CHECK_THROWS(a, game::actions::mustHavePlayedBase(planet), game::Exception);
}

// Give it a base. Still not sufficient
AFL_TEST("game.actions.Preconditions:mustHavePlayedBase:base", a)
{
    game::map::Planet planet(42);
    addBase(planet);
    AFL_CHECK_THROWS(a, game::actions::mustHavePlayedBase(planet), game::Exception);
}

// ReadOnly is not sufficient
AFL_TEST("game.actions.Preconditions:mustHavePlayedBase:ReadOnly", a)
{
    game::map::Planet planet(42);
    addBase(planet);
    planet.setPlayability(planet.ReadOnly);
    AFL_CHECK_THROWS(a, game::actions::mustHavePlayedBase(planet), game::Exception);
}

// Playable is sufficient
AFL_TEST("game.actions.Preconditions:mustHavePlayedBase:Playable", a)
{
    game::map::Planet planet(42);
    addBase(planet);
    planet.setPlayability(planet.Playable);
    AFL_CHECK_SUCCEEDS(a, game::actions::mustHavePlayedBase(planet));
}

// Playable planet throws if it has no base
AFL_TEST("game.actions.Preconditions:mustHavePlayedBase:Playable:no-base", a)
{
    game::map::Planet planet(42);
    planet.setPlayability(planet.Playable);
    AFL_CHECK_THROWS(a, game::actions::mustHavePlayedBase(planet), game::Exception);
}

// Add base (same as ":Playable")
AFL_TEST("game.actions.Preconditions:mustHavePlayedBase:Playable:with-base", a)
{
    game::map::Planet planet(42);
    planet.setPlayability(planet.Playable);
    addBase(planet);
    AFL_CHECK_SUCCEEDS(a, game::actions::mustHavePlayedBase(planet));
}

/*
 *  Session
 */

// Uninitialized Session throws everything
AFL_TEST("game.actions.Preconditions:session:empty", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    AFL_CHECK_THROWS(a("mustHaveShipList"), game::actions::mustHaveShipList(session), game::Exception);
    AFL_CHECK_THROWS(a("mustHaveRoot"), game::actions::mustHaveRoot(session), game::Exception);
    AFL_CHECK_THROWS(a("mustHaveGame"), game::actions::mustHaveGame(session), game::Exception);
}

// Just a ship list
AFL_TEST("game.actions.Preconditions:session:just-shiplist", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setShipList(new game::spec::ShipList());
    AFL_CHECK_SUCCEEDS(a("mustHaveShipList"), game::actions::mustHaveShipList(session));
    AFL_CHECK_THROWS(a("mustHaveRoot"), game::actions::mustHaveRoot(session), game::Exception);
    AFL_CHECK_THROWS(a("mustHaveGame"), game::actions::mustHaveGame(session), game::Exception);
}

// Just a root
AFL_TEST("game.actions.Preconditions:session:just-root", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(new game::Root(afl::io::InternalDirectory::create("game"),
                                   *new game::test::SpecificationLoader(),
                                   game::HostVersion(),
                                   std::auto_ptr<game::RegistrationKey>(new game::test::RegistrationKey(game::RegistrationKey::Unknown, 100)),
                                   std::auto_ptr<game::StringVerifier>(new game::test::StringVerifier()),
                                   std::auto_ptr<afl::charset::Charset>(new afl::charset::Utf8Charset()),
                                   game::Root::Actions_t()));
    AFL_CHECK_THROWS(a("mustHaveShipList"), game::actions::mustHaveShipList(session), game::Exception);
    AFL_CHECK_SUCCEEDS(a("mustHaveRoot"), game::actions::mustHaveRoot(session));
    AFL_CHECK_THROWS(a("mustHaveGame"), game::actions::mustHaveGame(session), game::Exception);
}

// Just a game
AFL_TEST("game.actions.Preconditions:session:just-game", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setGame(new game::Game());
    AFL_CHECK_THROWS(a("mustHaveShipList"), game::actions::mustHaveShipList(session), game::Exception);
    AFL_CHECK_THROWS(a("mustHaveRoot"), game::actions::mustHaveRoot(session), game::Exception);
    AFL_CHECK_SUCCEEDS(a("mustHaveGame"), game::actions::mustHaveGame(session));
}

// Everything
AFL_TEST("game.actions.Preconditions:session:full", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setShipList(new game::spec::ShipList());
    session.setRoot(new game::Root(afl::io::InternalDirectory::create("game"),
                                   *new game::test::SpecificationLoader(),
                                   game::HostVersion(),
                                   std::auto_ptr<game::RegistrationKey>(new game::test::RegistrationKey(game::RegistrationKey::Unknown, 100)),
                                   std::auto_ptr<game::StringVerifier>(new game::test::StringVerifier()),
                                   std::auto_ptr<afl::charset::Charset>(new afl::charset::Utf8Charset()),
                                   game::Root::Actions_t()));
    session.setGame(new game::Game());
    AFL_CHECK_SUCCEEDS(a("mustHaveShipList"), game::actions::mustHaveShipList(session));
    AFL_CHECK_SUCCEEDS(a("mustHaveRoot"), game::actions::mustHaveRoot(session));
    AFL_CHECK_SUCCEEDS(a("mustHaveGame"), game::actions::mustHaveGame(session));
}

/*
 *  mustAllowCommands, mustBeLocallyEditable
 */

AFL_TEST("game.actions.Preconditions:mustAllowCommands:success", a)
{
    game::Turn t;
    t.setLocalDataPlayers(game::PlayerSet_t(1));
    AFL_CHECK_THROWS(a, game::actions::mustAllowCommands(t, 1), game::Exception);
}

AFL_TEST("game.actions.Preconditions:mustAllowCommands:failure:empty", a)
{
    game::Turn t;
    AFL_CHECK_THROWS(a, game::actions::mustAllowCommands(t, 1), game::Exception);
}

AFL_TEST("game.actions.Preconditions:mustAllowCommands:failure:mismatch", a)
{
    game::Turn t;
    t.setLocalDataPlayers(game::PlayerSet_t(2));
    AFL_CHECK_THROWS(a, game::actions::mustAllowCommands(t, 1), game::Exception);
}

AFL_TEST("game.actions.Preconditions:mustBeLocallyEditable:success", a)
{
    game::Turn t;
    t.setCommandPlayers(game::PlayerSet_t(1));
    AFL_CHECK_THROWS(a, game::actions::mustBeLocallyEditable(t), game::Exception);
}

AFL_TEST("game.actions.Preconditions:mustBeLocallyEditable:failure", a)
{
    game::Turn t;
    AFL_CHECK_THROWS(a, game::actions::mustBeLocallyEditable(t), game::Exception);
}
