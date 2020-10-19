/**
  *  \file game/proxy/selectionproxy.cpp
  *  \brief Class game::proxy::SelectionProxy
  */

#include "game/proxy/selectionproxy.hpp"
#include "game/game.hpp"
#include "game/map/selections.hpp"
#include "game/map/selectionvector.hpp"
#include "game/turn.hpp"
#include "interpreter/error.hpp"
#include "interpreter/selectionexpression.hpp"
#include "interpreter/tokenizer.hpp"
#include "util/slaveobject.hpp"

using game::map::Selections;
using game::map::SelectionVector;
using interpreter::SelectionExpression;

namespace {
    const char CLEAR_CODE[] = { SelectionExpression::opZero, 0 };
}

/*
 *  Trampoline
 */

class game::proxy::SelectionProxy::Trampoline : public util::SlaveObject<Session> {
 public:
    Trampoline(util::RequestSender<SelectionProxy> reply);
    virtual void init(Session& session);
    virtual void done(Session& session);
    void onSelectionChange();
    void describe(Info& info) const;
    void setCurrentLayer(Session& session, size_t newLayer);
    void executeCompiledExpression(Session& session, const String_t& compiledExpression, size_t targetLayer);
    void executeCompiledExpressionAll(Session& session, const String_t& compiledExpression);
    void executeExpression(Session& session, const String_t& expression, size_t targetLayer);

 private:
    util::RequestSender<SelectionProxy> m_reply;
    afl::base::Ptr<Game> m_game;
    afl::base::SignalConnection conn_selectionChange;
};

inline
game::proxy::SelectionProxy::Trampoline::Trampoline(util::RequestSender<SelectionProxy> reply)
    : m_reply(reply), m_game(), conn_selectionChange()
{ }

void
game::proxy::SelectionProxy::Trampoline::init(Session& session)
{
    m_game = session.getGame();
    if (m_game.get() != 0) {
        // Register for changes
        Selections& sel = m_game->selections();
        conn_selectionChange = sel.sig_selectionChange.add(this, &Trampoline::onSelectionChange);

        // Update multi-selection view
        // ex client/dialogs/selmgr.cc:doSelectionManager
        game::map::Universe& univ = m_game->currentTurn().universe();
        sel.copyFrom(univ, sel.getCurrentLayer());
        sel.limitToExistingObjects(univ, sel.getCurrentLayer());
    }
}

void
game::proxy::SelectionProxy::Trampoline::done(Session& /*session*/)
{
    conn_selectionChange.disconnect();
}

void
game::proxy::SelectionProxy::Trampoline::onSelectionChange()
{
    class Task : public util::Request<SelectionProxy> {
     public:
        Task(const Trampoline& tpl)
            : m_info()
            { tpl.describe(m_info); }
        virtual void handle(SelectionProxy& proxy)
            { proxy.sig_selectionChange.raise(m_info); }
     private:
        Info m_info;
    };
    m_reply.postNewRequest(new Task(*this));
}

void
game::proxy::SelectionProxy::Trampoline::describe(Info& info) const
{
    if (m_game.get() != 0) {
        Selections& sel = m_game->selections();
        info.currentLayer = sel.getCurrentLayer();
        for (size_t i = 0, n = sel.getNumLayers(); i < n; ++i) {
            const SelectionVector* ships   = sel.get(Selections::Ship,   i);
            const SelectionVector* planets = sel.get(Selections::Planet, i);
            info.layers.push_back(Layer(ships   ? ships->getNumMarkedObjects()   : 0,
                                        planets ? planets->getNumMarkedObjects() : 0));
        }
    }
}

inline void
game::proxy::SelectionProxy::Trampoline::setCurrentLayer(Session& session, size_t newLayer)
{
    // Perform action
    if (m_game.get() != 0) {
        m_game->selections().setCurrentLayer(newLayer, m_game->currentTurn().universe());
    }

    // Signal other listeners
    session.notifyListeners();
}

inline void
game::proxy::SelectionProxy::Trampoline::executeCompiledExpression(Session& session, const String_t& compiledExpression, size_t targetLayer)
{
    // Perform action
    if (m_game.get() != 0) {
        m_game->selections().executeCompiledExpression(compiledExpression, targetLayer, m_game->currentTurn().universe());
    }

    // Signal other listeners
    session.notifyListeners();
}

inline void
game::proxy::SelectionProxy::Trampoline::executeCompiledExpressionAll(Session& session, const String_t& compiledExpression)
{
    // Perform action
    if (m_game.get() != 0) {
        m_game->selections().executeCompiledExpressionAll(compiledExpression, m_game->currentTurn().universe());
    }

    // Signal other listeners
    session.notifyListeners();
}

