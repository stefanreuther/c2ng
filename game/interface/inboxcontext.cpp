/**
  *  \file game/interface/inboxcontext.cpp
  *  \brief Class game::interface::InboxContext
  */

#include "game/interface/inboxcontext.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/format.hpp"
#include "game/game.hpp"
#include "game/interface/globalcommands.hpp"
#include "game/interface/referencecontext.hpp"
#include "game/root.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/procedurevalue.hpp"
#include "interpreter/process.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/values.hpp"
#include "interpreter/world.hpp"

using afl::base::Ptr;
using afl::string::Format;
using game::msg::Mailbox;
using interpreter::Arguments;

namespace {
    enum MessageProperty {
        impId,
        impLines,
        impGroup,
        impKilled,
        impText,
        impFullText,

        impTurn,
        impPrimaryLink,
        impSecondaryLink,
        impReplySet,
        impReplyAllSet,
        impDataStatus,
        impConfirmed,

        immWrite
    };
    enum { MessagePropertyDomain };

    const interpreter::NameTable MSG_MAPPING[] = {
        { "CONFIRMED",   impConfirmed,       MessagePropertyDomain, interpreter::thBool },
        { "DATASTATUS",  impDataStatus,      MessagePropertyDomain, interpreter::thString },
        { "FULLTEXT",    impFullText,        MessagePropertyDomain, interpreter::thString },
        { "GROUP",       impGroup,           MessagePropertyDomain, interpreter::thString },
        { "ID",          impId,              MessagePropertyDomain, interpreter::thInt },
        { "KILLED",      impKilled,          MessagePropertyDomain, interpreter::thBool },
        { "LINES",       impLines,           MessagePropertyDomain, interpreter::thInt },
        { "LINK",        impPrimaryLink,     MessagePropertyDomain, interpreter::thNone },
        { "LINK2",       impSecondaryLink,   MessagePropertyDomain, interpreter::thNone },
        { "PARTNER",     impReplySet,        MessagePropertyDomain, interpreter::thArray },
        { "PARTNER.ALL", impReplyAllSet,     MessagePropertyDomain, interpreter::thArray },
        { "TEXT",        impText,            MessagePropertyDomain, interpreter::thArray },
        { "TURN",        impTurn,            MessagePropertyDomain, interpreter::thInt },
        { "WRITE",       immWrite,           MessagePropertyDomain, interpreter::thProcedure },
    };

    /*
     *  Implementation of InMsg().Text - (1-based) array of text lines
     */

    class MessageTextValue : public interpreter::IndexableValue {
        // ex IntMessageText
     public:
        MessageTextValue(const Ptr<game::parser::MessageLines_t>& lines)
            : m_lines(lines)
            { }

        // IndexableValue:
        virtual afl::data::Value* get(Arguments& args)
            {
                args.checkArgumentCount(1);

                size_t index;
                if (!interpreter::checkIndexArg(index, args.getNext(), 1, m_lines->size())) {
                    return 0;
                }
                return interpreter::makeStringValue((*m_lines)[index]);
            }

        virtual void set(Arguments& args, const afl::data::Value* value)
            { rejectSet(args, value); }

        // CallableValue:
        virtual size_t getDimension(size_t which) const
            {
                if (which == 0) {
                    return 1;
                } else {
                    return m_lines->size() + 1;
                }
            }

        virtual interpreter::Context* makeFirstContext()
            { return rejectFirstContext(); }

        // BaseValue:
        virtual String_t toString(bool /*readable*/) const
            { return "#<array>"; }

        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
            { rejectStore(out, aux, ctx); }

        virtual MessageTextValue* clone() const
            { return new MessageTextValue(m_lines); }

     private:
        const Ptr<game::parser::MessageLines_t> m_lines;
    };


    /*
     *  Implementation of InMsg().Write (command)
     */

    class MessageWriteCommand : public interpreter::ProcedureValue {
        // ex IntMessageWriteProcedure
     public:
        MessageWriteCommand(int turnNumber, size_t messageIndex, Ptr<game::parser::MessageLines_t> lines)
            : m_lines(lines),
              m_turnNumber(turnNumber),
              m_messageIndex(messageIndex)
            { }

        virtual void call(interpreter::Process& proc, Arguments& args);

        virtual ProcedureValue* clone() const
            { return new MessageWriteCommand(m_turnNumber, m_messageIndex, m_lines); }

     private:
        const Ptr<game::parser::MessageLines_t> m_lines;
        const int m_turnNumber;
        const size_t m_messageIndex;
    };

    afl::data::Value* makeReferenceValue(game::Reference ref, game::Session& session)
    {
        if (ref.isSet()) {
            return new game::interface::ReferenceContext(ref, session);
        } else {
            return 0;
        }
    }

