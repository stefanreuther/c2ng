/**
  *  \file u/t_game_actions_preconditions.cpp
  *  \brief Test for game::actions::Preconditions
  */

#include "game/actions/preconditions.hpp"

#include "t_game_actions.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
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

namespace {
    void addBase(game::map::Planet& planet)
    {
        game::map::BaseData data;
        data.owner = 1;
        data.numBaseDefensePosts = 9;
        data.damage = 0;
        planet.addCurrentBaseData(data, game::PlayerSet_t(1));
        planet.addCurrentPlanetData(game::map::PlanetData(), game::PlayerSet_t(1));

        game::map::Configuration config;
        afl::string::NullTranslator tx;
        afl::sys::Log log;
        planet.internalCheck(config, tx, log);
    }
}

/** Test ship. */
void
TestGameActionsPreconditions::testShip()
{
    // Uninitialized object throws
    game::map::Ship ship(42);
    TS_ASSERT_THROWS(game::actions::mustBePlayed(ship), game::Exception);

    // ReadOnly is not sufficient
    ship.setPlayability(ship.ReadOnly);
    TS_ASSERT_THROWS(game::actions::mustBePlayed(ship), game::Exception);

    // Playable is sufficient
    ship.setPlayability(ship.Playable);
    TS_ASSERT_THROWS_NOTHING(game::actions::mustBePlayed(ship));
}

/** Test planet. */
void
TestGameActionsPreconditions::testPlanet()
{
    // Uninitialized object throws
    game::map::Planet planet(42);
    TS_ASSERT_THROWS(game::actions::mustBePlayed(planet), game::Exception);

    // ReadOnly is not sufficient
    planet.setPlayability(planet.ReadOnly);
    TS_ASSERT_THROWS(game::actions::mustBePlayed(planet), game::Exception);

    // Playable is sufficient
    planet.setPlayability(planet.Playable);
    TS_ASSERT_THROWS_NOTHING(game::actions::mustBePlayed(planet));
}

/** Test base. */
void
TestGameActionsPreconditions::testBase()
{
    {
        // Uninitialized object throws
        game::map::Planet planet(42);
        TS_ASSERT_THROWS(game::actions::mustHavePlayedBase(planet), game::Exception);

        // Give it a base. Still not sufficient
        addBase(planet);
        TS_ASSERT_THROWS(game::actions::mustHavePlayedBase(planet), game::Exception);

        // ReadOnly is not sufficient
        planet.setPlayability(planet.ReadOnly);
        TS_ASSERT_THROWS(game::actions::mustHavePlayedBase(planet), game::Exception);

        // Playable is sufficient
        planet.setPlayability(planet.Playable);
        TS_ASSERT_THROWS_NOTHING(game::actions::mustHavePlayedBase(planet));
    }

    {
        // Playable planet throws if it has no base
        game::map::Planet planet(42);
        planet.setPlayability(planet.Playable);
        TS_ASSERT_THROWS(game::actions::mustHavePlayedBase(planet), game::Exception);

        // Add base
        addBase(planet);
        TS_ASSERT_THROWS_NOTHING(game::actions::mustHavePlayedBase(planet));
    }
}

/** Test session. */
void
TestGameActionsPreconditions::testSession()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    {
        // Uninitialized Session throws everything
        game::Session session(tx, fs);
        TS_ASSERT_THROWS(game::actions::mustHaveShipList(session), game::Exception);
        TS_ASSERT_THROWS(game::actions::mustHaveRoot(session), game::Exception);
        TS_ASSERT_THROWS(game::actions::mustHaveGame(session), game::Exception);
    }
    {
        // Just a ship list
        game::Session session(tx, fs);
        session.setShipList(new game::spec::ShipList());
        TS_ASSERT_THROWS_NOTHING(game::actions::mustHaveShipList(session));
        TS_ASSERT_THROWS(game::actions::mustHaveRoot(session), game::Exception);
        TS_ASSERT_THROWS(game::actions::mustHaveGame(session), game::Exception);
    }
    {
        // Just a root
        game::Session session(tx, fs);
        session.setRoot(new game::Root(afl::io::InternalDirectory::create("game"),
                                       *new game::test::SpecificationLoader(),
                                       game::HostVersion(),
                                       std::auto_ptr<game::RegistrationKey>(new game::test::RegistrationKey(game::RegistrationKey::Unknown, 100)),
                                       std::auto_ptr<game::StringVerifier>(new game::test::StringVerifier()),
                                       std::auto_ptr<afl::charset::Charset>(new afl::charset::Utf8Charset()),
                                       game::Root::Actions_t()));
        TS_ASSERT_THROWS(game::actions::mustHaveShipList(session), game::Exception);
        TS_ASSERT_THROWS_NOTHING(game::actions::mustHaveRoot(session));
        TS_ASSERT_THROWS(game::actions::mustHaveGame(session), game::Exception);
    }
    {
        // Just a game
        game::Session session(tx, fs);
        session.setGame(new game::Game());
        TS_ASSERT_THROWS(game::actions::mustHaveShipList(session), game::Exception);
        TS_ASSERT_THROWS(game::actions::mustHaveRoot(session), game::Exception);
        TS_ASSERT_THROWS_NOTHING(game::actions::mustHaveGame(session));
    }
    {
        // Everything
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
        TS_ASSERT_THROWS_NOTHING(game::actions::mustHaveShipList(session));
        TS_ASSERT_THROWS_NOTHING(game::actions::mustHaveRoot(session));
        TS_ASSERT_THROWS_NOTHING(game::actions::mustHaveGame(session));
    }
}

