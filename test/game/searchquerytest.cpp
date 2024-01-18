/**
  *  \file test/game/searchquerytest.cpp
  *  \brief Test for game::SearchQuery
  */

#include "game/searchquery.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/session.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/process.hpp"
#include "interpreter/simplefunction.hpp"
#include "interpreter/structuretypedata.hpp"
#include "interpreter/structurevalue.hpp"
#include "interpreter/structurevaluedata.hpp"
#include "interpreter/subroutinevalue.hpp"
#include "interpreter/values.hpp"
#include "interpreter/world.hpp"

using interpreter::StructureTypeData;
using interpreter::StructureValueData;
using interpreter::StructureValue;
using game::SearchQuery;

namespace {
    void checkMatch(afl::test::Assert a, const SearchQuery& q, StructureValueData::Ref_t value, int expect)
    {
        // Create a world
        afl::sys::Log log;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        interpreter::World world(log, tx, fs);

        // Compile and run
        {
            interpreter::Process p(world, "name", 22);
            p.pushFrame(q.compileExpression(world), true).localValues.pushBackNew(new StructureValue(value));
            AFL_CHECK_SUCCEEDS(a("01. run"), p.run());
            a.checkEqual("02. result", interpreter::getBooleanValue(p.getResult()), expect);
        }

        // Same thing, without optimisation, because why not
        {
            interpreter::Process p(world, "name2", 22);
            SearchQuery q2(q);
            q2.setOptimisationLevel(-1);
            p.pushFrame(q2.compileExpression(world), true).localValues.pushBackNew(new StructureValue(value));
            AFL_CHECK_SUCCEEDS(a("11. run"), p.run());
            a.checkEqual("12. result", interpreter::getBooleanValue(p.getResult()), expect);
        }
    }

    /** Mock for OBJECTISAT function.
        Requires the object to be a struct starting with X,Y members.

        We re-use the game::interface::SimpleFunction wrapper which is an additional dependency
        SearchQuery normally doesn't need. */
    afl::data::Value* IFObjectIsAtMock(afl::test::Assert a, interpreter::Arguments& args)
    {
        // Verify that function is called correctly
        a.checkEqual("01. getNumArgs", args.getNumArgs(), 3U);

        StructureValue* sv = dynamic_cast<StructureValue*>(args.getNext());
        a.checkNonNull("11. arg is StructureValue", sv);

        int32_t xArg, yArg;
        a.check("21. x arg", interpreter::checkIntegerArg(xArg, args.getNext()));
        a.check("22. y arg", interpreter::checkIntegerArg(yArg, args.getNext()));

        // Obtain argument parameters
        int32_t xObj, yObj;
        a.check("31. x member", interpreter::checkIntegerArg(xObj, sv->getValue()->data().get(0)));
        a.check("32. y member", interpreter::checkIntegerArg(yObj, sv->getValue()->data().get(1)));

        // Produce return value
        return interpreter::makeBooleanValue((xArg == xObj) && (yArg == yObj));
    }
}

/** Test compilation and execution of some valid queries. */
AFL_TEST("game.SearchQuery:compileExpression", a)
{
    // Create a structure type
    StructureTypeData::Ref_t type(*new StructureTypeData());
    a.checkEqual("01. NAME",   type->names().add("NAME"),   0U);
    a.checkEqual("02. ID",     type->names().add("ID"),     1U);
    a.checkEqual("03. OWNER$", type->names().add("OWNER$"), 2U);         // Required for MatchAny

    // Create a value
    StructureValueData::Ref_t value(*new StructureValueData(type));
    value->data().setNew(0, interpreter::makeStringValue("Mambo #5"));
    value->data().setNew(1, interpreter::makeIntegerValue(42));
    value->data().setNew(2, interpreter::makeIntegerValue(3));

    // MatchAny
    checkMatch(a("11. empty name"), SearchQuery(SearchQuery::MatchName,     SearchQuery::allObjects(), ""),      value, 1);
    checkMatch(a("12. empty name"), SearchQuery(SearchQuery::MatchName,     SearchQuery::allObjects(), "    "),  value, 1);
    checkMatch(a("13. empty expr"), SearchQuery(SearchQuery::MatchTrue,     SearchQuery::allObjects(), ""),      value, 1);
    checkMatch(a("14. empty loc"),  SearchQuery(SearchQuery::MatchLocation, SearchQuery::allObjects(), "    "),  value, 1);

    // Match name
    checkMatch(a("21. name"), SearchQuery(SearchQuery::MatchName,     SearchQuery::allObjects(), "Mam"),   value, 1);    // Regular match
    checkMatch(a("22. name"), SearchQuery(SearchQuery::MatchName,     SearchQuery::allObjects(), "mam"),   value, 1);    // Case-insensitive
    checkMatch(a("23. name"), SearchQuery(SearchQuery::MatchName,     SearchQuery::allObjects(), "xyz"),   value, 0);    // Non-match
    checkMatch(a("24. num"),  SearchQuery(SearchQuery::MatchName,     SearchQuery::allObjects(), "5"),     value, 1);    // String match
    checkMatch(a("25. num"),  SearchQuery(SearchQuery::MatchName,     SearchQuery::allObjects(), "42"),    value, 1);    // Id match
    checkMatch(a("26. id"),   SearchQuery(SearchQuery::MatchName,     SearchQuery::allObjects(), "#5"),    value, 1);    // String match
    checkMatch(a("27. id"),   SearchQuery(SearchQuery::MatchName,     SearchQuery::allObjects(), "#42"),   value, 1);    // Id match
    checkMatch(a("28. id"),   SearchQuery(SearchQuery::MatchName,     SearchQuery::allObjects(), "#4"),    value, 0);    // Id match
    checkMatch(a("29. id"),   SearchQuery(SearchQuery::MatchName,     SearchQuery::allObjects(), "#  42"), value, 1);    // Id match

    // Match expression
    checkMatch(a("31. expr"), SearchQuery(SearchQuery::MatchTrue,     SearchQuery::allObjects(), "ID=42"), value, 1);
    checkMatch(a("32. expr"), SearchQuery(SearchQuery::MatchFalse,    SearchQuery::allObjects(), "ID=42"), value, 0);
    checkMatch(a("33. expr"), SearchQuery(SearchQuery::MatchTrue,     SearchQuery::allObjects(), "ID<42"), value, 0);
    checkMatch(a("34. expr"), SearchQuery(SearchQuery::MatchFalse,    SearchQuery::allObjects(), "ID<42"), value, 1);
}

