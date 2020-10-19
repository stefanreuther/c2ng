/**
  *  \file u/t_game_spec_info_browser.cpp
  *  \brief Test for game::spec::info::Browser
  */

#include "game/spec/info/browser.hpp"

#include "t_game_spec_info.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/spec/info/filter.hpp"
#include "game/spec/info/nullpicturenamer.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"

namespace gsi = game::spec::info;

namespace {
    using game::Player;
    using game::PlayerSet_t;

    struct TestHarness {
        gsi::NullPictureNamer picNamer;
        game::test::Root root;
        game::spec::ShipList shipList;
        afl::string::NullTranslator tx;

        TestHarness()
            : picNamer(), root(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0))),
              shipList(), tx()
            { }
    };

    void createEngine(TestHarness& h, int id, String_t name, int tech)
    {
        game::spec::Engine* e = h.shipList.engines().create(id);
        e->setName(name);
        e->setTechLevel(tech);
    }

    void createHullFunction(TestHarness& h, int id, String_t name, String_t description)
    {
        game::spec::BasicHullFunction* hf = h.shipList.basicHullFunctions().addFunction(id, name);
        hf->setDescription(description);
    }

    void createHull(TestHarness& h, int id, String_t name, int numEngines)
    {
        game::spec::Hull* p = h.shipList.hulls().create(id);
        p->setName(name);
        p->setNumEngines(numEngines);
    }

    const int VIEWPOINT_PLAYER = 3;

    const gsi::Attribute* findAttribute(const gsi::PageContent& c, String_t name)
    {
        for (size_t i = 0; i < c.attributes.size(); ++i) {
            if (c.attributes[i].name == name) {
                return &c.attributes[i];
            }
        }
        return 0;
    }

    const gsi::FilterInfo* findAttribute(const gsi::FilterInfos_t& f, gsi::FilterAttribute att)
    {
        for (size_t i = 0; i < f.size(); ++i) {
            if (f[i].elem.att == att) {
                return &f[i];
            }
        }
        return 0;
    }
}

/** Test describe(PlayerPage). */
void
TestGameSpecInfoBrowser::testDescribePlayer()
{
    // Create a player
    TestHarness h;
    Player* pl = h.root.playerList().create(7);
    TS_ASSERT(pl);
    pl->setName(Player::LongName, "The Sevens");
    pl->setName(Player::AdjectiveName, "sevenses");
    pl->setName(Player::EmailAddress, "e@mail.7");

    // Get it
    gsi::Browser testee(h.picNamer, h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);
    std::auto_ptr<gsi::PageContent> c(testee.describeItem(gsi::PlayerPage, 7));

    // Verify
    TS_ASSERT(c.get());
    TS_ASSERT_EQUALS(c->title, "The Sevens");
    TS_ASSERT(c->players.empty());

    const gsi::Attribute* a = findAttribute(*c, "Adjective");
    TS_ASSERT(a);
    TS_ASSERT_EQUALS(a->value, "sevenses");

    a = findAttribute(*c, "Short name");
    TS_ASSERT(!a);
}

/** Test describe(HullPage). */
void
TestGameSpecInfoBrowser::testDescribeHull()
{
    // Create a hull
    TestHarness h;

    const game::Id_t HULL_NR = 9;
    h.shipList.hullAssignments().add(2, 3, HULL_NR);
    h.shipList.hullAssignments().add(5, 9, HULL_NR);
    createHull(h, HULL_NR, "LUDMILLA", 3);

    // Get it
    gsi::Browser testee(h.picNamer, h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);
    std::auto_ptr<gsi::PageContent> c(testee.describeItem(gsi::HullPage, HULL_NR));

    // Verify
    TS_ASSERT(c.get());
    TS_ASSERT_EQUALS(c->title, "LUDMILLA");
    TS_ASSERT_EQUALS(c->players, PlayerSet_t() + 2 + 5);

    const gsi::Attribute* a = findAttribute(*c, "Engines");
    TS_ASSERT(a);
    TS_ASSERT_EQUALS(a->value, "3");
}