    afl::data::Value* makeDataStatus(Mailbox::DataStatus st)
    {
        switch (st) {
         case Mailbox::NoData:              break;
         case Mailbox::DataReceivable:      return interpreter::makeStringValue("receivable");
         case Mailbox::DataReceived:        return interpreter::makeStringValue("received");
         case Mailbox::DataExpired:         return interpreter::makeStringValue("expired");
         case Mailbox::DataWrongPasscode:   return interpreter::makeStringValue("wrong-passcode");
         case Mailbox::DataWrongChecksum:   return interpreter::makeStringValue("wrong-checksum");
         case Mailbox::DataFailed:          return interpreter::makeStringValue("failed");
        }
        return 0;
    }
}

void
MessageWriteCommand::call(interpreter::Process& proc, Arguments& args)
{
    // ex IntMessageWriteProcedure::call
    // ex fileint.pas:ScriptWriteMessage, msgint.pas:Mailbox_Write
    /* @q Write #fd:File, Optional flags:Str (Incoming Message Command)
       Write message to file.
       The file %fd must be a text file open for writing.

       By default, this writes the message in mailbox format.
       This way, you can later open the file with the View Mailbox function ([Alt-M]) in PCC.
       That is, PCC will automatically prepend a special header to the message text,
       to later be able to recognize message boundaries.
       By specifying the second, optional parameter as "r", these headers are omitted
       and just the raw text is written.

       @since PCC 1.1.16, PCC2 1.99.13, PCC2 2.40.8 */

    // Implementation
    afl::io::TextFile* tf;
    int32_t flags = 0;
    args.checkArgumentCount(1, 2);

    if (!proc.world().fileTable().checkFileArg(tf, args.getNext())) {
        return;
    }

    interpreter::checkFlagArg(flags, 0, args.getNext(), "R");

    if (flags == 0) {
        tf->writeLine(Format("=== Turn %d ===", m_turnNumber));
        tf->writeLine(Format("--- Message %d ---", m_messageIndex+1));
    }
    for (game::parser::MessageLines_t::iterator i = m_lines->begin(), e = m_lines->end(); i != e; ++i) {
        tf->writeLine(*i);
    }
}


game::interface::InboxContext::InboxContext(size_t index, game::Session& session, const afl::base::Ref<const Turn>& turn)
    : m_index(index),
      m_session(session),
      m_turn(turn),
      m_lineCache()
{ }

game::interface::InboxContext::~InboxContext()
{ }

interpreter::Context::PropertyAccessor*
game::interface::InboxContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    // ex IntMessageContext::lookup
    return lookupName(name, MSG_MAPPING, result) ? this : 0;
}

