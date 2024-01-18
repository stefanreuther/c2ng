/**
  *  \file test/game/proxy/costsummaryadaptortest.cpp
  *  \brief Test for game::proxy::CostSummaryAdaptor
  */

#include "game/proxy/costsummaryadaptor.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/proxy/exportproxy.hpp"
#include "game/test/waitindicator.hpp"
#include "interpreter/test/contextverifier.hpp"
#include <algorithm>

using afl::io::NullFileSystem;
using afl::string::NullTranslator;
using game::proxy::ExportAdaptor;
using game::spec::Cost;
using game::spec::CostSummary;
using interpreter::test::ContextVerifier;

/** Test normal behaviour (manually instantiated object). */
AFL_TEST("game.proxy.CostSummaryAdaptor:normal", a)
{
    // Environment
    afl::base::Ptr<CostSummary> cs(new CostSummary());
    cs->add(CostSummary::Item(1, 4, "Quad", Cost::fromString("10T 200$")));
    NullFileSystem fs;
    NullTranslator tx;

    // Testee
    game::proxy::CostSummaryAdaptor testee(fs, tx, cs);

    // Verify
    // - connected objects
    a.checkEqual("01. fileSystem", &testee.fileSystem(), &fs);
    a.checkEqual("02. translator", &testee.translator(), &tx);

    // - pre-initialized configuration
    interpreter::exporter::Configuration config;
    a.checkEqual("11. fieldList", config.fieldList().size(), 0U);
    AFL_CHECK_SUCCEEDS(a("12. initConfiguration"), testee.initConfiguration(config));
    a.check("13. fieldList", config.fieldList().size() > 0);

    // - configuration store
    AFL_CHECK_SUCCEEDS(a("21. saveConfiguration"), testee.saveConfiguration(config));

    // - context
    std::auto_ptr<interpreter::Context> ctx(testee.createContext());
    a.checkNonNull("31. get", ctx.get());

    // - context content
    ContextVerifier verif(*ctx, a);
    verif.verifyInteger("COUNT", 4);
}

AFL_TEST("game.proxy.CostSummaryAdaptor:makeCostSummaryAdaptor", a)
{
    // CostSummary
    CostSummary cs;
    cs.add(CostSummary::Item(1, 4, "Quad", Cost::fromString("10T 200$")));

    // Create adaptor
    std::auto_ptr<afl::base::Closure<ExportAdaptor*(game::Session&)> > closure(game::proxy::makeCostSummaryAdaptor(cs));
    a.checkNonNull("01. result", closure.get());

    // Apply adaptor to session
    NullFileSystem fs;
    NullTranslator tx;
    game::Session session(tx, fs);
    std::auto_ptr<ExportAdaptor> ad(closure->call(session));
    a.checkNonNull("11. call", ad.get());

    // We can now modify the CostSummary
    cs.clear();

    // Verify result
    std::auto_ptr<interpreter::Context> ctx(ad->createContext());
    a.checkNonNull("21. createContext", ctx.get());
    ContextVerifier verif(*ctx, a);
    verif.verifyInteger("COUNT", 4);
}

AFL_TEST("game.proxy.CostSummaryAdaptor:integration", a)
{
    // CostSummary
    CostSummary cs;
    cs.add(CostSummary::Item(1, 4, "Quad", Cost::fromString("10T 200$")));

    // Session
    NullFileSystem fs;
    NullTranslator tx;
    game::Session session(tx, fs);

    // ExportProxy
    game::test::WaitIndicator ind;
    util::RequestReceiver<game::Session> recv(ind, session);
    game::proxy::ExportProxy proxy(recv.getSender().makeTemporary(game::proxy::makeCostSummaryAdaptor(cs)), ind);

    // Verify by checking field list
    afl::data::StringList_t list;
    proxy.enumProperties(ind, list);
    a.check("01. list", list.size() > 0);
    a.check("02. find", std::find(list.begin(), list.end(), "COUNT") != list.end());
}
