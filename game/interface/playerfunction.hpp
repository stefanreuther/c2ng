/**
  *  \file game/interface/playerfunction.hpp
  *  \brief Class game::interface::PlayerFunction
  */
#ifndef C2NG_GAME_INTERFACE_PLAYERFUNCTION_HPP
#define C2NG_GAME_INTERFACE_PLAYERFUNCTION_HPP

#include "game/session.hpp"
#include "interpreter/indexablevalue.hpp"

namespace game { namespace interface {

    /** Implementation of the "Player" function. */
    class PlayerFunction : public interpreter::IndexableValue {
     public:
        /** Constructor.
            @param session Session */
        explicit PlayerFunction(Session& session);

        // IndexableValue:
        virtual interpreter::Context* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, const afl::data::Value* value);

        // CallableValue:
        virtual int32_t getDimension(int32_t which) const;
        virtual interpreter::Context* makeFirstContext();
        virtual PlayerFunction* clone() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        Session& m_session;
    };

} }

#endif
