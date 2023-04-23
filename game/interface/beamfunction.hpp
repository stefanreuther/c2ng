/**
  *  \file game/interface/beamfunction.hpp
  *  \brief Class game::interface::BeamFunction
  */
#ifndef C2NG_GAME_INTERFACE_BEAMFUNCTION_HPP
#define C2NG_GAME_INTERFACE_BEAMFUNCTION_HPP

#include "game/session.hpp"
#include "interpreter/indexablevalue.hpp"

namespace game { namespace interface {

    /** Implementation of the "Beam" function. */
    class BeamFunction : public interpreter::IndexableValue {
     public:
        /** Constructor.
            @param session Session */
        explicit BeamFunction(Session& session);

        // IndexableValue:
        virtual interpreter::Context* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, const afl::data::Value* value);

        // CallableValue:
        virtual int32_t getDimension(int32_t which) const;
        virtual interpreter::Context* makeFirstContext();
        virtual BeamFunction* clone() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        Session& m_session;
    };

} }

#endif