/** Test compilation invalid queries. */
AFL_TEST("game.SearchQuery:compileExpression:error", a)
{
    // Create a world
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    // Erroneous expressions
    // - compiler exceptions
    AFL_CHECK_THROWS(a("01. expr"), SearchQuery(SearchQuery::MatchTrue, SearchQuery::allObjects(), "ID=").compileExpression(world),    interpreter::Error);
    AFL_CHECK_THROWS(a("02. expr"), SearchQuery(SearchQuery::MatchFalse, SearchQuery::allObjects(), "ID=").compileExpression(world),   interpreter::Error);
    AFL_CHECK_THROWS(a("03. expr"), SearchQuery(SearchQuery::MatchFalse, SearchQuery::allObjects(), "ID)").compileExpression(world),   interpreter::Error);

    // - invalid X,Y
    AFL_CHECK_THROWS(a("11. pos"), SearchQuery(SearchQuery::MatchLocation, SearchQuery::allObjects(), "3").compileExpression(world),  interpreter::Error);
    AFL_CHECK_THROWS(a("12. pos"), SearchQuery(SearchQuery::MatchLocation, SearchQuery::allObjects(), "3,").compileExpression(world), interpreter::Error);
}

/** Test MatchLocation.
    This test needs a "OBJECTISAT" function. */
AFL_TEST("game.SearchQuery:compileExpression:MatchLocation", a)
{
    // Create a structure type
    StructureTypeData::Ref_t type(*new StructureTypeData());
    a.checkEqual("01. X", type->names().add("X"), 0U);
    a.checkEqual("02. Y", type->names().add("Y"), 1U);

    // Create a value
    StructureValueData::Ref_t value(*new StructureValueData(type));
    value->data().setNew(0, interpreter::makeIntegerValue(777));
    value->data().setNew(1, interpreter::makeIntegerValue(888));

    // Create a world
    afl::sys::Log log;
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    interpreter::World world(log, tx, fs);
    game::Session session(tx, fs);              // required for SimpleFunction, not otherwise needed
    world.setNewGlobalValue("OBJECTISAT", new interpreter::SimpleFunction<afl::test::Assert>(a("IFObjectIsAtMock"), IFObjectIsAtMock));

    // Verify
    // - match
    {
        SearchQuery q1(SearchQuery::MatchLocation, SearchQuery::allObjects(), "777, 888");
        interpreter::Process p(world, "name", 22);
        p.pushFrame(q1.compileExpression(world), true).localValues.pushBackNew(new StructureValue(value));
        AFL_CHECK_SUCCEEDS(a("11. run"), p.run());
        a.checkEqual("12. result", interpreter::getBooleanValue(p.getResult()), true);
    }
    // - mismatch
    {
        SearchQuery q2(SearchQuery::MatchLocation, SearchQuery::allObjects(), "666, 888");
        interpreter::Process p(world, "name", 22);
        p.pushFrame(q2.compileExpression(world), true).localValues.pushBackNew(new StructureValue(value));
        AFL_CHECK_SUCCEEDS(a("21. run"), p.run());
        a.checkEqual("22. result", interpreter::getBooleanValue(p.getResult()), false);
    }
}

