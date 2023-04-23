/**
  *  \file game/interface/shipfunction.hpp
  *  \brief Class game::interface::ShipFunction
  */
#ifndef C2NG_GAME_INTERFACE_SHIPFUNCTION_HPP
#define C2NG_GAME_INTERFACE_SHIPFUNCTION_HPP

#include "game/interface/shipcontext.hpp"
#include "game/session.hpp"
#include "interpreter/indexablevalue.hpp"

namespace game { namespace interface {

    /** Implementation of the "Ship()" function. */
    class ShipFunction : public interpreter::IndexableValue {
     public:
        /** Constructor.
            @param session Session */
        explicit ShipFunction(Session& session);

        // IndexableValue:
        virtual ShipContext* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, const afl::data::Value* value);

        // CallableValue:
        virtual int32_t getDimension(int32_t which) const;
        virtual ShipContext* makeFirstContext();
        virtual ShipFunction* clone() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        Session& m_session;
    };

} }

#endif
