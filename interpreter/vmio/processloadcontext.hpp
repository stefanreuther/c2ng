/**
  *  \file interpreter/vmio/processloadcontext.hpp
  */
#ifndef C2NG_INTERPRETER_VMIO_PROCESSLOADCONTEXT_HPP
#define C2NG_INTERPRETER_VMIO_PROCESSLOADCONTEXT_HPP

#include "interpreter/vmio/loadcontext.hpp"
#include "interpreter/process.hpp"

namespace interpreter { namespace vmio {

    class ProcessLoadContext : public LoadContext {
     public:
        ProcessLoadContext(LoadContext& parent, Process& proc);

        virtual afl::data::Value* loadBCO(uint32_t id);
        virtual afl::data::Value* loadArray(uint32_t id);
        virtual afl::data::Value* loadHash(uint32_t id);
        virtual afl::data::Value* loadStructureValue(uint32_t id);
        virtual afl::data::Value* loadStructureType(uint32_t id);
        virtual Context* loadContext(const TagNode& tag, afl::io::Stream& aux);
        virtual Context* loadMutex(const String_t& name, const String_t& note);
        virtual Process* createProcess();
        virtual void finishProcess(Process& proc);

     private:
        LoadContext& m_parent;
        Process& m_process;
    };

} }

#endif
