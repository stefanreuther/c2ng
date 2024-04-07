/**
  *  \file interpreter/vmio/processloadcontext.hpp
  *  \brief Class interpreter::vmio::ProcessLoadContext
  */
#ifndef C2NG_INTERPRETER_VMIO_PROCESSLOADCONTEXT_HPP
#define C2NG_INTERPRETER_VMIO_PROCESSLOADCONTEXT_HPP

#include "interpreter/process.hpp"
#include "interpreter/vmio/loadcontext.hpp"

namespace interpreter { namespace vmio {

    /** LoadContext for loading content of a process.
        In particular, satisfies loading of Frame and Mutex context references
        using the given process. */
    class ProcessLoadContext : public LoadContext {
     public:
        /** Constructor.
            @param parent Parent LoadContext. Used to satisfy requests to complex data not associated with a process.
                          Must live as long as this ProcessLoadContext.
            @param proc   Process. Must live as long as this ProcessLoadContext. */
        ProcessLoadContext(LoadContext& parent, Process& proc);

        // LoadContext:
        virtual afl::data::Value* loadBCO(uint32_t id);
        virtual afl::data::Value* loadArray(uint32_t id);
        virtual afl::data::Value* loadHash(uint32_t id);
        virtual afl::data::Value* loadStructureValue(uint32_t id);
        virtual afl::data::Value* loadStructureType(uint32_t id);
        virtual Context* loadContext(const TagNode& tag, afl::io::Stream& aux);
        virtual Process* createProcess();
        virtual void finishProcess(Process& proc);

     private:
        LoadContext& m_parent;
        Process& m_process;
    };

} }

#endif
