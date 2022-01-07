/**
  *  \file game/searchquery.cpp
  *  \brief Class game::SearchQuery
  *
  *  FIXME: as of 20190710, the code this generates swallows all exceptions.
  *  PCC1/PCC2 capture error messages and try to identify a common one
  *  (i.e. "Unknown identifier: MISION").
  */

#include <memory>
#include "game/searchquery.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/string/parse.hpp"
#include "game/map/point.hpp"
#include "interpreter/error.hpp"
#include "interpreter/expr/node.hpp"
#include "interpreter/expr/parser.hpp"
#include "interpreter/optimizer.hpp"
#include "interpreter/subroutinevalue.hpp"
#include "util/string.hpp"

namespace {
    using interpreter::Opcode;
    using interpreter::BytecodeObject;

    /** Address of "obj" parameter to the function we create. */
    const uint16_t OBJARG_ADDR = 0;

    /** Optimisation level. */
    const int DEFAULT_OPTIMISATION_LEVEL = 2;



    /** Top half of a 'Try xxx' instruction.
        \param[out] bco Target BCO
        \return catchLabel parameter for endTry() */
    BytecodeObject::Label_t startTry(interpreter::BytecodeObject& bco)
    {
        // catch L1
        BytecodeObject::Label_t catchLabel = bco.makeLabel();
        bco.addJump(Opcode::jCatch, catchLabel);
        return catchLabel;
    }

    /** Bottom half of a 'Try xxx' instruction.
        \param[out] bco Target BCO
        \param[in] catchLabel result of startTry() */
    void endTry(interpreter::BytecodeObject& bco, BytecodeObject::Label_t catchLabel)
    {
        // This is almost the regular 'Try xxx' sequence.
        // However, we don't preserve the error.
        //     j L2
        //   L1:
        //     drop 1       (would normally be 'popvar SYSTEM.ERR')
        //   L2:
        BytecodeObject::Label_t endLabel = bco.makeLabel();
        bco.addJump(Opcode::jAlways, endLabel);
        bco.addLabel(catchLabel);
        bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
        bco.addLabel(endLabel);
    }

    /** Generate code to load an attribute of the object being looked at (obj->name).
        \param[out] bco  Target BCO
        \param[in]  name Attribute name */
    void loadAttribute(interpreter::BytecodeObject& bco, const char* name)
    {
        bco.addInstruction(Opcode::maPush, Opcode::sLocal, OBJARG_ADDR);
        bco.addInstruction(Opcode::maMemref, Opcode::miIMLoad, bco.addName(name));
    }

