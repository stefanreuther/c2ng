/**
  *  \file game/interface/ufofunction.hpp
  *  \brief Class game::interface::UfoFunction
  */
#ifndef C2NG_GAME_INTERFACE_UFOFUNCTION_HPP
#define C2NG_GAME_INTERFACE_UFOFUNCTION_HPP

#include "game/interface/ufocontext.hpp"
#include "game/session.hpp"
#include "interpreter/indexablevalue.hpp"

namespace game { namespace interface {

    /** Implementation of the "Ufo" function. */
    class UfoFunction : public interpreter::IndexableValue {
     public:
        /** Constructor.
            @param session Session */
        explicit UfoFunction(Session& session);

        // IndexableValue:
        virtual UfoContext* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, afl::data::Value* value);

        // CallableValue:
        virtual int32_t getDimension(int32_t which) const;
        virtual UfoContext* makeFirstContext();
        virtual UfoFunction* clone() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        Session& m_session;
    };

} }

#endif
