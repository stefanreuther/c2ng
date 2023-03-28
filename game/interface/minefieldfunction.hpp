/**
  *  \file game/interface/minefieldfunction.hpp
  *  \brief Class game::interface::MinefieldFunction
  */
#ifndef C2NG_GAME_INTERFACE_MINEFIELDFUNCTION_HPP
#define C2NG_GAME_INTERFACE_MINEFIELDFUNCTION_HPP

#include "interpreter/indexablevalue.hpp"
#include "game/session.hpp"

namespace game { namespace interface {

    /** Implementation of "Minefield()" function. */
    class MinefieldFunction : public interpreter::IndexableValue {
     public:
        /** Constructor.
            @param session Session */
        explicit MinefieldFunction(Session& session);

        // IndexableValue:
        virtual interpreter::Context* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, afl::data::Value* value);

        // CallableValue:
        virtual int32_t getDimension(int32_t which) const;
        virtual interpreter::Context* makeFirstContext();
        virtual MinefieldFunction* clone() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        Session& m_session;
    };

} }

#endif
