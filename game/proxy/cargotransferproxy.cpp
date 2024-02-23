/**
  *  \file game/proxy/cargotransferproxy.cpp
  *  \brief Class game::proxy::CargoTransferProxy
  */

#include "game/proxy/cargotransferproxy.hpp"
#include "game/actions/cargotransfer.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"

using afl::base::Ref;
using game::actions::CargoTransferSetup;
using game::actions::MultiTransferSetup;

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
class game::proxy::CargoTransferProxy::Observer {
 public:
    Observer(Session& session, const util::RequestSender<CargoTransferProxy>& reply)
        : session(session),
          game(game::actions::mustHaveGame(session)),
          root(game::actions::mustHaveRoot(session)),
          shipList(game::actions::mustHaveShipList(session)),
          turn(game->viewpointTurn()),
          transfer(),
          limit(Element::end(*shipList)),
          reply(reply)
        {
            transfer.sig_change.add(this, &Observer::onChange);
        }

    void onChange()
        {
            for (size_t i = 0, n = transfer.getNumContainers(); i != n; ++i) {
                if (const CargoContainer* cont = transfer.get(i)) {
                    Cargo c;
                    getCargo(c, *cont, limit);
                    reply.postNewRequest(new Notifier(i, c));
                }
            }
        }

    // Data
    Session& session;
    Ref<Game> game;
    Ref<const Root> root;
    Ref<const game::spec::ShipList> shipList;
    Turn& turn;
    game::actions::CargoTransfer transfer;
    const Element::Type limit;
    util::RequestSender<CargoTransferProxy> reply;
};


class game::proxy::CargoTransferProxy::ObserverFromSession : public afl::base::Closure<Observer*(Session&)> {
 public:
    ObserverFromSession(const util::RequestSender<CargoTransferProxy>& proxy)
        : m_proxy(proxy)
        { }
    virtual Observer* call(Session& session)
        { return new Observer(session, m_proxy); }
 private:
    util::RequestSender<CargoTransferProxy> m_proxy;
};


// Constructor.
game::proxy::CargoTransferProxy::CargoTransferProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply)
    : m_gameSender(gameSender),
      m_reply(reply, *this),
      m_observerSender(gameSender.makeTemporary(new ObserverFromSession(m_reply.getSender())))
{ }

// Initialize for two-unit setup.
void
game::proxy::CargoTransferProxy::init(const game::actions::CargoTransferSetup& setup)
{
    class Task : public util::Request<Observer> {
     public:
        Task(const CargoTransferSetup& setup)
            : m_setup(setup)
            { }
        virtual void handle(Observer& obs)
            { m_setup.build(obs.transfer, obs.turn, obs.game->mapConfiguration(), obs.root->hostConfiguration(), *obs.shipList, obs.root->hostVersion()); }
     private:
        CargoTransferSetup m_setup;
    };
    m_observerSender.postNewRequest(new Task(setup));
}

// Initialize for multi-unit setup.
game::actions::MultiTransferSetup::Result
game::proxy::CargoTransferProxy::init(WaitIndicator& link, const game::actions::MultiTransferSetup& setup)
{
    class Task : public util::Request<Observer> {
     public:
        Task(MultiTransferSetup::Result& result, const MultiTransferSetup& setup)
            : m_result(result), m_setup(setup)
            { }
        virtual void handle(Observer& obs)
            { m_result = m_setup.build(obs.transfer, obs.turn.universe(), obs.session); }
     private:
        MultiTransferSetup::Result& m_result;
        MultiTransferSetup m_setup;
    };

    MultiTransferSetup::Result result;
    Task t(result, setup);
    link.call(m_observerSender, t);
    return result;
}

// Add new hold space.
void
game::proxy::CargoTransferProxy::addHoldSpace(const String_t& name)
{
    class Task : public util::Request<Observer> {
     public:
        Task(const String_t& name)
            : m_name(name)
            { }
        virtual void handle(Observer& obs)
            { obs.transfer.addHoldSpace(m_name); }
     private:
        String_t m_name;
    };
    m_observerSender.postNewRequest(new Task(name));
}

