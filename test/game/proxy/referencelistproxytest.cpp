/**
  *  \file test/game/proxy/referencelistproxytest.cpp
  *  \brief Test for game::proxy::ReferenceListProxy
  */

#include "game/proxy/referencelistproxy.hpp"

#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/hostversion.hpp"
#include "game/map/planet.hpp"
#include "game/map/universe.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"

namespace {
    using afl::base::Ptr;
    using game::Game;
    using game::HostVersion;
    using game::map::Planet;
    using game::map::Universe;
    using game::Reference;
    using game::config::ConfigurationOption;

    void prepare(game::test::SessionThread& h)
    {
        // Game
        Ptr<Game> g = new Game();
        Universe& u = g->currentTurn().universe();

        u.planets().create(1)->setName("One");
        u.planets().create(2)->setName("Two");
        u.planets().create(3)->setName("Three");
        u.planets().create(4)->setName("Four");
        u.planets().create(5)->setName("Five");

        h.session().setGame(g);

        // Root
        h.session().setRoot(game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(4,0,0))).asPtr());
    }

    class Initializer : public game::proxy::ReferenceListProxy::Initializer_t {
     public:
        virtual void call(game::Session&, game::ref::ListObserver& obs)
            {
                game::ref::List list;
                for (int i = 1; i <= 5; ++i) {
                    list.add(Reference(Reference::Planet, i));
                }
                obs.setList(list);
            }
    };

    struct UpdateReceiver {
        game::ref::UserList result;

        void onListChange(const game::ref::UserList& list)
            { result = list; }
    };
}

AFL_TEST("game.proxy.ReferenceListProxy:basics", a)
{
    // Environment
    game::test::SessionThread h;
    prepare(h);

    // Object under test
    game::test::WaitIndicator ind;
    game::proxy::ReferenceListProxy testee(h.gameSender(), ind);

    UpdateReceiver recv;
    testee.sig_listChange.add(&recv, &UpdateReceiver::onListChange);
    testee.setContentNew(std::auto_ptr<game::proxy::ReferenceListProxy::Initializer_t>(new Initializer()));
    a.check("01. isIdle", !testee.isIdle());
    testee.waitIdle(ind);
    a.check("02. isIdle", testee.isIdle());

    // Verify
    a.checkEqual("11. size", recv.result.size(), 5U);
    a.checkEqual("12. name", recv.result.get(0)->name, "Planet #1: One");
    a.checkEqual("13. name", recv.result.get(4)->name, "Planet #5: Five");

    // Check config. Default will be by Id
    game::ref::Configuration config = testee.getConfig(ind);
    a.checkEqual("21. first", config.order.first, game::ref::ConfigSortById);
    a.checkEqual("22. second", config.order.second, game::ref::ConfigSortById);

    // Sort
    config.order.first = game::ref::ConfigSortByName;
    config.order.second = game::ref::ConfigSortById;
    testee.setConfig(config);
    testee.waitIdle(ind);

    // Verify sorted list: Five / Four / One / Two / Three
    a.checkEqual("31. size", recv.result.size(), 5U);
    a.checkEqual("32. name", recv.result.get(0)->name, "Planet #5: Five");
    a.checkEqual("33. name", recv.result.get(4)->name, "Planet #2: Two");

    // Verify configuration
    const ConfigurationOption* opt = h.session().getRoot()->userConfiguration().getOptionByName("Sort.Ship");
    a.check("41. opt", opt);
    a.checkEqual("42. toString", opt->toString(), "10");    // sort-by-name
}

AFL_TEST("game.proxy.ReferenceListProxy:setConfigurationSelection", a)
{
    // Environment
    game::test::SessionThread h;
    prepare(h);

    // Configuration
    game::config::UserConfiguration& config = h.session().getRoot()->userConfiguration();
    config.setOption("Sort.Ship", "10", ConfigurationOption::Game);  // sort-by-name
    config.setOption("Sort.Cargo", "0", ConfigurationOption::Game);  // sort-by-Id

    // Object under test
    game::test::WaitIndicator ind;
    game::proxy::ReferenceListProxy testee(h.gameSender(), ind);

    UpdateReceiver recv;
    testee.sig_listChange.add(&recv, &UpdateReceiver::onListChange);
    testee.setContentNew(std::auto_ptr<game::proxy::ReferenceListProxy::Initializer_t>(new Initializer()));
    testee.waitIdle(ind);

    // Verify
    a.checkEqual("01. size", recv.result.size(), 5U);
    a.checkEqual("02. name", recv.result.get(0)->name, "Planet #5: Five");
    a.checkEqual("03. name", recv.result.get(4)->name, "Planet #2: Two");

    // Change sort order
    testee.setConfigurationSelection(game::ref::CARGO_TRANSFER);
    testee.waitIdle(ind);

    // Verify
    a.checkEqual("11. size", recv.result.size(), 5U);
    a.checkEqual("12. name", recv.result.get(0)->name, "Planet #1: One");
    a.checkEqual("13. name", recv.result.get(4)->name, "Planet #5: Five");
}
