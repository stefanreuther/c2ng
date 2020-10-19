/**
  *  \file game/proxy/cargotransferproxy.cpp
  *  \brief Class game::proxy::CargoTransferProxy
  */

#include "game/proxy/cargotransferproxy.hpp"
#include "game/actions/cargotransfer.hpp"
#include "game/actions/preconditions.hpp"
#include "game/turn.hpp"
#include "game/root.hpp"
#include "game/game.hpp"

/*
 *  Notifier: callback to UI
 */
class game::proxy::CargoTransferProxy::Notifier : public util::Request<CargoTransferProxy> {
 public:
    Notifier(size_t side, const Cargo& c)
        : m_side(side),
          m_cargo(c)
        { }
    virtual void handle(CargoTransferProxy& proxy)
        { proxy.sig_change.raise(m_side, m_cargo); }
 private:
    size_t m_side;
    Cargo m_cargo;
};

/*
 *  Observer: game-side object
 */
class game::proxy::CargoTransferProxy::Observer : public util::SlaveObject<Session> {
 public:
    Observer(util::RequestSender<CargoTransferProxy> reply)
        : transfer(),
          limit(),
          reply(reply)
        { transfer.sig_change.add(this, &Observer::onChange); }

    // Glue
    virtual void init(Session& session)
        { limit = Element::end(game::actions::mustHaveShipList(session)); }
    virtual void done(Session& /*session*/)
        { }

    void onChange()
        {
            // FIXME: as implemented, this notifier is O(n^2): a change to a container immediately notifies,
            // and each notification notifies all containers.
            for (size_t i = 0, n = transfer.getNumContainers(); i != n; ++i) {
                if (const CargoContainer* cont = transfer.get(i)) {
                    Cargo c;
                    getCargo(c, *cont, limit);
                    reply.postNewRequest(new Notifier(i, c));
                }
            }
        }

    // Data
    game::actions::CargoTransfer transfer;
    Element::Type limit;
    util::RequestSender<CargoTransferProxy> reply;
};


// Constructor.
game::proxy::CargoTransferProxy::CargoTransferProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply)
    : m_gameSender(gameSender),
      m_reply(reply, *this),
      m_observerSender(gameSender, new Observer(m_reply.getSender()))
{ }

// Initialize for two-unit setup.
void
game::proxy::CargoTransferProxy::init(const game::actions::CargoTransferSetup& setup)
{
    class Task : public util::SlaveRequest<Session, Observer> {
     public:
        Task(const game::actions::CargoTransferSetup& setup)
            : m_setup(setup)
            { }
        virtual void handle(Session& session, Observer& obs)
            {
                Turn& turn = game::actions::mustHaveGame(session).currentTurn();
                Root& root = game::actions::mustHaveRoot(session);

                m_setup.build(obs.transfer,
                              turn,
                              root.hostConfiguration(),
                              game::actions::mustHaveShipList(session),
                              root.hostVersion());

            }
     private:
        game::actions::CargoTransferSetup m_setup;
    };
    m_observerSender.postNewRequest(new Task(setup));
}

// Get general information.
void
game::proxy::CargoTransferProxy::getGeneralInformation(WaitIndicator& link, General& info)
{
    class Task : public util::SlaveRequest<Session, Observer> {
     public:
        Task(General& info)
            : m_info(info)
            { }

        virtual void handle(Session& session, Observer& obs)
            {
                // Clear in case the following throws
                m_info = General();

                game::spec::ShipList& shipList = game::actions::mustHaveShipList(session);
                afl::string::Translator& tx = session.translator();

                // Valid types
                m_info.validTypes = obs.transfer.getElementTypes(shipList);

                // Names etc
                for (Element::Type t = Element::begin(), e = Element::end(shipList); t != e; ++t) {
                    m_info.typeNames.set(t, Element::getName(t, tx, shipList));
                    m_info.typeUnits.set(t, Element::getUnit(t, tx, shipList));
                }

                // Actions
                m_info.allowUnload = obs.transfer.isUnloadAllowed();
                m_info.allowSupplySale = obs.transfer.isSupplySaleAllowed();
            }

     private:
        General& m_info;
    };

    Task t(info);
    link.call(m_observerSender, t);
}

// Get information about participant.
void
game::proxy::CargoTransferProxy::getParticipantInformation(WaitIndicator& link, size_t side, Participant& info)
{
    class Task : public util::SlaveRequest<Session, Observer> {
     public:
        Task(size_t side, Participant& info)
            : m_side(side), m_info(info)
            { }

        virtual void handle(Session& session, Observer& obs)
            {
                // Clean up
                m_info.name.clear();
                m_info.cargo.amount.clear();
                m_info.cargo.remaining.clear();
                m_info.isUnloadTarget = false;

                // Populate if possible
                if (const CargoContainer* c = obs.transfer.get(m_side)) {
                    m_info.name = c->getName(session.translator());
                    getCargo(m_info.cargo, *c, Element::end(game::actions::mustHaveShipList(session)));
                    m_info.isUnloadTarget = c->getFlags().contains(CargoContainer::UnloadTarget);
                }
            }

     private:
        size_t m_side;
        Participant& m_info;
    };

    Task t(side, info);
    link.call(m_observerSender, t);
}

// Move cargo.
void
game::proxy::CargoTransferProxy::move(Element::Type type, int32_t amount, size_t from, size_t to, bool sellSupplies)
{
    class Task : public util::SlaveRequest<Session, Observer> {
     public:
        Task(Element::Type type, int32_t amount, size_t from, size_t to, bool sellSupplies)
            : m_type(type), m_amount(amount), m_from(from), m_to(to), m_sellSupplies(sellSupplies)
            { }
        virtual void handle(Session& /*session*/, Observer& obs)
            { obs.transfer.move(m_type, m_amount, m_from, m_to, true, m_sellSupplies); }
     private:
        Element::Type m_type;
        int32_t m_amount;
        size_t m_from;
        size_t m_to;
        bool m_sellSupplies;
    };
    m_observerSender.postNewRequest(new Task(type, amount, from, to, sellSupplies));
}

// Unload.
void
game::proxy::CargoTransferProxy::unload(bool sellSupplies)
{
    class Task : public util::SlaveRequest<Session, Observer> {
     public:
        Task(bool sellSupplies)
            : m_sellSupplies(sellSupplies)
            { }
        virtual void handle(Session& /*session*/, Observer& obs)
            { obs.transfer.unload(m_sellSupplies); }
     private:
        bool m_sellSupplies;
    };
    m_observerSender.postNewRequest(new Task(sellSupplies));
}

// Commit the transaction.
void
game::proxy::CargoTransferProxy::commit()
{
    class Task : public util::SlaveRequest<Session, Observer> {
     public:
        virtual void handle(Session& /*session*/, Observer& obs)
            { obs.transfer.commit(); }
    };
    m_observerSender.postNewRequest(new Task());
}

void
game::proxy::CargoTransferProxy::getCargo(Cargo& out, const CargoContainer& cont, Element::Type limit)
{
    for (Element::Type t = Element::begin(); t != limit; ++t) {
        int32_t amount = cont.getEffectiveAmount(t);
        out.amount.set(t, amount);
        out.remaining.set(t, cont.getMaxAmount(t) - amount);
    }
}
