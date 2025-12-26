/**
  *  \file interpreter/assembler.hpp
  *  \brief Class interpreter::Assembler
  */
#ifndef C2NG_INTERPRETER_ASSEMBLER_HPP
#define C2NG_INTERPRETER_ASSEMBLER_HPP

#include <map>
#include "afl/base/types.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/io/textreader.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "interpreter/bytecodeobject.hpp"

namespace interpreter {

    class SaveContext;
    class BaseValue;

    /** Assembler.
        Compiles assembly-language code into input for a SaveContext.
        This class is mainly used for system testing of the interpreter;
        it should not be needed in the public end-user software.

        Usage:
        - construct
        - call compile()
        - call finish() to produce warnings/errors
        - call saveTo()

        @see interpreter::vmio::AssemblerSaveContext */
    class Assembler {
     public:
        class Tokenizer;

        /** Constructor.
            @param in Input */
        explicit Assembler(afl::io::TextReader& in);

        /** Destructor. */
        ~Assembler();

        /** Main entry point.
            Parses the input file and builds up result in memory.
            Call finish() to finish.
            @throw Error on error */
        void compile();

        /** Finish compilation.
            Checks that all declared elements are defined and writes appropriate log messages.
            @param log Logger
            @param tx  Translator
            @throw Error on error */
        void finish(afl::sys::LogListener& log, afl::string::Translator& tx);

        /** */
        BCORef_t saveTo(SaveContext& out);

     private:
        class Element;
        class BytecodeElement;
        class StructureElement;

        afl::io::TextReader& m_input;
        std::map<String_t, uint32_t> m_instructions;
        afl::container::PtrVector<Element> m_elements;
        std::map<String_t, Element*> m_elementsByName;
        BCOPtr_t m_lastCode;
        bool m_symbolicJumps;

        BytecodeObject& verifyLastCode();

        void handleDeclaration(Tokenizer& tok);
        void handleDefinition(Tokenizer& tok, bool isSub);
        void handleStructureDeclaration(Tokenizer& tok);
        void handleStructureDefinition(String_t name);
        void assemble(BytecodeObject& bco);
        uint16_t parseLiteral(BytecodeObject& bco, Tokenizer& tok);

        void initInstructions();
        uint32_t findInstruction(const String_t& name) const;
    };

}

#endif
