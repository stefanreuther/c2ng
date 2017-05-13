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
        - link to structure type (=name/slot mapping)
        - values */
    class StructureValueData : public afl::base::RefCounted {
     public:
        typedef afl::base::Ref<StructureValueData> Ref_t;

        /** Constructor.
            Makes an empty structure of the specified type.
            \param type */
        StructureValueData(StructureTypeData::Ref_t type);

        /** Destructor. */
        ~StructureValueData();

        // FIXME: for loading we currently need to be able to modify a StructureValue's assigned type to resolve forward references.
        // Therefore, this remains as a public member for now.
        StructureTypeData::Ref_t type;
        afl::data::Segment data;
    };

}

#endif
