/**
  *  \file test/game/spec/info/browsertest.cpp
  *  \brief Test for game::spec::info::Browser
  */

#include "game/spec/info/browser.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
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
        afl::base::Ref<game::Root> root;
        game::spec::ShipList shipList;
        afl::string::NullTranslator tx;

        TestHarness()
            : picNamer(), root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0)))),
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
AFL_TEST("game.spec.info.Browser:describeItem:PlayerPage", a)
{
    // Create a player
    TestHarness h;
    Player* pl = h.root->playerList().create(7);
    a.check("01", pl);
    pl->setName(Player::LongName, "The Sevens");
    pl->setName(Player::AdjectiveName, "sevenses");
    pl->setName(Player::EmailAddress, "e@mail.7");

    // Get it
    gsi::Browser testee(h.picNamer, *h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);
    std::auto_ptr<gsi::PageContent> c(testee.describeItem(gsi::PlayerPage, 7, true, 0));

    // Verify
    a.check("11. get", c.get());
    a.checkEqual("12. title", c->title, "The Sevens");
    a.check("13. players", c->players.empty());

    const gsi::Attribute* att = findAttribute(*c, "Adjective");
    a.checkNonNull("21. Adjective", att);
    a.checkEqual("22. value", att->value, "sevenses");

    att = findAttribute(*c, "User name");
    a.checkNull("31. user name", att);
}

/** Test describe(HullPage). */
AFL_TEST("game.spec.info.Browser:describeItem:HullPage", a)
{
    // Create a hull
    TestHarness h;

    const game::Id_t HULL_NR = 9;
    h.shipList.hullAssignments().add(2, 3, HULL_NR);
    h.shipList.hullAssignments().add(5, 9, HULL_NR);
    createHull(h, HULL_NR, "LUDMILLA", 3);

    // Get it
    gsi::Browser testee(h.picNamer, *h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);
    std::auto_ptr<gsi::PageContent> c(testee.describeItem(gsi::HullPage, HULL_NR, true, 0));

    // Verify
    a.check("01. get", c.get());
    a.checkEqual("02. title", c->title, "LUDMILLA");
    a.checkEqual("03. players. players", c->players, PlayerSet_t() + 2 + 5);

    const gsi::Attribute* att = findAttribute(*c, "Engines");
    a.checkNonNull("11. Engines", att);
    a.checkEqual("12. value", att->value, "3");
}

/** Test describe(RacialAbilitiesPage). */
AFL_TEST("game.spec.info.Browser:describeItem:RacialAbilitiesPage", a)
{
    // The default configuration creates a number of configuration abilities
    // (we have not created any hullfunc-based abilities).
    // We need to create players, though, because otherwise all abilities will be dropped.
    TestHarness h;
    h.root->playerList().create(1);
    h.root->playerList().create(2);
    gsi::Browser testee(h.picNamer, *h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);
    std::auto_ptr<gsi::PageContent> c(testee.describeItem(gsi::RacialAbilitiesPage, 0, true, 0));

    // Verify
    a.check("01. get", c.get());
    a.check("02. title", !c->title.empty());
    a.check("03. players", !c->players.empty());

    const gsi::Attribute* att = findAttribute(*c, "Origin");
    a.checkNonNull("11. Origin", att);
    a.checkEqual("12. value", att->value, "Host configuration");
}

/** Test describe(ShipAbilitiesPage). */
AFL_TEST("game.spec.info.Browser:describeItem:ShipAbilitiesPage", a)
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
    p->setName("Firefly");
    p->changeHullFunction(h.shipList.modifiedHullFunctions().getFunctionIdFromHostId(17), PlayerSet_t() + 3 + 4, PlayerSet_t(), true);
    h.shipList.hullAssignments().add(4, 1, HULL_NR);
    h.shipList.hullAssignments().add(5, 1, HULL_NR);

    // Get it
    // This is index-based access, 1=second (hf2)
    gsi::Browser testee(h.picNamer, *h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);
    std::auto_ptr<gsi::PageContent> c(testee.describeItem(gsi::ShipAbilitiesPage, 1, true, 0));

    // Verify
    a.check("01. get", c.get());
    a.checkEqual("02. title", c->title, "Do more stuff");
    a.checkEqual("03. players. players", c->players, PlayerSet_t() + 4);

    const gsi::Attribute* att = findAttribute(*c, "Id");
    a.checkNonNull("11. Id", att);
    a.checkEqual("12. value", att->value, "17");

    att = findAttribute(*c, "Name");
    a.checkNonNull("21. Name", att);
    a.checkEqual("22. value", att->value, "PlayToo");

    att = findAttribute(*c, "Sample hull");
    a.checkNonNull("31. Sample hull", att);
    a.checkEqual("32. value", att->value, "Firefly");
}

