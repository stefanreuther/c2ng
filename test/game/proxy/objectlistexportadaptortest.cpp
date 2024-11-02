/**
  *  \file test/game/proxy/objectlistexportadaptortest.cpp
  *  \brief Test for game::proxy::ObjectListExportAdaptor
  */

#include "game/proxy/objectlistexportadaptor.hpp"

#include "afl/data/access.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/universe.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "interpreter/tagnode.hpp"
#include "interpreter/test/contextverifier.hpp"

/** Test normal behaviour.
    Set up a normal situation and exercise general methods and sequences. */
AFL_TEST("game.proxy.ObjectListExportAdaptor:normal", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session s(tx, fs);
    s.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    s.setGame(new game::Game());
    s.setShipList(new game::spec::ShipList());
    for (int i = 1; i < 30; ++i) {
        game::map::Planet* pl = s.getGame()->currentTurn().universe().planets().create(i);
        pl->setPosition(game::map::Point(i, 1000));
    }
    s.postprocessTurn(s.getGame()->currentTurn(), game::PlayerSet_t(1), game::PlayerSet_t(1), game::map::Object::Playable);

    std::vector<game::Id_t> ids;
    ids.push_back(10);
    ids.push_back(15);

    game::proxy::ObjectListExportAdaptor testee(s, game::proxy::ObjectListExportAdaptor::Planets, ids);

    // Verify links
    a.checkEqual("01. fileSystem", &testee.fileSystem(), &s.world().fileSystem());
    a.checkEqual("02. translator", &testee.translator(), &tx);

    // Verify configuration access
    // - initConfiguration
    s.getRoot()->userConfiguration()[game::config::UserConfiguration::ExportPlanetFields].set("ID,NAME");
    interpreter::exporter::Configuration config;
    testee.initConfiguration(config);
    a.checkEqual("11. fieldList", config.fieldList().toString(), "ID,NAME");

    // - saveConfiguration
    config.fieldList().add("OWNER");
    testee.saveConfiguration(config);
    a.checkEqual("21. ExportPlanetFields", s.getRoot()->userConfiguration()[game::config::UserConfiguration::ExportPlanetFields](), "ID,NAME,OWNER");

    // Verify context and iteration
    {
        std::auto_ptr<interpreter::Context> ctx(testee.createContext());
        a.checkNonNull("31. ctx", ctx.get());

        interpreter::test::ContextVerifier ctxv(*ctx, a("context"));
        ctxv.verifyTypes();
        ctxv.verifyInteger("ID", 10);

        game::map::Object* obj = dynamic_cast<game::map::Object*>(ctx->getObject());
        a.checkNonNull("41. obj", obj);
        a.checkEqual("42. getId", obj->getId(), 10);

        // - second object
        a.checkEqual("51. next", ctx->next(), true);
        ctxv.verifyInteger("ID", 15);

        obj = dynamic_cast<game::map::Object*>(ctx->getObject());
        a.checkNonNull("61. obj", obj);
        a.checkEqual("62. getId", obj->getId(), 15);

        // - No more objects
        a.checkEqual("71. next", ctx->next(), false);
    }

    // Verify basics/cloning
    {
        std::auto_ptr<interpreter::Context> ctx(testee.createContext());
        a.checkNonNull("81. ctx", ctx.get());

        interpreter::test::ContextVerifier verif(*ctx, a("basics"));
        verif.verifyBasics();
        verif.verifyNotSerializable();
    }
}

/** Test abnormal case: empty session.
    We can still iterate, but objects pretend to have no content. */
AFL_TEST("game.proxy.ObjectListExportAdaptor:null", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session s(tx, fs);

    std::vector<game::Id_t> ids;
    ids.push_back(10);
    ids.push_back(15);

    game::proxy::ObjectListExportAdaptor testee(s, game::proxy::ObjectListExportAdaptor::Planets, ids);

    // Verify context and iteration
    {
        std::auto_ptr<interpreter::Context> ctx(testee.createContext());
        a.checkNonNull("01. ctx", ctx.get());

        // Cannot look up
        size_t propertyIndex;
        interpreter::Context::PropertyAccessor* propertyAccess = ctx->lookup("ID", propertyIndex);
        a.checkNull("11. lookup", propertyAccess);

        // Cannot get an object
        afl::base::Deletable* obj = ctx->getObject();
        a.checkNull("21. obj", obj);

        // - second slot
        a.checkEqual("31. next", ctx->next(), true);

        // - No more objects
        a.checkEqual("41. next", ctx->next(), false);
    }
}

/** Test configuration handling, special case.
    An invalid value in the configuration is not an error. */
AFL_TEST("game.proxy.ObjectListExportAdaptor:config-error", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session s(tx, fs);
    s.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    s.getRoot()->userConfiguration()[game::config::UserConfiguration::ExportShipFields].set("-");

    std::vector<game::Id_t> ids;
    ids.push_back(10);
    ids.push_back(15);

    game::proxy::ObjectListExportAdaptor testee(s, game::proxy::ObjectListExportAdaptor::Ships, ids);

    interpreter::exporter::Configuration config;
    AFL_CHECK_SUCCEEDS(a("01. initConfiguration"), testee.initConfiguration(config));
    a.checkEqual("02. fieldList", config.fieldList().toString(), "");
}
