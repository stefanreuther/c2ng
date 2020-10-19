/**
  *  \file interpreter/vmio/worldloadcontext.hpp
  *  \brief Class interpreter::vmio::WorldLoadContext
  */
#ifndef C2NG_INTERPRETER_VMIO_WORLDLOADCONTEXT_HPP
#define C2NG_INTERPRETER_VMIO_WORLDLOADCONTEXT_HPP

#include "interpreter/vmio/loadcontext.hpp"
#include "interpreter/world.hpp"
#include "interpreter/processlist.hpp"

namespace interpreter { namespace vmio {

    /** Load Context using a World: Loads Processes.
        To load an object file that can contain processes, use a WorldLoadContext with ObjectLoader.
        This will allow
        - loading of processes
        - loading of mutexes

        Application data (loadContext etc.) will be handled a parent LoadContext. */
    class WorldLoadContext : public LoadContext {
     public:
        /** Constructor.
            \param parent Parent context (mainly needed for loading application's contexts)
            \param processList ProcessList used to create processes
            \param world World used to create mutexes */
        WorldLoadContext(LoadContext& parent, ProcessList& processList, World& world);

        // LoadContext:
        virtual afl::data::Value* loadBCO(uint32_t id);
        virtual afl::data::Value* loadArray(uint32_t id);
        virtual afl::data::Value* loadHash(uint32_t id);
        virtual afl::data::Value* loadStructureValue(uint32_t id);
        virtual afl::data::Value* loadStructureType(uint32_t id);
        virtual Context* loadContext(const TagNode& tag, afl::io::Stream& aux);
        virtual Context* loadMutex(const String_t& name, const String_t& note, Process* owner);
        virtual Process* createProcess();
        virtual void finishProcess(Process& proc);

     private:
        LoadContext& m_parent;
        ProcessList& m_processList;
        World& m_world;
    };

} }

#endif