afl::data::Value*
game::interface::InboxContext::get(PropertyIndex_t index)
{
    // ex IntMessageContext::get
    // ex msgint.pas:CMailboxContext.ResolveValue, CMailboxContext.ResolveFuncall
    if (const Root* root = m_session.getRoot().get()) {
        switch (MessageProperty(MSG_MAPPING[index].index)) {
         case impId:
            /* @q Id:Int (Incoming Message Property)
               Id of message.
               This is the index into {InMsg()} to access this very message. */
            return interpreter::makeSizeValue(m_index+1);

         case impLines:
            /* @q Lines:Int (Incoming Message Property)
               Number of lines in message. */
            return interpreter::makeSizeValue(getLineCache()->size());

         case impGroup:
            /* @q Group:Str (Incoming Message Property)
               Group of this message.
               Similar messages are grouped using this string for the message list.
               The message filter also operates based on this string. */
            return interpreter::makeStringValue(mailbox().getMessageHeading(m_index, m_session.translator(), root->playerList()));

         case impKilled:
            /* @q Killed:Bool (Incoming Message Property)
               True if this message is filtered and skipped by default. */
            if (const Game* g = m_session.getGame().get()) {
                return interpreter::makeBooleanValue(g->messageConfiguration().isHeadingFiltered(mailbox().getMessageHeading(m_index, m_session.translator(), root->playerList())));
            } else {
                return 0;
            }

         case impText:
            /* @q Text:Str() (Incoming Message Property)
               Message text, line by line. */
            return new MessageTextValue(getLineCache());

         case impFullText:
            /* @q FullText:Str (Incoming Message Property)
               Message text, in one big string. */
            return interpreter::makeStringValue(mailbox().getMessageBodyText(m_index, m_session.translator(), root->playerList()));

         case impTurn:
            /* @q Turn:Int (Incoming Message Property)
               Message turn number. */
            if (int turnNr = getCurrentMetadata(*root).turnNumber) {
                return interpreter::makeIntegerValue(turnNr);
            } else {
                return 0;
            }

         case impPrimaryLink:
            /* @q Link:Reference (Incoming Message Property)
               First object or location linked by message.
               In messages from host, the object sending the message if recognized correctly
               (e.g. in a message from a planet reporting overtemperature, the planet).
               EMPTY if none.
               @since PCC2 2.41.3
               @see Link2 (Incoming Message Property) */
            return makeReferenceValue(getCurrentMetadata(*root).primaryLink, m_session);

         case impSecondaryLink:
            /* @q Link2:Reference (Incoming Message Property)
               Second object or location linked by message.
               Typically, first X,Y coordinate mentioned in message.
               EMPTY if none.
               @since PCC2 2.41.3
               @see Link (Incoming Message Property) */
            return makeReferenceValue(getCurrentMetadata(*root).secondaryLink, m_session);

         case impReplySet:
            /* @q Partner:Int() (Incoming Message Property)
               List of players to send a "reply-to-sender" message to.
               For normal player-to-player messages, sender of the message;
               for anonymous messages, all players.

               The return value is an array containing player numbers,
               compatible with the first parameter of {SendMessage}.
               Value is EMPTY if there are no players.
               @since PCC2 2.41.3
               @see Partner.All (Incoming Message Property) */
            return makePlayerSet(getCurrentMetadata(*root).reply);

         case impReplyAllSet:
            /* @q Partner.All:Int() (Incoming Message Property)
               List of players to send a "reply-to-all" message to.
               If a message to multiple players has been recognized,
               this includes the sender of the message and all other receivers.

               The return value is an array containing player numbers,
               compatible with the first parameter of {SendMessage}.
               Value is EMPTY if there are no players.
               @since PCC2 2.41.3
               @see Partner (Incoming Message Property) */
            return makePlayerSet(getCurrentMetadata(*root).replyAll);

         case impDataStatus:
            /* @q DataStatus:Str (Incoming Message Property)
               Status of data-transmission message.
               One of:
               - receivable
               - received
               - expired
               - wrong-passcode
               - wrong-checksum
               - failed

               EMPTY if message does not contain a data transmission.
               @since PCC2 2.41.3 */
            return makeDataStatus(getCurrentMetadata(*root).dataStatus);

         case impConfirmed:
            /* @q Confirmed:Bool (Incoming Message Property)
               If message represents a {Notify (Global Command)|notification message},
               its confirmation status.

               @since PCC2 2.41.3 */
            return interpreter::makeBooleanValue(getCurrentMetadata(*root).flags.contains(Mailbox::Confirmed));

         case immWrite:
            // @change PCC2 uses the game's turn number; we have a message turn number
            return new MessageWriteCommand(getCurrentMetadata(*root).turnNumber, m_index, getLineCache());
        }
    }
    return 0;
}

bool
game::interface::InboxContext::next()
{
    // ex IntMessageContext::next
    size_t nextIndex = m_index + 1;
    if (nextIndex < mailbox().getNumMessages()) {
        clearLineCache();
        m_index = nextIndex;
        return true;
    } else {
        return false;
    }
}

game::interface::InboxContext*
game::interface::InboxContext::clone() const
{
    // ex IntMessageContext::clone
    return new InboxContext(m_index, m_session, m_turn);
}

afl::base::Deletable*
game::interface::InboxContext::getObject()
{
    // ex IntMessageContext::getObject
    return 0;
}

void
game::interface::InboxContext::enumProperties(interpreter::PropertyAcceptor& acceptor) const
{
    // ex IntMessageContext::enumProperties
    acceptor.enumTable(MSG_MAPPING);
}

String_t
game::interface::InboxContext::toString(bool /*readable*/) const
{
    // ex IntMessageContext::toString
    return "#<message>";
}

void
game::interface::InboxContext::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    // ex IntMessageContext::store
    rejectStore(out, aux, ctx);
}

const game::msg::Mailbox&
game::interface::InboxContext::mailbox()
{
    return m_turn->inbox();
}

void
game::interface::InboxContext::clearLineCache()
{
    m_lineCache.reset();
}

afl::base::Ptr<game::parser::MessageLines_t>
game::interface::InboxContext::getLineCache()
{
    // ex IntMessageContext::createLineCache
    if (m_lineCache.get() == 0) {
        m_lineCache = new game::parser::MessageLines_t();
        if (const Root* r = m_session.getRoot().get()) {
            game::parser::splitMessage(*m_lineCache, mailbox().getMessageBodyText(m_index, m_session.translator(), r->playerList()));
        }
    }
    return m_lineCache;
}

game::msg::Mailbox::Metadata
game::interface::InboxContext::getCurrentMetadata(const Root& root)
{
    return mailbox().getMessageMetadata(m_index, m_session.translator(), root.playerList());
}
