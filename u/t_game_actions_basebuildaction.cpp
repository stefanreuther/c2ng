/**
  *  \file u/t_game_actions_basebuildaction.cpp
  *  \brief Test for game::actions::BaseBuildAction
  */

#include "game/actions/basebuildaction.hpp"

#include "t_game_actions.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/actions/basebuildexecutor.hpp"
#include "game/exception.hpp"
#include "game/map/configuration.hpp"
#include "game/map/universe.hpp"
#include "game/test/cargocontainer.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/specificationloader.hpp"
#include "game/test/stringverifier.hpp"
#include "afl/charset/utf8charset.hpp"

namespace {
    class TestHarness {
     public:
        TestHarness()
            : univ(),
              planet(*univ.planets().create(72)),
              container(),
              shipList(),
              root(afl::io::InternalDirectory::create("game dir"),
                   *new game::test::SpecificationLoader(),
                   game::HostVersion(game::HostVersion::Host, MKVERSION(3, 22, 47)),
                   std::auto_ptr<game::RegistrationKey>(new game::test::RegistrationKey(game::RegistrationKey::Unregistered, 5)),
                   std::auto_ptr<game::StringVerifier>(new game::test::StringVerifier()),
                   std::auto_ptr<afl::charset::Charset>(new afl::charset::Utf8Charset()),
                   game::Root::Actions_t())
            { root.hostConfiguration().setDefaultValues(); }

        game::map::Universe univ;
        game::map::Planet& planet;
        game::test::CargoContainer container;
        game::spec::ShipList shipList;
        game::Root root;
        afl::string::NullTranslator tx;
        afl::sys::Log log;
    };

    class TestAction : public game::actions::BaseBuildAction {
     public:
        explicit TestAction(TestHarness& h)
            : BaseBuildAction(h.planet, h.container, h.shipList, h.root),
              m_tech(1)
            { }

        virtual void perform(game::actions::BaseBuildExecutor& exec)
            { exec.setBaseTechLevel(game::BeamTech, m_tech); }

        void setTechLevel(int n)
            { m_tech = n; }

     private:
        int m_tech;
    };
}


/** Test error case: instantiating BaseBuildAction on a planet that does not have a base. */
void
TestGameActionsBaseBuildAction::testError()
{
    TestHarness h;

    // Define planet without base
    h.planet.setPosition(game::map::Point(1111, 2222));
    h.planet.addCurrentPlanetData(game::map::PlanetData(), game::PlayerSet_t(7));
    h.planet.setOwner(7);
    h.planet.internalCheck(game::map::Configuration(), h.tx, h.log);
    h.planet.combinedCheck2(h.univ, game::PlayerSet_t(7), 12);
    h.planet.setPlayability(game::map::Object::Playable);

    TS_ASSERT_THROWS((TestAction(h)), game::Exception);
}

/** Test success case: instantiating BaseBuildAction on a planet with base, and working on it. */
void
TestGameActionsBaseBuildAction::testSuccess()
{
    TestHarness h;

    // Define planet with base
    h.planet.setPosition(game::map::Point(1111, 2222));
    h.planet.addCurrentPlanetData(game::map::PlanetData(), game::PlayerSet_t(7));
    h.planet.addCurrentBaseData(game::map::BaseData(), game::PlayerSet_t(7));
    h.planet.setOwner(7);
    h.planet.setBaseTechLevel(game::HullTech, 1);
    h.planet.setBaseTechLevel(game::EngineTech, 1);
    h.planet.setBaseTechLevel(game::BeamTech, 1);
    h.planet.setBaseTechLevel(game::TorpedoTech, 1);
    h.planet.internalCheck(game::map::Configuration(), h.tx, h.log);
    h.planet.combinedCheck2(h.univ, game::PlayerSet_t(7), 12);
    h.planet.setPlayability(game::map::Object::Playable);

    // This must have produced a base
    TS_ASSERT(h.planet.hasBase());

    // Make an action.
    TestAction a(h);

    // Set null operation; must have cost zero. The update() is normally in the descendant's method.
    a.setTechLevel(1);
    a.update();
    TS_ASSERT(a.isValid());
    TS_ASSERT(a.costAction().getCost().isZero());
    TS_ASSERT_EQUALS(a.getStatus(), game::actions::BaseBuildAction::Success);

    // Set invalid (unregistered)
    a.setTechLevel(6);
    a.update();
    TS_ASSERT(!a.isValid());
    TS_ASSERT_EQUALS(a.costAction().getCost().get(game::spec::Cost::Money), 1500);
    TS_ASSERT_EQUALS(a.getStatus(), game::actions::BaseBuildAction::DisallowedTech);

    // Set valid tech level
    a.setTechLevel(4);
    a.update();
    TS_ASSERT(a.isValid());
    TS_ASSERT(!a.costAction().getCost().isZero());
    TS_ASSERT_EQUALS(a.costAction().getCost().get(game::spec::Cost::Money), 600);
    TS_ASSERT_EQUALS(a.getStatus(), game::actions::BaseBuildAction::Success);

    // Chance price configuration. This automatically updates.
    h.root.hostConfiguration()[game::config::HostConfiguration::BaseTechCost].set(270);
    h.root.hostConfiguration().notifyListeners();
    TS_ASSERT(a.isValid());
    TS_ASSERT_EQUALS(a.costAction().getCost().get(game::spec::Cost::Money), 1620);
    TS_ASSERT_EQUALS(h.container.getChange(game::Element::Money), -1620);
    TS_ASSERT_EQUALS(a.getStatus(), game::actions::BaseBuildAction::Success);

    // Change even more; this time exceeding the available money (5000).
    h.root.hostConfiguration()[game::config::HostConfiguration::BaseTechCost].set(1000);
    h.root.hostConfiguration().notifyListeners();
    TS_ASSERT(!a.isValid());
    TS_ASSERT_EQUALS(a.costAction().getCost().get(game::spec::Cost::Money), 6000);
    TS_ASSERT_EQUALS(h.container.getChange(game::Element::Money), -6000);
    TS_ASSERT_EQUALS(a.getStatus(), game::actions::BaseBuildAction::MissingResources);

    // Change back, and commit
    h.root.hostConfiguration()[game::config::HostConfiguration::BaseTechCost].set(100);
    h.root.hostConfiguration().notifyListeners();
    TS_ASSERT_EQUALS(a.costAction().getCost().get(game::spec::Cost::Money), 600);
    TS_ASSERT_EQUALS(h.container.getChange(game::Element::Money), -600);
    TS_ASSERT_THROWS_NOTHING(a.commit());
    TS_ASSERT_EQUALS(h.planet.getBaseTechLevel(game::BeamTech).orElse(0), 4);
}
