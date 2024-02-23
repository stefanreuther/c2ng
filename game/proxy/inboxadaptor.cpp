/**
  *  \file game/proxy/inboxadaptor.cpp
  *  \brief Inbox Adaptors
  */

#include "game/proxy/inboxadaptor.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/msg/browser.hpp"
#include "game/msg/subsetmailbox.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/values.hpp"

namespace {
    /*
     *  Current Message Persistence
     */

    /* @q CCUI$CurrentInMsg:Int (Internal Variable)
       Zero-based index of current inbox message.
       @since PCC2 2.40.4 */
    const char*const INDEX_VAR_NAME = "CCUI$CURRENTINMSG";

    afl::base::Optional<size_t> getCurrentMessage(game::Session& session)
    {
        try {
            size_t i;
            if (interpreter::checkIndexArg(i, session.world().getGlobalValue(INDEX_VAR_NAME), 0, size_t(-1))) {
                return i;
            }
        }
        catch (...)
        { }
        return afl::base::Nothing;
    }

    void setCurrentMessage(game::Session& session, size_t msgNr)
    {
        session.world().setNewGlobalValue(INDEX_VAR_NAME, interpreter::makeSizeValue(msgNr));
    }


    /*
     *  InboxAdaptor
     */

    class InboxAdaptor : public game::proxy::MailboxAdaptor {
     public:
        InboxAdaptor(game::Session& session)
            : m_session(session),
              m_game(game::actions::mustHaveGame(session))
            { }

        virtual game::Session& session() const
            { return m_session; }

        virtual game::msg::Mailbox& mailbox() const
            { return m_game->viewpointTurn().inbox(); }

        virtual game::msg::Configuration* getConfiguration() const
            { return &m_game->messageConfiguration(); }

        virtual size_t getCurrentMessage() const
            {
                afl::base::Optional<size_t> msgNr = ::getCurrentMessage(m_session);
                if (const size_t* p = msgNr.get()) {
                    if (*p < mailbox().getNumMessages()) {
                        return *p;
                    }
                }
                return game::msg::Browser(mailbox(), m_session.translator(), game::actions::mustHaveRoot(m_session).playerList(), getConfiguration())
                    .findFirstMessage();
            }

        virtual void setCurrentMessage(size_t n)
            { ::setCurrentMessage(m_session, n); }

     private:
        game::Session& m_session;
        afl::base::Ref<game::Game> m_game;
    };

    /*
     *  InboxSubsetAdaptor
     */

    class InboxSubsetAdaptor : public game::proxy::MailboxAdaptor {
     public:
        InboxSubsetAdaptor(game::Session& session, std::vector<size_t> indexes)
            : m_session(session),
              m_game(game::actions::mustHaveGame(session)),
              m_mailbox(m_game->viewpointTurn().inbox(), indexes)
            { }

        virtual game::Session& session() const
            { return m_session; }

        virtual game::msg::Mailbox& mailbox() const
            { return m_mailbox; }

        virtual game::msg::Configuration* getConfiguration() const
            { return &m_game->messageConfiguration(); }

        virtual size_t getCurrentMessage() const
            {
                afl::base::Optional<size_t> msgNr = ::getCurrentMessage(m_session);
                if (const size_t* p = msgNr.get()) {
                    afl::base::Optional<size_t> ourIndex = m_mailbox.find(*p);
                    if (const size_t* q = ourIndex.get()) {
                        return *q;
                    }
                }
                return game::msg::Browser(m_mailbox, m_session.translator(), game::actions::mustHaveRoot(m_session).playerList(), getConfiguration())
                    .findFirstMessage();
            }

        virtual void setCurrentMessage(size_t n)
            { ::setCurrentMessage(m_session, m_mailbox.getOuterIndex(n)); }

     private:
        game::Session& m_session;
        afl::base::Ref<game::Game> m_game;
        mutable game::msg::SubsetMailbox m_mailbox;
    };
}

/*
 *  Entry Points
 */

game::proxy::InboxAdaptor_t*
game::proxy::makeInboxAdaptor()
{
    class AdaptorFromSession : public InboxAdaptor_t {
     public:
        virtual MailboxAdaptor* call(Session& s)
            { return new InboxAdaptor(s); }
    };
    return new AdaptorFromSession();
}

game::proxy::InboxAdaptor_t*
game::proxy::makePlanetInboxAdaptor(Id_t planetId)
{
    class AdaptorFromSession : public InboxAdaptor_t {
     public:
        AdaptorFromSession(Id_t planetId)
            : m_planetId(planetId)
            { }

        virtual MailboxAdaptor* call(Session& s)
            {
                std::vector<size_t> indexes;
                if (const Game* g = s.getGame().get()) {
                    if (const game::map::Planet* p = g->viewpointTurn().universe().planets().get(m_planetId)) {
                        indexes = p->messages().get();
                    }
                }
                return new InboxSubsetAdaptor(s, indexes);
            }
     private:
        Id_t m_planetId;
    };
    return new AdaptorFromSession(planetId);
}

game::proxy::InboxAdaptor_t*
game::proxy::makeShipInboxAdaptor(Id_t shipId)
{
    class AdaptorFromSession : public InboxAdaptor_t {
     public:
        AdaptorFromSession(Id_t shipId)
            : m_shipId(shipId)
            { }

        virtual MailboxAdaptor* call(Session& s)
            {
                std::vector<size_t> indexes;
                if (const Game* g = s.getGame().get()) {
                    if (const game::map::Ship* p = g->viewpointTurn().universe().ships().get(m_shipId)) {
                        indexes = p->messages().get();
                    }
                }
                return new InboxSubsetAdaptor(s, indexes);
            }
     private:
        Id_t m_shipId;
    };
    return new AdaptorFromSession(shipId);
}
