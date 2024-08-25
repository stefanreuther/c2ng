/**
  *  \file game/interface/drawingfunction.hpp
  *  \brief Class game::interface::DrawingFunction
  */
#ifndef C2NG_GAME_INTERFACE_DRAWINGFUNCTION_HPP
#define C2NG_GAME_INTERFACE_DRAWINGFUNCTION_HPP

#include "game/interface/drawingcontext.hpp"
#include "game/session.hpp"
#include "interpreter/indexablevalue.hpp"

namespace game { namespace interface {

    /** Implementation of the "Drawing" function. */
    class DrawingFunction : public interpreter::IndexableValue {
     public:
        /** Constructor.
            @param session Session */
        explicit DrawingFunction(Session& session);

        // IndexableValue:
        virtual afl::data::Value* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, const afl::data::Value* value);

        // CallableValue:
        virtual size_t getDimension(size_t which) const;
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
