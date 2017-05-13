/**
  *  \file interpreter/vmio/processsavecontext.hpp
  *  \brief Class interpreter::vmio::ProcessSaveContext
  */
#ifndef C2NG_INTERPRETER_VMIO_PROCESSSAVECONTEXT_HPP
#define C2NG_INTERPRETER_VMIO_PROCESSSAVECONTEXT_HPP

#include "interpreter/savecontext.hpp"
#include "interpreter/process.hpp"

namespace interpreter { namespace vmio {

    /** Save context for a process.
        This class implements the isCurrentProcess() method for a given process,
        and can therefore be implemented to save data associated with that process (stack frames). */
    class ProcessSaveContext : public SaveContext {
     public:
        /** Constructor.
            \param parent Parent context. Must live at least as long as the ProcessSaveContext.
            \param process Process to work on. Must live at least as long as the ProcessSaveContext. */
        ProcessSaveContext(SaveContext& parent, const Process& process);

        // SaveContext:
        virtual uint32_t addBCO(const BytecodeObject& bco);
        virtual uint32_t addHash(const afl::data::Hash& hash);
        virtual uint32_t addArray(const ArrayData& array);
        virtual uint32_t addStructureType(const StructureTypeData& type);
        virtual uint32_t addStructureValue(const StructureValueData& value);
        virtual bool isCurrentProcess(const Process* p);

     private:
        SaveContext& m_parent;
        const Process& m_process;
    };

} }

#endif