/** Test describe(EnginePage). */
AFL_TEST("game.spec.info.Browser:describeItem:EnginePage", a)
{
    // Create an engine
    TestHarness h;
    createEngine(h, 8, "6 litre V8", 3);

    // Get it
    gsi::Browser testee(h.picNamer, *h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);
    std::auto_ptr<gsi::PageContent> c(testee.describeItem(gsi::EnginePage, 8, true, 0));

    // Verify
    a.check("01. get", c.get());
    a.checkEqual("02. title", c->title, "6 litre V8");
    a.checkEqual("03. players. players", c->players, PlayerSet_t());

    const gsi::Attribute* att = findAttribute(*c, "Tech level");
    a.checkNonNull("11. Tech level", att);
    a.checkEqual("12. value", att->value, "3");
}

/** Test describe(BeamPage). */
AFL_TEST("game.spec.info.Browser:describeItem:BeamPage", a)
{
    // Create a beam
    TestHarness h;
    game::test::initStandardBeams(h.shipList);

    // Get it
    gsi::Browser testee(h.picNamer, *h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);
    std::auto_ptr<gsi::PageContent> c(testee.describeItem(gsi::BeamPage, 2, true, 0));

    // Verify
    a.check("01. get", c.get());
    a.checkEqual("02. title", c->title, "X-Ray Laser");
    a.checkEqual("03. players. players", c->players, PlayerSet_t());

    const gsi::Attribute* att = findAttribute(*c, "Kill");
    a.checkNonNull("11. Kill", att);
    a.checkEqual("12. value", att->value, "15");
}

/** Test describe(TorpedoPage). */
AFL_TEST("game.spec.info.Browser:describeItem:TorpedoPage", a)
{
    // Create a beam
    TestHarness h;
    game::test::initStandardTorpedoes(h.shipList);

    // Get it
    gsi::Browser testee(h.picNamer, *h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);
    std::auto_ptr<gsi::PageContent> c(testee.describeItem(gsi::TorpedoPage, 7, true, 0));

    // Verify
    a.check("01. get", c.get());
    a.checkEqual("02. title", c->title, "Mark 5 Photon");
    a.checkEqual("03. players. players", c->players, PlayerSet_t());

    const gsi::Attribute* att = findAttribute(*c, "Kill");
    a.checkNonNull("11. Kill", att);
    a.checkEqual("12. value", att->value, "34");  // note: doubled!
}

/** Test describe(FighterPage). */
AFL_TEST("game.spec.info.Browser:describeItem:FighterPage", a)
{
    // Create a beam
    TestHarness h;
    Player* pl3 = h.root->playerList().create(3);
    pl3->setName(Player::LongName, "The Birds");
    pl3->setName(Player::ShortName, "Birds");
    pl3->setName(Player::AdjectiveName, "Bird");

    // Get it
    gsi::Browser testee(h.picNamer, *h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);
    std::auto_ptr<gsi::PageContent> c(testee.describeItem(gsi::FighterPage, 3, true, 0));

    // Verify
    a.check("01. get", c.get());
    a.checkEqual("02. title", c->title, "Bird fighter");
    a.checkEqual("03. players. players", c->players, PlayerSet_t());

    const gsi::Attribute* att = findAttribute(*c, "Kill");
    a.checkNonNull("11. Kill", att);
    a.checkEqual("12. value", att->value, "2");
}

