/**
  *  \file u/t_game_proxy_costsummaryadaptor.cpp
  *  \brief Test for game::proxy::CostSummaryAdaptor
  */

#include <algorithm>
#include "game/proxy/costsummaryadaptor.hpp"

#include "t_game_proxy.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/proxy/exportproxy.hpp"
#include "game/test/waitindicator.hpp"
#include "interpreter/test/contextverifier.hpp"

using afl::io::NullFileSystem;
using afl::string::NullTranslator;
using game::proxy::ExportAdaptor;
using game::spec::Cost;
using game::spec::CostSummary;
using interpreter::test::ContextVerifier;

/** Test normal behaviour (manually instantiated object). */
void
TestGameProxyCostSummaryAdaptor::testNormal()
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
    TS_ASSERT_EQUALS(&testee.fileSystem(), &fs);
    TS_ASSERT_EQUALS(&testee.translator(), &tx);

    // - pre-initialized configuration
    interpreter::exporter::Configuration config;
    TS_ASSERT_EQUALS(config.fieldList().size(), 0U);
    TS_ASSERT_THROWS_NOTHING(testee.initConfiguration(config));
    TS_ASSERT(config.fieldList().size() > 0);

    // - configuration store
    TS_ASSERT_THROWS_NOTHING(testee.saveConfiguration(config));

    // - context
    std::auto_ptr<interpreter::Context> ctx(testee.createContext());
    TS_ASSERT(ctx.get() != 0);

    // - context content
    ContextVerifier verif(*ctx, "testNormal");
    verif.verifyInteger("COUNT", 4);
}

void
TestGameProxyCostSummaryAdaptor::testMake()
{
    // CostSummary
    CostSummary cs;
    cs.add(CostSummary::Item(1, 4, "Quad", Cost::fromString("10T 200$")));

    // Create adaptor
    std::auto_ptr<afl::base::Closure<ExportAdaptor*(game::Session&)> > closure(game::proxy::makeCostSummaryAdaptor(cs));
    TS_ASSERT(closure.get() != 0);

    // Apply adaptor to session
    NullFileSystem fs;
    NullTranslator tx;
    game::Session session(tx, fs);
    std::auto_ptr<ExportAdaptor> ad(closure->call(session));
    TS_ASSERT(ad.get() != 0);

    // We can now modify the CostSummary
    cs.clear();

    // Verify result
    std::auto_ptr<interpreter::Context> ctx(ad->createContext());
    TS_ASSERT(ctx.get() != 0);
    ContextVerifier verif(*ctx, "testMake");
    verif.verifyInteger("COUNT", 4);
}

void
TestGameProxyCostSummaryAdaptor::testIntegration()
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
    TS_ASSERT(list.size() > 0);
    TS_ASSERT(std::find(list.begin(), list.end(), "COUNT") != list.end());
}