/** Test describe(RacialAbilitiesPage). */
void
TestGameSpecInfoBrowser::testDescribeRacial()
{
    // The default configuration creates a number of configuration abilities
    // (we have not created any hullfunc-based abilities).
    // We need to create players, though, because otherwise all abilities will be dropped.
    TestHarness h;
    h.root.playerList().create(1);
    h.root.playerList().create(2);
    gsi::Browser testee(h.picNamer, h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);
    std::auto_ptr<gsi::PageContent> c(testee.describeItem(gsi::RacialAbilitiesPage, 0));

    // Verify
    TS_ASSERT(c.get());
    TS_ASSERT(!c->title.empty());
    TS_ASSERT(!c->players.empty());

    const gsi::Attribute* a = findAttribute(*c, "Origin");
    TS_ASSERT(a);
    TS_ASSERT_EQUALS(a->value, "Host configuration");
}

/** Test describe(ShipAbilitiesPage). */
void
TestGameSpecInfoBrowser::testDescribeShip()
{
    // Create a hull function
    const game::Id_t HULL_NR = 17;
    TestHarness h;
    createHullFunction(h, 12, "Play", "Do stuff");
    createHullFunction(h, 17, "PlayToo", "Do more stuff");

    // Create a hull that has this function
    // - function available to player 3 + 4
    // - hull buildable by 4 + 5
    game::spec::Hull* p = h.shipList.hulls().create(HULL_NR);
    p->changeHullFunction(h.shipList.modifiedHullFunctions().getFunctionIdFromHostId(17), PlayerSet_t() + 3 + 4, PlayerSet_t(), true);
    h.shipList.hullAssignments().add(4, 1, HULL_NR);
    h.shipList.hullAssignments().add(5, 1, HULL_NR);

    // Get it
    // This is index-based access, 1=second (hf2)
    gsi::Browser testee(h.picNamer, h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);
    std::auto_ptr<gsi::PageContent> c(testee.describeItem(gsi::ShipAbilitiesPage, 1));

    // Verify
    TS_ASSERT(c.get());
    TS_ASSERT_EQUALS(c->title, "Do more stuff");
    TS_ASSERT_EQUALS(c->players, PlayerSet_t() + 4);

    const gsi::Attribute* a = findAttribute(*c, "Id");
    TS_ASSERT(a);
    TS_ASSERT_EQUALS(a->value, "17");

    a = findAttribute(*c, "Name");
    TS_ASSERT(a);
    TS_ASSERT_EQUALS(a->value, "PlayToo");
}

/** Test describe(EnginePage). */
void
TestGameSpecInfoBrowser::testDescribeEngine()
{
    // Create an engine
    TestHarness h;
    createEngine(h, 8, "6 litre V8", 3);

    // Get it
    gsi::Browser testee(h.picNamer, h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);
    std::auto_ptr<gsi::PageContent> c(testee.describeItem(gsi::EnginePage, 8));

    // Verify
    TS_ASSERT(c.get());
    TS_ASSERT_EQUALS(c->title, "6 litre V8");
    TS_ASSERT_EQUALS(c->players, PlayerSet_t());

    const gsi::Attribute* a = findAttribute(*c, "Tech level");
    TS_ASSERT(a);
    TS_ASSERT_EQUALS(a->value, "3");
}

/** Test describe(BeamPage). */
void
TestGameSpecInfoBrowser::testDescribeBeam()
{
    // Create a beam
    TestHarness h;
    game::test::initStandardBeams(h.shipList);

    // Get it
    gsi::Browser testee(h.picNamer, h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);
    std::auto_ptr<gsi::PageContent> c(testee.describeItem(gsi::BeamPage, 2));

    // Verify
    TS_ASSERT(c.get());
    TS_ASSERT_EQUALS(c->title, "X-Ray Laser");
    TS_ASSERT_EQUALS(c->players, PlayerSet_t());

    const gsi::Attribute* a = findAttribute(*c, "Kill");
    TS_ASSERT(a);
    TS_ASSERT_EQUALS(a->value, "15");
}

