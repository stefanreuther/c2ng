/**
  *  \file game/interface/beamfunction.hpp
  */
#ifndef C2NG_GAME_INTERFACE_BEAMFUNCTION_HPP
#define C2NG_GAME_INTERFACE_BEAMFUNCTION_HPP

#include "interpreter/indexablevalue.hpp"
#include "game/session.hpp"

namespace game { namespace interface {

    class BeamFunction : public interpreter::IndexableValue {
     public:
        BeamFunction(Session& session);

        // IndexableValue:
        virtual afl::data::Value* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, afl::data::Value* value);

        // CallableValue:
        virtual int32_t getDimension(int32_t which);
        virtual interpreter::Context* makeFirstContext();
        virtual BeamFunction* clone() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext* ctx) const;

     private:
        Session& m_session;
    };

} }

#endif
