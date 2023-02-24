/**
  *  \file u/t_game_actions_buildammo.cpp
  *  \brief Test for game::actions::BuildAmmo
  */

#include "game/actions/buildammo.hpp"

#include "t_game_actions.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/exception.hpp"
#include "game/map/configuration.hpp"
#include "game/map/planet.hpp"
#include "game/map/planetstorage.hpp"
#include "game/map/reverter.hpp"
#include "game/map/universe.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/cargocontainer.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/specificationloader.hpp"
#include "game/test/stringverifier.hpp"

using game::Element;

namespace {
    const int X = 1234;
    const int Y = 2345;
    const int OWNER = 4;
    const int TURN_NR = 12;
    const int PLANET_ID = 363;

    class TestReverter : public game::map::Reverter {
     public:
        virtual afl::base::Optional<int> getMinBuildings(int /*planetId*/, game::PlanetaryBuilding /*building*/) const
            { return 0; }
        virtual int getSuppliesAllowedToBuy(int /*planetId*/) const
            { return 0; }
        virtual afl::base::Optional<int> getMinTechLevel(int /*planetId*/, game::TechLevel /*techLevel*/) const
            { return 0; }
        virtual afl::base::Optional<int> getMinBaseStorage(int /*planetId*/, game::TechLevel /*area*/, int /*slot*/) const
            { return 0; }
        virtual int getNumTorpedoesAllowedToSell(int /*planetId*/, int /*slot*/) const
            { return 5; }
        virtual int getNumFightersAllowedToSell(int /*planetId*/) const
            { return 7; }
        virtual afl::base::Optional<String_t> getPreviousShipFriendlyCode(game::Id_t /*shipId*/) const
            { return afl::base::Nothing; }
        virtual afl::base::Optional<String_t> getPreviousPlanetFriendlyCode(game::Id_t /*planetId*/) const
            { return afl::base::Nothing; }
        virtual bool getPreviousShipMission(int /*shipId*/, int& /*m*/, int& /*i*/, int& /*t*/) const
            { return false; }
        virtual bool getPreviousShipBuildOrder(int /*planetId*/, game::ShipBuildOrder& /*result*/) const
            { return false; }
        virtual game::map::LocationReverter* createLocationReverter(game::map::Point /*pt*/) const
            { return 0; }
    };

    struct TestHarness {
        game::map::Universe univ;
        game::map::Planet& planet;
        afl::base::Ref<game::spec::ShipList> shipList;
        afl::base::Ref<game::Root> root;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session;
        game::config::HostConfiguration& config;

        TestHarness()
            : univ(),
              planet(*univ.planets().create(PLANET_ID)),
              shipList(*new game::spec::ShipList()),
              root(*new game::Root(afl::io::InternalDirectory::create("game dir"),
                                   *new game::test::SpecificationLoader(),
                                   game::HostVersion(game::HostVersion::Host, MKVERSION(3, 22, 47)),
                                   std::auto_ptr<game::RegistrationKey>(new game::test::RegistrationKey(game::RegistrationKey::Unregistered, 5)),
                                   std::auto_ptr<game::StringVerifier>(new game::test::StringVerifier()),
                                   std::auto_ptr<afl::charset::Charset>(new afl::charset::Utf8Charset()),
                                   game::Root::Actions_t())),
              tx(),
              fs(),
              session(tx, fs),
              config(root->hostConfiguration())
            {
                root->hostConfiguration().setDefaultValues();
                session.setShipList(shipList.asPtr());
                session.setRoot(root.asPtr());
            }
    };

