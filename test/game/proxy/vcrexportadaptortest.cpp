/**
  *  \file test/game/proxy/vcrexportadaptortest.cpp
  *  \brief Test for game::proxy::VcrExportAdaptor
  */

#include "game/proxy/vcrexportadaptor.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include "game/vcr/classic/database.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "util/simplerequestdispatcher.hpp"

namespace gvc = game::vcr::classic;

using afl::base::Ref;
using afl::io::NullFileSystem;
using afl::string::NullTranslator;
using game::HostVersion;
using game::Root;
using game::proxy::ExportAdaptor;
using game::proxy::VcrDatabaseAdaptor;
using game::spec::ShipList;
using game::vcr::Object;
using interpreter::test::ContextVerifier;

namespace {

    struct Environment {
        Ref<Root> root;
        Ref<ShipList> shipList;
        game::TeamSettings* pTeamSettings;
        Ref<gvc::Database> battles;
        NullTranslator translator;
        afl::sys::Log log;
        NullFileSystem fileSystem;

        Environment()
            : root(game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(4,0,0)))),
              shipList(*new ShipList()), pTeamSettings(0), battles(*new game::vcr::classic::Database()), translator(), fileSystem()
            {
                game::test::initStandardBeams(*shipList);
                game::test::initStandardTorpedoes(*shipList);
            }
    };

    class TestAdaptor : public VcrDatabaseAdaptor {
     public:
        TestAdaptor(Environment& env)
            : m_env(env)
            { }
        virtual Ref<const Root> getRoot() const
            { return m_env.root; }
        virtual Ref<const ShipList> getShipList() const
            { return m_env.shipList; }
        virtual const game::TeamSettings* getTeamSettings() const
            { return m_env.pTeamSettings; }
        virtual Ref<game::vcr::Database> getBattles()
            { return m_env.battles; }
        virtual afl::string::Translator& translator()
            { return m_env.translator; }
        virtual afl::sys::LogListener& log()
            { return m_env.log; }
        virtual afl::io::FileSystem& fileSystem()
            { return m_env.fileSystem; }
        virtual size_t getCurrentBattle() const
            { return 0; }
        virtual void setCurrentBattle(size_t /*n*/)
            { }
        virtual game::sim::Setup* getSimulationSetup() const
            { return 0; }
        virtual bool isGameObject(const Object&) const
            { return false; }
     private:
        Environment& m_env;
    };

    Object makeLeftShip(int id)
    {
        Object left;
        left.setMass(150);
        left.setCrew(2);
        left.setId(id);
        left.setOwner(2);
        left.setBeamType(0);
        left.setNumBeams(0);
        left.setNumBays(0);
        left.setTorpedoType(0);
        left.setNumLaunchers(0);
        left.setNumTorpedoes(0);
        left.setNumFighters(0);
        left.setShield(100);
        left.setName("Liz");
        return left;
    }

    Object makeRightShip(int id)
    {
        Object right;
        right.setMass(233);
        right.setCrew(240);
        right.setId(id);
        right.setOwner(3);
        right.setBeamType(5);
        right.setNumBeams(6);
        right.setNumBays(0);
        right.setTorpedoType(7);
        right.setNumLaunchers(4);
        right.setNumTorpedoes(0);
        right.setNumFighters(0);
        right.setShield(100);
        right.setName("Bird");
        return right;
    }

    void addBattles(Environment& env)
    {
        env.battles->addNewBattle(new gvc::Battle(makeLeftShip(10), makeRightShip(20), 42, 0))
            ->setType(gvc::PHost4, 0);
        env.battles->addNewBattle(new gvc::Battle(makeLeftShip(70), makeRightShip(60), 42, 0))
            ->setType(gvc::PHost4, 0);
    }
}

AFL_TEST("game.proxy.VcrExportAdaptor:makeVcrExportAdaptor", a)
{
    // Make simple environment
    Environment env;
    addBattles(env);

    // Test setup
    TestAdaptor ad(env);
    std::auto_ptr<game::proxy::VcrExportAdaptor_t> converter(game::proxy::makeVcrExportAdaptor());
    std::auto_ptr<ExportAdaptor> result(converter->call(ad));

    // Verify general attributes
    a.checkNonNull("01. result", result.get());
    a.checkEqual("02. fileSystem", &result->fileSystem(), &env.fileSystem);
    a.checkEqual("03. translator", &result->translator(), &env.translator);

    // Configuration
    interpreter::exporter::Configuration config;
    result->initConfiguration(config);
    a.checkDifferent("11. fieldList", config.fieldList().size(), 0U);
    AFL_CHECK_SUCCEEDS(a("12. saveConfiguration"), result->saveConfiguration(config));

    // Context
    std::auto_ptr<interpreter::Context> ctx(result->createContext());
    a.checkNonNull("21. ctx", ctx.get());

    ContextVerifier verif(*ctx, a("22. context"));
    verif.verifyBasics();
    verif.verifyTypes();
    verif.verifyInteger("LEFT.ID", 10);
    verif.verifyInteger("RIGHT.ID", 20);

    a.check("31. next", ctx->next());
    verif.verifyInteger("LEFT.ID", 70);
    verif.verifyInteger("RIGHT.ID", 60);
}

AFL_TEST("game.proxy.VcrExportAdaptor:makeVcrSideExportAdaptor", a)
{
    // Make simple environment
    Environment env;
    addBattles(env);

    // Test setup
    TestAdaptor ad(env);
    std::auto_ptr<game::proxy::VcrExportAdaptor_t> converter(game::proxy::makeVcrSideExportAdaptor(1));
    std::auto_ptr<ExportAdaptor> result(converter->call(ad));

    // Verify general attributes
    a.checkNonNull("01. result", result.get());
    a.checkEqual("02. fileSystem", &result->fileSystem(), &env.fileSystem);
    a.checkEqual("03. translator", &result->translator(), &env.translator);

    // Configuration
    interpreter::exporter::Configuration config;
    result->initConfiguration(config);
    a.checkDifferent("11. fieldList", config.fieldList().size(), 0U);
    AFL_CHECK_SUCCEEDS(a("12. saveConfiguration"), result->saveConfiguration(config));

    // Context
    std::auto_ptr<interpreter::Context> ctx(result->createContext());
    a.checkNonNull("21. ctx", ctx.get());

    ContextVerifier verif(*ctx, a("22. context"));
    verif.verifyBasics();
    verif.verifyTypes();
    verif.verifyInteger("ID", 70);
    verif.verifyString("NAME", "Liz");

    a.check("31. next", ctx->next());
    verif.verifyInteger("ID", 60);
    verif.verifyString("NAME", "Bird");
}