// Get general information.
void
game::proxy::CargoTransferProxy::getGeneralInformation(WaitIndicator& link, General& info)
{
    class Task : public util::Request<Observer> {
     public:
        Task(General& info)
            : m_info(info)
            { }

        virtual void handle(Observer& obs)
            {
                // Clear in case the following throws
                m_info = General();

                afl::string::Translator& tx = obs.session.translator();

                // Valid types
                m_info.validTypes = obs.transfer.getElementTypes(*obs.shipList);

                // Names etc
                for (Element::Type t = Element::begin(), e = obs.limit; t != e; ++t) {
                    m_info.typeNames.set(t, Element::getName(t, tx, *obs.shipList));
                    m_info.typeUnits.set(t, Element::getUnit(t, tx, *obs.shipList));
                }

                // Actions
                m_info.allowUnload = obs.transfer.isUnloadAllowed();
                m_info.allowSupplySale = obs.transfer.isSupplySaleAllowed();

                // Number of participant
                m_info.numParticipants = obs.transfer.getNumContainers();
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
    class Task : public util::Request<Observer> {
     public:
        Task(size_t side, Participant& info)
            : m_side(side), m_info(info)
            { }

        virtual void handle(Observer& obs)
            {
                // Clean up
                m_info.name.clear();
                m_info.info1.clear();
                m_info.info2.clear();
                m_info.cargo.amount.clear();
                m_info.cargo.remaining.clear();
                m_info.isUnloadTarget = false;
                m_info.isTemporary = false;

                // Populate if possible
                if (const CargoContainer* c = obs.transfer.get(m_side)) {
                    m_info.name = c->getName(obs.session.translator());
                    m_info.info1 = c->getInfo1(obs.session.translator());
                    m_info.info2 = c->getInfo2(obs.session.translator());
                    getCargo(m_info.cargo, *c, obs.limit);

                    CargoContainer::Flags_t flags = c->getFlags();
                    m_info.isUnloadTarget = flags.contains(CargoContainer::UnloadTarget);
                    m_info.isTemporary = flags.contains(CargoContainer::Temporary);
                }
            }

     private:
        size_t m_side;
        Participant& m_info;
    };

    Task t(side, info);
    link.call(m_observerSender, t);
}

// Set overload permission.
void
game::proxy::CargoTransferProxy::setOverload(bool enable)
{
    class Task : public util::Request<Observer> {
     public:
        Task(bool enable)
            : m_enable(enable)
            { }
        virtual void handle(Observer& obs)
            { obs.transfer.setOverload(m_enable); }
     private:
        bool m_enable;
    };
    m_observerSender.postNewRequest(new Task(enable));
}

// Move cargo.
void
game::proxy::CargoTransferProxy::move(Element::Type type, int32_t amount, size_t from, size_t to, bool sellSupplies)
{
    class Task : public util::Request<Observer> {
     public:
        Task(Element::Type type, int32_t amount, size_t from, size_t to, bool sellSupplies)
            : m_type(type), m_amount(amount), m_from(from), m_to(to), m_sellSupplies(sellSupplies)
            { }
        virtual void handle(Observer& obs)
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

// Move with extension.
void
game::proxy::CargoTransferProxy::moveExt(Element::Type type, int32_t amount, size_t from, size_t to, size_t extension, bool sellSupplies)
{
    class Task : public util::Request<Observer> {
     public:
        Task(Element::Type type, int32_t amount, size_t from, size_t to, size_t ext, bool sellSupplies)
            : m_type(type), m_amount(amount), m_from(from), m_to(to), m_extension(ext), m_sellSupplies(sellSupplies)
            { }
        virtual void handle(Observer& obs)
            { obs.transfer.moveExt(m_type, m_amount, m_from, m_to, m_extension, m_sellSupplies); }
     private:
        Element::Type m_type;
        int32_t m_amount;
        size_t m_from;
        size_t m_to;
        size_t m_extension;
        bool m_sellSupplies;
    };
    m_observerSender.postNewRequest(new Task(type, amount, from, to, extension, sellSupplies));
}

// Move all cargo to a given unit.
void
game::proxy::CargoTransferProxy::moveAll(Element::Type type, size_t to, size_t except, bool sellSupplies)
{
    class Task : public util::Request<Observer> {
     public:
        Task(Element::Type type, size_t to, size_t except, bool sellSupplies)
            : m_type(type), m_to(to), m_except(except), m_sellSupplies(sellSupplies)
            { }
        virtual void handle(Observer& obs)
            { obs.transfer.moveAll(m_type, m_to, m_except, m_sellSupplies); }
     private:
        Element::Type m_type;
        size_t m_to;
        size_t m_except;
        bool m_sellSupplies;
    };
    m_observerSender.postNewRequest(new Task(type, to, except, sellSupplies));
}

// Distribute cargo.
void
game::proxy::CargoTransferProxy::distribute(Element::Type type, size_t from, size_t except, game::actions::CargoTransfer::DistributeMode mode)
{
    class Task : public util::Request<Observer> {
     public:
        Task(Element::Type type, size_t from, size_t except, game::actions::CargoTransfer::DistributeMode mode)
            : m_type(type), m_from(from), m_except(except), m_mode(mode)
            { }
        virtual void handle(Observer& obs)
            { obs.transfer.distribute(m_type, m_from, m_except, m_mode); }
     private:
        Element::Type m_type;
        size_t m_from;
        size_t m_except;
        game::actions::CargoTransfer::DistributeMode m_mode;
    };
    m_observerSender.postNewRequest(new Task(type, from, except, mode));
}

// Unload.
void
game::proxy::CargoTransferProxy::unload(bool sellSupplies)
{
    class Task : public util::Request<Observer> {
     public:
        Task(bool sellSupplies)
            : m_sellSupplies(sellSupplies)
            { }
        virtual void handle(Observer& obs)
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
    class Task : public util::Request<Observer> {
     public:
        virtual void handle(Observer& obs)
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
