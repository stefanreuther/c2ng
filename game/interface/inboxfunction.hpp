/**
  *  \file game/interface/inboxfunction.hpp
  */
#ifndef C2NG_GAME_INTERFACE_INBOXFUNCTION_HPP
#define C2NG_GAME_INTERFACE_INBOXFUNCTION_HPP

#include "interpreter/indexablevalue.hpp"
#include "game/session.hpp"

namespace game { namespace interface {

    class InboxFunction : public interpreter::IndexableValue {
     public:
        InboxFunction(Session& session);
        ~InboxFunction();

        // IndexableValue:
        virtual afl::data::Value* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, afl::data::Value* value);

        // CallableValue:
        virtual int32_t getDimension(int32_t which) const;
        virtual interpreter::Context* makeFirstContext();

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext& ctx) const;
        virtual InboxFunction* clone() const;

     private:
        Session& m_session;
    };


} }

#endif
