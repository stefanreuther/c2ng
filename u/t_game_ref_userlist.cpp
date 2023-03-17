/**
  *  \file u/t_game_ref_userlist.cpp
  *  \brief Test for game::ref::UserList
  */

#include "game/ref/userlist.hpp"

#include "t_game_ref.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/player.hpp"
#include "game/ref/sortby.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include "game/turn.hpp"

namespace {
    struct Environment {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session;

        Environment()
            : tx(), fs(), session(tx, fs)
            { }
    };

    game::Root& addRoot(Environment& env)
    {
        if (env.session.getRoot().get() == 0) {
            env.session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
        }
        return *env.session.getRoot();
    }

    game::Game& addGame(Environment& env)
    {
        if (env.session.getGame().get() == 0) {
            env.session.setGame(new game::Game());
        }
        return *env.session.getGame();
    }

    game::spec::ShipList& addShipList(Environment& env)
    {
        if (env.session.getShipList().get() == 0) {
            env.session.setShipList(new game::spec::ShipList());
        }
        return *env.session.getShipList();
    }

    game::map::Ship& addShip(Environment& env, int id, int owner)
    {
        game::map::Ship& sh = *addGame(env).currentTurn().universe().ships().create(id);
        sh.addShipXYData(game::map::Point(1000, 1000), owner, 200, game::PlayerSet_t(1));
        return sh;
    }

    void addPlayer(Environment& env, int id, String_t name, String_t adj)
    {
        game::Player& pl = *addRoot(env).playerList().create(id);
        pl.setName(game::Player::ShortName, name);
        pl.setName(game::Player::AdjectiveName, adj);
    }
}

/** Test makeReferenceItem(). */
void
TestGameRefUserList::testMakeReferenceItem()
{
    // Set up
    Environment env;

    // - add a player
    const int PLAYER_NR = 10;
    env.session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    env.session.getRoot()->playerList().create(PLAYER_NR)->setName(game::Player::ShortName, "The Rebels");

    // - add a ship
    const int SHIP_NR = 17;
    game::map::Ship& sh = addShip(env, SHIP_NR, 7);
    sh.setName("USS Yamok");
    sh.setPlayability(game::map::Object::ReadOnly);
    sh.setIsMarked(true);

    // Test: Player reference
    const game::Reference ref1(game::Reference::Player, PLAYER_NR);
    const game::ref::UserList::Item it1 = game::ref::UserList::makeReferenceItem(ref1, env.session);
    TS_ASSERT_EQUALS(it1.type, game::ref::UserList::ReferenceItem);
    TS_ASSERT_EQUALS(it1.name, "Player #10: The Rebels");
    TS_ASSERT_EQUALS(it1.reference, ref1);
    TS_ASSERT_EQUALS(it1.marked, false);
    TS_ASSERT_EQUALS(it1.playability, game::map::Object::NotPlayable);
    TS_ASSERT_EQUALS(it1.color, util::SkinColor::Static);

    // Test: Object reference
    const game::Reference ref2(game::Reference::Ship, SHIP_NR);
    const game::ref::UserList::Item it2 = game::ref::UserList::makeReferenceItem(ref2, env.session);
    TS_ASSERT_EQUALS(it2.type, game::ref::UserList::ReferenceItem);
    TS_ASSERT_EQUALS(it2.name, "Ship #17: USS Yamok");
    TS_ASSERT_EQUALS(it2.reference, ref2);
    TS_ASSERT_EQUALS(it2.marked, true);
    TS_ASSERT_EQUALS(it2.playability, game::map::Object::ReadOnly);
    TS_ASSERT_EQUALS(it2.color, util::SkinColor::Red);
}

/** Test add(details), add(UserList), and accessors (get(), find(), size(), empty(), equals). */
void
TestGameRefUserList::testAdd()
{
    // Verify initial status
    game::ref::UserList testee;
    TS_ASSERT_EQUALS(testee.empty(), true);
    TS_ASSERT_EQUALS(testee.size(), 0U);
    TS_ASSERT_EQUALS(testee == game::ref::UserList(), true);
    TS_ASSERT_EQUALS(testee != game::ref::UserList(), false);
    TS_ASSERT(testee.get(0) == 0);

    // Add something and verify status
    testee.add(game::ref::UserList::OtherItem,     "o",  game::Reference(),                           false, game::map::Object::Editable, util::SkinColor::Blue);
    testee.add(game::ref::UserList::ReferenceItem, "pl", game::Reference(game::Reference::Planet, 7), true,  game::map::Object::ReadOnly, util::SkinColor::Red);
    TS_ASSERT_EQUALS(testee.empty(), false);
    TS_ASSERT_EQUALS(testee.size(), 2U);
    TS_ASSERT_EQUALS(testee == game::ref::UserList(), false);
    TS_ASSERT_EQUALS(testee != game::ref::UserList(), true);
    TS_ASSERT(testee.get(0) != 0);
    TS_ASSERT_EQUALS(testee.get(0)->name, "o");
    TS_ASSERT_EQUALS(testee.get(0)->color, util::SkinColor::Blue);

    size_t pos = 999;
    TS_ASSERT_EQUALS(testee.find(game::Reference(game::Reference::Planet, 7), pos), true);
    TS_ASSERT_EQUALS(pos, 1U);
    TS_ASSERT_EQUALS(testee.get(pos)->name, "pl");

    // Duplicate
    game::ref::UserList copy1(testee);
    game::ref::UserList copy2;
    copy2.add(testee);

    TS_ASSERT_EQUALS(copy1.size(), 2U);
    TS_ASSERT_EQUALS(copy2.size(), 2U);
    TS_ASSERT_EQUALS(copy1 == copy2, true);
    TS_ASSERT_EQUALS(copy1 != copy2, false);
}

