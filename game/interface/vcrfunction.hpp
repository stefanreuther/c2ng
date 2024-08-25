/**
  *  \file game/interface/vcrfunction.hpp
  *  \brief Class game::interface::VcrFunction
  */
#ifndef C2NG_GAME_INTERFACE_VCRFUNCTION_HPP
#define C2NG_GAME_INTERFACE_VCRFUNCTION_HPP

#include "game/interface/vcrcontext.hpp"
#include "interpreter/indexablevalue.hpp"

namespace game { namespace interface {

    /** Implementation of the "Vcr()" function. */
    class VcrFunction : public interpreter::IndexableValue {
     public:
        /** Constructor.
            @param session Session */
        explicit VcrFunction(Session& session);

        // IndexableValue:
        virtual VcrContext* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, const afl::data::Value* value);

        // CallableValue:
        virtual size_t getDimension(size_t which) const;
        virtual VcrContext* makeFirstContext();
        virtual VcrFunction* clone() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        size_t getNumBattles() const;

        Session& m_session;
    };

} }

#endif
