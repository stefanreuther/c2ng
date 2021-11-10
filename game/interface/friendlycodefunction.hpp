/**
  *  \file game/interface/friendlycodefunction.hpp
  */
#ifndef C2NG_GAME_INTERFACE_FRIENDLYCODEFUNCTION_HPP
#define C2NG_GAME_INTERFACE_FRIENDLYCODEFUNCTION_HPP

#include "game/session.hpp"
#include "interpreter/indexablevalue.hpp"
#include "game/interface/friendlycodecontext.hpp"

namespace game { namespace interface {

    class FriendlyCodeFunction : public interpreter::IndexableValue {
     public:
        FriendlyCodeFunction(Session& session);

        // IndexableValue:
        virtual FriendlyCodeContext* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, afl::data::Value* value);

        // CallableValue:
        virtual int32_t getDimension(int32_t which) const;
        virtual FriendlyCodeContext* makeFirstContext();
        virtual FriendlyCodeFunction* clone() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        Session& m_session;
    };

} }

#endif
