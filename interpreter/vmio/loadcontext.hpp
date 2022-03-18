/**
  *  \file interpreter/vmio/loadcontext.hpp
  *  \brief Interface interpreter::vmio::LoadContext
  */
#ifndef C2NG_INTERPRETER_VMIO_LOADCONTEXT_HPP
#define C2NG_INTERPRETER_VMIO_LOADCONTEXT_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/types.hpp"
#include "afl/data/value.hpp"
#include "afl/io/stream.hpp"
#include "interpreter/context.hpp"
#include "interpreter/tagnode.hpp"
#include "interpreter/process.hpp"

namespace interpreter { namespace vmio {

    /** Context for loading virtual machine data.
        All member functions have common behaviour:
        - if the desired object is not available or cannot be loaded, they return NULL.
          This causes the caller to refuse the data and fail the load.
        - the returned object may be a placeholder object that is not yet fully populated.
          This happens when forward references appear in a VM file.

        Responsibilities for loading mutexes are split into two halves:
        - the context knowing about the current process (ProcessLoadContext) implements loadContext() for mutexes
          because it knows how to deserialize mutex data and associate it with a process; it calls loadMutex()
          to actually create the mutex.
        - the context knowing about the script world (WorldLoadContext) implements loadMutex() to actually
          create the mutex. */
    class LoadContext : public afl::base::Deletable {
     public:
        /** Load BCO (SubroutineValue).
            \param id Object Id
            \return value or null */
        virtual afl::data::Value* loadBCO(uint32_t id) = 0;

        /** Load array (ArrayValue).
            \param id Object Id
            \return value or null */
        virtual afl::data::Value* loadArray(uint32_t id) = 0;

        /** Load hash (HashValue).
            \param id Object Id
            \return value or null */
        virtual afl::data::Value* loadHash(uint32_t id) = 0;

        /** Load structure value (StructureValue).
            \param id Object Id
            \return value or null */
        virtual afl::data::Value* loadStructureValue(uint32_t id) = 0;

        /** Load structure type (StructureType).
            \param id Object Id
            \return value or null */
        virtual afl::data::Value* loadStructureType(uint32_t id) = 0;

        /** Load context value.
            This loads all sorts of contexts.
            \param tag Tag node
            \param aux Auxiliary data can be read here
            \return context value or null */
        virtual Context* loadContext(const TagNode& tag, afl::io::Stream& aux) = 0;

        /** Load mutex context value.
            May throw if there is a mutex conflict.
            \param name Mutex name (identifier)
            \param note Mutex note
            \param owner Owning process or null
            \return context value or null */
        virtual Context* loadMutex(const String_t& name, const String_t& note, Process* owner) = 0;

        /** Create a process.
            \return newly-created process or null */
        virtual Process* createProcess() = 0;

        /** Finish a process.
            Must be called after a process created using createProcess() has been completed.
            \param proc Process */
        virtual void finishProcess(Process& proc) = 0;
    };

} }

#endif
