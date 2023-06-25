/**
  *  \file game/proxy/historyturnproxy.cpp
  *  \brief Class game::proxy::HistoryTurnProxy
  */

#include "game/proxy/historyturnproxy.hpp"
#include "afl/string/format.hpp"
#include "game/game.hpp"
#include "game/interface/globalcommands.hpp"
#include "game/turn.hpp"
#include "interpreter/simpleprocedure.hpp"
#include "interpreter/values.hpp"

using afl::string::Format;
using game::proxy::HistoryTurnProxy;
using game::HistoryTurn;

namespace {
    const char* LOG_NAME = "game.proxy.HistoryTurnProxy";

    /** Convert HistoryTurn::Status into TurnListbox::Status. */
    HistoryTurnProxy::Status convertStatus(HistoryTurn::Status status)
    {
        switch (status) {
         case HistoryTurn::Unknown:           return HistoryTurnProxy::Unknown;
         case HistoryTurn::Unavailable:       return HistoryTurnProxy::Unavailable;
         case HistoryTurn::StronglyAvailable: return HistoryTurnProxy::StronglyAvailable;
         case HistoryTurn::WeaklyAvailable:   return HistoryTurnProxy::WeaklyAvailable;
         case HistoryTurn::Failed:            return HistoryTurnProxy::Failed;
         case HistoryTurn::Loaded:            return HistoryTurnProxy::Loaded;
        }
        return HistoryTurnProxy::Unknown;
    }

    /** Prepare list item for a history turn.
        \param content [out] List item will be appended here
        \param game [in] Game
        \param turnNumber [in] Turn number */
    void prepareListItem(HistoryTurnProxy::Items_t& content, const game::Game& g, int turnNumber)
    {
        content.push_back(HistoryTurnProxy::Item(turnNumber, g.previousTurns().getTurnTimestamp(turnNumber), convertStatus(g.previousTurns().getTurnStatus(turnNumber))));
    }
}

/*
 *  Initial (handleSetup()) response
 */
class game::proxy::HistoryTurnProxy::InitialResponse : public util::Request<HistoryTurnProxy> {
 public:
    InitialResponse(Items_t& content, int turnNumber)
        : m_content(),
          m_turnNumber(turnNumber)
        { m_content.swap(content); }
    void handle(HistoryTurnProxy& proxy)
        { proxy.sig_setup.raise(m_content, m_turnNumber); }
 private:
    Items_t m_content;
    int m_turnNumber;
};

/*
 *  Initial request.
 *  Determines what to display and creates dialog initialisation data.
 */
class game::proxy::HistoryTurnProxy::InitialRequest : public util::Request<Session> {
 public:
    InitialRequest(util::RequestSender<HistoryTurnProxy> response, int maxTurns)
        : m_response(response),
          m_maxTurns(maxTurns)
        { }
    void handle(Session& s)
        {
            Items_t content;
            int activeTurn = 0;
            s.log().write(afl::sys::LogListener::Trace, LOG_NAME, Format("-> Initial(max=%d)", m_maxTurns));
            if (const Game* g = s.getGame().get()) {
                // Fetch current turn
                const int currentTurn = g->currentTurn().getTurnNumber();
                activeTurn = g->getViewpointTurnNumber();

                // Fetch status of all turns below that.
                // Limit turn count to avoid bogus data overloading us.
                const int minTurn = (currentTurn <= m_maxTurns ? 1 : currentTurn - m_maxTurns + 1);
                for (int i = minTurn; i < currentTurn; ++i) {
                    prepareListItem(content, *g, i);
                }

                // Current turn
                content.push_back(Item(currentTurn, g->currentTurn().getTimestamp(), Current));
            }
            s.log().write(afl::sys::LogListener::Trace, LOG_NAME, Format("<- Initial(size=%d,turn=%d)", content.size(), activeTurn));
            m_response.postNewRequest(new InitialResponse(content, activeTurn));
        }
 private:
    util::RequestSender<HistoryTurnProxy> m_response;
    int m_maxTurns;
};

/*
 *  Partial update response
 */
class game::proxy::HistoryTurnProxy::UpdateResponse : public util::Request<HistoryTurnProxy> {
 public:
    UpdateResponse(Items_t& content)
        : m_content()
        { m_content.swap(content); }
    void handle(HistoryTurnProxy& proxy)
        { proxy.sig_update.raise(m_content); }
 private:
    Items_t m_content;
};

/*
 *  Update request.
 *  Determines whether there are any turns that can be updated, and if so, updates their metainformation.
 */
