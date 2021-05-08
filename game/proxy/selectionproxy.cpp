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

using game::map::Selections;
using game::map::SelectionVector;
using interpreter::SelectionExpression;

namespace {
    const char CLEAR_CODE[] = { SelectionExpression::opZero, 0 };
}


/*
 *  Trampoline
 */

class game::proxy::SelectionProxy::Trampoline {
 public:
    Trampoline(Session& session, const util::RequestSender<SelectionProxy>& reply);
    void onSelectionChange();
    void describe(Info& info) const;
    void setCurrentLayer(LayerReference_t newLayer);
    void executeCompiledExpression(String_t compiledExpression, LayerReference_t targetLayer);
    void executeCompiledExpressionAll(String_t compiledExpression);
    void executeExpression(const String_t& expression, LayerReference_t targetLayer);
    void markList(LayerReference_t targetLayer, const game::ref::List& list, bool mark);
    void markObjectsInRange(game::map::Point a, game::map::Point b, bool revertFirst);
    void revertCurrentLayer();

 private:
    Session& m_session;
    util::RequestSender<SelectionProxy> m_reply;
    afl::base::Ptr<Game> m_game;
    afl::base::SignalConnection conn_selectionChange;
};


class game::proxy::SelectionProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(const util::RequestSender<SelectionProxy>& reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session, m_reply); }
 private:
    util::RequestSender<SelectionProxy> m_reply;
};



inline
game::proxy::SelectionProxy::Trampoline::Trampoline(Session& session, const util::RequestSender<SelectionProxy>& reply)
    : m_session(session), m_reply(reply), m_game(), conn_selectionChange()
{
    m_game = session.getGame();
    if (m_game.get() != 0) {
        // Register for changes
        Selections& sel = m_game->selections();
        conn_selectionChange = sel.sig_selectionChange.add(this, &Trampoline::onSelectionChange);

        // Update multi-selection view
        // ex client/dialogs/selmgr.cc:doSelectionManager, WSelectChartMode::WSelectChartMode
        game::map::Universe& univ = m_game->currentTurn().universe();
        sel.copyFrom(univ, sel.getCurrentLayer());
        sel.limitToExistingObjects(univ, sel.getCurrentLayer());
    }
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
game::proxy::SelectionProxy::Trampoline::setCurrentLayer(LayerReference_t newLayer)
{
    // Perform action
    if (m_game.get() != 0) {
        m_game->selections().setCurrentLayer(newLayer.resolve(m_game->selections()), m_game->currentTurn().universe());
    }

    // Signal other listeners
    m_session.notifyListeners();
}

inline void
game::proxy::SelectionProxy::Trampoline::executeCompiledExpression(String_t compiledExpression, LayerReference_t targetLayer)
{
    // Perform action
    if (m_game.get() != 0) {
        m_game->selections().executeCompiledExpression(compiledExpression, targetLayer, m_game->currentTurn().universe());
    }

    // Signal other listeners
    m_session.notifyListeners();
}

inline void
game::proxy::SelectionProxy::Trampoline::executeCompiledExpressionAll(String_t compiledExpression)
{
    // Perform action
    if (m_game.get() != 0) {
        m_game->selections().executeCompiledExpressionAll(compiledExpression, m_game->currentTurn().universe());
    }

    // Signal other listeners
    m_session.notifyListeners();
}

void
game::proxy::SelectionProxy::Trampoline::executeExpression(const String_t& expression, LayerReference_t targetLayer)
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
    m_session.notifyListeners();
}

void
game::proxy::SelectionProxy::Trampoline::markList(LayerReference_t targetLayer, const game::ref::List& list, bool mark)
{
    // Perform action
    if (m_game.get() != 0) {
        m_game->selections().markList(targetLayer, list, mark, m_game->currentTurn().universe());
    }

    // Signal other listeners
    m_session.notifyListeners();
}

