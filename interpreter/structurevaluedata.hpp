/**
  *  \file interpreter/structurevaluedata.hpp
  *  \brief Class interpreter::StructureValueData
  */
#ifndef C2NG_INTERPRETER_STRUCTUREVALUEDATA_HPP
#define C2NG_INTERPRETER_STRUCTUREVALUEDATA_HPP

#include "afl/base/refcounted.hpp"
#include "afl/data/segment.hpp"
#include "interpreter/structuretypedata.hpp"

namespace interpreter {

    /** Structure value.
        This contains the data for a structure value:
        - link to structure type (=name/slot mapping), shared by all values of that type
        - values for this instance */
    class StructureValueData : public afl::base::RefCounted {
     public:
        typedef afl::base::Ref<StructureValueData> Ref_t;

        /** Constructor.
            Makes an empty structure of the specified type.
            \param type */
        StructureValueData(StructureTypeData::Ref_t type);

        /** Destructor. */
        ~StructureValueData();

        /** Access underlying type.
            \return type */
        StructureTypeData& type() const
            { return *m_type; }

        /** Access data.
            \return data */
        afl::data::Segment& data()
            { return m_data; }
        const afl::data::Segment& data() const
            { return m_data; }

        /** Change type.
            This function is intended to be used while loading ONLY.
            If we encounter a forward reference to a structure value,
            we need to create that value with a dummy type, and replace that by the correct type later.

            This function is not to be used in normal operation
            (which is why it's called changeType(), not setType() like a regular setter).

            \param type New type */
        void changeType(const StructureTypeData::Ref_t& type);

     private:
        StructureTypeData::Ref_t m_type;
        afl::data::Segment m_data;
    };

}

#endif