    void preparePlanet(game::map::Planet& pl, int x, int y, int owner)
    {
        // Define base storage. This is the only way to reserve memory for base storage.
        // Planet::setBaseStorage only accesses present slots and never creates new ones.
        game::map::BaseData bd;
        for (int i = 0; i < 20; ++i) {
            bd.launcherStorage.set(i, 2);
            bd.torpedoStorage.set(i, 2);
        }
        bd.numFighters = 0;
        for (size_t i = 0; i < game::NUM_TECH_AREAS; ++i) {
            bd.techLevels[i] = 1;
        }

        afl::sys::Log log;
        afl::string::NullTranslator tx;

        // Define planet with base
        pl.setPosition(game::map::Point(x, y));
        pl.addCurrentPlanetData(game::map::PlanetData(), game::PlayerSet_t(owner));
        pl.addCurrentBaseData(bd, game::PlayerSet_t(owner));
        pl.setOwner(owner);
        pl.setBaseTechLevel(game::HullTech, 1);
        pl.setBaseTechLevel(game::EngineTech, 1);
        pl.setBaseTechLevel(game::BeamTech, 1);
        pl.setBaseTechLevel(game::TorpedoTech, 1);
        pl.setCargo(Element::Money, 600);
        pl.setCargo(Element::Supplies, 100);
        pl.setCargo(Element::Tritanium, 1000);
        pl.setCargo(Element::Duranium, 1000);
        pl.setCargo(Element::Molybdenum, 1000);
        pl.internalCheck(game::map::Configuration(), game::PlayerSet_t(owner), TURN_NR, tx, log);
        pl.setPlayability(game::map::Object::Playable);
    }

    void prepareShip(game::map::Ship& sh, int x, int y, int owner)
    {
        // Seed the ship to make it visible
        game::map::ShipData sd;
        sd.x = x;
        sd.y = y;
        sd.owner = owner;
        sh.addCurrentShipData(sd, game::PlayerSet_t(owner));
        sh.internalCheck(game::PlayerSet_t(owner), TURN_NR);
        sh.setPlayability(game::map::Object::Playable);

        sh.setNumLaunchers(3);
        sh.setTorpedoType(7);
        sh.setAmmo(77);
    }

    void prepare(TestHarness& h)
    {
        preparePlanet(h.planet, X, Y, OWNER);

        // Define torpedoes
        for (int i = 1; i <= 10; ++i) {
            game::spec::TorpedoLauncher* tl = h.shipList->launchers().create(i);
            tl->setTechLevel(i);
            tl->cost()        = game::spec::Cost::fromString("2M 10S") * i;
            tl->torpedoCost() = game::spec::Cost::fromString("1TM 2$") * i;  // note no duranium!
        }
    }
}

/** Test failure.
    If the planet has no base, constructing the action must fail. */
void
TestGameActionsBuildAmmo::testFail()
{
    TestHarness h;
    afl::sys::Log log;

    // Define planet without base
    h.planet.setPosition(game::map::Point(X, Y));
    h.planet.addCurrentPlanetData(game::map::PlanetData(), game::PlayerSet_t(7));
    h.planet.setOwner(7);
    h.planet.internalCheck(game::map::Configuration(), game::PlayerSet_t(7), 12, h.tx, log);
    h.planet.setPlayability(game::map::Object::Playable);

    game::test::CargoContainer container;
    TS_ASSERT_THROWS((game::actions::BuildAmmo(h.planet, container, container, *h.shipList, *h.root)), game::Exception);
}

/** Test success case.
    Exercise a normal action which must work.  */
