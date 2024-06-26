/**
  *  \file game/interface/loadcontext.hpp
  *  \brief Class game::interface::LoadContext
  */
#ifndef C2NG_GAME_INTERFACE_LOADCONTEXT_HPP
#define C2NG_GAME_INTERFACE_LOADCONTEXT_HPP

#include "game/session.hpp"
#include "interpreter/vmio/loadcontext.hpp"

namespace game { namespace interface {

    /** LoadContext implementation for game data.
        Allows loading of game-related values (ShipContext etc.),
        but not script data (BCOs, complex values, etc.); for those, returns null. */
    class LoadContext : public interpreter::vmio::LoadContext {
     public:
        /** Constructor.
            @param session Session */
        explicit LoadContext(Session& session);

        /** Destructor. */
        ~LoadContext();

        // LoadContext:
        virtual afl::data::Value* loadBCO(uint32_t id);
        virtual afl::data::Value* loadArray(uint32_t id);
        virtual afl::data::Value* loadHash(uint32_t id);
        virtual afl::data::Value* loadStructureValue(uint32_t id);
        virtual afl::data::Value* loadStructureType(uint32_t id);
        virtual interpreter::Context* loadContext(const interpreter::TagNode& tag, afl::io::Stream& aux);
        virtual interpreter::Process* createProcess();
        virtual void finishProcess(interpreter::Process& proc);

     private:
        Session& m_session;
    };

} }

#endif
