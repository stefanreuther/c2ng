/**
  *  \file test/game/proxy/searchproxytest.cpp
  *  \brief Test for game::proxy::SearchProxy
  */

#include "game/proxy/searchproxy.hpp"

#include "afl/data/stringvalue.hpp"
#include "afl/test/testrunner.hpp"
#include "game/interface/referencelistcontext.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "interpreter/subroutinevalue.hpp"
#include "util/simplerequestdispatcher.hpp"

using afl::base::Ref;
using game::Reference;
using game::SearchQuery;
using game::interface::ReferenceListContext;
using game::proxy::SearchProxy;
using game::test::SessionThread;
using interpreter::BCORef_t;
using interpreter::BytecodeObject;
using interpreter::Opcode;
using util::SimpleRequestDispatcher;

namespace {
    /*
     *  Callbacks
     */

    struct SuccessReceiver {
        game::ref::List list;

        void onSuccess(const game::ref::List& list)
            { this->list = list; }
    };

    struct ErrorReceiver {
        String_t error;

        void onError(String_t error)
            { this->error = error; }
    };

    /*
     *  Utilities
     */

    BCORef_t createSearchFunction(SessionThread& s)
    {
        BCORef_t bco = BytecodeObject::create(false);
        bco->addArgument("A", false);
        bco->addArgument("B", false);
        s.session().world().setNewGlobalValue("CCUI$SEARCH", new interpreter::SubroutineValue(bco));
        return bco;
    }

    SearchQuery::SearchObjects_t objects()
    {
        return SearchQuery::SearchObjects_t() + SearchQuery::SearchShips + SearchQuery::SearchPlanets;
    }
}

/** Test search, success case.
    A: set it up such that CC$SEARCH returns a ReferenceList.
    E: sig_success called with that list */
AFL_TEST("game.proxy.SearchProxy:success", a)
{
    SessionThread s;
    const Reference REF1(Reference::Ship, 1701);
    const Reference REF2(Reference::Planet, 363);

    // CC$SEARCH that produces a ReferenceList
    {
        Ref<ReferenceListContext::Data> data = *new ReferenceListContext::Data();
        data->list.add(REF1);
        data->list.add(REF2);
        ReferenceListContext value(data, s.session());

        BCORef_t ref = createSearchFunction(s);
        ref->addPushLiteral(&value);
    }

    // Invoke search
    SimpleRequestDispatcher disp;
    SearchProxy proxy(s.gameSender(), disp);

    SuccessReceiver recv;
    proxy.sig_success.add(&recv, &SuccessReceiver::onSuccess);
    proxy.search(SearchQuery(SearchQuery::MatchName, objects(), "a"), true);

    while (recv.list.size() == 0) {
        a.check("01. wait", disp.wait(1000));
    }

    // Verify result
    a.checkEqual("11. size", recv.list.size(), 2U);
    a.checkEqual("12. item 0", recv.list[0], REF1);
    a.checkEqual("13. item 1", recv.list[1], REF2);
}

/** Test search, failure to compile.
    A: submit a search query that does not compile.
    E: sig_error called */
AFL_TEST("game.proxy.SearchProxy:error:compile", a)
{
    SessionThread s;
    // no CC$SEARCH, we don't get that far

    // Invoke search
    SimpleRequestDispatcher disp;
    SearchProxy proxy(s.gameSender(), disp);

    ErrorReceiver recv;
    proxy.sig_error.add(&recv, &ErrorReceiver::onError);
    proxy.search(SearchQuery(SearchQuery::MatchTrue, objects(), "*"), true);

    while (recv.error.empty()) {
        a.check("01. wait", disp.wait(1000));
    }
}

/** Test search, failure: search suspends unexpectedly.
    A: set it up such that CC$SEARCH suspends.
    E: sig_error called */
AFL_TEST("game.proxy.SearchProxy:error:suspend", a)
{
    SessionThread s;

    // CC$SEARCH that suspends
    createSearchFunction(s)->addInstruction(Opcode::maSpecial, Opcode::miSpecialSuspend, 0);

    // Invoke search
    SimpleRequestDispatcher disp;
    SearchProxy proxy(s.gameSender(), disp);

    ErrorReceiver recv;
    proxy.sig_error.add(&recv, &ErrorReceiver::onError);
    proxy.search(SearchQuery(SearchQuery::MatchName, objects(), "a"), true);

    while (recv.error.empty()) {
        a.check("01. wait", disp.wait(1000));
    }
}

/** Test search, failure: search returns error.
    A: set it up such that CC$SEARCH reports an error (string result).
    E: sig_error called */
AFL_TEST("game.proxy.SearchProxy:error:end", a)
{
    SessionThread s;

    // CC$SEARCH that returns a string
    afl::data::StringValue sv("boom");
    createSearchFunction(s)->addPushLiteral(&sv);

    // Invoke search
    SimpleRequestDispatcher disp;
    SearchProxy proxy(s.gameSender(), disp);

    ErrorReceiver recv;
    proxy.sig_error.add(&recv, &ErrorReceiver::onError);
    proxy.search(SearchQuery(SearchQuery::MatchName, objects(), "a"), true);

    while (recv.error.empty()) {
        a.check("01. wait", disp.wait(1000));
    }
    a.checkEqual("02. error", recv.error, "boom");
}

