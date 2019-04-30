/**
  *  \file server/interface/mailqueueserver.cpp
  */

#include <stdexcept>
#include "server/interface/mailqueueserver.hpp"
#include "server/types.hpp"
#include "afl/data/stringlist.hpp"
#include "interpreter/arguments.hpp"
#include "afl/base/optional.hpp"
#include "server/interface/mailqueue.hpp"
#include "server/errors.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"

server::interface::MailQueueServer::MailQueueServer(MailQueue& impl)
    : m_implementation(impl)
{ }

server::interface::MailQueueServer::~MailQueueServer()
{ }

server::interface::MailQueueServer::Value_t*
server::interface::MailQueueServer::call(const Segment_t& command)
{
    // ex MailoutConnection::handleCommand (sort-of)

    // Fetch command
    interpreter::Arguments args(command, 0, command.size());
    args.checkArgumentCountAtLeast(1);
    String_t cmd = afl::string::strUCase(toString(args.getNext()));

    // Dispatch command
    if (cmd == "PING") {
        /* @q PING (Mailout Command)
           Alive test.
           @retval Str "PONG". */
        return makeStringValue("PONG");
    } else if (cmd == "HELP") {
        /* @q HELP (Mailout Command)
           @retval Str Help page. */
        return makeStringValue("Mailout (c2ng)\n"
                               " PING\n"
                               " CONFIRM mail key [info]\n"
                               " CANCEL uid\n"
                               " REQUEST user\n"
                               " RUNQUEUE\n"
                               " STATUS user\n"
                               "Send mail:\n"
                               " MAIL tpl [uid]\n"
                               " PARAM name value\n"
                               " ATTACH url\n"
                               " SEND user|mail...\n");
    } else if (cmd == "MAIL") {
        /* @q MAIL tpl:Str, [uniq:Str] (Mailout Command)
           Start a new mail.
           If a %uniq is specified, a previous message with this value will become obsolete.
           @err 406 Sequence error (an unfinished transaction is active)
           @uses mqueue:uniqid, mqueue:msg:id, mqueue:preparing */
        args.checkArgumentCount(1, 2);
        String_t templateName = toString(args.getNext());
        afl::base::Optional<String_t> uniqueId;
        if (args.getNumArgs() > 0) {
            uniqueId = toString(args.getNext());
        }
        m_implementation.startMessage(templateName, uniqueId);
        return makeStringValue("OK");
    } else if (cmd == "PARAM") {
        /* @q PARAM name:Str, val:Str (Mailout Command)
           Define a template parameter for the current mail.
           @err 406 Sequence error (no preceding {MAIL} command)
           @uses mqueue:msg:$ID:args */
        args.checkArgumentCount(2);
        String_t name = toString(args.getNext());
        String_t value = toString(args.getNext());
        m_implementation.addParameter(name, value);
        return makeStringValue("OK");
    } else if (cmd == "ATTACH") {
        /* @q ATTACH url:Str (Mailout Command)
           Create attachment for the current mail.
           See {mqueue:msg:$ID:attach} for a format description.

           Note that this will only remember the attachment URL, not actually resolve it.
           Resolution will happen at an unspecified later time.
           @err 406 Sequence error (no preceding {MAIL} command)
           @uses mqueue:msg:$ID:attach */
        args.checkArgumentCount(1);
        m_implementation.addAttachment(toString(args.getNext()));
        return makeStringValue("OK");
    } else if (cmd == "SEND") {
        /* @q SEND addr:Str... (Mailout Command)
           Send current mail.
           Any number of receivers can be specified.
           See {mqueue:msg:$ID:to} for valid receiver formats.

           This command will eventually send the message.
           Message transmission happens asynchronously.

           @err 406 Sequence error (no preceding {MAIL} command)
           @uses mqueue:preparing, mqueue:sending, mqueue:msg:$ID:to */
        afl::data::StringList_t addrs;
        while (args.getNumArgs() > 0) {
            addrs.push_back(toString(args.getNext()));
        }
        m_implementation.send(addrs);
        return makeStringValue("OK, queued");
    } else if (cmd == "CANCEL") {
        /* @q CANCEL uniq:Str (Mailout Command)
           Cancels the mail with the given %uniq Id. */
        args.checkArgumentCount(1);
        m_implementation.cancelMessage(toString(args.getNext()));
        return makeStringValue("OK");
    } else if (cmd == "CONFIRM") {
        /* @q CONFIRM addr:Str, key:Str, [info:Str] (Mailout Command)
           Confirm email address using a confirmation key.
           This registers the email address as valid and starts sending mail to it.
           @err 401 Authentication error (key is not valid) */
        args.checkArgumentCount(2, 3);
        String_t mail = toString(args.getNext());
        String_t key = toString(args.getNext());
        afl::base::Optional<String_t> info;
        if (args.getNumArgs() > 0) {
            info = toString(args.getNext());
        }
        m_implementation.confirmAddress(mail, key, info);
        return makeStringValue("OK");
    } else if (cmd == "REQUEST") {
        /* @q REQUEST user:UID (Mailout Command)
           Request confirmation for a user.
           If the user has an unconfirmed, unrequested email address, sends the confirmation request.
           @err 400 Unable to request this (request cannot be sent, for example, user has no email address) */
        args.checkArgumentCount(1);
        m_implementation.requestAddress(toString(args.getNext()));
        return makeStringValue("OK");
    } else if (cmd == "RUNQUEUE") {
        /* @q RUNQUEUE (Mailout Command)
           Trigger sending of queued mail. */
        args.checkArgumentCount(0);
        m_implementation.runQueue();
        return makeStringValue("OK");
    } else if (cmd == "STATUS") {
        /* @q STATUS user:UID (Mailout Command)
           Get user's email status.
           @retkey address:Str Email address
           @retkey status:Str Address status ("u", "r", "c")
           @uses email:$EMAIL:status */
        args.checkArgumentCount(1);
        MailQueue::UserStatus st = m_implementation.getUserStatus(toString(args.getNext()));

        afl::data::Hash::Ref_t h = afl::data::Hash::create();
        h->setNew("address", makeStringValue(st.address));
        h->setNew("status",  makeStringValue(MailQueue::formatAddressStatus(st.status)));
        return new afl::data::HashValue(h);
    } else {
        throw std::runtime_error(UNKNOWN_COMMAND);
    }
}

void
server::interface::MailQueueServer::callVoid(const Segment_t& command)
{
    delete call(command);
}
