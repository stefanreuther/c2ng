/**
  *  \file game/interface/enginefunction.hpp
  *  \brief Class game::interface::EngineFunction
  */
#ifndef C2NG_GAME_INTERFACE_ENGINEFUNCTION_HPP
#define C2NG_GAME_INTERFACE_ENGINEFUNCTION_HPP

#include "game/session.hpp"
#include "interpreter/indexablevalue.hpp"

namespace game { namespace interface {

    /** Implementation of the "Engine" function. */
    class EngineFunction : public interpreter::IndexableValue {
     public:
        /** Constructor.
            @param session Session */
        explicit EngineFunction(Session& session);

        // IndexableValue:
        virtual interpreter::Context* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, const afl::data::Value* value);

        // CallableValue:
        virtual size_t getDimension(size_t which) const;
        virtual interpreter::Context* makeFirstContext();
        virtual EngineFunction* clone() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        Session& m_session;
    };

} }

#endif