/** Test describe(TorpedoPage). */
void
TestGameSpecInfoBrowser::testDescribeTorpedo()
{
    // Create a beam
    TestHarness h;
    game::test::initStandardTorpedoes(h.shipList);

    // Get it
    gsi::Browser testee(h.picNamer, h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);
    std::auto_ptr<gsi::PageContent> c(testee.describeItem(gsi::TorpedoPage, 7));

    // Verify
    TS_ASSERT(c.get());
    TS_ASSERT_EQUALS(c->title, "Mark 5 Photon");
    TS_ASSERT_EQUALS(c->players, PlayerSet_t());

    const gsi::Attribute* a = findAttribute(*c, "Kill");
    TS_ASSERT(a);
    TS_ASSERT_EQUALS(a->value, "34");  // note: doubled!
}

/** Test listItems(PlayerPage). */
void
TestGameSpecInfoBrowser::testListPlayer()
{
    TestHarness h;
    Player* pl1 = h.root.playerList().create(1);
    pl1->setName(Player::LongName, "The Federation");
    pl1->setName(Player::ShortName, "Federation");

    Player* pl2 = h.root.playerList().create(2);
    pl2->setName(Player::LongName, "The Lizards");
    pl2->setName(Player::ShortName, "Lizard");

    Player* pl3 = h.root.playerList().create(3);
    pl3->setName(Player::LongName, "The Birds");
    pl3->setName(Player::ShortName, "Bird");

    gsi::Browser testee(h.picNamer, h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);

    // Check 1:
    std::auto_ptr<gsi::ListContent> c(testee.listItems(gsi::PlayerPage, gsi::Filter(), gsi::String_Name));
    TS_ASSERT(c.get());
    TS_ASSERT_EQUALS(c->content.size(), 3U);
    TS_ASSERT_EQUALS(c->content[0].name, "Bird");
    TS_ASSERT_EQUALS(c->content[0].id, 3);
    TS_ASSERT_EQUALS(c->content[1].name, "Federation");
    TS_ASSERT_EQUALS(c->content[1].id, 1);
    TS_ASSERT_EQUALS(c->content[2].name, "Lizard");
    TS_ASSERT_EQUALS(c->content[2].id, 2);

    // Check 2: with filter. 'th ds' matches 'The Lizards', 'The Birds'.
    gsi::Filter f;
    f.setNameFilter("th ds");
    c = testee.listItems(gsi::PlayerPage, f, gsi::Range_Id);

    TS_ASSERT(c.get());
    TS_ASSERT_EQUALS(c->content.size(), 2U);
    TS_ASSERT_EQUALS(c->content[0].name, "Lizard");
    TS_ASSERT_EQUALS(c->content[0].id, 2);
    TS_ASSERT_EQUALS(c->content[1].name, "Bird");
    TS_ASSERT_EQUALS(c->content[1].id, 3);
}