/** Test listItems(PlayerPage). */
AFL_TEST("game.spec.info.Browser:listItems:PlayerPage", a)
{
    TestHarness h;
    Player* pl1 = h.root->playerList().create(1);
    pl1->setName(Player::LongName, "The Federation");
    pl1->setName(Player::ShortName, "Federation");

    Player* pl2 = h.root->playerList().create(2);
    pl2->setName(Player::LongName, "The Lizards");
    pl2->setName(Player::ShortName, "Lizard");

    Player* pl3 = h.root->playerList().create(3);
    pl3->setName(Player::LongName, "The Birds");
    pl3->setName(Player::ShortName, "Bird");

    gsi::Browser testee(h.picNamer, *h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);

    // Check 1:
    std::auto_ptr<gsi::ListContent> c(testee.listItems(gsi::PlayerPage, gsi::Filter(), gsi::String_Name));
    a.check("01. get", c.get());
    a.checkEqual("02. size",   c->content.size(), 3U);
    a.checkEqual("03. 0.name", c->content[0].name, "Bird");
    a.checkEqual("04. 0.id",   c->content[0].id, 3);
    a.checkEqual("05. 1.name", c->content[1].name, "Federation");
    a.checkEqual("06. 1.id",   c->content[1].id, 1);
    a.checkEqual("07. 2.name", c->content[2].name, "Lizard");
    a.checkEqual("08. 2.id",   c->content[2].id, 2);

    // Check 2: with filter. 'th ds' matches 'The Lizards', 'The Birds'.
    gsi::Filter f;
    f.setNameFilter("th ds");
    c = testee.listItems(gsi::PlayerPage, f, gsi::Range_Id);

    a.check("11. get", c.get());
    a.checkEqual("12. size",   c->content.size(), 2U);
    a.checkEqual("13. 0.name", c->content[0].name, "Lizard");
    a.checkEqual("14. 0.id",   c->content[0].id, 2);
    a.checkEqual("15. 1.name", c->content[1].name, "Bird");
    a.checkEqual("16. 1.id",   c->content[1].id, 3);
}

/** Test listItems(HullPage). */
AFL_TEST("game.spec.info.Browser:listItems:HullPage", a)
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
    gsi::Browser testee(h.picNamer, *h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);

    // Check 1: full list
    std::auto_ptr<gsi::ListContent> c(testee.listItems(gsi::HullPage, gsi::Filter(), gsi::Range_Id));
    a.check("01. get", c.get());
    a.checkEqual("02. size",   c->content.size(), 6U);
    a.checkEqual("03. 0.name", c->content[0].name, "FIRST CLASS CRUISER");
    a.checkEqual("04. 0.id",   c->content[0].id, 1);
    a.checkEqual("05. 5.name", c->content[5].name, "SIXTH CLASS LIGHT CRUISER");
    a.checkEqual("06. 5.id",   c->content[5].id, 6);

    // Check 2: filter name:"light class", numEngines:1, sort by name. Produces [5,3]
    gsi::Filter f;
    f.setNameFilter("light class");
    f.add(gsi::FilterElement(gsi::Range_NumEngines, 0, gsi::IntRange_t::fromValue(1)));
    c = testee.listItems(gsi::HullPage, f, gsi::String_Name);
    a.check("11. get", c.get());
    a.checkEqual("12. size",   c->content.size(), 2U);
    a.checkEqual("13. 0.name", c->content[0].name, "LIGHT FIFTH CLASS CRUISER");
    a.checkEqual("14. 0.id",   c->content[0].id, 5);
    a.checkEqual("15. 1.name", c->content[1].name, "THIRD CLASS LIGHT CRUISER");
    a.checkEqual("16. 1.id",   c->content[1].id, 3);

    // Check 3: filter by player
    gsi::Filter f2;
    f2.add(gsi::FilterElement(gsi::Value_Player, 4, gsi::IntRange_t()));
    c = testee.listItems(gsi::HullPage, f2, gsi::Range_Id);
    a.check("21. get", c.get());
    a.checkEqual("22. size",   c->content.size(), 3U);
    a.checkEqual("23. 0.name", c->content[0].name, "LIGHT FIFTH CLASS CRUISER");
    a.checkEqual("24. 0.id",   c->content[0].id, 5);
    a.checkEqual("25. 1.name", c->content[1].name, "SIXTH CLASS LIGHT CRUISER");
    a.checkEqual("26. 1.id",   c->content[1].id, 6);
    a.checkEqual("27. 2.name", c->content[2].name, "FIRST CLASS CRUISER");
    a.checkEqual("28. 2.id",   c->content[2].id, 1);
}

