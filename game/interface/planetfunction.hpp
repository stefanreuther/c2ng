/**
  *  \file game/interface/planetfunction.hpp
  */
#ifndef C2NG_GAME_INTERFACE_PLANETFUNCTION_HPP
#define C2NG_GAME_INTERFACE_PLANETFUNCTION_HPP

#include "interpreter/indexablevalue.hpp"
#include "game/session.hpp"

namespace game { namespace interface {

    class PlanetFunction : public interpreter::IndexableValue {
     public:
        PlanetFunction(Session& session);

        // IndexableValue:
        virtual afl::data::Value* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, afl::data::Value* value);

        // CallableValue:
        virtual int32_t getDimension(int32_t which);
        virtual interpreter::Context* makeFirstContext();
        virtual PlanetFunction* clone() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext* ctx) const;

     private:
        Session& m_session;
    };

} }

#endif