inline void
game::proxy::SelectionProxy::Trampoline::executeExpression(Session& session, const String_t& expression, size_t targetLayer)
{
    // Compile
    String_t compiledExpression;
    interpreter::Tokenizer tok(expression);
    SelectionExpression::compile(tok, compiledExpression);

    // Error?
    if (tok.getCurrentToken() != tok.tEnd) {
        throw interpreter::Error::garbageAtEnd(true);
    }

    // Execute
    if (m_game.get() != 0) {
        m_game->selections().executeCompiledExpression(compiledExpression, targetLayer, m_game->currentTurn().universe());
    }
    session.notifyListeners();
}



/*
 *  Proxy
 */

game::proxy::SelectionProxy::SelectionProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply)
    : m_reply(reply, *this),
      m_request(gameSender, new Trampoline(m_reply.getSender()))
{ }

game::proxy::SelectionProxy::~SelectionProxy()
{ }

// Get state, synchronously.
void
game::proxy::SelectionProxy::init(WaitIndicator& ind, Info& result)
{
    class Task : public util::SlaveRequest<Session, Trampoline> {
     public:
        Task(Info& result)
            : m_result(result)
            { }
        virtual void handle(Session&, Trampoline& tpl)
            { tpl.describe(m_result); }
     private:
        Info& m_result;
    };
    Task t(result);
    ind.call(m_request, t);
}

// Set current layer, asynchronously.
void
game::proxy::SelectionProxy::setCurrentLayer(size_t newLayer)
{
    class Task : public util::SlaveRequest<Session, Trampoline> {
     public:
        Task(size_t newLayer)
            : m_newLayer(newLayer)
            { }
        virtual void handle(Session& session, Trampoline& tpl)
            { tpl.setCurrentLayer(session, m_newLayer); }
     private:
        size_t m_newLayer;
    };
    m_request.postNewRequest(new Task(newLayer));
}

// Execute user-provided expression, synchronously.
bool
game::proxy::SelectionProxy::executeExpression(WaitIndicator& ind, const String_t& expression, size_t targetLayer, String_t& error)
{
    class Task : public util::SlaveRequest<Session, Trampoline> {
     public:
        Task(size_t targetLayer, String_t expression)
            : m_targetLayer(targetLayer), m_expression(expression),
              m_ok(false), m_error()
            { }
        virtual void handle(Session& session, Trampoline& tpl)
            {
                try {
                    tpl.executeExpression(session, m_expression, m_targetLayer);
                    m_ok = true;
                }
                catch (std::exception& e) {
                    m_error = e.what();
                }
            }
        bool isOK() const
            { return m_ok; }
        String_t getError() const
            { return m_error; }
     private:
        size_t m_targetLayer;
        String_t m_expression;
        bool m_ok;
        String_t m_error;
    };
    Task t(targetLayer, expression);
    ind.call(m_request, t);

    if (t.isOK()) {
        return true;
    } else {
        error = t.getError();
        return false;
    }
}

// Clear layer, asynchronously.
void
game::proxy::SelectionProxy::clearLayer(size_t targetLayer)
{
    executeCompiledExpression(CLEAR_CODE, targetLayer);
}

// Invert layer, asynchronously.
void
game::proxy::SelectionProxy::invertLayer(size_t targetLayer)
{
    // In executeCompiledExpression, opCurrent means current, not target, so we need to build a custom expression for each invocation
    const char INVERT_CODE[] = { char(SelectionExpression::opFirstLayer + targetLayer), SelectionExpression::opNot, 0 };
    executeCompiledExpression(INVERT_CODE, targetLayer);
}

// Clear all layers, asynchronously.
void
game::proxy::SelectionProxy::clearAllLayers()
{
    executeCompiledExpressionAll(CLEAR_CODE);
}

// Invert all layers, asynchronously.
void
game::proxy::SelectionProxy::invertAllLayers()
{
    const char INVERT_CODE[] = { SelectionExpression::opCurrent, SelectionExpression::opNot, 0 };
    executeCompiledExpressionAll(INVERT_CODE);
}

void
game::proxy::SelectionProxy::executeCompiledExpression(String_t compiledExpression, size_t targetLayer)
{
    class Task : public util::SlaveRequest<Session, Trampoline> {
     public:
        Task(size_t targetLayer, String_t compiledExpression)
            : m_targetLayer(targetLayer), m_compiledExpression(compiledExpression)
            { }
        virtual void handle(Session& session, Trampoline& tpl)
            { tpl.executeCompiledExpression(session, m_compiledExpression, m_targetLayer); }
     private:
        size_t m_targetLayer;
        String_t m_compiledExpression;
    };
    m_request.postNewRequest(new Task(targetLayer, compiledExpression));
}

void
game::proxy::SelectionProxy::executeCompiledExpressionAll(String_t compiledExpression)
{
    class Task : public util::SlaveRequest<Session, Trampoline> {
     public:
        Task(String_t compiledExpression)
            : m_compiledExpression(compiledExpression)
            { }
        virtual void handle(Session& session, Trampoline& tpl)
            { tpl.executeCompiledExpressionAll(session, m_compiledExpression); }
     private:
        String_t m_compiledExpression;
    };
    m_request.postNewRequest(new Task(compiledExpression));
}