/** Test listItems(RacialAbilitiesPage). */
AFL_TEST("game.spec.info.Browser:listItems:RacialAbilitiesPage", a)
{
    TestHarness h;
    h.root->playerList().create(1);
    h.root->playerList().create(2);
    gsi::Browser testee(h.picNamer, *h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);

    // Check
    std::auto_ptr<gsi::ListContent> c(testee.listItems(gsi::RacialAbilitiesPage, gsi::Filter(), gsi::Range_Id));
    a.check("01. get", c.get());
    a.checkGreaterThan("02. size", c->content.size(), 0U);
}

/** Test listItems(ShipAbilitiesPage). */
AFL_TEST("game.spec.info.Browser:listItems:ShipAbilitiesPage", a)
{
    TestHarness h;
    createHullFunction(h, 7,  "a", "eat");
    createHullFunction(h, 10, "b", "Drink");
    createHullFunction(h, 3,  "c", "Sleep");
    createHullFunction(h, 9,  "d", "Repeat");
    gsi::Browser testee(h.picNamer, *h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);

    // Check 1:
    std::auto_ptr<gsi::ListContent> c(testee.listItems(gsi::ShipAbilitiesPage, gsi::Filter(), gsi::Range_Id));
    a.check("01. get", c.get());
    a.checkEqual("02. size",   c->content.size(), 4U);
    a.checkEqual("03. 0.name", c->content[0].name, "eat");
    a.checkEqual("04. 0.id",   c->content[0].id, 0);
    a.checkEqual("05. 3.name", c->content[3].name, "Repeat");
    a.checkEqual("06. 3.id",   c->content[3].id, 3);

    // CHeck 2:
    gsi::Filter f;
    f.setNameFilter("e");
    f.add(gsi::FilterElement(gsi::Range_Tech, 0, gsi::IntRange_t(1, 100))); // pointless filter, ignored
    c = testee.listItems(gsi::ShipAbilitiesPage, f, gsi::String_Name);
    a.check("11. get", c.get());
    a.checkEqual("12. size",   c->content.size(), 3U);
    a.checkEqual("13. 0.name", c->content[0].name, "eat");
    a.checkEqual("14. 0.id",   c->content[0].id, 0);
    a.checkEqual("15. 1.name", c->content[1].name, "Repeat");
    a.checkEqual("16. 1.id",   c->content[1].id, 3);
    a.checkEqual("17. 2.name", c->content[2].name, "Sleep");
    a.checkEqual("18. 2.id",   c->content[2].id, 2);
}

/** Test listItems(EnginePage). */
AFL_TEST("game.spec.info.Browser:listItems:EnginePage", a)
{
    TestHarness h;
    createEngine(h, 2, "Two-speed", 3);
    createEngine(h, 3, "Three-speed", 4);
    createEngine(h, 4, "Four-speed", 5);
    gsi::Browser testee(h.picNamer, *h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);

    // Check 1:
    std::auto_ptr<gsi::ListContent> c(testee.listItems(gsi::EnginePage, gsi::Filter(), gsi::Range_Id));
    a.check("01. get", c.get());
    a.checkEqual("02. size",   c->content.size(), 3U);
    a.checkEqual("03. 0.name", c->content[0].name, "Two-speed");
    a.checkEqual("04. 0.id",   c->content[0].id, 2);
    a.checkEqual("05. 2.name", c->content[2].name, "Four-speed");
    a.checkEqual("06. 2.id",   c->content[2].id, 4);

    // Check 2:
    gsi::Filter f;
    f.setNameFilter("o");
    f.add(gsi::FilterElement(gsi::Range_Tech, 0, gsi::IntRange_t(4, 100)));
    c = testee.listItems(gsi::EnginePage, f, gsi::Range_Id);
    a.check("11. get", c.get());
    a.checkEqual("12. size",   c->content.size(), 1U);
    a.checkEqual("13. 0.name", c->content[0].name, "Four-speed");
    a.checkEqual("14. 0.id",   c->content[0].id, 4);
}