    /** Generate code to accept a match.
        If an expression "X" has been compiled, this turns it into "If X Then Return True".
        \param[out] bco  Target BCO */
    void checkMatch(interpreter::BytecodeObject& bco)
    {
        BytecodeObject::Label_t endLabel = bco.makeLabel();
        bco.addJump(Opcode::jIfFalse | Opcode::jIfEmpty | Opcode::jPopAlways, endLabel);
        bco.addInstruction(Opcode::maPush, Opcode::sBoolean, 1);
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialReturn, 1);
        bco.addLabel(endLabel);
    }

    /** Compile a "match any" query (empty search string).
        \param[out] bco  Target BCO */
    void compileMatchAny(interpreter::BytecodeObject& bco)
    {
        // Try Return Not IsEmpty(obj->Owner)
        BytecodeObject::Label_t catchLabel = startTry(bco);
        loadAttribute(bco, "OWNER$");
        bco.addInstruction(Opcode::maUnary, interpreter::unIsEmpty, 0);
        bco.addInstruction(Opcode::maUnary, interpreter::unNot, 0);
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialReturn, 1);
        endTry(bco, catchLabel);

        // Return true if we do not have an Owner attribute.
        // This applies to Ufos and Ion Storms.
        bco.addInstruction(Opcode::maPush, Opcode::sBoolean, 1);
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialReturn, 1);
    }

    /** Compile a "match name" query.
        \param[out] bco    Target BCO
        \param[in]  expr   Query string */
    void compileMatchName(interpreter::BytecodeObject& bco, const String_t& expr)
    {
        // If Id given,
        //    Try If Obj->Id = <id> Return True
        BytecodeObject::Label_t catchLabel;
        int id;
        if (afl::string::strToInteger(expr, id)
            || (expr[0] == '#' && afl::string::strToInteger(expr.substr(1), id)))
        {
            afl::data::IntegerValue iv(id);
            catchLabel = startTry(bco);
            loadAttribute(bco, "ID");
            bco.addPushLiteral(&iv);
            bco.addInstruction(Opcode::maBinary, interpreter::biCompareEQ_NC, 0);
            checkMatch(bco);
            endTry(bco, catchLabel);
        }

        // Match name:
        //   Try If InStr(obj->Name, <word>) Then Return True
        afl::data::StringValue sv(afl::string::strUCase(expr));
        catchLabel = startTry(bco);
        loadAttribute(bco, "NAME");
        bco.addPushLiteral(&sv);
        bco.addInstruction(Opcode::maBinary, interpreter::biFindStr_NC, 0);
        checkMatch(bco);
        endTry(bco, catchLabel);

        // Match comment:
        //   Try If InStr(obj->Comment, <word>) Then Return True
        catchLabel = startTry(bco);
        loadAttribute(bco, "COMMENT");
        bco.addPushLiteral(&sv);
        bco.addInstruction(Opcode::maBinary, interpreter::biFindStr_NC, 0);
        checkMatch(bco);
        endTry(bco, catchLabel);
    }

    /** Compile a "match expression" query.
        \param[out] bco    Target BCO
        \param[in]  expr   Query expression
        \param[in]  negate true for MatchFalse
        \param[in]  world  Interpreter world */
    void compileMatchExpression(interpreter::BytecodeObject& bco, const String_t& expr, bool negate, interpreter::World& world)
    {
        // FIXME: this uses compileValue() (same as PCC2) and therefore does not benefit
        // from the shorter code compileCondition() can create.

        // Parse expression
        interpreter::Tokenizer tok(expr);
        std::auto_ptr<interpreter::expr::Node> node(interpreter::expr::Parser(tok).parse());
        if (tok.getCurrentToken() != interpreter::Tokenizer::tEnd) {
            throw interpreter::Error::garbageAtEnd(true);
        }

        // Try With Obj Do If <expr> Then Return True
        BytecodeObject::Label_t catchLabel = startTry(bco);
        bco.addInstruction(Opcode::maPush, Opcode::sLocal, OBJARG_ADDR);
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialWith, 0);
        node->compileValue(bco, interpreter::CompilationContext(world));

        // Negate if necessary
        if (negate) {
            bco.addInstruction(Opcode::maUnary, interpreter::unNot2, 0);
        }

        checkMatch(bco);
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialEndWith, 0);
        endTry(bco, catchLabel);
    }

    /** Compile a "match location" query.
        \param[out] bco    Target BCO
        \param[in]  expr   Query ("X,Y" string) */
    void compileMatchLocation(interpreter::BytecodeObject& bco, const String_t& query)
    {
        // Parse coordinates
        game::map::Point pt;
        if (!pt.parseCoordinates(query)) {
            // FIXME: i18n
            throw interpreter::Error("Unable to parse coordinates");
        }

        // Try Return ObjectIsAt(obj, <x>, <y>)
        afl::data::IntegerValue x(pt.getX()), y(pt.getY());
        BytecodeObject::Label_t catchLabel = startTry(bco);
        bco.addInstruction(Opcode::maPush, Opcode::sLocal, OBJARG_ADDR);
        bco.addPushLiteral(&x);
        bco.addPushLiteral(&y);
        bco.addInstruction(Opcode::maPush, Opcode::sNamedShared, bco.addName("OBJECTISAT"));
        bco.addInstruction(Opcode::maIndirect, Opcode::miIMLoad, 3);
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialReturn, 1);
        endTry(bco, catchLabel);
    }
}



// Constructor.
game::SearchQuery::SearchQuery()
    : m_matchType(MatchName), m_objects(allObjects()), m_playedOnly(false), m_query(), m_optimisationLevel(DEFAULT_OPTIMISATION_LEVEL)
{
    // ex initSearch
}

// Construct query from parameters.
game::SearchQuery::SearchQuery(MatchType matchType, SearchObjects_t objs, String_t query)
    : m_matchType(matchType), m_objects(objs), m_playedOnly(false), m_query(query), m_optimisationLevel(DEFAULT_OPTIMISATION_LEVEL)
{ }

// Destructor.
game::SearchQuery::~SearchQuery()
{ }

// Set of all object types.
game::SearchQuery::SearchObjects_t
game::SearchQuery::allObjects()
{
    return SearchObjects_t()
        + SearchShips
        + SearchPlanets
        + SearchBases
        + SearchUfos
        + SearchOthers;
}

// Set match type.
void
game::SearchQuery::setMatchType(MatchType matchType)
{
    m_matchType = matchType;
}

// Get match type.
game::SearchQuery::MatchType
game::SearchQuery::getMatchType() const
{
    return m_matchType;
}