void
TestGameActionsBuildAmmo::testSuccess()
{
    TestHarness h;
    prepare(h);

    game::map::PlanetStorage container(h.planet, h.config);
    game::actions::BuildAmmo a(h.planet, container, container, *h.shipList, *h.root);

    // Add 5 type-1 torps
    TS_ASSERT_EQUALS(a.getAmount(Element::fromTorpedoType(1)), 2);
    TS_ASSERT_EQUALS(a.add(Element::fromTorpedoType(1), 5, false), 5);
    TS_ASSERT_EQUALS(a.costAction().getCost().toCargoSpecString(), "5T 5M 10$");
    TS_ASSERT_EQUALS(a.getAmount(Element::fromTorpedoType(1)), 7);

    // Add 5 type-3 torps. This will add two tech levels
    TS_ASSERT_EQUALS(a.getAmount(Element::fromTorpedoType(3)), 2);
    TS_ASSERT_EQUALS(a.add(Element::fromTorpedoType(3), 5, false), 5);
    TS_ASSERT_EQUALS(a.costAction().getCost().toCargoSpecString(), "20T 20M 340$");
    TS_ASSERT_EQUALS(a.getAmount(Element::fromTorpedoType(3)), 7);

    // Add a fighter
    TS_ASSERT_EQUALS(a.getAmount(Element::Fighters), 0);
    TS_ASSERT_EQUALS(a.add(Element::Fighters, 1, false), 1);
    TS_ASSERT_EQUALS(a.costAction().getCost().toCargoSpecString(), "23T 22M 440$");
    TS_ASSERT_EQUALS(a.getAmount(Element::Fighters), 1);

    // Transaction validity
    TS_ASSERT(a.isValid());
    TS_ASSERT_EQUALS(a.getStatus(), a.Success);

    // Commit
    TS_ASSERT_THROWS_NOTHING(a.commit());
    TS_ASSERT_EQUALS(h.planet.getBaseTechLevel(game::TorpedoTech).orElse(1), 3);
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::Fighters).orElse(0),           1);   // was 1 before action
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::fromTorpedoType(1)).orElse(0), 7);   // was 2 before action
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::fromTorpedoType(2)).orElse(0), 2);   // unchanged
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::fromTorpedoType(3)).orElse(0), 7);   // was 2 before action
}

/** Test limitation by capacity.
    Adding must limit according to maximum capacity of target. */
void
TestGameActionsBuildAmmo::testLimitCapacity()
{
    TestHarness h;
    prepare(h);

    // Make fighters cheap; place 5 fighters on base
    h.config[game::config::HostConfiguration::BaseFighterCost].set("1TDM 1$");
    h.planet.setCargo(Element::Fighters, 5);

    // Do it: full add won't work, partial add will
    game::map::PlanetStorage container(h.planet, h.config);
    game::actions::BuildAmmo a(h.planet, container, container, *h.shipList, *h.root);
    TS_ASSERT_EQUALS(a.add(Element::Fighters, 100, false), 0);
    TS_ASSERT_EQUALS(a.add(Element::Fighters, 100, true), 55);
    TS_ASSERT_EQUALS(a.getAmount(Element::Fighters), 60);
}

/** Test limitation by resources.
    addLimitCash must limit according to available resources. */
void
TestGameActionsBuildAmmo::testLimitResource()
{
    TestHarness h;
    prepare(h);
    h.planet.setCargo(Element::Money, 610);
    h.planet.setCargo(Element::Supplies, 110);
    h.planet.setCargo(Element::Fighters, 10);

    // Attempt to add 1000 fighters: since we have 720$, we must end up with 7 (and 20S remaining).
    game::map::PlanetStorage container(h.planet, h.config);
    game::actions::BuildAmmo a(h.planet, container, container, *h.shipList, *h.root);
    TS_ASSERT_EQUALS(a.addLimitCash(Element::Fighters, 1000), 7);

    // Try to build 1000 more, must fail
    TS_ASSERT_EQUALS(a.addLimitCash(Element::Fighters, 1000), 0);

    // Verify result
    TS_ASSERT_THROWS_NOTHING(a.commit());
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::Money).orElse(-1), 0);
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::Supplies).orElse(-1), 20);
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::Fighters).orElse(-1), 17);
}

/** Test limitation by resource, key limit.
    addLimitCash/add must not add things that we can pay if we don't have the key for it. */
void
TestGameActionsBuildAmmo::testLimitKey()
{
    TestHarness h;
    prepare(h);
    h.planet.setCargo(Element::Money, 100000);
    h.planet.setCargo(Element::Supplies, 100000);

    // Attempt to add tech 10 torps, which our key disallows
    game::map::PlanetStorage container(h.planet, h.config);
    game::actions::BuildAmmo a(h.planet, container, container, *h.shipList, *h.root);
    TS_ASSERT_EQUALS(a.addLimitCash(Element::fromTorpedoType(10), 1000), 0);
    TS_ASSERT_EQUALS(a.add(Element::fromTorpedoType(10), 1, false), 0);
    TS_ASSERT(a.isValid());
    TS_ASSERT(a.costAction().getCost().isZero());
}

/** Test non-limitation by key limit.
    addLimitCash/add must work if the planet has sufficient tech, even if the key does not allow. */