/** Test listItems(BeamPage). */
AFL_TEST("game.spec.info.Browser:listItems:BeamPage", a)
{
    TestHarness h;
    game::test::initStandardBeams(h.shipList);
    gsi::Browser testee(h.picNamer, *h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);

    // Check 1:
    std::auto_ptr<gsi::ListContent> c(testee.listItems(gsi::BeamPage, gsi::Filter(), gsi::Range_Id));
    a.check("01. get", c.get());
    a.checkEqual("02. size",   c->content.size(), 10U);
    a.checkEqual("03. 0.name", c->content[0].name, "Laser");
    a.checkEqual("04. 0.id",   c->content[0].id, 1);
    a.checkEqual("05. 9.name", c->content[9].name, "Heavy Phaser");
    a.checkEqual("06. 9.id",   c->content[9].id, 10);

    // Check 2:
    gsi::Filter f;
    f.setNameFilter("aser");
    f.add(gsi::FilterElement(gsi::Range_KillPower, 0, gsi::IntRange_t(15, 30)));
    c = testee.listItems(gsi::BeamPage, f, gsi::String_Name);
    a.check("11. get", c.get());
    a.checkEqual("12. size",   c->content.size(), 2U);
    a.checkEqual("13. 0.name", c->content[0].name, "Phaser");
    a.checkEqual("14. 0.id",   c->content[0].id, 8);
    a.checkEqual("15. 1.name", c->content[1].name, "X-Ray Laser");
    a.checkEqual("16. 1.id",   c->content[1].id, 2);
}

/** Test listItems(TorpedoPage). */
AFL_TEST("game.spec.info.Browser:listItems:TorpedoPage", a)
{
    TestHarness h;
    game::test::initPListTorpedoes(h.shipList);
    gsi::Browser testee(h.picNamer, *h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);

    // Check 1:
    std::auto_ptr<gsi::ListContent> c(testee.listItems(gsi::TorpedoPage, gsi::Filter(), gsi::Range_Id));
    a.check("01. get", c.get());
    a.checkEqual("02. size",   c->content.size(), 10U);
    a.checkEqual("03. 0.name", c->content[0].name, "Space Rocket");
    a.checkEqual("04. 0.id",   c->content[0].id, 1);
    a.checkEqual("05. 9.name", c->content[9].name, "Selphyr-Fataro-Dev.");
    a.checkEqual("06. 9.id",   c->content[9].id, 10);

    // Check 2:
    gsi::Filter f;
    f.setNameFilter("bomb on"); // Fusion bomb, Graviton bomb, Arkon bomb
    f.add(gsi::FilterElement(gsi::Range_Tech, 0, gsi::IntRange_t(8, 100)));
    c = testee.listItems(gsi::TorpedoPage, f, gsi::Range_DamagePower);
    a.check("11. get", c.get());
    a.checkEqual("12. size",   c->content.size(), 2U);
    a.checkEqual("13. 0.name", c->content[0].name, "Arkon Bomb");
    a.checkEqual("14. 0.id",   c->content[0].id, 7);
    a.checkEqual("15. 1.name", c->content[1].name, "Graviton Bomb");
    a.checkEqual("16. 1.id",   c->content[1].id, 6);
}

