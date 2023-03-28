/**
  *  \file game/interface/notificationfunctions.cpp
  *  \brief Notification-related script commands
  */

#include "game/interface/notificationfunctions.hpp"
#include "afl/string/format.hpp"
#include "game/map/object.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/values.hpp"

using afl::string::Format;

/*
 *  CC$NotifyConfirmed() function
 */

/* @q CCNotifyConfirmed():Bool (Internal)
   Checks whether the process calling this function has a confirmed message.

   This function is part of the implementation of {Notify}.
   It is not part of the public API, but part of the ABI shared with PCC2 (serialized VM format).

   @since PCC2 2.40.8, PCC2 1.99.16 */
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
    return rejectFirstContext();
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
game::interface::NotifyConfirmedFunction::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    // ex IntCCNotifyConfirmed::store
    rejectStore(out, aux, ctx);
}

/*
 *  CC$Notify
 */

/* @q CC$Notify msg:Str, associateWithProcess:Bool (Internal)
   This function is part of the implementation of {Notify} and {AddNotify}.
   It is not part of the public API, but part of the ABI shared with PCC2 (serialized VM format).
   @since PCC2 2.40.8, PCC2 1.99.16 */
void
game::interface::IFCCNotify(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args)
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

    const afl::base::Deletable* obj = proc.getCurrentObject();
    Reference ref;
    if (const game::map::Planet* pl = dynamic_cast<const game::map::Planet*>(obj)) {
        header = Format(tx("(-p%04d)<<< Planet >>>\n\n"), pl->getId());
        ref = Reference(Reference::Planet, pl->getId());
    } else if (const game::map::Ship* sh = dynamic_cast<const game::map::Ship*>(obj)) {
        header = Format(tx("(-s%04d)<<< Ship >>>\n\n"), sh->getId());
        ref = Reference(Reference::Ship, sh->getId());
    } else {
        header = tx("(-X0000)<<< Notification >>>\n\n");
    }

    header += Format(tx("FROM: %s\n\n"), proc.getName());

    session.notifications().addMessage(assoc ? NotificationStore::ProcessAssociation_t(proc.getProcessId()) : NotificationStore::ProcessAssociation_t(),
                                       header,
                                       text,
                                       ref);
}

/* @q CC$NumNotifications():Int (Internal)
   Get number of notifications.
   This is a temporary stop-gap measure before notifications are published entirely.
   @since PCC2 2.40.10 */
afl::data::Value*
game::interface::IFCCNumNotifications(game::Session& session, interpreter::Arguments& args)
{
    args.checkArgumentCount(0);
    return interpreter::makeSizeValue(session.notifications().getNumMessages());
}