/** Test search, failure: search returns invalid value.
    This does not happen normally with the implementation of CC$SEARCH in core.q.
    A: set it up such that CC$SEARCH reports an invalid value (integer result).
    E: sig_error called */
AFL_TEST("game.proxy.SearchProxy:error:invalid", a)
{
    SessionThread s;

    // CC$SEARCH that returns an integer
    createSearchFunction(s)->addInstruction(Opcode::maPush, Opcode::sInteger, 42);

    // Invoke search
    SimpleRequestDispatcher disp;
    SearchProxy proxy(s.gameSender(), disp);

    ErrorReceiver recv;
    proxy.sig_error.add(&recv, &ErrorReceiver::onError);
    proxy.search(SearchQuery(SearchQuery::MatchName, objects(), "a"), true);

    while (recv.error.empty()) {
        a.check("01. wait", disp.wait(1000));
    }
}

/** Test search, failure: search terminates.
    A: set it up such that CC$SEARCH terminates.
    E: sig_error called */
AFL_TEST("game.proxy.SearchProxy:error:terminate", a)
{
    SessionThread s;

    // CC$SEARCH that terminates
    createSearchFunction(s)->addInstruction(Opcode::maSpecial, Opcode::miSpecialTerminate, 42);

    // Invoke search
    SimpleRequestDispatcher disp;
    SearchProxy proxy(s.gameSender(), disp);

    ErrorReceiver recv;
    proxy.sig_error.add(&recv, &ErrorReceiver::onError);
    proxy.search(SearchQuery(SearchQuery::MatchName, objects(), "a"), true);

    while (recv.error.empty()) {
        a.check("01. wait", disp.wait(1000));
    }
}

/** Test search, failure: search aborts with exception.
    This does not happen normally with the implementation of CC$SEARCH in core.q.
    A: set it up such that CC$SEARCH throws.
    E: sig_error called */
AFL_TEST("game.proxy.SearchProxy:error:exception", a)
{
    SessionThread s;

    // CC$SEARCH that throws
    {
        BCORef_t bco = createSearchFunction(s);
        bco->addInstruction(Opcode::maPush, Opcode::sInteger, 7);
        bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialThrow, 0);
    }

    // Invoke search
    SimpleRequestDispatcher disp;
    SearchProxy proxy(s.gameSender(), disp);

    ErrorReceiver recv;
    proxy.sig_error.add(&recv, &ErrorReceiver::onError);
    proxy.search(SearchQuery(SearchQuery::MatchName, objects(), "a"), true);

    while (recv.error.empty()) {
        a.check("01. wait", disp.wait(1000));
    }
}

/** Test search, optional saving of the query. */
AFL_TEST("game.proxy.SearchProxy:save-query", a)
{
    SessionThread s;

    // CC$SEARCH that produces a one-element ReferenceList (nonzero just that we recognize that we got a result)
    {
        Ref<ReferenceListContext::Data> data = *new ReferenceListContext::Data();
        data->list.add(game::Reference());
        ReferenceListContext value(data, s.session());

        BCORef_t ref = createSearchFunction(s);
        ref->addPushLiteral(&value);
    }

    // Invoke search
    SimpleRequestDispatcher disp;
    SearchProxy proxy(s.gameSender(), disp);

    SuccessReceiver recv;
    proxy.sig_success.add(&recv, &SuccessReceiver::onSuccess);
    proxy.search(SearchQuery(SearchQuery::MatchName, objects(), "a"), true);

    while (recv.list.size() == 0) {
        a.check("01. wait", disp.wait(1000));
    }

    // Verify that query has been stored
    a.checkEqual("11. savedQuery", SearchProxy::savedQuery(s.session()).getQuery(), "a");

    // Verify using proxy
    game::test::WaitIndicator ind;
    a.checkEqual("21. getSavedQuery", proxy.getSavedQuery(ind).getQuery(), "a");

    // Same thing again, now don't store
    recv.list.clear();
    proxy.search(SearchQuery(SearchQuery::MatchName, objects(), "b"), false);
    while (recv.list.size() == 0) {
        a.check("31. wait", disp.wait(1000));
    }
    a.checkEqual("32. savedQuery", SearchProxy::savedQuery(s.session()).getQuery(), "a");

    // Now, store again
    recv.list.clear();
    proxy.search(SearchQuery(SearchQuery::MatchName, objects(), "c"), true);
    while (recv.list.size() == 0) {
        a.check("41. wait", disp.wait(1000));
    }
    a.checkEqual("42. savedQuery", SearchProxy::savedQuery(s.session()).getQuery(), "c");
}
