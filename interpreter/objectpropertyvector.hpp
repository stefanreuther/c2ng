/**
  *  \file interpreter/objectpropertyvector.hpp
  *  \brief Class interpreter::ObjectPropertyVector
  */
#ifndef C2NG_INTERPRETER_OBJECTPROPERTYVECTOR_HPP
#define C2NG_INTERPRETER_OBJECTPROPERTYVECTOR_HPP

#include "afl/container/ptrvector.hpp"
#include "afl/data/segment.hpp"

namespace interpreter {

    /** Container for object properties.
        Objects (ships, planets) have properties that are managed separately.
        ObjectPropertyVector allows storing a afl::data::Segment for each object, identified by Id.

        Object properties are stored separately, not within the objects.
        Mainly, this allows us to separate interpreter and game code.
        It also separates global state (object properties) from turn state. */
    class ObjectPropertyVector {
     public:
        /** Constructor.
            Makes an empty vector. */
        ObjectPropertyVector();

        /** Destructor. */
        ~ObjectPropertyVector();

        /** Get or create a segment.
            Use this when you wish to set an object property.
            If the segment already exists, returns it; otherwise, creates one.
            \param id Object Id (> 0)
            \return segment; null if Id is out of range */
        afl::data::Segment* create(int id);

        /** Get segment.
            Use this when you wish to access an object property.
            \param id Object Id (> 0)
            \return segment; null if no segment exists or Id is out of range */
        afl::data::Segment* get(int id) const;

        /** Get value.
            This is a shortcut to getting a segment and indexing it.
            \param id Object Id (> 0)
            \param index Index into segment (property index)
            \return property value; null if property not set on this object or Id out of range */
        afl::data::Value* get(int id, afl::data::Segment::Index_t index) const;

        /** Clear.
            Forgets all content. */
        void clear();

     private:
        afl::container::PtrVector<afl::data::Segment> m_data;
    };

}

#endif