/** Test listItems(HullPage). */
void
TestGameSpecInfoBrowser::testListHull()
{
    TestHarness h;
    createHull(h, 1, "FIRST CLASS CRUISER", 2);
    createHull(h, 2, "SECOND CLASS CRUISER", 3);
    createHull(h, 3, "THIRD CLASS LIGHT CRUISER", 1);
    createHull(h, 4, "FOURTH CLASS BATTLESHIP", 2);
    createHull(h, 5, "LIGHT FIFTH CLASS CRUISER", 1);
    createHull(h, 6, "SIXTH CLASS LIGHT CRUISER", 4);
    h.shipList.hullAssignments().add(4, 1, 5);
    h.shipList.hullAssignments().add(4, 2, 6);
    h.shipList.hullAssignments().add(4, 7, 1);
    gsi::Browser testee(h.picNamer, h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);

    // Check 1: full list
    std::auto_ptr<gsi::ListContent> c(testee.listItems(gsi::HullPage, gsi::Filter(), gsi::Range_Id));
    TS_ASSERT(c.get());
    TS_ASSERT_EQUALS(c->content.size(), 6U);
    TS_ASSERT_EQUALS(c->content[0].name, "FIRST CLASS CRUISER");
    TS_ASSERT_EQUALS(c->content[0].id, 1);
    TS_ASSERT_EQUALS(c->content[5].name, "SIXTH CLASS LIGHT CRUISER");
    TS_ASSERT_EQUALS(c->content[5].id, 6);

    // Check 2: filter name:"light class", numEngines:1, sort by name. Produces [5,3]
    gsi::Filter f;
    f.setNameFilter("light class");
    f.add(gsi::FilterElement(gsi::Range_NumEngines, 0, gsi::IntRange_t::fromValue(1)));
    c = testee.listItems(gsi::HullPage, f, gsi::String_Name);
    TS_ASSERT(c.get());
    TS_ASSERT_EQUALS(c->content.size(), 2U);
    TS_ASSERT_EQUALS(c->content[0].name, "LIGHT FIFTH CLASS CRUISER");
    TS_ASSERT_EQUALS(c->content[0].id, 5);
    TS_ASSERT_EQUALS(c->content[1].name, "THIRD CLASS LIGHT CRUISER");
    TS_ASSERT_EQUALS(c->content[1].id, 3);

    // Check 3: filter by player
    gsi::Filter f2;
    f2.add(gsi::FilterElement(gsi::Value_Player, 4, gsi::IntRange_t()));
    c = testee.listItems(gsi::HullPage, f2, gsi::Range_Id);
    TS_ASSERT(c.get());
    TS_ASSERT_EQUALS(c->content.size(), 3U);
    TS_ASSERT_EQUALS(c->content[0].name, "LIGHT FIFTH CLASS CRUISER");
    TS_ASSERT_EQUALS(c->content[0].id, 5);
    TS_ASSERT_EQUALS(c->content[1].name, "SIXTH CLASS LIGHT CRUISER");
    TS_ASSERT_EQUALS(c->content[1].id, 6);
    TS_ASSERT_EQUALS(c->content[2].name, "FIRST CLASS CRUISER");
    TS_ASSERT_EQUALS(c->content[2].id, 1);
}

/** Test listItems(RacialAbilitiesPage). */
void
TestGameSpecInfoBrowser::testListRacial()
{
    TestHarness h;
    h.root.playerList().create(1);
    h.root.playerList().create(2);
    gsi::Browser testee(h.picNamer, h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);

    // Check
    std::auto_ptr<gsi::ListContent> c(testee.listItems(gsi::RacialAbilitiesPage, gsi::Filter(), gsi::Range_Id));
    TS_ASSERT(c.get());
    TS_ASSERT(c->content.size() > 0);
}

/** Test listItems(ShipAbilitiesPage). */
void
TestGameSpecInfoBrowser::testListShip()
{
    TestHarness h;
    createHullFunction(h, 7,  "a", "eat");
    createHullFunction(h, 10, "b", "Drink");
    createHullFunction(h, 3,  "c", "Sleep");
    createHullFunction(h, 9,  "d", "Repeat");
    gsi::Browser testee(h.picNamer, h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);

    // Check 1:
    std::auto_ptr<gsi::ListContent> c(testee.listItems(gsi::ShipAbilitiesPage, gsi::Filter(), gsi::Range_Id));
    TS_ASSERT(c.get());
    TS_ASSERT_EQUALS(c->content.size(), 4U);
    TS_ASSERT_EQUALS(c->content[0].name, "eat");
    TS_ASSERT_EQUALS(c->content[0].id, 0);
    TS_ASSERT_EQUALS(c->content[3].name, "Repeat");
    TS_ASSERT_EQUALS(c->content[3].id, 3);

    // CHeck 2:
    gsi::Filter f;
    f.setNameFilter("e");
    f.add(gsi::FilterElement(gsi::Range_Tech, 0, gsi::IntRange_t(1, 100))); // pointless filter, ignored
    c = testee.listItems(gsi::ShipAbilitiesPage, f, gsi::String_Name);
    TS_ASSERT(c.get());
    TS_ASSERT_EQUALS(c->content.size(), 3U);
    TS_ASSERT_EQUALS(c->content[0].name, "eat");
    TS_ASSERT_EQUALS(c->content[0].id, 0);
    TS_ASSERT_EQUALS(c->content[1].name, "Repeat");
    TS_ASSERT_EQUALS(c->content[1].id, 3);
    TS_ASSERT_EQUALS(c->content[2].name, "Sleep");
    TS_ASSERT_EQUALS(c->content[2].id, 2);
}

