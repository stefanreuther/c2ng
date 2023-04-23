/**
  *  \file game/interface/ionstormfunction.hpp
  *  \brief Class game::interface::IonStormFunction
  */
#ifndef C2NG_GAME_INTERFACE_IONSTORMFUNCTION_HPP
#define C2NG_GAME_INTERFACE_IONSTORMFUNCTION_HPP

#include "interpreter/indexablevalue.hpp"
#include "game/session.hpp"

namespace game { namespace interface {

    /** Implementation of the "Storm" function. */
    class IonStormFunction : public interpreter::IndexableValue {
     public:
        /** Constructor.
            @param session Session */
        explicit IonStormFunction(Session& session);

        // IndexableValue:
        virtual interpreter::Context* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, const afl::data::Value* value);

        // CallableValue:
        virtual int32_t getDimension(int32_t which) const;
        virtual interpreter::Context* makeFirstContext();
        virtual IonStormFunction* clone() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        Session& m_session;
    };

} }

#endif