/** Test accessors. */
AFL_TEST("game.SearchQuery:accessor", a)
{
    SearchQuery t1;
    a.checkEqual("01. getQuery",                 t1.getQuery(), "");
    a.checkEqual("02. getMatchType",             t1.getMatchType(), SearchQuery::MatchName);
    a.checkEqual("03. getSearchObjects",         t1.getSearchObjects(), SearchQuery::allObjects());
    a.checkEqual("04. getPlayedOnly",            t1.getPlayedOnly(), false);
    a.checkEqual("05. getSearchObjectsAsString", t1.getSearchObjectsAsString(), "spbuo");

    SearchQuery t2(SearchQuery::MatchLocation, SearchQuery::SearchObjects_t(), "x");
    a.checkEqual("11. getQuery",         t2.getQuery(), "x");
    a.checkEqual("12. getMatchType",     t2.getMatchType(), SearchQuery::MatchLocation);
    a.checkEqual("13. getSearchObjects", t2.getSearchObjects(), SearchQuery::SearchObjects_t());
    a.checkEqual("14. getPlayedOnly",    t2.getPlayedOnly(), false);

    t1.setQuery("y");
    t1.setMatchType(SearchQuery::MatchFalse);
    t1.setSearchObjects(SearchQuery::SearchObjects_t(SearchQuery::SearchPlanets));
    t1.setPlayedOnly(true);
    a.checkEqual("21. getQuery",                 t1.getQuery(), "y");
    a.checkEqual("22. getMatchType",             t1.getMatchType(), SearchQuery::MatchFalse);
    a.checkEqual("23. getSearchObjects",         t1.getSearchObjects(), SearchQuery::SearchObjects_t(SearchQuery::SearchPlanets));
    a.checkEqual("24. getPlayedOnly",            t1.getPlayedOnly(), true);
    a.checkEqual("25. getSearchObjectsAsString", t1.getSearchObjectsAsString(), "pm");
}

/** Test formatSearchObjects(). */
AFL_TEST("game.SearchQuery:formatSearchObjects", a)
{
    using game::SearchQuery;
    afl::string::NullTranslator tx;

    // All or nothing
    a.checkEqual("01. all",  SearchQuery::formatSearchObjects(SearchQuery::allObjects(), tx), "all");
    a.checkEqual("02. none", SearchQuery::formatSearchObjects(SearchQuery::SearchObjects_t(), tx), "none");

    // Singles
    SearchQuery::SearchObjects_t ss(SearchQuery::SearchShips);
    SearchQuery::SearchObjects_t pp(SearchQuery::SearchPlanets);
    SearchQuery::SearchObjects_t bb(SearchQuery::SearchBases);
    SearchQuery::SearchObjects_t uu(SearchQuery::SearchUfos);
    SearchQuery::SearchObjects_t oo(SearchQuery::SearchOthers);

    a.checkEqual("11. single", SearchQuery::formatSearchObjects(ss, tx), "ships");
    a.checkEqual("12. single", SearchQuery::formatSearchObjects(pp, tx), "planets");
    a.checkEqual("13. single", SearchQuery::formatSearchObjects(bb, tx), "starbases");
    a.checkEqual("14. single", SearchQuery::formatSearchObjects(uu, tx), "ufos");
    a.checkEqual("15. single", SearchQuery::formatSearchObjects(oo, tx), "others");

    // Planets+bases shown as planets
    a.checkEqual("21. planet+base", SearchQuery::formatSearchObjects(pp + bb, tx), "planets");

    // Random combos
    a.checkEqual("31. combo", SearchQuery::formatSearchObjects(pp + ss, tx), "ships, planets");
    a.checkEqual("32. combo", SearchQuery::formatSearchObjects(uu + oo, tx), "ufos, others");
    a.checkEqual("33. combo", SearchQuery::formatSearchObjects(ss + pp + bb + uu, tx), "ships, planets, ufos");
    a.checkEqual("34. combo", SearchQuery::formatSearchObjects(ss + bb + uu, tx), "ships, starbases, ufos");
}

/** Test compile().
    compile() will create code to invoke CCUI$Search; test just that. */
AFL_TEST("game.SearchQuery:compile", a)
{
    // Query
    game::SearchQuery testee(SearchQuery::MatchName, SearchQuery::allObjects(), "#77");

    // Create a world
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    // Create a binary function CCUI$Search that returns a constant value
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    bco->addArgument("A", false);
    bco->addArgument("B", false);
    bco->addInstruction(interpreter::Opcode::maPush, interpreter::Opcode::sInteger, 42);
    world.setNewGlobalValue("CCUI$SEARCH", new interpreter::SubroutineValue(bco));

    // Compile and run
    interpreter::Process p(world, "name", 22);
    p.pushFrame(testee.compile(world), true);
    AFL_CHECK_SUCCEEDS(a("01. run"), p.run());

    int32_t iv;
    a.checkEqual("11. result", interpreter::checkIntegerArg(iv, p.getResult()), true);
    a.checkEqual("12. result", iv, 42);
}
