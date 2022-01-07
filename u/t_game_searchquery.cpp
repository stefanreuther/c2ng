/**
  *  \file u/t_game_searchquery.cpp
  *  \brief Test for game::SearchQuery
  */

#include "game/searchquery.hpp"

#include "t_game.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/interface/simplefunction.hpp"
#include "game/session.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/process.hpp"
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
    void checkMatch(const SearchQuery& q, StructureValueData::Ref_t value, int expect)
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
            TSM_ASSERT_THROWS_NOTHING(q.getQuery().c_str(), p.run());
            TSM_ASSERT_EQUALS(q.getQuery().c_str(), interpreter::getBooleanValue(p.getResult()), expect);
        }

        // Same thing, without optimisation, because why not
        {
            interpreter::Process p(world, "name2", 22);
            SearchQuery q2(q);
            q2.setOptimisationLevel(-1);
            p.pushFrame(q2.compileExpression(world), true).localValues.pushBackNew(new StructureValue(value));
            TSM_ASSERT_THROWS_NOTHING(q2.getQuery().c_str(), p.run());
            TSM_ASSERT_EQUALS(q2.getQuery().c_str(), interpreter::getBooleanValue(p.getResult()), expect);
        }
    }

    /** Mock for OBJECTISAT function.
        Requires the object to be a struct starting with X,Y members.

        We re-use the game::interface::SimpleFunction wrapper which is an additional dependency
        SearchQuery normally doesn't need. */
    afl::data::Value* IFObjectIsAtMock(game::Session&, interpreter::Arguments& args)
    {
        // Verify that function is called correctly
        TS_ASSERT_EQUALS(args.getNumArgs(), 3U);

        StructureValue* sv = dynamic_cast<StructureValue*>(args.getNext());
        TS_ASSERT(sv != 0);

        int32_t xArg, yArg;
        TS_ASSERT(interpreter::checkIntegerArg(xArg, args.getNext()));
        TS_ASSERT(interpreter::checkIntegerArg(yArg, args.getNext()));

        // Obtain argument parameters
        int32_t xObj, yObj;
        TS_ASSERT(interpreter::checkIntegerArg(xObj, sv->getValue()->data.get(0)));
        TS_ASSERT(interpreter::checkIntegerArg(yObj, sv->getValue()->data.get(1)));

        // Produce return value
        return interpreter::makeBooleanValue((xArg == xObj) && (yArg == yObj));
    }
}

/** Test compilation and execution of some valid queries. */
void
TestGameSearchQuery::testCompileExpression()
{
    // Create a structure type
    StructureTypeData::Ref_t type(*new StructureTypeData());
    TS_ASSERT_EQUALS(type->names().add("NAME"),   0U);
    TS_ASSERT_EQUALS(type->names().add("ID"),     1U);
    TS_ASSERT_EQUALS(type->names().add("OWNER$"), 2U);         // Required for MatchAny

    // Create a value
    StructureValueData::Ref_t value(*new StructureValueData(type));
    value->data.setNew(0, interpreter::makeStringValue("Mambo #5"));
    value->data.setNew(1, interpreter::makeIntegerValue(42));
    value->data.setNew(2, interpreter::makeIntegerValue(3));

    // MatchAny
    checkMatch(SearchQuery(SearchQuery::MatchName,     SearchQuery::allObjects(), ""),      value, 1);
    checkMatch(SearchQuery(SearchQuery::MatchName,     SearchQuery::allObjects(), "    "),  value, 1);
    checkMatch(SearchQuery(SearchQuery::MatchTrue,     SearchQuery::allObjects(), ""),      value, 1);
    checkMatch(SearchQuery(SearchQuery::MatchLocation, SearchQuery::allObjects(), "    "),  value, 1);

    // Match name
    checkMatch(SearchQuery(SearchQuery::MatchName,     SearchQuery::allObjects(), "Mam"),   value, 1);    // Regular match
    checkMatch(SearchQuery(SearchQuery::MatchName,     SearchQuery::allObjects(), "mam"),   value, 1);    // Case-insensitive
    checkMatch(SearchQuery(SearchQuery::MatchName,     SearchQuery::allObjects(), "xyz"),   value, 0);    // Non-match
    checkMatch(SearchQuery(SearchQuery::MatchName,     SearchQuery::allObjects(), "5"),     value, 1);    // String match
    checkMatch(SearchQuery(SearchQuery::MatchName,     SearchQuery::allObjects(), "42"),    value, 1);    // Id match
    checkMatch(SearchQuery(SearchQuery::MatchName,     SearchQuery::allObjects(), "#5"),    value, 1);    // String match
    checkMatch(SearchQuery(SearchQuery::MatchName,     SearchQuery::allObjects(), "#42"),   value, 1);    // Id match
    checkMatch(SearchQuery(SearchQuery::MatchName,     SearchQuery::allObjects(), "#4"),    value, 0);    // Id match
    checkMatch(SearchQuery(SearchQuery::MatchName,     SearchQuery::allObjects(), "#  42"), value, 1);    // Id match

    // Match expression
    checkMatch(SearchQuery(SearchQuery::MatchTrue,     SearchQuery::allObjects(), "ID=42"), value, 1);
    checkMatch(SearchQuery(SearchQuery::MatchFalse,    SearchQuery::allObjects(), "ID=42"), value, 0);
    checkMatch(SearchQuery(SearchQuery::MatchTrue,     SearchQuery::allObjects(), "ID<42"), value, 0);
    checkMatch(SearchQuery(SearchQuery::MatchFalse,    SearchQuery::allObjects(), "ID<42"), value, 1);
}