/** Test add(List) with dividers. */
void
TestGameRefUserList::testAddList()
{
    // Set up
    Environment env;

    // - add players
    addPlayer(env, 1, "Feds", "federal");
    addPlayer(env, 2, "Gorn", "gorn");

    // - add specs
    game::test::addAnnihilation(addShipList(env));
    game::test::addOutrider(addShipList(env));

    // - add some ships
    {
        game::map::Ship& sh = addShip(env, 1, 1);
        sh.setName("Fed One");
        sh.setHull(game::test::OUTRIDER_HULL_ID);
    }
    {
        game::map::Ship& sh = addShip(env, 2, 1);
        sh.setName("Fed Two");
        sh.setHull(game::test::ANNIHILATION_HULL_ID);
    }
    {
        game::map::Ship& sh = addShip(env, 3, 2);
        sh.setName("Gorn Three");
        sh.setHull(game::test::ANNIHILATION_HULL_ID);
    }
    {
        game::map::Ship& sh = addShip(env, 4, 2);
        sh.setName("Gorn Four");
        sh.setHull(game::test::ANNIHILATION_HULL_ID);
    }

    // Prepare a list
    game::ref::List list;
    for (int i = 1; i <= 4; ++i) {
        list.add(game::Reference(game::Reference::Ship, i));
    }

    // Convert to UserList
    const game::ref::SortBy::Owner divi(addGame(env).currentTurn().universe(),
                                        addRoot(env).playerList(),
                                        env.tx);
    const game::ref::SortBy::HullType subdivi(addGame(env).currentTurn().universe(),
                                              addShipList(env),
                                              env.tx);
    game::ref::UserList testee;
    testee.add(list, env.session, divi, subdivi);

    // Verify
    //   0: == Fed ==
    //   1: -- Outrider --
    //   2: Fed One
    //   3: -- Annihilation --
    //   4: Fed Two
    //   5: == Gorn ==
    //   6: -- Annihilation --
    //   7: Gorn Three
    //   8: Gorn Four
    TS_ASSERT_EQUALS(testee.size(), 9U);
    TS_ASSERT_EQUALS(testee.get(0)->name, "Feds");
    TS_ASSERT_EQUALS(testee.get(1)->name, "OUTRIDER CLASS SCOUT");
    TS_ASSERT_EQUALS(testee.get(2)->name, "Ship #1: Fed One (federal OUTRIDER CLASS SCOUT)");
    TS_ASSERT_EQUALS(testee.get(3)->name, "ANNIHILATION CLASS BATTLESHIP");
    TS_ASSERT_EQUALS(testee.get(4)->name, "Ship #2: Fed Two (federal ANNIHILATION CLASS BATTLESHIP)");
    TS_ASSERT_EQUALS(testee.get(5)->name, "Gorn");
    TS_ASSERT_EQUALS(testee.get(6)->name, "ANNIHILATION CLASS BATTLESHIP");
    TS_ASSERT_EQUALS(testee.get(7)->name, "Ship #3: Gorn Three (gorn ANNIHILATION CLASS BATTLESHIP)");
    TS_ASSERT_EQUALS(testee.get(8)->name, "Ship #4: Gorn Four (gorn ANNIHILATION CLASS BATTLESHIP)");

    TS_ASSERT_EQUALS(testee.get(0)->type, game::ref::UserList::DividerItem);
    TS_ASSERT_EQUALS(testee.get(1)->type, game::ref::UserList::SubdividerItem);
    TS_ASSERT_EQUALS(testee.get(2)->type, game::ref::UserList::ReferenceItem);
    TS_ASSERT_EQUALS(testee.get(3)->type, game::ref::UserList::SubdividerItem);
    TS_ASSERT_EQUALS(testee.get(4)->type, game::ref::UserList::ReferenceItem);
    TS_ASSERT_EQUALS(testee.get(5)->type, game::ref::UserList::DividerItem);
    TS_ASSERT_EQUALS(testee.get(6)->type, game::ref::UserList::SubdividerItem);
    TS_ASSERT_EQUALS(testee.get(7)->type, game::ref::UserList::ReferenceItem);
    TS_ASSERT_EQUALS(testee.get(8)->type, game::ref::UserList::ReferenceItem);
}