class game::proxy::HistoryTurnProxy::UpdateRequest : public util::Request<Session> {
 public:
    /** Constructor.
        \param response Response channel
        \param firstTurn First turn we're interested in. If the dialog display turns 300 .. 1300, it makes no sense to update turn 100.
        \param maxTurns Maximum number of turns */
    UpdateRequest(util::RequestSender<HistoryTurnProxy> response, int firstTurn, int maxTurns)
        : m_response(response),
          m_firstTurn(firstTurn),
          m_maxTurns(maxTurns)
        { }
    void handle(Session& s)
        {
            Items_t content;
            Game* g = s.getGame().get();
            Root* r = s.getRoot().get();
            s.log().write(afl::sys::LogListener::Trace, LOG_NAME, Format("-> Update(first=%d, max=%d)", m_firstTurn, m_maxTurns));
            if (g != 0 && r != 0 && r->getTurnLoader().get() != 0) {
                int lastTurn = g->previousTurns().findNewestUnknownTurnNumber(g->currentTurn().getTurnNumber());
                if (lastTurn >= m_firstTurn) {
                    // Update
                    int firstTurn = std::max(m_firstTurn, lastTurn - (m_maxTurns-1));
                    g->previousTurns().initFromTurnScores(g->scores(), firstTurn, lastTurn - firstTurn + 1);
                    g->previousTurns().initFromTurnLoader(*r->getTurnLoader(), *r, g->getViewpointPlayer(), firstTurn, lastTurn - firstTurn + 1);

                    // Build result
                    for (int i = firstTurn; i <= lastTurn; ++i) {
                        prepareListItem(content, *g, i);
                    }
                }
            }
            sendUpdateResponse(s, m_response, content);
        }
 private:
    util::RequestSender<HistoryTurnProxy> m_response;
    const int m_firstTurn;
    const int m_maxTurns;
};

/*
 *  Load request.
 */
class game::proxy::HistoryTurnProxy::LoadRequest : public util::Request<Session> {
 public:
    LoadRequest(util::RequestSender<HistoryTurnProxy> response, int turnNumber)
        : m_response(response),
          m_turnNumber(turnNumber)
        { }
    void handle(Session& s)
        {
            // Implemented as a helper process to re-use the IFHistoryLoadTurn command and its ability to suspend/interact;
            // otherwise, we'd have to implement storage for a suspended process.
            class Finalizer : public interpreter::Process::Finalizer {
             public:
                Finalizer(util::RequestSender<HistoryTurnProxy> response, Session& session, int turnNumber)
                    : m_response(response), m_session(session), m_turnNumber(turnNumber)
                    { }
                void finalizeProcess(interpreter::Process& /*proc*/)
                    {
                        Items_t content;
                        if (const Game* g = m_session.getGame().get()) {
                            // Mark turn failed.
                            // If it did not actually fail, this will be a no-op.
                            // However, if loading did fail, we may not have set any state at all.
                            if (HistoryTurn* ht = g->previousTurns().get(m_turnNumber)) {
                                ht->handleLoadFailed();
                            }

                            prepareListItem(content, *g, m_turnNumber);
                        }
                        sendUpdateResponse(m_session, m_response, content);
                    }
             private:
                util::RequestSender<HistoryTurnProxy> m_response;
                Session& m_session;
                const int m_turnNumber;
            };

            s.log().write(afl::sys::LogListener::Trace, LOG_NAME, Format("-> Load(%d)", m_turnNumber));
            interpreter::Process& proc = s.processList().create(s.world(), "<LoadRequest>");
            interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
            proc.pushNewValue(interpreter::makeIntegerValue(m_turnNumber));
            proc.pushNewValue(new interpreter::SimpleProcedure<Session&>(s, game::interface::IFHistoryLoadTurn));
            bco->addInstruction(interpreter::Opcode::maIndirect, interpreter::Opcode::miIMCall, 1);
            proc.pushFrame(bco, false);
            proc.setNewFinalizer(new Finalizer(m_response, s, m_turnNumber));

            uint32_t pgid = s.processList().allocateProcessGroup();
            s.processList().resumeProcess(proc, pgid);
            s.processList().startProcessGroup(pgid);
            s.processList().run();
            s.processList().removeTerminatedProcesses();
        }
 private:
    util::RequestSender<HistoryTurnProxy> m_response;
    const int m_turnNumber;
};


/*
 *  HistoryTurnProxy
 */

game::proxy::HistoryTurnProxy::HistoryTurnProxy(util::RequestSender<Session> sender, util::RequestDispatcher& reply)
    : m_reply(reply, *this),
      m_request(sender)
{ }

game::proxy::HistoryTurnProxy::~HistoryTurnProxy()
{ }

void
game::proxy::HistoryTurnProxy::requestSetup(int maxTurns)
{
    m_request.postNewRequest(new InitialRequest(m_reply.getSender(), maxTurns));
}

void
game::proxy::HistoryTurnProxy::requestUpdate(int firstTurn, int maxTurns)
{
    m_request.postNewRequest(new UpdateRequest(m_reply.getSender(), firstTurn, maxTurns));
}

void
game::proxy::HistoryTurnProxy::requestLoad(int turnNumber)
{
    m_request.postNewRequest(new LoadRequest(m_reply.getSender(), turnNumber));
}

void
game::proxy::HistoryTurnProxy::sendUpdateResponse(Session& session, util::RequestSender<HistoryTurnProxy>& response, Items_t& content)
{
    session.log().write(afl::sys::LogListener::Trace, LOG_NAME, Format("<- Update(size=%d)", content.size()));
    response.postNewRequest(new UpdateResponse(content));
}
