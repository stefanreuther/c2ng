/**
  *  \file game/interface/loadcontext.hpp
  */
#ifndef C2NG_GAME_INTERFACE_LOADCONTEXT_HPP
#define C2NG_GAME_INTERFACE_LOADCONTEXT_HPP

#include "game/session.hpp"
#include "interpreter/vmio/loadcontext.hpp"

namespace game { namespace interface {

    class LoadContext : public interpreter::vmio::LoadContext {
     public:
        LoadContext(Session& session);
        ~LoadContext();

        virtual afl::data::Value* loadBCO(uint32_t id);
        virtual afl::data::Value* loadArray(uint32_t id);
        virtual afl::data::Value* loadHash(uint32_t id);
        virtual afl::data::Value* loadStructureValue(uint32_t id);
        virtual afl::data::Value* loadStructureType(uint32_t id);
        virtual interpreter::Context* loadContext(const interpreter::TagNode& tag, afl::io::Stream& aux);
        virtual interpreter::Context* loadMutex(const String_t& name, const String_t& note);
        virtual interpreter::Process* createProcess();
        virtual void finishProcess(interpreter::Process& proc);

     private:
        Session& m_session;
    };

} }

#endif
