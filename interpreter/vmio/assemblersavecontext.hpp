/**
  *  \file interpreter/vmio/assemblersavecontext.hpp
  */
#ifndef C2NG_INTERPRETER_VMIO_ASSEMBLERSAVECONTEXT_HPP
#define C2NG_INTERPRETER_VMIO_ASSEMBLERSAVECONTEXT_HPP

#include <vector>
#include <set>
#include "afl/io/textwriter.hpp"
#include "afl/container/ptrmap.hpp"
#include "interpreter/savecontext.hpp"
#include "afl/data/value.hpp"
#include "interpreter/opcode.hpp"

namespace interpreter { namespace vmio {

    class AssemblerSaveContext : public SaveContext {
     public:
        AssemblerSaveContext();
        ~AssemblerSaveContext();

        virtual uint32_t addBCO(const interpreter::BytecodeObject& bco);
        virtual uint32_t addHash(const afl::data::Hash& hash);
        virtual uint32_t addArray(const interpreter::ArrayData& array);
        virtual uint32_t addStructureType(const interpreter::StructureTypeData& type);
        virtual uint32_t addStructureValue(const interpreter::StructureValueData& value);
        virtual bool isCurrentProcess(const interpreter::Process* p);

        void save(afl::io::TextWriter& out);

        void setDebugInformation(bool flag);
        bool isDebugInformationEnabled() const;

     private:
        struct MetaObject {
            bool needDeclaration;
            bool isSequenced;
            String_t name;
            MetaObject()
                : needDeclaration(false),
                  isSequenced(false)
                { }
            virtual ~MetaObject() {}
            virtual void writeDeclaration(AssemblerSaveContext& asc, afl::io::TextWriter&) = 0;
            virtual void writeBody(AssemblerSaveContext& asc, afl::io::TextWriter&) = 0;
        };
        typedef afl::container::PtrMap<const void*,MetaObject> Map_t;
        Map_t m_metadata;
        std::vector<MetaObject*> m_sequence;
        std::set<String_t> m_usedNames;
        uint32_t m_counter;
        bool m_debugInformationEnabled;

        String_t formatLiteral(const afl::data::Value* value);
        String_t formatInstruction(const Opcode& opc, const BytecodeObject& bco);
        String_t formatSubroutineReference(BytecodeObject& bco);
        String_t formatStructureTypeReference(StructureTypeData& type);
        MetaObject* find(void* p) const;
    };

} }

#endif