void
TestGameActionsBuildAmmo::testNoLimitKey()
{
    TestHarness h;
    prepare(h);
    h.planet.setCargo(Element::Money, 100000);
    h.planet.setCargo(Element::Supplies, 100000);
    h.planet.setBaseTechLevel(game::TorpedoTech, 10);

    // Attempt to add tech 10 torps, which our key disallows
    game::map::PlanetStorage container(h.planet, h.config);
    game::actions::BuildAmmo a(h.planet, container, container, *h.shipList, *h.root);
    TS_ASSERT_EQUALS(a.addLimitCash(Element::fromTorpedoType(10), 20), 20);
    TS_ASSERT_EQUALS(a.add(Element::fromTorpedoType(10), 1, false), 1);
    TS_ASSERT(a.isValid());
    TS_ASSERT(!a.costAction().getCost().isZero());
}

/** Test limitation by resource, key limit.
    Key limits must be enforced even if things happen behind our back. */
void
TestGameActionsBuildAmmo::testLimitKeyDowngrade()
{
    TestHarness h;
    prepare(h);
    h.planet.setCargo(Element::Money, 100000);
    h.planet.setCargo(Element::Supplies, 100000);
    h.planet.setBaseTechLevel(game::TorpedoTech, 10);

    // Attempt to add tech 10 torps
    game::map::PlanetStorage container(h.planet, h.config);
    game::actions::BuildAmmo a(h.planet, container, container, *h.shipList, *h.root);
    TS_ASSERT_EQUALS(a.addLimitCash(Element::fromTorpedoType(10), 1), 1);
    TS_ASSERT(a.isValid());
    TS_ASSERT_EQUALS(container.getChange(Element::Money), -20);
    TS_ASSERT_EQUALS(container.getChange(Element::fromTorpedoType(10)), 1);
    TS_ASSERT_EQUALS(a.costAction().getCost().toCargoSpecString(), "10T 10M 20$");

    // Change tech level behind our back
    h.planet.setBaseTechLevel(game::TorpedoTech, 1);
    h.univ.notifyListeners();

    // Listener notification must immediately update things
    TS_ASSERT_EQUALS(container.getChange(Element::Money), -4520);
    TS_ASSERT_EQUALS(container.getChange(Element::fromTorpedoType(10)), 1);
    TS_ASSERT_EQUALS(a.costAction().getCost().toCargoSpecString(), "10T 10M 4520$");

    TS_ASSERT(!a.isValid());
    TS_ASSERT_EQUALS(a.getStatus(), a.DisallowedTech);
    TS_ASSERT_THROWS(a.commit(), game::Exception);
}

/** Test limitation by resource, key limit.
    Key limits must be enforced even if no listener notification is called. */
void
TestGameActionsBuildAmmo::testLimitKeyDowngradeNoListener()
{
    TestHarness h;
    prepare(h);
    h.planet.setCargo(Element::Money, 100000);
    h.planet.setCargo(Element::Supplies, 100000);
    h.planet.setBaseTechLevel(game::TorpedoTech, 10);

    // Attempt to add tech 10 torps
    game::map::PlanetStorage container(h.planet, h.config);
    game::actions::BuildAmmo a(h.planet, container, container, *h.shipList, *h.root);
    TS_ASSERT_EQUALS(a.addLimitCash(Element::fromTorpedoType(10), 1), 1);
    TS_ASSERT(a.isValid());

    // Change tech level behind our back
    h.planet.setBaseTechLevel(game::TorpedoTech, 1);

    // Still fails
    TS_ASSERT(!a.isValid());
    TS_ASSERT_EQUALS(a.getStatus(), a.DisallowedTech);
    TS_ASSERT_THROWS(a.commit(), game::Exception);
}

/** Test limitation by tech cost.
    If the new transaction implies a tech cost, this must be honored in computing the target amount. */
