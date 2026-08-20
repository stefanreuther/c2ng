/**
  *  \file test/game/proxy/simulationexportadaptortest.cpp
  *  \brief Test for game::proxy::SimulationExportAdaptor
  */

#include "game/proxy/simulationexportadaptor.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/sys/loglistener.hpp"
#include "afl/test/testrunner.hpp"
#include "game/root.hpp"
#include "game/sim/session.hpp"
#include "game/sim/ship.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "util/randomnumbergenerator.hpp"

using afl::base::Ref;
using afl::io::NullFileSystem;
using afl::string::NullTranslator;
using game::HostVersion;
using game::Root;
using game::TeamSettings;
using game::sim::Session;
using game::spec::ShipList;
using interpreter::test::ContextVerifier;
using util::RandomNumberGenerator;

namespace {
    struct Environment {
        Ref<Session> simSession;
        Ref<Root> root;
        Ref<ShipList> shipList;
        NullTranslator translator;
        afl::sys::Log log;
        NullFileSystem fileSystem;
        RandomNumberGenerator rng;

        Environment()
            : simSession(*new Session()),
              root(game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(4,0,0)))),
              shipList(*new ShipList()),
              translator(),
              log(),
              fileSystem(),
              rng(42)
            { }
    };

    class TestAdaptor : public game::proxy:: SimulationAdaptor {
     public:
        TestAdaptor(Environment& env)
            : m_environment(env)
            { }
        virtual Session& simSession()
            { return *m_environment.simSession; }
        virtual afl::base::Ptr<const Root> getRoot() const
            { return m_environment.root.asPtr(); }
        virtual afl::base::Ptr<const game::spec::ShipList> getShipList() const
            { return m_environment.shipList.asPtr(); }
        virtual const TeamSettings* getTeamSettings() const
            { return 0; }
        virtual afl::string::Translator& translator()
            { return m_environment.translator; }
        virtual afl::sys::LogListener& log()
            { return m_environment.log; }
        virtual afl::io::FileSystem& fileSystem()
            { return m_environment.fileSystem; }
        virtual util::RandomNumberGenerator& rng()
            { return m_environment.rng; }
        virtual bool isGameObject(const game::vcr::Object& /*obj*/) const
            { return false; }
        virtual size_t getNumProcessors() const
            { return 42; }
     private:
        Environment& m_environment;
    };

}

AFL_TEST("game.proxy.SimulationExportAdaptor", a)
{
    Environment env;
    env.simSession->setup().addShip().setName("Pegasus");
    TestAdaptor ad(env);

    // Verify that we can create a closure
    std::auto_ptr<game::proxy::SimulationExportAdaptor_t> closure(game::proxy::makeSimulationExportAdaptor());
    a.checkNonNull("01. closure", closure.get());

    // Verify that the closure produces a result
    std::auto_ptr<game::proxy::ExportAdaptor> result(closure->call(ad));
    a.checkNonNull("02. result", result.get());

    // Verify attributes
    a.checkEqual("11. fs", &result->fileSystem(), &env.fileSystem);
    a.checkEqual("12. tx", &result->translator(), &env.translator);

    // Verify configuration
    interpreter::exporter::Configuration config;
    result->initConfiguration(config);
    a.checkDifferent("21. fieldList", config.fieldList().size(), 0U);
    AFL_CHECK_SUCCEEDS(a("22. saveConfiguration"), result->saveConfiguration(config));

    // Verify contaxt
    std::auto_ptr<interpreter::Context> ctx(result->createContext());
    a.checkNonNull("31. ctx", ctx.get());

    // Coarsely verify contet xcontent
    ContextVerifier verif(*ctx, a("32. ctx"));
    verif.verifyBasics();
    verif.verifyTypes();
    verif.verifyString("NAME", "Pegasus");
}