/** Test compilation invalid queries. */
void
TestGameSearchQuery::testErrors()
{
    // Create a world
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    // Erroneous expressions
    // - compiler exceptions
    TS_ASSERT_THROWS(SearchQuery(SearchQuery::MatchTrue, SearchQuery::allObjects(), "ID=").compileExpression(world),    interpreter::Error);
    TS_ASSERT_THROWS(SearchQuery(SearchQuery::MatchFalse, SearchQuery::allObjects(), "ID=").compileExpression(world),   interpreter::Error);
    TS_ASSERT_THROWS(SearchQuery(SearchQuery::MatchFalse, SearchQuery::allObjects(), "ID)").compileExpression(world),   interpreter::Error);

    // - invalid X,Y
    TS_ASSERT_THROWS(SearchQuery(SearchQuery::MatchLocation, SearchQuery::allObjects(), "3").compileExpression(world),  interpreter::Error);
    TS_ASSERT_THROWS(SearchQuery(SearchQuery::MatchLocation, SearchQuery::allObjects(), "3,").compileExpression(world), interpreter::Error);
}

/** Test MatchLocation.
    This test needs a "OBJECTISAT" function. */
void
TestGameSearchQuery::testLocation()
{
    // Create a structure type
    StructureTypeData::Ref_t type(*new StructureTypeData());
    TS_ASSERT_EQUALS(type->names().add("X"), 0U);
    TS_ASSERT_EQUALS(type->names().add("Y"), 1U);

    // Create a value
    StructureValueData::Ref_t value(*new StructureValueData(type));
    value->data.setNew(0, interpreter::makeIntegerValue(777));
    value->data.setNew(1, interpreter::makeIntegerValue(888));

    // Create a world
    afl::sys::Log log;
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    interpreter::World world(log, tx, fs);
    game::Session session(tx, fs);              // required for SimpleFunction, not otherwise needed
    world.setNewGlobalValue("OBJECTISAT", new game::interface::SimpleFunction(session, IFObjectIsAtMock));

    // Verify
    // - match
    {
        SearchQuery q1(SearchQuery::MatchLocation, SearchQuery::allObjects(), "777, 888");
        interpreter::Process p(world, "name", 22);
        p.pushFrame(q1.compileExpression(world), true).localValues.pushBackNew(new StructureValue(value));
        TS_ASSERT_THROWS_NOTHING(p.run());
        TS_ASSERT_EQUALS(interpreter::getBooleanValue(p.getResult()), true);
    }
    // - mismatch
    {
        SearchQuery q2(SearchQuery::MatchLocation, SearchQuery::allObjects(), "666, 888");
        interpreter::Process p(world, "name", 22);
        p.pushFrame(q2.compileExpression(world), true).localValues.pushBackNew(new StructureValue(value));
        TS_ASSERT_THROWS_NOTHING(p.run());
        TS_ASSERT_EQUALS(interpreter::getBooleanValue(p.getResult()), false);
    }
}

