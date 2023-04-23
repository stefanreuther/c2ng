/**
  *  \file game/interface/planetfunction.hpp
  *  \brief Class game::interface::PlanetFunction
  */
#ifndef C2NG_GAME_INTERFACE_PLANETFUNCTION_HPP
#define C2NG_GAME_INTERFACE_PLANETFUNCTION_HPP

#include "game/interface/planetcontext.hpp"
#include "game/session.hpp"
#include "interpreter/indexablevalue.hpp"

namespace game { namespace interface {

    /** Implementation of "Planet()" function. */
    class PlanetFunction : public interpreter::IndexableValue {
     public:
        /** Constructor.
            @param session Session */
        explicit PlanetFunction(Session& session);

        // IndexableValue:
        virtual PlanetContext* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, const afl::data::Value* value);

        // CallableValue:
        virtual int32_t getDimension(int32_t which) const;
        virtual interpreter::Context* makeFirstContext();
        virtual PlanetFunction* clone() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        Session& m_session;
    };

} }

#endif
