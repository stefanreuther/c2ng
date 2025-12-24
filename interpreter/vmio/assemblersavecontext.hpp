/**
  *  \file interpreter/vmio/assemblersavecontext.hpp
  *  \brief Class interpreter::vmio::AssemblerSaveContext
  */
#ifndef C2NG_INTERPRETER_VMIO_ASSEMBLERSAVECONTEXT_HPP
#define C2NG_INTERPRETER_VMIO_ASSEMBLERSAVECONTEXT_HPP

#include <vector>
#include <set>
#include "afl/container/ptrmap.hpp"
#include "afl/data/value.hpp"
#include "afl/io/textwriter.hpp"
#include "interpreter/opcode.hpp"
#include "interpreter/savecontext.hpp"

namespace interpreter { namespace vmio {

    /** SaveContext that produces textual "assembler" output.
        Primarily, BCOs are output as sequence of disassembled instructions,
        with some meta-information attached.

        To use,
        - construct;
        - at any time, configure (setDebugInformation());
        - call addBCO() etc as needed;
        - produce output using save().

        The SaveContext methods (addBCO() etc.) schedule objects for output only.

        This feature is aimed at developers, not end-users. */
    class AssemblerSaveContext : public SaveContext {
     public:
        /** Constructor.
            Makes an empty object. */
        AssemblerSaveContext();

        /** Destructor. */
        ~AssemblerSaveContext();

        // SaveContext:
        virtual uint32_t addBCO(const BCORef_t& bco);
        virtual uint32_t addHash(const afl::data::Hash::Ref_t& hash);
        virtual uint32_t addArray(const ArrayData::Ref_t& array);
        virtual uint32_t addStructureType(const interpreter::StructureTypeData::Ref_t& type);
        virtual uint32_t addStructureValue(const interpreter::StructureValueData::Ref_t& value);
        virtual bool isCurrentProcess(const interpreter::Process* p);

        /** Generate output.
            This method produces output.
            @param out Output stream */
        void save(afl::io::TextWriter& out);

        /** Configure debug information.
            If enabled, debug information (".line" directives) are produced.
            @param flag Flag */
        void setDebugInformation(bool flag);

        /** Check for debug information.
            @return value of setDebugInformation() */
        bool isDebugInformationEnabled() const;

     private:
        /** Metadata about an object being output.
            Describes how an object can be declared if needed,
            and how its body can be formatted.
            Driver code will set its attributes. */
        struct MetaObject {
            bool needDeclaration;            ///< true if a declaration is needed (this object is needed before its body is written).
            bool isSequenced;                ///< true if this object is part of the output sequence already.
            String_t name;                   ///< Name of object.
            MetaObject()
                : needDeclaration(false),
                  isSequenced(false)
                { }
            virtual ~MetaObject() {}
            virtual void writeDeclaration(AssemblerSaveContext& asc, afl::io::TextWriter&) = 0;
            virtual void writeBody(AssemblerSaveContext& asc, afl::io::TextWriter&) = 0;
        };
        typedef afl::container::PtrMap<const void*,MetaObject> Map_t;

        /** Mapping of output objects to metadata.
            Keys are the addresses of BytecodeObject, Hash, etc.
            This container owns the MetaObjects.*/
        Map_t m_metadata;

        /** Objects in order scheduled for output.
            Each MetaObject in this sequence is contained in m_metadata.
            Objects in this vector have m_sequence=true.
            Objects may exist in m_metadata that are not yet sequenced here; those have m_sequence=false. */
        std::vector<MetaObject*> m_sequence;

        /** User-visible names.
            Used to assign unique names to the objects used in assembler output.
            On the CCScript side, objects may have identical names, e.g.
            "If cond / Sub NN / ... / EndSub / Else / Sub NN / ... / EndSub / EndIf".
            We must make sure that those can be distinguished on assembler level. */
        std::set<String_t> m_usedNames;

        /** Counter to generate names. */
        uint32_t m_counter;

        /** Debug information flag. */
        bool m_debugInformationEnabled;

        String_t formatLiteral(const afl::data::Value* value);
        String_t formatInstruction(const Opcode& opc, const BytecodeObject& bco);
        String_t formatSubroutineReference(BytecodeObject& bco);
        String_t formatStructureTypeReference(StructureTypeData& type);
        MetaObject* find(void* p) const;
    };

} }

#endif
