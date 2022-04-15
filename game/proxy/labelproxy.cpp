/**
  *  \file game/proxy/labelproxy.cpp
  *  \brief Class game::proxy::LabelProxy
  */

#include "game/proxy/labelproxy.hpp"
#include "game/interface/labelextra.hpp"

using game::interface::LabelExtra;
using game::interface::LabelVector;

namespace {
    void packError(afl::base::Optional<String_t>& out, const LabelVector& in)
    {
        if (in.hasError()) {
            out = in.getLastError();
        } else {
            out.clear();
        }
    }
}

/*
 *  Trampoline
 *
 *  To avoid race conditions, we forward LabelExtra::sig_update only when the user expects it.
 *  The signal is always connected but only forwarded when needed.
 *  Otherwise, UI requests are handed straight-through.
 */

class game::proxy::LabelProxy::Trampoline {
 public:
    Trampoline(Session& session, const util::RequestSender<LabelProxy>& reply)
        : m_session(session),
          m_reply(reply),
          m_expectChange(false),
          conn_labelChange()
        {
            if (LabelExtra* ex = LabelExtra::get(session)) {
                conn_labelChange = ex->sig_change.add(this, &Trampoline::onLabelChange);
            }
        }

    void getConfiguration(String_t& shipExpr, String_t& planetExpr);
    void setConfiguration(afl::base::Optional<String_t> shipExpr, afl::base::Optional<String_t> planetExpr);
    void onLabelChange(bool flag);
    void packStatus(Status& st);

 private:
    Session& m_session;
    util::RequestSender<LabelProxy> m_reply;
    bool m_expectChange;
    afl::base::SignalConnection conn_labelChange;
};

/* Implementation of LabelProxy::getConfiguration */
inline void
game::proxy::LabelProxy::Trampoline::getConfiguration(String_t& shipExpr, String_t& planetExpr)
{
    if (const LabelExtra* ex = LabelExtra::get(m_session)) {
        shipExpr   = ex->shipLabels().getExpression();
        planetExpr = ex->planetLabels().getExpression();
    } else {
        shipExpr.clear();
        planetExpr.clear();
    }
}

/* Implementation of LabelProxy::setConfiguration */
void
game::proxy::LabelProxy::Trampoline::setConfiguration(afl::base::Optional<String_t> shipExpr, afl::base::Optional<String_t> planetExpr)
{
    m_expectChange = true;
    if (LabelExtra* ex = LabelExtra::get(m_session)) {
        ex->setConfiguration(shipExpr, planetExpr);
    } else {
        onLabelChange(false);
    }
}

/* Handler for LabelExtra::sig_change */
void
game::proxy::LabelProxy::Trampoline::onLabelChange(bool /*flag*/)
{
    class Task : public util::Request<LabelProxy> {
     public:
        Task(Trampoline& tpl)
            { tpl.packStatus(m_status); }
        virtual void handle(LabelProxy& proxy)
            { proxy.sig_configurationApplied.raise(m_status); }
     private:
        Status m_status;
    };

    // Signal UI only if we're expecting a planned change
    if (m_expectChange) {
        m_expectChange = false;
        m_reply.postNewRequest(new Task(*this));
    }
}

/* Build status */
void
game::proxy::LabelProxy::Trampoline::packStatus(Status& st)
{
    if (const LabelExtra* ex = LabelExtra::get(m_session)) {
        packError(st.shipError,   ex->shipLabels());
        packError(st.planetError, ex->planetLabels());
    } else {
        st.shipError = st.planetError = m_session.translator()("Labels not available");
    }
}


/*
 *  TrampolineFromSession
 */

class game::proxy::LabelProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    TrampolineFromSession(const util::RequestSender<LabelProxy>& reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session, m_reply); }
 private:
    util::RequestSender<LabelProxy> m_reply;
};


/*
 *  LabelProxy
 */

game::proxy::LabelProxy::LabelProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& receiver)
    : m_receiver(receiver, *this),
      m_sender(gameSender.makeTemporary(new TrampolineFromSession(m_receiver.getSender())))
{ }

// Get active configuration.
void
game::proxy::LabelProxy::getConfiguration(WaitIndicator& ind, String_t& shipExpr, String_t& planetExpr)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(String_t& shipExpr, String_t& planetExpr)
            : m_shipExpr(shipExpr), m_planetExpr(planetExpr)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.getConfiguration(m_shipExpr, m_planetExpr); }
     private:
        String_t& m_shipExpr;
        String_t& m_planetExpr;
    };
    Task t(shipExpr, planetExpr);
    ind.call(m_sender, t);
}

// Set configuration.
void
game::proxy::LabelProxy::setConfiguration(afl::base::Optional<String_t> shipExpr, afl::base::Optional<String_t> planetExpr)
{
    m_sender.postRequest(&Trampoline::setConfiguration, shipExpr, planetExpr);
}
