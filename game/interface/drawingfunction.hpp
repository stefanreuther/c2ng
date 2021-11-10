/**
  *  \file game/interface/drawingfunction.hpp
  */
#ifndef C2NG_GAME_INTERFACE_DRAWINGFUNCTION_HPP
#define C2NG_GAME_INTERFACE_DRAWINGFUNCTION_HPP

#include "interpreter/indexablevalue.hpp"
#include "game/session.hpp"
#include "game/interface/drawingcontext.hpp"

namespace game { namespace interface {

    class DrawingFunction : public interpreter::IndexableValue {
     public:
        DrawingFunction(Session& session);

        // IndexableValue:
        virtual afl::data::Value* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, afl::data::Value* value);

        // CallableValue:
        virtual int32_t getDimension(int32_t which) const;
        virtual DrawingContext* makeFirstContext();
        virtual DrawingFunction* clone() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        Session& m_session;
    };

} }

#endif
