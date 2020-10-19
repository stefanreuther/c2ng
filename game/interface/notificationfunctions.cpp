/**
  *  \file game/interface/notificationfunctions.cpp
  */

#include "game/interface/notificationfunctions.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/values.hpp"
#include "game/map/ship.hpp"
#include "game/map/planet.hpp"
#include "game/map/object.hpp"
#include "afl/string/format.hpp"

using afl::string::Format;
using interpreter::Error;

/*
 *  CC$NotifyConfirmed() function
 *
 *  SimpleFunction does not give us a process, so we need to implement a full CallableValue here.
 */

game::interface::NotifyConfirmedFunction::NotifyConfirmedFunction(Session& session)
    : m_session(session)
{
    // ex IntCCNotifyConfirmed::IntCCNotifyConfirmed
}

game::interface::NotifyConfirmedFunction::~NotifyConfirmedFunction()
{
    // ex IntCCNotifyConfirmed::~IntCCNotifyConfirmed
}

void
game::interface::NotifyConfirmedFunction::call(interpreter::Process& proc, afl::data::Segment& args, bool want_result)
{
    // ex IntCCNotifyConfirmed::call
    interpreter::checkArgumentCount(args.size(), 0, 0);

    if (want_result) {
        NotificationStore& ns = m_session.notifications();
        NotificationStore::Message* msg = ns.findMessageByProcessId(proc.getProcessId());
        proc.pushNewValue(interpreter::makeBooleanValue(ns.isMessageConfirmed(msg)));
    }
}

bool
game::interface::NotifyConfirmedFunction::isProcedureCall() const
{
    // ex IntCCNotifyConfirmed::isProcedureCall
    return false;
}

int32_t
game::interface::NotifyConfirmedFunction::getDimension(int32_t /*which*/) const
{
    // ex IntCCNotifyConfirmed::getDimension
    return 0;
}

interpreter::Context*
game::interface::NotifyConfirmedFunction::makeFirstContext()
{
    // ex IntCCNotifyConfirmed::makeFirstContext
    throw Error::typeError(Error::ExpectIterable);
}

game::interface::NotifyConfirmedFunction*
game::interface::NotifyConfirmedFunction::clone() const
{
    // ex IntCCNotifyConfirmed::clone
    return new NotifyConfirmedFunction(m_session);
}

// BaseValue:
String_t
game::interface::NotifyConfirmedFunction::toString(bool /*readable*/) const
{
    // ex IntCCNotifyConfirmed::toString
    return "CC$NotifyConfirmed";
}

void
game::interface::NotifyConfirmedFunction::store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
{
    // ex IntCCNotifyConfirmed::store
    throw Error::notSerializable();
}

/*
 *  CC$Notify
 */

/* @q CC$Notify msg:Str, associateWithProcess:Bool (Internal)
   Back-end to {Notify} and {AddNotify}.
   @since PCC2 2.40.8 */
void
game::interface::IFCCNotify(interpreter::Process& proc, game::Session& session, interpreter::Arguments& args)
{
    // ex IFCCNotify
    // ex ccexec.pas:GenerateNotifyMessage (Struc_CC_Notify, Struc_AddNotify)
    // Args: message, associate-with-process flag
    args.checkArgumentCount(2);

    String_t text;
    bool assoc;
    if (!interpreter::checkStringArg(text, args.getNext())
        || !interpreter::checkBooleanArg(assoc, args.getNext()))
    {
        return;
    }

    afl::string::Translator& tx = session.translator();
    String_t header;

    game::map::Object* obj = proc.getInvokingObject();
    if (dynamic_cast<game::map::Planet*>(obj) != 0) {
        header = Format(tx("(-p%04d)<<< Planet >>>\n\n"), obj->getId());
    } else if (dynamic_cast<game::map::Ship*>(obj) != 0) {
        header = Format(tx("(-s%04d)<<< Ship >>>\n\n"), obj->getId());
    } else {
        header = tx("(-X0000)<<< Notification >>>\n\n");
    }

    header += Format(tx("FROM: %s\n\n"), proc.getName());

    session.notifications().addMessage(assoc ? NotificationStore::ProcessAssociation_t(proc.getProcessId()) : NotificationStore::ProcessAssociation_t(),
                                       header,
                                       text);
}