/** Test listItems(EnginePage). */
void
TestGameSpecInfoBrowser::testListEngine()
{
    TestHarness h;
    createEngine(h, 2, "Two-speed", 3);
    createEngine(h, 3, "Three-speed", 4);
    createEngine(h, 4, "Four-speed", 5);
    gsi::Browser testee(h.picNamer, h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);

    // Check 1:
    std::auto_ptr<gsi::ListContent> c(testee.listItems(gsi::EnginePage, gsi::Filter(), gsi::Range_Id));
    TS_ASSERT(c.get());
    TS_ASSERT_EQUALS(c->content.size(), 3U);
    TS_ASSERT_EQUALS(c->content[0].name, "Two-speed");
    TS_ASSERT_EQUALS(c->content[0].id, 2);
    TS_ASSERT_EQUALS(c->content[2].name, "Four-speed");
    TS_ASSERT_EQUALS(c->content[2].id, 4);

    // Check 2:
    gsi::Filter f;
    f.setNameFilter("o");
    f.add(gsi::FilterElement(gsi::Range_Tech, 0, gsi::IntRange_t(4, 100)));
    c = testee.listItems(gsi::EnginePage, f, gsi::Range_Id);
    TS_ASSERT(c.get());
    TS_ASSERT_EQUALS(c->content.size(), 1U);
    TS_ASSERT_EQUALS(c->content[0].name, "Four-speed");
    TS_ASSERT_EQUALS(c->content[0].id, 4);
}

/** Test listItems(BeamPage). */
void
TestGameSpecInfoBrowser::testListBeam()
{
    TestHarness h;
    game::test::initStandardBeams(h.shipList);
    gsi::Browser testee(h.picNamer, h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);

    // Check 1:
    std::auto_ptr<gsi::ListContent> c(testee.listItems(gsi::BeamPage, gsi::Filter(), gsi::Range_Id));
    TS_ASSERT(c.get());
    TS_ASSERT_EQUALS(c->content.size(), 10U);
    TS_ASSERT_EQUALS(c->content[0].name, "Laser");
    TS_ASSERT_EQUALS(c->content[0].id, 1);
    TS_ASSERT_EQUALS(c->content[9].name, "Heavy Phaser");
    TS_ASSERT_EQUALS(c->content[9].id, 10);

    // Check 2:
    gsi::Filter f;
    f.setNameFilter("aser");
    f.add(gsi::FilterElement(gsi::Range_KillPower, 0, gsi::IntRange_t(15, 30)));
    c = testee.listItems(gsi::BeamPage, f, gsi::String_Name);
    TS_ASSERT(c.get());
    TS_ASSERT_EQUALS(c->content.size(), 2U);
    TS_ASSERT_EQUALS(c->content[0].name, "Phaser");
    TS_ASSERT_EQUALS(c->content[0].id, 8);
    TS_ASSERT_EQUALS(c->content[1].name, "X-Ray Laser");
    TS_ASSERT_EQUALS(c->content[1].id, 2);
}

