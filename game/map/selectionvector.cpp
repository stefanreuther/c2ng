/**
  *  \file game/map/selectionvector.cpp
  *  \brief game::map::SelectionVector
  */

#include "game/map/selectionvector.hpp"
#include "afl/base/memory.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/bits/bits.hpp"
#include "game/exception.hpp"
#include "game/map/object.hpp"
#include "interpreter/selectionexpression.hpp"
#include "util/translation.hpp"

namespace {
    void selectionError()
    {
        throw game::Exception("Invalid selection operation", _("Invalid selection operation"));
    }
}

// Constructor.
game::map::SelectionVector::SelectionVector()
    : m_data()
{
    // ex GSelection::GSelection
    static_assert(sizeof(Word_t) * 8 == NUM_BITS_PER_WORD, "sizeof SelectionVector::Word_t");
}

// Destructor.
game::map::SelectionVector::~SelectionVector()
{ }

// Clear.
void
game::map::SelectionVector::clear()
{
    // ex GSelection::clear
    m_data.clear();
}

// Merge.
void
game::map::SelectionVector::mergeFrom(const SelectionVector& other)
{
    size_t otherSize = other.m_data.size();
    if (otherSize > m_data.size()) {
        m_data.resize(otherSize);
    }
    for (size_t i = 0; i < otherSize; ++i) {
        m_data[i] |= other.m_data[i];
    }
}

// Initialize from ObjectType.
void
game::map::SelectionVector::copyFrom(ObjectType& type)
{
    // ex GSelection::copyFrom [part]
    afl::base::Memory<Word_t>(m_data).fill(0);
    for (Id_t i = type.getNextIndex(0); i != 0; i = type.getNextIndex(i)) {
        if (const Object* p = type.getObjectByIndex(i)) {
            if (p->isMarked()) {
                set(i, true);
            }
        }
    }
}

// Copy to universe.
void
game::map::SelectionVector::copyTo(ObjectType& type) const
{
    // ex GSelection::copyTo [part]
    for (Id_t i = type.getNextIndex(0); i != 0; i = type.getNextIndex(i)) {
        if (Object* p = type.getObjectByIndex(i)) {
            p->setIsMarked(get(i));
        }
    }
}

// Limit to existing objects.
void
game::map::SelectionVector::limitToExistingObjects(ObjectType& type)
{
    // ex GSelection::limitToExistingObjects [part]
    size_t limit = m_data.size() * NUM_BITS_PER_WORD;
    for (size_t i = 0; i < limit; ++i) {
        if (type.getObjectByIndex(Id_t(i)) == 0) {
            set(Id_t(i), 0);
        }
    }
}

// Get number of marked objects.
size_t
game::map::SelectionVector::getNumMarkedObjects() const
{
    // ex GSelection::getNumShips, GSelection::getNumPlanets [sort-of]
    size_t result = 0;
    for (size_t i = 0, n = m_data.size(); i < n; ++i) {
        result += afl::bits::bitPop(m_data[i]);
    }
    return result;
}

// Get status for single object.
bool
game::map::SelectionVector::get(Id_t id) const
{
    // ex GSelection::get [sort-of]
    if (id < 0) {
        return false;
    } else {
        size_t index = size_t(id) / NUM_BITS_PER_WORD;
        size_t bitNr = size_t(id) % NUM_BITS_PER_WORD;
        if (index < m_data.size()) {
            return (m_data[index] & (1 << bitNr)) != 0;
        } else {
            return false;
        }
    }
}

// Set status for single object.
void
game::map::SelectionVector::set(Id_t id, bool value)
{
    // ex GSelection::set [sort-of]
    if (id >= 0) {
        size_t index = size_t(id) / NUM_BITS_PER_WORD;
        size_t bitNr = size_t(id) % NUM_BITS_PER_WORD;
        if (index >= m_data.size() && value) {
            m_data.resize(index+1);
        }
        if (index < m_data.size()) {
            if (value) {
                m_data[index] |= (Word_t(1) << bitNr);
            } else {
                m_data[index] &= ~(Word_t(1) << bitNr);
            }
        }
    }
}

// Evaluate compiled expression.
void
game::map::SelectionVector::executeCompiledExpression(const String_t& compiledExpression,
                                                      size_t currentLayer,
                                                      const afl::base::Memory<SelectionVector>& otherVectors,
                                                      size_t limit,
                                                      bool isPlanet)
{
    // ex GMultiSelection::executeSelectionExpression
    using interpreter::SelectionExpression;

    // Determine size
    const size_t wordLimit = (limit / NUM_BITS_PER_WORD) + 1;
    m_data.resize(wordLimit);
    
    // Process
    for (size_t i = 0; i < wordLimit; ++i) {
        std::vector<Word_t> stack;
        for (size_t x = 0; x < compiledExpression.size(); ++x) {
            switch (compiledExpression[x]) {
             case SelectionExpression::opAnd:
                if (stack.size() < 2) {
                    selectionError();
                }
                stack[stack.size()-2] &= stack[stack.size()-1];
                stack.pop_back();
                break;
             case SelectionExpression::opOr:
                if (stack.size() < 2) {
                    selectionError();
                }
                stack[stack.size()-2] |= stack[stack.size()-1];
                stack.pop_back();
                break;
             case SelectionExpression::opXor:
                if (stack.size() < 2) {
                    selectionError();
                }
                stack[stack.size()-2] ^= stack[stack.size()-1];
                stack.pop_back();
                break;
             case SelectionExpression::opNot:
                if (stack.size() < 1) {
                    selectionError();
                }
                stack[stack.size()-1] = ~stack[stack.size()-1];
                break;
             case SelectionExpression::opCurrent:
                if (const SelectionVector* otherVector = otherVectors.at(currentLayer)) {
                    stack.push_back(otherVector->getWord(i));
                } else {
                    selectionError();
                }
                break;
             case SelectionExpression::opShip:
                stack.push_back(isPlanet ? Word_t(0) : Word_t(-1));
                break;
             case SelectionExpression::opPlanet:
                stack.push_back(isPlanet ? Word_t(-1) : Word_t(0));
                break;
             case SelectionExpression::opZero:
                stack.push_back(0);
                break;
             case SelectionExpression::opOne:
                stack.push_back(Word_t(-1));
                break;
             default:
                const SelectionVector* otherVector = (compiledExpression[x] >= SelectionExpression::opFirstLayer
                                                      ? otherVectors.at(compiledExpression[x] - SelectionExpression::opFirstLayer)
                                                      : 0);
                if (otherVector != 0) {
                    stack.push_back(otherVector->getWord(i));
                } else {
                    selectionError();
                }
                break;
            }
        }
        if (stack.size() != 1) {
            selectionError();
        }
        m_data[i] = stack[0];
    }
}

game::map::SelectionVector::Word_t
game::map::SelectionVector::getWord(size_t index) const
{
    // ex GSelection::operator[] [sort-of]
    return (index < m_data.size()
            ? m_data[index]
            : 0);
}
