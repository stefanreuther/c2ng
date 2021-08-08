/**
  *  \file game/interface/inboxcontext.cpp
  *  \brief Class game::interface::InboxContext
  */

#include "game/interface/inboxcontext.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/format.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/process.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/values.hpp"
#include "interpreter/world.hpp"

using afl::string::Format;
using interpreter::Error;

namespace {
    enum MessageProperty {
        impId,
        impLines,
        impGroup,
        impKilled,
        impText,
        impFullText,

        immWrite
    };
    enum { MessagePropertyDomain };

    const interpreter::NameTable MSG_MAPPING[] = {
        { "FULLTEXT", impFullText, MessagePropertyDomain, interpreter::thString },
        { "GROUP",    impGroup,    MessagePropertyDomain, interpreter::thString },
        { "ID",       impId,       MessagePropertyDomain, interpreter::thInt },
        { "KILLED",   impKilled,   MessagePropertyDomain, interpreter::thBool },
        { "LINES",    impLines,    MessagePropertyDomain, interpreter::thInt },
        { "TEXT",     impText,     MessagePropertyDomain, interpreter::thArray },
        { "WRITE",    immWrite,    MessagePropertyDomain, interpreter::thProcedure },
    };

    /*
     *  Implementation of InMsg().Text - (1-based) array of text lines
     */

    class MessageTextValue : public interpreter::IndexableValue {
        // ex IntMessageText
     public:
        MessageTextValue(afl::base::Ptr<game::parser::MessageLines_t> lines)
            : m_lines(lines)
            { }

        // IndexableValue:
        virtual afl::data::Value* get(interpreter::Arguments& args)
            {
                args.checkArgumentCount(1);

                // FIXME: use checkIndexArg
                int32_t index;
                if (!interpreter::checkIntegerArg(index, args.getNext(), 1, int32_t(m_lines->size()))) {
                    return 0;
                }
                return interpreter::makeStringValue((*m_lines)[index-1]);
            }

        virtual void set(interpreter::Arguments& /*args*/, afl::data::Value* /*value*/)
            { throw Error::notAssignable(); }

        // CallableValue:
        virtual int32_t getDimension(int32_t which) const
            {
                if (which == 0) {
                    return 1;
                } else {
                    return int32_t(m_lines->size()) + 1;
                }
            }

        virtual interpreter::Context* makeFirstContext()
            { throw Error::typeError(Error::ExpectIterable); }

        // BaseValue:
        virtual String_t toString(bool /*readable*/) const
            { return "#<array>"; }

        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
            { throw Error::notSerializable(); }

        virtual MessageTextValue* clone() const
            { return new MessageTextValue(m_lines); }

     private:
        afl::base::Ptr<game::parser::MessageLines_t> m_lines;
    };


    /*
     *  Implementation of InMsg().Write (command)
     */

    class MessageWriteCommand : public interpreter::CallableValue {
        // ex IntMessageWriteProcedure
     public:
        MessageWriteCommand(int turnNumber, size_t messageIndex,
                            afl::base::Ptr<game::parser::MessageLines_t> lines)
            : m_lines(lines),
              m_turnNumber(turnNumber),
              m_messageIndex(messageIndex)
            { }

        virtual void call(interpreter::Process& proc, afl::data::Segment& args, bool want_result);

        virtual bool isProcedureCall() const
            { return true; }

        virtual int32_t getDimension(int32_t /*which*/) const
            { return 0; }

        virtual interpreter::Context* makeFirstContext()
            { throw Error::typeError(Error::ExpectIterable); }

        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
            { throw Error::notSerializable(); }

        virtual String_t toString(bool /*readable*/) const
            { return "#<procedure>"; }
        virtual CallableValue* clone() const
            { return new MessageWriteCommand(m_turnNumber, m_messageIndex, m_lines); }

     private:
        afl::base::Ptr<game::parser::MessageLines_t> m_lines;
        int m_turnNumber;
        size_t m_messageIndex;
    };
}

void
MessageWriteCommand::call(interpreter::Process& proc, afl::data::Segment& args, bool want_result)
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

    /* Boilerplate */
    if (want_result) {
        proc.pushNewValue(0);
    }

    interpreter::Arguments a(args, 0, args.size());

    // Implementation
    afl::io::TextFile* tf;
    int32_t flags = 0;
    a.checkArgumentCount(1, 2);

    if (!proc.world().fileTable().checkFileArg(tf, a.getNext())) {
        return;
    }

    interpreter::checkFlagArg(flags, 0, a.getNext(), "R");

    if (flags == 0) {
        // FIXME: PCC1 tracks the last "Turn" header written and omits duplicates
        tf->writeLine(Format("=== Turn %d ===", m_turnNumber));
        tf->writeLine(Format("--- Message %d ---", m_messageIndex+1));
    }
    for (game::parser::MessageLines_t::iterator i = m_lines->begin(), e = m_lines->end(); i != e; ++i) {
        tf->writeLine(*i);
    }
}


game::interface::InboxContext::InboxContext(size_t index,
                                            afl::string::Translator& tx,
                                            afl::base::Ref<const Root> root,
                                            afl::base::Ref<const Game> game)
    : m_index(index),
      m_translator(tx),
      m_root(root),
      m_game(game),
      m_lineCache()
{ }

game::interface::InboxContext::~InboxContext()
{ }

game::interface::InboxContext*
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
        return interpreter::makeStringValue(mailbox().getMessageHeading(m_index, m_translator, m_root->playerList()));

     case impKilled:
        /* @q Killed:Bool (Incoming Message Property)
           True if this message is filtered and skipped by default. */
        return interpreter::makeBooleanValue(m_game->messageConfiguration().isHeadingFiltered(mailbox().getMessageHeading(m_index, m_translator, m_root->playerList())));

     case impText:
        /* @q Text:Str() (Incoming Message Property)
           Message text, line by line. */
        return new MessageTextValue(getLineCache());

     case impFullText:
        /* @q FullText:Str (Incoming Message Property)
           Message text, in one big string. */
        return interpreter::makeStringValue(mailbox().getMessageText(m_index, m_translator, m_root->playerList()));

     case immWrite:
        // @change PCC2 uses the game's turn number; we have a message turn number
        return new MessageWriteCommand(mailbox().getMessageTurnNumber(m_index), m_index, getLineCache());
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
    return new InboxContext(m_index, m_translator, m_root, m_game);
}

game::map::Object*
game::interface::InboxContext::getObject()
{
    // ex IntMessageContext::getObject
    return 0;
}

void
game::interface::InboxContext::enumProperties(interpreter::PropertyAcceptor& acceptor)
{
    // ex IntMessageContext::enumProperties
    acceptor.enumTable(MSG_MAPPING);
}

String_t
game::interface::InboxContext::toString(bool /*readable*/) const
{
    // ex IntMessageContext::toString
    // FIXME: we can do better
    return "#<message>";
}

void
game::interface::InboxContext::store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
{
    // ex IntMessageContext::store
    throw Error::notSerializable();
}

const game::msg::Mailbox&
game::interface::InboxContext::mailbox()
{
    return m_game->currentTurn().inbox();
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
        game::parser::splitMessage(*m_lineCache, mailbox().getMessageText(m_index, m_translator, m_root->playerList()));
    }
    return m_lineCache;
}
