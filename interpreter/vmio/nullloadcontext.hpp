/**
  *  \file interpreter/vmio/nullloadcontext.hpp
  */
#ifndef C2NG_INTERPRETER_VMIO_NULLLOADCONTEXT_HPP
#define C2NG_INTERPRETER_VMIO_NULLLOADCONTEXT_HPP

#include "interpreter/vmio/loadcontext.hpp"

namespace interpreter { namespace vmio {

    /** A Null LoadContext.
        Refuses loading any structured data.
        A ValueLoader using this LoadContext will load only scalar data, no structures, contexts or processes. */
    class NullLoadContext : public LoadContext {
     public:
        virtual ~NullLoadContext();
        virtual afl::data::Value* loadBCO(uint32_t id);
        virtual afl::data::Value* loadArray(uint32_t id);
        virtual afl::data::Value* loadHash(uint32_t id);
        virtual afl::data::Value* loadStructureValue(uint32_t id);
        virtual afl::data::Value* loadStructureType(uint32_t id);
        virtual interpreter::Context* loadContext(const interpreter::TagNode& tag, afl::io::Stream& aux);
        virtual interpreter::Context* loadMutex(const String_t& name, const String_t& note, interpreter::Process* owner);
        virtual interpreter::Process* createProcess();
    };


} }

#endif
