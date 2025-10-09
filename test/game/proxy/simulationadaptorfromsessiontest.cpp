/**
  *  \file test/game/proxy/simulationadaptorfromsessiontest.cpp
  *  \brief Test for game::proxy::SimulationAdaptorFromSession
  */

#include <memory>
#include "game/proxy/simulationadaptorfromsession.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/hostversion.hpp"
#include "game/session.hpp"
#include "game/sim/sessionextra.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"

AFL_TEST("game.proxy.SimulationAdaptorFromSession:empty", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    util::SystemInformation info;
    info.numProcessors = 42;
    session.setSystemInformation(info);

    std::auto_ptr<game::proxy::SimulationAdaptor> testee(game::proxy::SimulationAdaptorFromSession().call(session));

    a.checkEqual("01. simSession",        &testee->simSession(), &*game::sim::getSimulatorSession(session));
    a.checkNull ("02. getRoot",            testee->getRoot().get());
    a.checkNull ("03. getShipList",        testee->getShipList().get());
    a.checkNull ("04. getTeamSettings",    testee->getTeamSettings());
    a.checkEqual("05. translator",        &testee->translator(), &session.translator());
    a.checkEqual("06. log",               &testee->log(), &session.log());
    a.checkEqual("07. fileSystem",        &testee->fileSystem(), &session.world().fileSystem());
    a.checkEqual("08. rng",               &testee->rng(), &session.rng());
    a.check     ("09. isGameObject",      !testee->isGameObject(game::vcr::Object()));
    a.checkEqual("10. getNumProcessors",   testee->getNumProcessors(), 42U);
}

AFL_TEST("game.proxy.SimulationAdaptorFromSession:full", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    afl::base::Ptr<game::Game> g = new game::Game();
    afl::base::Ptr<game::Root> r = game::test::makeRoot(game::HostVersion()).asPtr();
    afl::base::Ptr<game::spec::ShipList> sl = new game::spec::ShipList();
    session.setGame(g);
    session.setRoot(r);
    session.setShipList(sl);

    std::auto_ptr<game::proxy::SimulationAdaptor> testee(game::proxy::SimulationAdaptorFromSession().call(session));

    a.checkEqual("01. simSession",        &testee->simSession(), &*game::sim::getSimulatorSession(session));
    a.checkEqual("02. getRoot",            testee->getRoot().get(), r.get());
    a.checkEqual("03. getShipList",        testee->getShipList().get(), sl.get());
    a.checkEqual("04. getTeamSettings",    testee->getTeamSettings(), &g->teamSettings());
    a.checkEqual("05. translator",        &testee->translator(), &session.translator());
    a.checkEqual("06. log",               &testee->log(), &session.log());
    a.checkEqual("07. fileSystem",        &testee->fileSystem(), &session.world().fileSystem());
    a.checkEqual("08. rng",               &testee->rng(), &session.rng());
}
