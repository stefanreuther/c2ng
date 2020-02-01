/**
  *  \file client/proxy/convertsuppliesproxy.cpp
  *  \brief Class client::proxy::ConvertSuppliesProxy
  */

#include <memory>
#include "client/proxy/convertsuppliesproxy.hpp"
#include "game/actions/convertsupplies.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/map/universe.hpp"
#include "game/turn.hpp"

using game::actions::ConvertSupplies;

class client::proxy::ConvertSuppliesProxy::Trampoline : public util::SlaveObject<game::Session> {
 public:
    virtual void init(game::Session&)
        { }

    virtual void done(game::Session&)
        { m_action.reset(); }

    Status init(game::Session& session, game::Id_t planetId, int32_t reservedSupplies, int32_t reservedMoney)
        {
            // Reset previous state, if any
            m_action.reset();

            // Build new state
            Status st;
            try {
                game::Game& g = game::actions::mustHaveGame(session);
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

 private:
    std::auto_ptr<ConvertSupplies> m_action;
};

client::proxy::ConvertSuppliesProxy::ConvertSuppliesProxy(util::RequestSender<game::Session> gameSender)
    : m_slave(gameSender, new Trampoline())
{ }

client::proxy::ConvertSuppliesProxy::Status
client::proxy::ConvertSuppliesProxy::init(Downlink& link, game::Id_t planetId, int32_t reservedSupplies, int32_t reservedMoney)
{
    class Task : public util::SlaveRequest<game::Session, Trampoline> {
     public:
        Task(game::Id_t planetId, int32_t reservedSupplies, int32_t reservedMoney)
            : m_planetId(planetId),
              m_reservedSupplies(reservedSupplies),
              m_reservedMoney(reservedMoney),
              m_status()
            { }
        virtual void handle(game::Session& session, Trampoline& tpl)
            { m_status = tpl.init(session, m_planetId, m_reservedSupplies, m_reservedMoney); }
        const Status& get() const
            { return m_status; }
     private:
        game::Id_t m_planetId;
        int32_t m_reservedSupplies;
        int32_t m_reservedMoney;
        Status m_status;
    };
    Task t(planetId, reservedSupplies, reservedMoney);
    link.call(m_slave, t);
    return t.get();
}

void
client::proxy::ConvertSuppliesProxy::sellSupplies(int32_t amount)
{
    class Task : public util::SlaveRequest<game::Session, Trampoline> {
     public:
        Task(int32_t amount)
            : m_amount(amount)
            { }
        virtual void handle(game::Session& session, Trampoline& tpl)
            {
                if (ConvertSupplies* pa = tpl.getAction()) {
                    pa->sellSupplies(m_amount, true);

                    // Notify listeners.
                    // THIS IS A HACK.
                    // If the sell-supplies dialog is invoked from another dialog,
                    // it does not have a script to drive the notifications,
                    // causing its result not be re-considered from the other dialog's action.
                    session.notifyListeners();
                }
            }
     private:
        int32_t m_amount;
    };
    m_slave.postNewRequest(new Task(amount));
}

void
client::proxy::ConvertSuppliesProxy::buySupplies(int32_t amount)
{
    sellSupplies(-amount);
}