void
TestGameActionsBuildAmmo::testLimitTechCost()
{
    TestHarness h;
    prepare(h);
    h.planet.setCargo(Element::Money, 70);
    h.planet.setCargo(Element::Supplies, 130);

    // Attempt to add tech 3 torps. The upgrade costs 300, but we only have 200.
    game::map::PlanetStorage container(h.planet, h.config);
    game::actions::BuildAmmo a(h.planet, container, container, *h.shipList, *h.root);
    TS_ASSERT_EQUALS(a.addLimitCash(Element::fromTorpedoType(3), 1000), 0);
    TS_ASSERT(a.isValid());
    TS_ASSERT_EQUALS(a.getAmount(Element::fromTorpedoType(3)), 2);

    // Tech 2 torps: Tech upgrade costs 100, so we have 100 more to spend on torps. Each torp costs 4.
    TS_ASSERT_EQUALS(a.addLimitCash(Element::fromTorpedoType(2), 1000), 25);
    TS_ASSERT(a.isValid());
    TS_ASSERT_EQUALS(a.getAmount(Element::fromTorpedoType(2)), 27);  // 2 present before
    TS_ASSERT_EQUALS(a.costAction().getCost().toCargoSpecString(), "50T 50M 200$");

    // Commit and verify
    TS_ASSERT_THROWS_NOTHING(a.commit());
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::fromTorpedoType(2)).orElse(-1), 27);
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::fromTorpedoType(3)).orElse(-1), 2);
    TS_ASSERT_EQUALS(h.planet.getBaseTechLevel(game::TorpedoTech).orElse(0), 2);
}

/** Test supply sale without a reverter.
    Revert must allow undoing the current built amount. */
void
TestGameActionsBuildAmmo::testSellNoReverter()
{
    TestHarness h;
    prepare(h);
    game::map::PlanetStorage container(h.planet, h.config);
    game::actions::BuildAmmo a(h.planet, container, container, *h.shipList, *h.root);

    // Query ranges
    TS_ASSERT_EQUALS(a.getMinAmount(Element::fromTorpedoType(1)), 2);
    TS_ASSERT_EQUALS(a.getAmount(Element::fromTorpedoType(1)), 2);

    TS_ASSERT_EQUALS(a.getMinAmount(Element::Fighters), 0);
    TS_ASSERT_EQUALS(a.getAmount(Element::Fighters), 0);
    TS_ASSERT_EQUALS(a.getMaxAmount(Element::Fighters), 60);

    // Add 10 torpedoes
    TS_ASSERT_EQUALS(a.add(Element::fromTorpedoType(1), 10, false), 10);
    TS_ASSERT_EQUALS(a.getMinAmount(Element::fromTorpedoType(1)), 2);
    TS_ASSERT_EQUALS(a.getAmount(Element::fromTorpedoType(1)), 12);
    TS_ASSERT(a.getMaxAmount(Element::fromTorpedoType(1)) >= 10000);

    // Remove, failure
    TS_ASSERT_EQUALS(a.add(Element::fromTorpedoType(1), -30, false), 0);

    // Remove, success
    TS_ASSERT_EQUALS(a.add(Element::fromTorpedoType(1), -30, true), -10);

    // Same thing, using addLimitCash
    TS_ASSERT_EQUALS(a.addLimitCash(Element::fromTorpedoType(1), 10), 10);
    TS_ASSERT_EQUALS(a.addLimitCash(Element::fromTorpedoType(1), -30), -10);
}

/** Test supply sale with a reverter.
    Revert must allow undoing the current built amount plus what the reverter says. */
void
TestGameActionsBuildAmmo::testSellReverter()
{
    TestHarness h;
    prepare(h);
    h.univ.setNewReverter(new TestReverter());

    game::map::PlanetStorage container(h.planet, h.config);
    game::actions::BuildAmmo a(h.planet, container, container, *h.shipList, *h.root);

    // Query ranges (initial)
    TS_ASSERT_EQUALS(a.getMinAmount(Element::fromTorpedoType(1)), 2);
    TS_ASSERT_EQUALS(a.getMinAmount(Element::Fighters), 0);

    // Configure undo and query ranges again
    a.setUndoInformation(h.univ);
    TS_ASSERT_EQUALS(a.getMinAmount(Element::fromTorpedoType(1)), 0);
    TS_ASSERT_EQUALS(a.getMinAmount(Element::Fighters), 0);

    // Add stuff to the planet
    h.planet.setCargo(Element::fromTorpedoType(1), 30);
    h.planet.setCargo(Element::Fighters, 20);

    // Query ranges
    TS_ASSERT_EQUALS(a.getMinAmount(Element::fromTorpedoType(1)), 25);
    TS_ASSERT_EQUALS(a.getMinAmount(Element::Fighters), 13);

    // Exercise limits
    TS_ASSERT_EQUALS(a.add(Element::fromTorpedoType(1), -100, true), -5);
    TS_ASSERT_EQUALS(a.add(Element::Fighters, -100, true), -7);

    // Cost must represent that we're saving money
    TS_ASSERT_EQUALS(a.costAction().getCost().toCargoSpecString(), "-26T -19M -710$");
}

