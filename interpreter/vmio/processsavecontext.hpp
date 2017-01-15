/**
  *  \file interpreter/vmio/processsavecontext.hpp
  */
#ifndef C2NG_INTERPRETER_VMIO_PROCESSSAVECONTEXT_HPP
#define C2NG_INTERPRETER_VMIO_PROCESSSAVECONTEXT_HPP

#include "interpreter/savecontext.hpp"
#include "interpreter/process.hpp"

namespace interpreter { namespace vmio {

    class ProcessSaveContext : public SaveContext {
     public:
        ProcessSaveContext(SaveContext& parent, Process& process);

        virtual uint32_t addBCO(BytecodeObject& bco);
        virtual uint32_t addHash(HashData& hash);
        virtual uint32_t addArray(ArrayData& array);
        virtual uint32_t addStructureType(StructureTypeData& type);
        virtual uint32_t addStructureValue(StructureValueData& value);
        virtual bool isCurrentProcess(Process* p);

     private:
        SaveContext& m_parent;
        Process& m_process;
    };

} }

#endif
