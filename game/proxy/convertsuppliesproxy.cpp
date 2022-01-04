/**
  *  \file game/proxy/convertsuppliesproxy.cpp
  *  \brief Class game::proxy::ConvertSuppliesProxy
  */

#include <memory>
#include "game/proxy/convertsuppliesproxy.hpp"
#include "game/actions/convertsupplies.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/map/universe.hpp"
#include "game/turn.hpp"

using game::actions::ConvertSupplies;

class game::proxy::ConvertSuppliesProxy::Trampoline {
 public:
    Trampoline(Session& session)
        : m_session(session)
        { }

    Status init(Id_t planetId, int32_t reservedSupplies, int32_t reservedMoney)
        {
            // Reset previous state, if any
            m_action.reset();

            // Build new state
            Status st;
            try {
                Game& g = game::actions::mustHaveGame(m_session);
                game::map::Universe& univ = g.currentTurn().universe();
                game::map::Planet& pl = game::actions::mustExist(univ.planets().get(planetId));

                m_action.reset(new ConvertSupplies(pl));
                m_action->setUndoInformation(univ);
                m_action->setReservedSupplies(reservedSupplies);
                m_action->setReservedMoney(reservedMoney);

                st.maxSuppliesToSell = m_action->getMaxSuppliesToSell();
                st.maxSuppliesToBuy  = m_action->getMaxSuppliesToBuy();
                st.valid = true;
            }
            catch (std::exception& e) {
                // Leave status default-initialized, i.e. valid=false
            }
            return st;
        }

    ConvertSupplies* getAction()
        { return m_action.get(); }

    void notifyListeners()
        { m_session.notifyListeners(); }

 private:
    Session& m_session;
    std::auto_ptr<ConvertSupplies> m_action;
};


class game::proxy::ConvertSuppliesProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session); }
};


game::proxy::ConvertSuppliesProxy::ConvertSuppliesProxy(util::RequestSender<Session> gameSender)
    : m_trampoline(gameSender.makeTemporary(new TrampolineFromSession()))
{ }

game::proxy::ConvertSuppliesProxy::Status
game::proxy::ConvertSuppliesProxy::init(WaitIndicator& link, Id_t planetId, int32_t reservedSupplies, int32_t reservedMoney)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(Id_t planetId, int32_t reservedSupplies, int32_t reservedMoney)
            : m_planetId(planetId),
              m_reservedSupplies(reservedSupplies),
              m_reservedMoney(reservedMoney),
              m_status()
            { }
        virtual void handle(Trampoline& tpl)
            { m_status = tpl.init(m_planetId, m_reservedSupplies, m_reservedMoney); }
        const Status& get() const
            { return m_status; }
     private:
        Id_t m_planetId;
        int32_t m_reservedSupplies;
        int32_t m_reservedMoney;
        Status m_status;
    };
    Task t(planetId, reservedSupplies, reservedMoney);
    link.call(m_trampoline, t);
    return t.get();
}

void
game::proxy::ConvertSuppliesProxy::sellSupplies(int32_t amount)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(int32_t amount)
            : m_amount(amount)
            { }
        virtual void handle(Trampoline& tpl)
            {
                if (ConvertSupplies* pa = tpl.getAction()) {
                    pa->sellSupplies(m_amount, true);

                    // Notify listeners.
                    // THIS IS A HACK.
                    // If the sell-supplies dialog is invoked from another dialog,
                    // it does not have a script to drive the notifications,
                    // causing its result not be re-considered from the other dialog's action.
                    tpl.notifyListeners();
                }
            }
     private:
        int32_t m_amount;
    };
    m_trampoline.postNewRequest(new Task(amount));
}

void
game::proxy::ConvertSuppliesProxy::buySupplies(int32_t amount)
{
    sellSupplies(-amount);
}