// Set set of objects.
void
game::SearchQuery::setSearchObjects(SearchObjects_t objs)
{
    m_objects = objs;
}

// Get set of objects.
game::SearchQuery::SearchObjects_t
game::SearchQuery::getSearchObjects() const
{
    return m_objects;
}

// Set query string.
void
game::SearchQuery::setQuery(String_t query)
{
    m_query = query;
}

// Get query string.
String_t
game::SearchQuery::getQuery() const
{
    return m_query;
}

// Get limitation to played objects.
void
game::SearchQuery::setPlayedOnly(bool flag)
{
    m_playedOnly = flag;
}

// Get limitation to played objects.
bool
game::SearchQuery::getPlayedOnly() const
{
    return m_playedOnly;
}

// Set optimisation level.
void
game::SearchQuery::setOptimisationLevel(int level)
{
    m_optimisationLevel = level;
}

// Get search objects as string.
String_t
game::SearchQuery::getSearchObjectsAsString() const
{
    String_t result;
    if (m_objects.contains(SearchShips)) {
        result += 's';
    }
    if (m_objects.contains(SearchPlanets)) {
        result += 'p';
    }
    if (m_objects.contains(SearchBases)) {
        result += 'b';
    }
    if (m_objects.contains(SearchUfos)) {
        result += 'u';
    }
    if (m_objects.contains(SearchOthers)) {
        result += 'o';
    }
    if (m_playedOnly) {
        result += 'm';
    }
    return result;
}

// Compile search expression into code.
interpreter::BCORef_t
game::SearchQuery::compileExpression(interpreter::World& world) const
{
    // ex client/search.cc:prepareSearchQuery
    // Create function:
    //    Function match(obj)
    interpreter::BCORef_t fun = interpreter::BytecodeObject::create(false);
    fun->addArgument("OBJ", false);
    fun->setSubroutineName("(Search Query)");

    // Create function body according to search type.
    // Each of these function bodies returns True on match.
    String_t expr = afl::string::strTrim(m_query);
    if (expr.empty()) {
        compileMatchAny(*fun);
    } else {
        switch (m_matchType) {
         case MatchName:
            compileMatchName(*fun, expr);
            break;

         case MatchTrue:
            compileMatchExpression(*fun, expr, false, world);
            break;

         case MatchFalse:
            compileMatchExpression(*fun, expr, true, world);
            break;

         case MatchLocation:
            compileMatchLocation(*fun, expr);
            break;
        }
    }

    // We end up here if the above does not match; return false
    fun->addInstruction(Opcode::maPush, Opcode::sBoolean, 0);
    fun->addInstruction(Opcode::maSpecial, Opcode::miSpecialReturn, 1);

    // Finalize the function
    if (m_optimisationLevel > 0) {
        optimize(world, *fun, m_optimisationLevel);
    }
    if (m_optimisationLevel >= 0) {
        fun->relocate();
    }
    return fun;
}

// Compile search query into code.
interpreter::BCORef_t
game::SearchQuery::compile(interpreter::World& world) const
{
    // Build a subroutine that executes CCUI$Search(flags, match).
    // CCUI$Search is defined in core.q.
    interpreter::BCORef_t fun = BytecodeObject::create(false);
    fun->setSubroutineName("(Search Query)");

    afl::data::StringValue flagValue(getSearchObjectsAsString());
    fun->addPushLiteral(&flagValue);

    interpreter::SubroutineValue matchValue(compileExpression(world));
    fun->addPushLiteral(&matchValue);

    fun->addInstruction(Opcode::maPush, Opcode::sNamedShared, fun->addName("CCUI$SEARCH"));
    fun->addInstruction(Opcode::maIndirect, Opcode::miIMLoad, 2);

    return fun;
}

String_t
game::SearchQuery::formatSearchObjects(SearchObjects_t objs, afl::string::Translator& tx)
{
    if (objs.contains(allObjects())) {
        return tx("all");
    } else if ((objs & allObjects()).empty()) {
        return tx("none");
    } else {
        String_t result;
        if (objs.contains(SearchShips)) {
            util::addListItem(result, ", ", tx("ships"));
        }
        if (objs.contains(SearchPlanets)) {
            util::addListItem(result, ", ", tx("planets"));
        } else if (objs.contains(SearchBases)) {
            util::addListItem(result, ", ", tx("starbases"));
        }
        if (objs.contains(SearchUfos)) {
            util::addListItem(result, ", ", tx("ufos"));
        }
        if (objs.contains(SearchOthers)) {
            util::addListItem(result, ", ", tx("others"));
        }
        return result;
    }
}
