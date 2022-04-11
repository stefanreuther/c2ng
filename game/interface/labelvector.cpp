/**
  *  \file game/interface/labelvector.cpp
  *  \brief Class game::interface::LabelVector
  */

#include "game/interface/labelvector.hpp"
#include "afl/base/deleter.hpp"
#include "afl/data/integervalue.hpp"
#include "game/map/object.hpp"
#include "interpreter/compilationcontext.hpp"
#include "interpreter/error.hpp"
#include "interpreter/expr/node.hpp"
#include "interpreter/expr/parser.hpp"
#include "interpreter/optimizer.hpp"
#include "interpreter/subroutinevalue.hpp"
#include "interpreter/tokenizer.hpp"
#include "interpreter/values.hpp"

using interpreter::BytecodeObject;
using interpreter::Opcode;

namespace {
    // Values for m_labelStatus
    const uint8_t LABEL_UPDATING = 1;
    const uint8_t LABEL_DIRTY    = 2;

    // Optimisation level for all compilation
    const int OPTIMIZATION_LEVEL = 1;

    /*
     *  Make function to recompute a unit's label.
     *  Essentially, this generates code for
     *
     *      Sub <anon>(Id)
     *        Try
     *          With <arrayFunction>(Id) Do <updateFunction>(Id, <compiledExpression>(), 1)
     *        Else
     *          <updateFunction>(Id, System.Err, 0)
     *        EndTry
     *      EndSub
     *
     *  but with in a little optimized way not expressible in actual scripts.
     *  In particular, the functions/expressions are baked in, so we don't have any namespace trouble.
     */
    interpreter::BCORef_t makeSingleUpdater(const interpreter::CallableValue& arrayFunction, const interpreter::CallableValue& updateFunction, interpreter::BCOPtr_t compiledExpression)
    {
        interpreter::BCORef_t bco = BytecodeObject::create(true);
        bco->addArgument("ID", false);

        if (compiledExpression.get() != 0) {
            // Expression given:
            //   <updateFunction>(ID, With(<arrayFunction>(ID), <compiledExpression>()), 1)
            //   <updateFunction>(ID, <Error>, 0)
            const BytecodeObject::Label_t lcatch = bco->makeLabel();
            const BytecodeObject::Label_t lfinally = bco->makeLabel();

            interpreter::SubroutineValue compiledExpressionValue(*compiledExpression);

            //    pushloc ID                              ID
            //    catch 1F
            //     pushloc ID                             ID:ID
            //     pushvar <arrayFunction>                ID:ID:<arrayFunction>
            //     funcind 1                              ID:<arrayFunction>(ID)
            //     swith                                  ID
            //      pushlit <compiledExpression>          ID:<compiledExpression>
            //      funcind 0                             ID:<compiledExpression>()
            //     sendwith
            //    suncatch
            //    pushlit 1                               ID:<compiledExpression>():1
            //    j 2F
            // 1:
            //    pushlit 0                               ID:<ERROR>:0
            // 2:
            //    pushvar <updateFunction>
            //    funcind 3
            bco->addInstruction(Opcode::maPush, Opcode::sLocal, 0);
            bco->addJump(Opcode::jCatch, lcatch);
            bco->addInstruction(Opcode::maPush, Opcode::sLocal, 0);
            bco->addPushLiteral(&arrayFunction);
            bco->addInstruction(Opcode::maIndirect, Opcode::miIMLoad, 1);
            bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialWith, 0);
            bco->addPushLiteral(&compiledExpressionValue);
            bco->addInstruction(Opcode::maIndirect, Opcode::miIMLoad, 0);
            bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialEndWith, 0);
            bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialUncatch, 0);
            bco->addInstruction(Opcode::maPush, Opcode::sInteger, 1);
            bco->addJump(Opcode::jAlways, lfinally);
            bco->addLabel(lcatch);
            bco->addInstruction(Opcode::maPush, Opcode::sInteger, 0);
            bco->addLabel(lfinally);
            bco->addPushLiteral(&updateFunction);
            bco->addInstruction(Opcode::maIndirect, Opcode::miIMLoad, 3);
        } else {
            // No expression given: <updateFunction>(ID, Z(0), 1)
            bco->addInstruction(Opcode::maPush, Opcode::sLocal, 0);
            bco->addPushLiteral(0);
            bco->addInstruction(Opcode::maPush, Opcode::sInteger, 1);
            bco->addPushLiteral(&updateFunction);
            bco->addInstruction(Opcode::maIndirect, Opcode::miIMLoad, 3);
        }
        bco->relocate();
        return bco;
    }
}

/*
 *  LabelVector
 */

game::interface::LabelVector::LabelVector()
    : m_labelValues(1),
      m_labelStatus(1),
      m_hasDirtyLabels(false),
      m_hasUpdatingLabels(false),
      m_hasChangedLabels(false),
      m_hasSuccess(false),
      m_hasError(false),
      m_expressionState(ExpressionEmpty),
      m_expression(),
      m_expressionError(),
      m_compiledExpression()
{ }

game::interface::LabelVector::~LabelVector()
{ }

void
game::interface::LabelVector::clear()
{
    m_labelValues.clear();
    m_labelStatus.clear();
    m_hasDirtyLabels = false;
    m_hasUpdatingLabels = false;
    m_hasChangedLabels = false;
}

String_t
game::interface::LabelVector::getLabel(Id_t id) const
{
    // ex getLabel(LabelKind kind, int id)
    return m_labelValues.get(id);
}

