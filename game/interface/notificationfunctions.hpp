/**
  *  \file game/interface/notificationfunctions.hpp
  */
#ifndef C2NG_GAME_INTERFACE_NOTIFICATIONFUNCTIONS_HPP
#define C2NG_GAME_INTERFACE_NOTIFICATIONFUNCTIONS_HPP

#include "interpreter/process.hpp"
#include "interpreter/arguments.hpp"
#include "game/session.hpp"
#include "interpreter/callablevalue.hpp"

namespace game { namespace interface {

    class NotifyConfirmedFunction : public interpreter::CallableValue {
     public:
        NotifyConfirmedFunction(Session& session);
        ~NotifyConfirmedFunction();

        // CallableValue:
        virtual void call(interpreter::Process& proc, afl::data::Segment& args, bool want_result);
        virtual bool isProcedureCall() const;
        virtual int32_t getDimension(int32_t which) const;
        virtual interpreter::Context* makeFirstContext();
        virtual NotifyConfirmedFunction* clone() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        Session& m_session;
    };

    void IFCCNotify(interpreter::Process& proc, game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFCCNumNotifications(game::Session& session, interpreter::Arguments& args);

} }

#endif
