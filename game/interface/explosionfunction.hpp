/**
  *  \file game/interface/explosionfunction.hpp
  *  \brief Class game::interface::ExplosionFunction
  */
#ifndef C2NG_GAME_INTERFACE_EXPLOSIONFUNCTION_HPP
#define C2NG_GAME_INTERFACE_EXPLOSIONFUNCTION_HPP

#include "game/interface/explosioncontext.hpp"
#include "interpreter/indexablevalue.hpp"

namespace game { namespace interface {

    /** Implementation of the "Explosion" function. */
    class ExplosionFunction : public interpreter::IndexableValue {
     public:
        /** Constructor.
            @param session Session */
        explicit ExplosionFunction(Session& session);

        // IndexableValue:
        virtual ExplosionContext* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, const afl::data::Value* value);

        // CallableValue:
        virtual size_t getDimension(size_t which) const;
        virtual ExplosionContext* makeFirstContext();
        virtual ExplosionFunction* clone() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        Session& m_session;
    };

} }

#endif