void
game::interface::LabelVector::checkObjects(game::map::ObjectType& ty)
{
    for (Id_t id = ty.findNextIndex(0); id != 0; id = ty.findNextIndex(id)) {
        if (const game::map::Object* obj = ty.getObjectByIndex(id)) {
            if (obj->isDirty()) {
                // Mark it dirty only if it is not updating to avoid loops
                // if the update expression marks it dirty again.
                if (m_labelStatus.get(id) == 0) {
                    m_labelStatus.set(id, LABEL_DIRTY);
                    m_hasDirtyLabels = true;
                }
            }
        }
    }
}

void
game::interface::LabelVector::markObjects(game::map::ObjectType& ty)
{
    for (Id_t id = ty.findNextIndex(0); id != 0; id = ty.findNextIndex(id)) {
        if (ty.getObjectByIndex(id) != 0) {
            m_labelStatus.set(id, m_labelStatus.get(id) | LABEL_DIRTY);
            m_hasDirtyLabels = true;
        }
    }
}

void
game::interface::LabelVector::markClean()
{
    for (Id_t id = 1, n = m_labelStatus.size(); id < n; ++id) {
        m_labelStatus.set(id, uint8_t(m_labelStatus.get(id) & ~LABEL_DIRTY));
    }
    m_hasDirtyLabels = false;
}

void
game::interface::LabelVector::updateLabel(Id_t id, bool success, String_t value)
{
    if (success) {
        m_hasSuccess = true;
        setLabel(id, value);
    } else {
        setLabel(id, String_t());
        m_hasError = true;
        m_lastError = value;
    }
}

bool
game::interface::LabelVector::hasDirtyLabels() const
{
    return m_hasDirtyLabels;
}

bool
game::interface::LabelVector::hasUpdatingLabels() const
{
    return m_hasUpdatingLabels;
}

bool
game::interface::LabelVector::hasChangedLabels() const
{
    return m_hasChangedLabels;
}

void
game::interface::LabelVector::markLabelsUnchanged()
{
    m_hasChangedLabels = false;
}

void
game::interface::LabelVector::setExpression(String_t expr, interpreter::World& world)
{
    // ex setLabelExpr (part)
    try {
        interpreter::Tokenizer tok(expr);
        if (tok.getCurrentToken() != interpreter::Tokenizer::tEnd) {
            // Expression given, parse it
            afl::base::Deleter del;
            const interpreter::expr::Node& expr = interpreter::expr::Parser(tok, del).parse();
            if (tok.getCurrentToken() != interpreter::Tokenizer::tEnd) {
                throw interpreter::Error::garbageAtEnd(true);
            }

            interpreter::BCORef_t bco = BytecodeObject::create(false);
            expr.compileValue(*bco, interpreter::CompilationContext(world));

            if (OPTIMIZATION_LEVEL > 0) {
                optimize(world, *bco, OPTIMIZATION_LEVEL);
            }
            if (OPTIMIZATION_LEVEL >= 0) {
                bco->relocate();
            }

            // Set state: compiled successfully
            m_expressionState = ExpressionCompiled;
            m_expressionError = String_t();
            m_compiledExpression = bco.asPtr();
        } else {
            // Set state: empty
            m_expressionState = ExpressionEmpty;
            m_expressionError = String_t();
            m_compiledExpression = 0;
        }
    }
    catch (interpreter::Error& e) {
        // Set state: error
        m_expressionState = ExpressionError;
        m_expressionError = e.what();
        m_compiledExpression = 0;
    }
    m_expression = expr;
}

String_t
game::interface::LabelVector::getExpression() const
{
    // ex getLabelExpr(LabelKind kind)
    return m_expression;
}

int
game::interface::LabelVector::compileUpdater(interpreter::BytecodeObject& bco, const interpreter::CallableValue& arrayFunction, const interpreter::CallableValue& updateFunction)
{
    int count = 0;
    interpreter::SubroutineValue singleUpdater(makeSingleUpdater(arrayFunction, updateFunction, m_compiledExpression));
    for (Id_t id = 1, n = m_labelStatus.size(); id < n; ++id) {
        if ((m_labelStatus.get(id) & LABEL_DIRTY) != 0) {
            m_labelStatus.set(id, LABEL_UPDATING); // clears LABEL_DIRTY!
            m_hasUpdatingLabels = true;

            afl::data::IntegerValue iv(id);
            bco.addPushLiteral(&iv);
            bco.addPushLiteral(&singleUpdater);
            bco.addInstruction(Opcode::maIndirect, Opcode::miIMLoad, 1);
            ++count;
        }
    }
    m_hasDirtyLabels = false;
    return count;
}

void
game::interface::LabelVector::finishUpdate()
{
    for (Id_t id = 1, n = m_labelStatus.size(); id < n; ++id) {
        m_labelStatus.set(id, uint8_t(m_labelStatus.get(id) & ~LABEL_UPDATING));
    }
    m_hasUpdatingLabels = false;
}

bool
game::interface::LabelVector::hasError() const
{
    return m_expressionState == ExpressionError || (m_hasError && !m_hasSuccess);
}

String_t
game::interface::LabelVector::getLastError() const
{
    return m_expressionState == ExpressionError
        ? m_expressionError
        : m_lastError;
}

void
game::interface::LabelVector::clearErrorStatus()
{
    m_hasSuccess = false;
    m_hasError = false;
}

void
game::interface::LabelVector::setLabel(Id_t id, const String_t& value)
{
    if (m_labelValues.get(id) != value) {
        m_labelValues.set(id, value);
        m_hasChangedLabels = true;
    }
}
