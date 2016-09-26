/**
  *  \file game/interface/shipfunction.hpp
  */
#ifndef C2NG_GAME_INTERFACE_SHIPFUNCTION_HPP
#define C2NG_GAME_INTERFACE_SHIPFUNCTION_HPP

#include "interpreter/indexablevalue.hpp"
#include "game/session.hpp"

namespace game { namespace interface {

    class ShipFunction : public interpreter::IndexableValue {
     public:
        ShipFunction(Session& session);

        // IndexableValue:
        virtual afl::data::Value* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, afl::data::Value* value);

        // CallableValue:
        virtual int32_t getDimension(int32_t which);
        virtual interpreter::Context* makeFirstContext();
        virtual ShipFunction* clone() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext* ctx) const;

     private:
        Session& m_session;
    };

} }

#endif
