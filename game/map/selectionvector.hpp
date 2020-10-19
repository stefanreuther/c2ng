/**
  *  \file game/map/selectionvector.hpp
  *  \brief game::map::SelectionVector
  */
#ifndef C2NG_GAME_MAP_SELECTIONVECTOR_HPP
#define C2NG_GAME_MAP_SELECTIONVECTOR_HPP

#include "afl/base/types.hpp"
#include "game/map/objecttype.hpp"
#include "game/types.hpp"

namespace game { namespace map {

    /** Bit-set of marked objects (a selection).
        Stores an array of bits corresponding to marked objects from one ObjectType.

        Objects are identified by an Id.
        SelectionVector assumes that Ids are packed closely to be able to allocate an array,
        but has no built-in limit and adjusts to size as needed.

        Each object is marked (true) or unmarked (false). */
    class SelectionVector {
     public:
        /** Constructor.
            Makes an empty vector. */
        SelectionVector();

        /** Destructor. */
        ~SelectionVector();

        /** Clear.
            Set all values to zero (unmarked).
            \post get(x)==0 for all x */
        void clear();

        /** Merge.
            Add the other vector's data to this one.
            \post get(x)=true if get(x) was true before, or other.get(x) was true. */
        void mergeFrom(const SelectionVector& other);

        /** Initialize from ObjectType.
            Copies all isMarked() bits from the ObjectType into this SelectionVector.
            \param type ObjectType
            \post get(x) <=> type.get(x) && isMarked() */
        void copyFrom(ObjectType& type);

        /** Copy to universe.
            Sets all isMarked() bits of the ObjectType from this SelectionVector.
            \param type ObjectType
            \post p->isMarked(x) == get(p->getId()) */
        void copyTo(ObjectType& type) const;

        /** Limit to existing objects.
            Removes all objects that do not exist in the given ObjectType from this selection.
            \param type ObjectType */
        void limitToExistingObjects(ObjectType& type);

        /** Get number of marked objects.
            \return count */
        size_t getNumMarkedObjects() const;

        /** Get status for single object.
            \param id Id
            \return status */
        bool get(Id_t id) const;

        /** Set status for single object.
            \param id Id
            \param value New status */
        void set(Id_t id, bool value);

        /** Evaluate compiled expression.
            Replaces this SelectionVector's content with the result of the given expression.
            \param compiledExpression Compiled expression (see interpreter::SelectionExpression)
            \param currentLayer       Index for the current layer (index into otherVectors; for opCurrent)
            \param otherVectors       Other SelectionVector's (for layer references in expression)
            \param limit              Upper limit for Ids
            \param isPlanet           true if this vector represents planets (for SHIPS/PLANETS references) */
        void executeCompiledExpression(const String_t& compiledExpression,
                                       size_t currentLayer,
                                       const afl::base::Memory<SelectionVector>& otherVectors,
                                       size_t limit,
                                       bool isPlanet);

     private:
        // One word in the bit array
        typedef uint32_t Word_t;

        // Number of bits in the word
        static const size_t NUM_BITS_PER_WORD = 32;

        // Get word of representation. Out-of-range values handled.
        Word_t getWord(size_t index) const;

        // Representation.
        std::vector<Word_t> m_data;
    };

} }

#endif