/** Test listItems(TorpedoPage). */
void
TestGameSpecInfoBrowser::testListTorpedo()
{
    TestHarness h;
    game::test::initPListTorpedoes(h.shipList);
    gsi::Browser testee(h.picNamer, h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);

    // Check 1:
    std::auto_ptr<gsi::ListContent> c(testee.listItems(gsi::TorpedoPage, gsi::Filter(), gsi::Range_Id));
    TS_ASSERT(c.get());
    TS_ASSERT_EQUALS(c->content.size(), 10U);
    TS_ASSERT_EQUALS(c->content[0].name, "Space Rocket");
    TS_ASSERT_EQUALS(c->content[0].id, 1);
    TS_ASSERT_EQUALS(c->content[9].name, "Selphyr-Fataro-Dev.");
    TS_ASSERT_EQUALS(c->content[9].id, 10);

    // Check 2:
    gsi::Filter f;
    f.setNameFilter("bomb on"); // Fusion bomb, Graviton bomb, Arkon bomb
    f.add(gsi::FilterElement(gsi::Range_Tech, 0, gsi::IntRange_t(8, 100)));
    c = testee.listItems(gsi::TorpedoPage, f, gsi::Range_DamagePower);
    TS_ASSERT(c.get());
    TS_ASSERT_EQUALS(c->content.size(), 2U);
    TS_ASSERT_EQUALS(c->content[0].name, "Arkon Bomb");
    TS_ASSERT_EQUALS(c->content[0].id, 7);
    TS_ASSERT_EQUALS(c->content[1].name, "Graviton Bomb");
    TS_ASSERT_EQUALS(c->content[1].id, 6);
}

/** Test describeFilters. */
void
TestGameSpecInfoBrowser::testDescribeFilter()
{
    TestHarness h;

    Player* pl = h.root.playerList().create(3);
    pl->setName(Player::ShortName, "Playboy");

    gsi::Browser testee(h.picNamer, h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);
    gsi::Filter f;
    f.setNameFilter("bork");
    f.add(gsi::FilterElement(gsi::Value_Player, 3, gsi::IntRange_t()));
    f.add(gsi::FilterElement(gsi::Range_Tech, 0, gsi::IntRange_t(2, 5)));
    f.add(gsi::FilterElement(gsi::Range_DamagePower, 0, gsi::IntRange_t(0, 100)));

    // Check existing filters
    std::auto_ptr<gsi::FilterInfos_t> result = testee.describeFilters(gsi::EnginePage, f);
    TS_ASSERT(result.get());
    TS_ASSERT_EQUALS(result->size(), 4U);

    TS_ASSERT_EQUALS(result->at(0).name, "Player");
    TS_ASSERT_EQUALS(result->at(0).value, "Playboy");
    TS_ASSERT_EQUALS(result->at(0).active, false);

    TS_ASSERT_EQUALS(result->at(1).name, "Tech level");
    TS_ASSERT_EQUALS(result->at(1).value, "2...5");
    TS_ASSERT_EQUALS(result->at(1).active, true);

    TS_ASSERT_EQUALS(result->at(2).name, "Damage power");
    TS_ASSERT_EQUALS(result->at(2).value, "up to 100");
    TS_ASSERT_EQUALS(result->at(2).active, false);

    TS_ASSERT_EQUALS(result->at(3).name, "Name");
    TS_ASSERT_EQUALS(result->at(3).value, "bork");
    TS_ASSERT_EQUALS(result->at(3).active, true);

    // Check available filters
    // We're on the engine page; engine has cost attributes.
    // Name and tech filters have been removed because they're on the existing filter.
    std::auto_ptr<gsi::FilterInfos_t> avail = testee.getAvailableFilters(gsi::EnginePage, f);
    TS_ASSERT(avail.get());
    TS_ASSERT( findAttribute(*avail, gsi::Range_CostD));
    TS_ASSERT(!findAttribute(*avail, gsi::Range_Tech));
    TS_ASSERT(!findAttribute(*avail, gsi::String_Name));
    TS_ASSERT(!findAttribute(*avail, gsi::Range_Id));
}