/** Test accessors. */
void
TestGameSearchQuery::testAccessor()
{
    SearchQuery t1;
    TS_ASSERT_EQUALS(t1.getQuery(), "");
    TS_ASSERT_EQUALS(t1.getMatchType(), SearchQuery::MatchName);
    TS_ASSERT_EQUALS(t1.getSearchObjects(), SearchQuery::allObjects());
    TS_ASSERT_EQUALS(t1.getPlayedOnly(), false);
    TS_ASSERT_EQUALS(t1.getSearchObjectsAsString(), "spbuo");

    SearchQuery t2(SearchQuery::MatchLocation, SearchQuery::SearchObjects_t(), "x");
    TS_ASSERT_EQUALS(t2.getQuery(), "x");
    TS_ASSERT_EQUALS(t2.getMatchType(), SearchQuery::MatchLocation);
    TS_ASSERT_EQUALS(t2.getSearchObjects(), SearchQuery::SearchObjects_t());
    TS_ASSERT_EQUALS(t2.getPlayedOnly(), false);

    t1.setQuery("y");
    t1.setMatchType(SearchQuery::MatchFalse);
    t1.setSearchObjects(SearchQuery::SearchObjects_t(SearchQuery::SearchPlanets));
    t1.setPlayedOnly(true);
    TS_ASSERT_EQUALS(t1.getQuery(), "y");
    TS_ASSERT_EQUALS(t1.getMatchType(), SearchQuery::MatchFalse);
    TS_ASSERT_EQUALS(t1.getSearchObjects(), SearchQuery::SearchObjects_t(SearchQuery::SearchPlanets));
    TS_ASSERT_EQUALS(t1.getPlayedOnly(), true);
    TS_ASSERT_EQUALS(t1.getSearchObjectsAsString(), "pm");
}

/** Test formatSearchObjects(). */
void
TestGameSearchQuery::testFormat()
{
    using game::SearchQuery;
    afl::string::NullTranslator tx;

    // All or nothing
    TS_ASSERT_EQUALS(SearchQuery::formatSearchObjects(SearchQuery::allObjects(), tx), "all");
    TS_ASSERT_EQUALS(SearchQuery::formatSearchObjects(SearchQuery::SearchObjects_t(), tx), "none");

    // Singles
    SearchQuery::SearchObjects_t ss(SearchQuery::SearchShips);
    SearchQuery::SearchObjects_t pp(SearchQuery::SearchPlanets);
    SearchQuery::SearchObjects_t bb(SearchQuery::SearchBases);
    SearchQuery::SearchObjects_t uu(SearchQuery::SearchUfos);
    SearchQuery::SearchObjects_t oo(SearchQuery::SearchOthers);

    TS_ASSERT_EQUALS(SearchQuery::formatSearchObjects(ss, tx), "ships");
    TS_ASSERT_EQUALS(SearchQuery::formatSearchObjects(pp, tx), "planets");
    TS_ASSERT_EQUALS(SearchQuery::formatSearchObjects(bb, tx), "starbases");
    TS_ASSERT_EQUALS(SearchQuery::formatSearchObjects(uu, tx), "ufos");
    TS_ASSERT_EQUALS(SearchQuery::formatSearchObjects(oo, tx), "others");

    // Planets+bases shown as planets
    TS_ASSERT_EQUALS(SearchQuery::formatSearchObjects(pp + bb, tx), "planets");

    // Random combos
    TS_ASSERT_EQUALS(SearchQuery::formatSearchObjects(pp + ss, tx), "ships, planets");
    TS_ASSERT_EQUALS(SearchQuery::formatSearchObjects(uu + oo, tx), "ufos, others");
    TS_ASSERT_EQUALS(SearchQuery::formatSearchObjects(ss + pp + bb + uu, tx), "ships, planets, ufos");
    TS_ASSERT_EQUALS(SearchQuery::formatSearchObjects(ss + bb + uu, tx), "ships, starbases, ufos");
}

/** Test compile().
    compile() will cause CCUI$Search to be invoked and its value returned; test just that. */
void
TestGameSearchQuery::testCompile()
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
    TS_ASSERT_THROWS_NOTHING(p.run());

    int32_t iv;
    TS_ASSERT_EQUALS(interpreter::checkIntegerArg(iv, p.getResult()), true);
    TS_ASSERT_EQUALS(iv, 42);
}