/** Test invalid types.
    Element types other than torpedoes/fighters must be immediately rejected. */
void
TestGameActionsBuildAmmo::testInvalidTypes()
{
    TestHarness h;
    prepare(h);
    game::map::PlanetStorage container(h.planet, h.config);
    game::actions::BuildAmmo a(h.planet, container, container, *h.shipList, *h.root);

    // We can query ranges
    TS_ASSERT_EQUALS(a.getMinAmount(Element::Tritanium), 1000);
    TS_ASSERT_EQUALS(a.getAmount(Element::Tritanium), 1000);

    // We cannot modify
    TS_ASSERT_EQUALS(a.add(Element::Tritanium, 1000, false), 0);
    TS_ASSERT_EQUALS(a.add(Element::Tritanium, 1000, true), 0);
    TS_ASSERT_EQUALS(a.add(Element::Tritanium, -1000, true), 0);
    TS_ASSERT_EQUALS(a.addLimitCash(Element::Tritanium, 1000), 0);
}

/** Test simple operation with different containers.
    This is the same as testSuccess(), but using two containers as financier and receiver.
    It must work equally well. */
void
TestGameActionsBuildAmmo::testDifferentContainers()
{
    TestHarness h;
    prepare(h);

    game::map::PlanetStorage financier(h.planet, h.config);
    game::map::PlanetStorage receiver(h.planet, h.config);
    game::actions::BuildAmmo a(h.planet, financier, receiver, *h.shipList, *h.root);

    // Add
    TS_ASSERT_EQUALS(a.add(Element::fromTorpedoType(1), 5, false), 5);
    TS_ASSERT_EQUALS(a.add(Element::fromTorpedoType(3), 5, false), 5);
    TS_ASSERT_EQUALS(a.add(Element::Fighters, 1, false), 1);
    TS_ASSERT_EQUALS(a.costAction().getCost().toCargoSpecString(), "23T 22M 440$");

    // Transaction validity
    TS_ASSERT(a.isValid());
    TS_ASSERT_EQUALS(a.getStatus(), a.Success);

    // Commit
    TS_ASSERT_THROWS_NOTHING(a.commit());
    TS_ASSERT_EQUALS(h.planet.getBaseTechLevel(game::TorpedoTech).orElse(1), 3);
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::Fighters).orElse(0),           1);   // was 1 before action
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::fromTorpedoType(1)).orElse(0), 7);   // was 2 before action
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::fromTorpedoType(2)).orElse(0), 2);   // unchanged
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::fromTorpedoType(3)).orElse(0), 7);   // was 2 before action
}

