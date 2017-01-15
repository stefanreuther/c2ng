/**
  *  \file interpreter/vmio/filesavecontext.hpp
  */
#ifndef C2NG_INTERPRETER_VMIO_FILESAVECONTEXT_HPP
#define C2NG_INTERPRETER_VMIO_FILESAVECONTEXT_HPP

#include <map>
#include "interpreter/savecontext.hpp"
#include "afl/io/stream.hpp"
#include "afl/base/uncopyable.hpp"
#include "interpreter/process.hpp"

namespace interpreter { namespace vmio {

    class FileSaveContext : public SaveContext, private afl::base::Uncopyable {
     public:
        // FileSaveContext:
        explicit FileSaveContext(afl::charset::Charset& cs);
        ~FileSaveContext();

        void setDebugInformation(bool enable);

        size_t getNumPreparedObjects() const;
        void addProcess(Process& proc);

        void save(afl::io::Stream& out);

        // Conversion functions:
        uint32_t toWord(uint32_t value);
        uint32_t toWord(Opcode insn);
        uint32_t toWord(BCORef_t bco);

        // SaveContext:
        virtual uint32_t addBCO(BytecodeObject& bco);
        virtual uint32_t addHash(HashData& hash);
        virtual uint32_t addArray(ArrayData& array);
        virtual uint32_t addStructureType(StructureTypeData& type);
        virtual uint32_t addStructureValue(StructureValueData& value);
        virtual bool isCurrentProcess(Process* p);

     private:
        afl::charset::Charset& m_charset;
        bool m_debugInformationEnabled;

        // Map of object to Id. The object can be anything (process, bytecode, array, hash).
        std::map<void*,uint32_t> obj_to_id;

        // Object Id counter. Contains the last Id assigned.
        uint32_t obj_id_counter;

        // Save plan.
        enum PlanObject {
            poBytecode,
            poProcess,
            poArray,
            poHash,
            poStructType,
            poStructValue
        };
        std::vector<void*> plan_objs;
        std::vector<PlanObject> plan_types;

        // // Current process.
        // IntExecutionContext* current_process;

        void saveBCO(afl::io::Stream& out, const BytecodeObject& bco, uint32_t id);
        void saveHash(afl::io::Stream& out, const HashData& hash, uint32_t id);
        void saveArray(afl::io::Stream& out, const ArrayData& array, uint32_t id);
        void saveStructureType(afl::io::Stream& out, const StructureTypeData& type, uint32_t id);
        void saveStructureValue(afl::io::Stream& out, const StructureValueData& value, uint32_t id);

        void saveFrame(afl::io::Stream& out, const Process::Frame& fr);
        void saveProcess(afl::io::Stream& out, Process& proc);
    };

} }

#endif
