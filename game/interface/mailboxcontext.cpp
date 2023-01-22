/**
  *  \file game/interface/mailboxcontext.cpp
  *  \brief Class game::interface::MailboxContext
  */

#include "game/interface/mailboxcontext.hpp"
#include "afl/string/format.hpp"
#include "game/actions/preconditions.hpp"
#include "game/exception.hpp"
#include "game/game.hpp"
#include "game/interface/referencecontext.hpp"
#include "game/msg/file.hpp"
#include "game/root.hpp"
#include "game/specificationloader.hpp"
#include "game/turn.hpp"
#include "game/v3/udata/messagebuilder.hpp"
#include "game/v3/udata/sessionnameprovider.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/simpleprocedure.hpp"

namespace {
    struct Data {
        afl::base::Ptr<game::msg::Inbox> mailbox;
        game::Session& session;

        Data(const afl::base::Ptr<game::msg::Inbox>& mailbox, game::Session& session)
            : mailbox(mailbox), session(session)
            { }
    };

    // @since PCC2 2.41
    void IFMailbox_Add(Data d, interpreter::Process& /*proc*/, interpreter::Arguments& args)
    {
        // Preconditions
        game::Game& g = game::actions::mustHaveGame(d.session);

        // Parse arguments
        args.checkArgumentCount(1, 3);
        String_t text;
        int turnNumber = g.currentTurn().getTurnNumber();
        game::Reference ref;
        if (!interpreter::checkStringArg(text, args.getNext())) {
            return;
        }
        interpreter::checkIntegerArg(turnNumber, args.getNext());
        game::interface::checkReferenceArg(ref, args.getNext());

        // Do it
        size_t n = d.mailbox->addMessage(text, turnNumber);
        d.mailbox->setMessagePrimaryLink(n, ref);
    }

    // @since PCC2 2.41
    void IFMailbox_LoadUtilData(Data d, interpreter::Process& /*proc*/, interpreter::Arguments& args)
    {
        // For now, no parameters
        args.checkArgumentCount(0);

        // Preconditions
        game::Root& root = game::actions::mustHaveRoot(d.session);
        game::Game& g = game::actions::mustHaveGame(d.session);

        // Load parser definition
        game::v3::udata::SessionNameProvider provider(d.session);
        game::v3::udata::MessageBuilder builder(provider, root.charset(), d.session.translator());
        {
            afl::base::Ref<afl::io::Stream> file(root.specificationLoader().openSpecificationFile("utildata.ini"));
            builder.loadDefinition(*file, d.session.log());
        }

        // Load messages
        {
            afl::base::Ref<afl::io::Stream> file(root.gameDirectory().openFile(afl::string::Format("util%d.dat", g.getViewpointPlayer()), afl::io::FileSystem::OpenRead));
            builder.loadFile(*file, *d.mailbox);
        }
    }

    // @since PCC2 2.41
    void IFMailbox_LoadFile(Data d, interpreter::Process& proc, interpreter::Arguments& args)
    {
        // LoadFile #fd
        args.checkArgumentCount(1);

        afl::io::TextFile* tf = 0;
        if (!proc.world().fileTable().checkFileArg(tf, args.getNext())) {
            return;
        }

        game::msg::loadMessages(*tf, *d.mailbox);
    }


    /*
     *  Mapping
     */

    enum {
        mcAdd,
        mcLoadFile,
        mcLoadUtilData
    };

    const interpreter::NameTable TABLE[] = {
        { "ADD",           mcAdd,          0, interpreter::thProcedure },
        { "LOADFILE",      mcLoadFile,     0, interpreter::thProcedure },
        { "LOADUTILDATA",  mcLoadUtilData, 0, interpreter::thProcedure },
    };
}


game::interface::MailboxContext::MailboxContext(const afl::base::Ptr<game::msg::Inbox> mailbox, Session& session)
    : SingleContext(),
      m_mailbox(mailbox),
      m_session(session)
{ }

game::interface::MailboxContext::~MailboxContext()
{ }

game::msg::Mailbox&
game::interface::MailboxContext::mailbox()
{
    return *m_mailbox;
}

game::interface::MailboxContext*
game::interface::MailboxContext::create(Session& session)
{
    return new MailboxContext(new game::msg::Inbox(), session);
}

// Context:
interpreter::Context::PropertyAccessor*
game::interface::MailboxContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    return lookupName(name, TABLE, result) ? this : 0;
}

afl::data::Value*
game::interface::MailboxContext::get(PropertyIndex_t index)
{
    switch (TABLE[index].index) {
     case mcAdd:
        return new interpreter::SimpleProcedure<Data>(Data(m_mailbox, m_session), IFMailbox_Add);
     case mcLoadFile:
        return new interpreter::SimpleProcedure<Data>(Data(m_mailbox, m_session), IFMailbox_LoadFile);
     case mcLoadUtilData:
        return new interpreter::SimpleProcedure<Data>(Data(m_mailbox, m_session), IFMailbox_LoadUtilData);
    }
    return 0;
}

game::interface::MailboxContext*
game::interface::MailboxContext::clone() const
{
    return new MailboxContext(m_mailbox, m_session);
}

afl::base::Deletable*
game::interface::MailboxContext::getObject()
{
    return 0;
}

void
game::interface::MailboxContext::enumProperties(interpreter::PropertyAcceptor& acceptor) const
{
    acceptor.enumTable(TABLE);
}

// BaseValue:
String_t
game::interface::MailboxContext::toString(bool /*readable*/) const
{
    return "#<Mailbox>";
}

void
game::interface::MailboxContext::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}

/* @q Mailbox():Obj (Function)
   Create an empty mailbox.
   The mailbox can later be populated with content and presented to the user.

   For now, this interface is temporary.
   Operations on the mailbox:
   - Add msg:Str, Optional turn:Int, ref:Reference] (add single message)
   - LoadUtilData (load util.dat)

   @since PCC2 2.41 */
afl::data::Value*
game::interface::IFMailbox(game::Session& session, interpreter::Arguments& args)
{
    args.checkArgumentCount(0);
    return MailboxContext::create(session);
}
