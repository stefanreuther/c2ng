/**
  *  \file interpreter/vmio/nullsavecontext.hpp
  *  \brief Class interpreter::vmio::NullSaveContext
  */
#ifndef C2NG_INTERPRETER_VMIO_NULLSAVECONTEXT_HPP
#define C2NG_INTERPRETER_VMIO_NULLSAVECONTEXT_HPP

#include "interpreter/savecontext.hpp"

namespace interpreter { namespace vmio {

    /** Implementation of SaveContext that fails every request.
        Thus, NullSaveContext can be used to serialize scalar data that has no inter-object links.
        This would be the case for starchart files. */
    class NullSaveContext : public SaveContext {
     public:
        virtual uint32_t addBCO(BytecodeObject& bco);
        virtual uint32_t addHash(HashData& hash);
        virtual uint32_t addArray(ArrayData& array);
        virtual uint32_t addStructureType(StructureTypeData& type);
        virtual uint32_t addStructureValue(StructureValueData& value);
        virtual bool isCurrentProcess(Process* p);
    };

} }

#endif
