/**
  *  \file game/interface/notificationfunctions.hpp
  *  \brief Notification-related script commands
  */
#ifndef C2NG_GAME_INTERFACE_NOTIFICATIONFUNCTIONS_HPP
#define C2NG_GAME_INTERFACE_NOTIFICATIONFUNCTIONS_HPP

#include "game/session.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/process.hpp"

namespace game { namespace interface {

    /** Implementation of the "CC$NotifyConfirmed()" function.

        Our simplified function implementation (SimpleFunction) does not give us a process,
        so we need to implement a full CallableValue here. */
    class NotifyConfirmedFunction : public interpreter::CallableValue {
     public:
        /** Constructor.
            @param session Session */
        explicit NotifyConfirmedFunction(Session& session);

        /** Destructor. */
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

    /** Implementation of the "CC$Notify" command.
        For use with SimpleProcedure<Session&>.
        @param session Session
        @param proc    Process
        @param args    Parameters */
    void IFCCNotify(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args);

    /** Implementation of the "CC$NumNotifications()" function.
        For use with SimpleFunction<Session&>.
        @param session Session
        @param args    Parameters
        @return newly-allocated values */
    afl::data::Value* IFCCNumNotifications(game::Session& session, interpreter::Arguments& args);

} }

#endif