/** Test isValidCombination(). */
void
TestGameActionsBuildAmmo::testIsValidCombination()
{
    // Create some planets:
    game::map::Universe univ;

    // - my planet (base case)
    game::map::Planet& myPlanet = *univ.planets().create(100);
    preparePlanet(myPlanet, X, Y, OWNER);

    // - their planet
    game::map::Planet& theirPlanet = *univ.planets().create(200);
    preparePlanet(theirPlanet, X, Y, OWNER+1);

    // - far planet
    game::map::Planet& farPlanet = *univ.planets().create(300);
    preparePlanet(farPlanet, X+10, Y, OWNER);

    // - unplayed planet
    game::map::Planet& unPlanet = *univ.planets().create(400);
    preparePlanet(unPlanet, X, Y, OWNER);
    unPlanet.setPlayability(game::map::Object::NotPlayable);

    // Create own ship and check against all planets
    game::map::Ship& myShip = *univ.ships().create(1);
    prepareShip(myShip, X, Y, OWNER);
    {
        game::Exception ex("");
        TS_ASSERT_EQUALS(game::actions::BuildAmmo::isValidCombination(myPlanet, myShip, ex), true);
    }
    {
        game::Exception ex("");
        TS_ASSERT_EQUALS(game::actions::BuildAmmo::isValidCombination(theirPlanet, myShip, ex), false);
        TS_ASSERT_DIFFERS(ex.what(), String_t());
    }
    {
        game::Exception ex("");
        TS_ASSERT_EQUALS(game::actions::BuildAmmo::isValidCombination(farPlanet, myShip, ex), false);
        TS_ASSERT_DIFFERS(ex.what(), String_t());
    }
    {
        game::Exception ex("");
        TS_ASSERT_EQUALS(game::actions::BuildAmmo::isValidCombination(unPlanet, myShip, ex), false);
        TS_ASSERT_DIFFERS(ex.what(), String_t());
    }

    // Create unplayed ship and check against all planets
    game::map::Ship& theirShip = *univ.ships().create(2);
    prepareShip(theirShip, X, Y, OWNER);
    theirShip.setPlayability(game::map::Object::NotPlayable);
    {
        game::Exception ex("");
        TS_ASSERT_EQUALS(game::actions::BuildAmmo::isValidCombination(myPlanet, theirShip, ex), false);
        TS_ASSERT_DIFFERS(ex.what(), String_t());
    }
    {
        game::Exception ex("");
        TS_ASSERT_EQUALS(game::actions::BuildAmmo::isValidCombination(theirPlanet, theirShip, ex), false);
        TS_ASSERT_DIFFERS(ex.what(), String_t());
    }
    {
        game::Exception ex("");
        TS_ASSERT_EQUALS(game::actions::BuildAmmo::isValidCombination(farPlanet, theirShip, ex), false);
        TS_ASSERT_DIFFERS(ex.what(), String_t());
    }
    {
        game::Exception ex("");
        TS_ASSERT_EQUALS(game::actions::BuildAmmo::isValidCombination(unPlanet, theirShip, ex), false);
        TS_ASSERT_DIFFERS(ex.what(), String_t());
    }
}

/** Test isValidCombination(), varying ship equipment. */
void
TestGameActionsBuildAmmo::testIsValidCombination2()
{
    // Create some planets:
    game::map::Universe univ;

    // - my planet (base case)
    game::map::Planet& myPlanet = *univ.planets().create(100);
    preparePlanet(myPlanet, X, Y, OWNER);

    // - torpedo ship
    game::map::Ship& torpShip = *univ.ships().create(1);
    prepareShip(torpShip, X, Y, OWNER);
    torpShip.setTorpedoType(3);
    torpShip.setNumLaunchers(2);
    torpShip.setNumBays(0);

    game::map::Ship& fighterShip = *univ.ships().create(2);
    prepareShip(fighterShip, X, Y, OWNER);
    fighterShip.setTorpedoType(0);
    fighterShip.setNumLaunchers(0);
    fighterShip.setNumBays(5);

    game::map::Ship& freightShip = *univ.ships().create(3);
    prepareShip(freightShip, X, Y, OWNER);
    freightShip.setTorpedoType(0);
    freightShip.setNumLaunchers(0);
    freightShip.setNumBays(0);

    // Compare
    {
        game::Exception ex("");
        TS_ASSERT_EQUALS(game::actions::BuildAmmo::isValidCombination(myPlanet, torpShip, ex), true);
    }
    {
        game::Exception ex("");
        TS_ASSERT_EQUALS(game::actions::BuildAmmo::isValidCombination(myPlanet, fighterShip, ex), true);
    }
    {
        game::Exception ex("");
        TS_ASSERT_EQUALS(game::actions::BuildAmmo::isValidCombination(myPlanet, freightShip, ex), false);
        TS_ASSERT_DIFFERS(ex.what(), String_t());
    }
}

