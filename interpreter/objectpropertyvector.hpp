/**
  *  \file interpreter/objectpropertyvector.hpp
  */
#ifndef C2NG_INTERPRETER_OBJECTPROPERTYVECTOR_HPP
#define C2NG_INTERPRETER_OBJECTPROPERTYVECTOR_HPP

#include "afl/container/ptrvector.hpp"
#include "afl/data/segment.hpp"

namespace interpreter {

    class ObjectPropertyVector {
     public:
        ObjectPropertyVector();

        ~ObjectPropertyVector();

        afl::data::Segment* create(int id);

        afl::data::Segment* get(int id) const;

        afl::data::Value* get(int id, afl::data::Segment::Index_t index) const;

        void clear();

     private:
        afl::container::PtrVector<afl::data::Segment> m_data;
    };

}

#endif