/** Test listItems(FighterPage). */
AFL_TEST("game.spec.info.Browser:listItems:FighterPage", a)
{
    TestHarness h;
    Player* pl1 = h.root->playerList().create(1);
    pl1->setName(Player::LongName, "The Federation");
    pl1->setName(Player::ShortName, "Federation");
    pl1->setName(Player::AdjectiveName, "Fed");

    Player* pl2 = h.root->playerList().create(2);
    pl2->setName(Player::LongName, "The Lizards");
    pl2->setName(Player::ShortName, "Lizard");
    pl2->setName(Player::AdjectiveName, "Liz");

    h.root->hostConfiguration().setOption("FighterBeamKill", "5,3,2,2,2", game::config::ConfigurationOption::Game);

    gsi::Browser testee(h.picNamer, *h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);

    // Check 1:
    std::auto_ptr<gsi::ListContent> c(testee.listItems(gsi::FighterPage, gsi::Filter(), gsi::Range_Id));
    a.check("01. get", c.get());
    a.checkEqual("02. size",   c->content.size(), 2U);
    a.checkEqual("03. 0.name", c->content[0].name, "Fed fighter");
    a.checkEqual("04. 0.id",   c->content[0].id, 1);
    a.checkEqual("05. 1.name", c->content[1].name, "Liz fighter");
    a.checkEqual("06. 1.id",   c->content[1].id, 2);

    // Check 2: filter by name
    gsi::Filter f;
    f.setNameFilter("z"); // Liz
    c = testee.listItems(gsi::FighterPage, f, gsi::Range_Id);
    a.check("11. get", c.get());
    a.checkEqual("12. size",   c->content.size(), 1U);
    a.checkEqual("13. 0.name", c->content[0].name, "Liz fighter");
    a.checkEqual("14. 0.id",   c->content[0].id, 2);

    // Check 3: filter by property
    gsi::Filter f2;
    f2.add(gsi::FilterElement(gsi::Range_KillPower, 0, gsi::IntRange_t(4, 6)));
    c = testee.listItems(gsi::FighterPage, f2, gsi::Range_Id);
    a.check("21. get", c.get());
    a.checkEqual("22. size",   c->content.size(), 1U);
    a.checkEqual("23. 0.name", c->content[0].name, "Fed fighter");
    a.checkEqual("24. 0.id",   c->content[0].id, 1);
}

/** Test describeFilters. */
AFL_TEST("game.spec.info.Browser:describeFilters", a)
{
    TestHarness h;

    Player* pl = h.root->playerList().create(3);
    pl->setName(Player::ShortName, "Playboy");

    gsi::Browser testee(h.picNamer, *h.root, h.shipList, VIEWPOINT_PLAYER, h.tx);
    gsi::Filter f;
    f.setNameFilter("bork");
    f.add(gsi::FilterElement(gsi::Value_Player, 3, gsi::IntRange_t()));
    f.add(gsi::FilterElement(gsi::Range_Tech, 0, gsi::IntRange_t(2, 5)));
    f.add(gsi::FilterElement(gsi::Range_DamagePower, 0, gsi::IntRange_t(0, 100)));

    // Check existing filters
    std::auto_ptr<gsi::FilterInfos_t> result = testee.describeFilters(gsi::EnginePage, f);
    a.check("01. get", result.get());
    a.checkEqual("02. size", result->size(), 4U);

    a.checkEqual("11. name",   result->at(0).name, "Player");
    a.checkEqual("12. value",  result->at(0).value, "Playboy");
    a.checkEqual("13. active", result->at(0).active, false);

    a.checkEqual("21. name",   result->at(1).name, "Tech level");
    a.checkEqual("22. value",  result->at(1).value, "2 to 5");
    a.checkEqual("23. active", result->at(1).active, true);

    a.checkEqual("31. name",   result->at(2).name, "Damage power");
    a.checkEqual("32. value",  result->at(2).value, "up to 100");
    a.checkEqual("33. active", result->at(2).active, false);

    a.checkEqual("41. name",   result->at(3).name, "Name");
    a.checkEqual("42. value",  result->at(3).value, "bork");
    a.checkEqual("43. active", result->at(3).active, true);

    // Check available filters
    // We're on the engine page; engine has cost attributes.
    // Name and tech filters have been removed because they're on the existing filter.
    std::auto_ptr<gsi::FilterInfos_t> avail = testee.getAvailableFilters(gsi::EnginePage, f);
    a.check("51. get", avail.get());
    a.check("52. Range_CostD",  findAttribute(*avail, gsi::Range_CostD));
    a.check("53. Range_Tech",  !findAttribute(*avail, gsi::Range_Tech));
    a.check("54. String_Name", !findAttribute(*avail, gsi::String_Name));
    a.check("55. Range_Id",    !findAttribute(*avail, gsi::Range_Id));
}
