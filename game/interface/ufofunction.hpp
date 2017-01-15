/**
  *  \file game/interface/ufofunction.hpp
  */
#ifndef C2NG_GAME_INTERFACE_UFOFUNCTION_HPP
#define C2NG_GAME_INTERFACE_UFOFUNCTION_HPP

#include "interpreter/indexablevalue.hpp"
#include "game/session.hpp"
#include "game/interface/ufocontext.hpp"

namespace game { namespace interface {

    class UfoFunction : public interpreter::IndexableValue {
     public:
        UfoFunction(Session& session);

        // IndexableValue:
        virtual UfoContext* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, afl::data::Value* value);

        // CallableValue:
        virtual int32_t getDimension(int32_t which) const;
        virtual UfoContext* makeFirstContext();
        virtual UfoFunction* clone() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext& ctx) const;

     private:
        Session& m_session;
    };

} }

#endif