void
game::proxy::SelectionProxy::Trampoline::markObjectsInRange(game::map::Point a, game::map::Point b, bool revertFirst)
{
    // Perform action
    if (m_game.get() != 0) {
        game::map::Universe& univ = m_game->currentTurn().universe();
        game::map::Selections& sel = m_game->selections();
        if (revertFirst) {
            sel.copyTo(univ, sel.getCurrentLayer());
        }
        int n = univ.markObjectsInRange(a, b);

        // Response
        m_reply.postRequest(&SelectionProxy::reportObjectsInRange, n);
    }

    // Signal other listeners
    m_session.notifyListeners();
}

void
game::proxy::SelectionProxy::Trampoline::revertCurrentLayer()
{
    // Perform action
    if (m_game.get() != 0) {
        game::map::Universe& univ = m_game->currentTurn().universe();
        game::map::Selections& sel = m_game->selections();

        sel.copyTo(univ, sel.getCurrentLayer());
    }

    // Signal other listeners
    m_session.notifyListeners();
}


/*
 *  Proxy
 */

game::proxy::SelectionProxy::SelectionProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply)
    : m_reply(reply, *this),
      m_request(gameSender.makeTemporary(new TrampolineFromSession(m_reply.getSender())))
{ }

game::proxy::SelectionProxy::~SelectionProxy()
{ }

// Get state, synchronously.
void
game::proxy::SelectionProxy::init(WaitIndicator& ind, Info& result)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(Info& result)
            : m_result(result)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.describe(m_result); }
     private:
        Info& m_result;
    };
    Task t(result);
    ind.call(m_request, t);
}

// Set current layer, asynchronously.
void
game::proxy::SelectionProxy::setCurrentLayer(LayerReference_t newLayer)
{
    m_request.postRequest(&Trampoline::setCurrentLayer, newLayer);
}

// Execute user-provided expression, synchronously.
bool
game::proxy::SelectionProxy::executeExpression(WaitIndicator& ind, const String_t& expression, LayerReference_t targetLayer, String_t& error)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(LayerReference_t targetLayer, String_t expression)
            : m_targetLayer(targetLayer), m_expression(expression),
              m_ok(false), m_error()
            { }
        virtual void handle(Trampoline& tpl)
            {
                try {
                    tpl.executeExpression(m_expression, m_targetLayer);
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
        LayerReference_t m_targetLayer;
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

// Mark objects given as list, asynchronously.
void
game::proxy::SelectionProxy::markList(LayerReference_t targetLayer, const game::ref::List& list, bool mark)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(LayerReference_t targetLayer, const game::ref::List& list, bool mark)
            : m_targetLayer(targetLayer), m_list(list), m_mark(mark)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.markList(m_targetLayer, m_list, m_mark); }
     private:
        LayerReference_t m_targetLayer;
        game::ref::List m_list;
        bool m_mark;
    };
    m_request.postNewRequest(new Task(targetLayer, list, mark));

}

// Clear layer, asynchronously.
void
game::proxy::SelectionProxy::clearLayer(LayerReference_t targetLayer)
{
    executeCompiledExpression(CLEAR_CODE, targetLayer);
}

// Invert layer, asynchronously.
void
game::proxy::SelectionProxy::invertLayer(size_t targetLayer)
{
    // In executeCompiledExpression, opCurrent means current, not target, so we need to build a custom expression for each invocation
    // This means we cannot currently take a LayerReference_t.
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

// Mark objects in range, asynchronously.
void
game::proxy::SelectionProxy::markObjectsInRange(game::map::Point a, game::map::Point b, bool revertFirst)
{
    m_request.postRequest(&Trampoline::markObjectsInRange, a, b, revertFirst);
}

// Revert current layer, asynchronously.
void
game::proxy::SelectionProxy::revertCurrentLayer()
{
    m_request.postRequest(&Trampoline::revertCurrentLayer);
}


void
game::proxy::SelectionProxy::executeCompiledExpression(String_t compiledExpression, LayerReference_t targetLayer)
{
    m_request.postRequest(&Trampoline::executeCompiledExpression, compiledExpression, targetLayer);
}

void
game::proxy::SelectionProxy::executeCompiledExpressionAll(String_t compiledExpression)
{
    m_request.postRequest(&Trampoline::executeCompiledExpressionAll, compiledExpression);
}

void
game::proxy::SelectionProxy::reportObjectsInRange(int n)
{
    sig_numObjectsInRange.raise(n);
}
