/**
  *  \file client/cargotransfer.cpp
  */

#include "client/cargotransfer.hpp"
#include "afl/base/signal.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/cargotransferdialog.hpp"
#include "client/proxy/cargotransferproxy.hpp"
#include "client/proxy/cargotransfersetupproxy.hpp"
#include "client/proxy/referencelistproxy.hpp"
#include "client/widgets/referencelistbox.hpp"
#include "game/actions/cargotransfersetup.hpp"
#include "game/game.hpp"
#include "game/map/anyshiptype.hpp"
#include "game/map/universe.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"

namespace {
    const int Special_Jettison = 1;
    const int Special_BeamUpMultiple = 2;

    class ObjectSelectionDialog {
     public:
        ObjectSelectionDialog(ui::Root& root, client::proxy::ReferenceListProxy& proxy)
            : m_root(root),
              m_list(root),
              m_proxy(proxy),
              m_loop(root)
            {
                m_list.setNumLines(15);
                m_list.setWidth(root.provider().getFont(gfx::FontRequest())->getCellSize().getX() * 40);
                m_proxy.sig_listChange.add(this, &ObjectSelectionDialog::onListChange);
            }

        void onListChange(const game::ref::UserList& list)
            { m_list.setContent(list); }

        game::Reference run(const String_t& title)
            {
                // ex WObjectList::doStandardDialog2, sort-of
                game::Reference result;
                if (ui::widgets::doStandardDialog(title, String_t(), m_list, false, m_root)) {
                    result = m_list.getCurrentReference();
                }
                return result;
            }

     private:
        ui::Root& m_root;
        client::widgets::ReferenceListbox m_list;
        client::proxy::ReferenceListProxy& m_proxy;
        ui::EventLoop m_loop;
    };
}

void
client::doCargoTransfer(ui::Root& root,
                        util::RequestSender<game::Session> gameSender,
                        afl::string::Translator& tx,
                        game::actions::CargoTransferSetup setup)
{
    // ex doCargoTransfer (sort-of)
    // FIXME: handle "proxy required" case
    // FIXME: handle "must cancel a transfer" case
    if (!setup.isValid()) {
        return;
    }

    // Prepare
    client::proxy::CargoTransferProxy proxy(root, gameSender);
    proxy.init(setup);

    // Build dialog
    client::dialogs::CargoTransferDialog dlg(root, proxy);
    if (dlg.run(tx, gameSender)) {
        proxy.commit();
    }
}


void
client::doShipCargoTransfer(ui::Root& root,
                            util::RequestSender<game::Session> gameSender,
                            afl::string::Translator& tx,
                            game::Id_t shipId)
{
    // ex doShipCargoTransfer
    class Initializer : public client::proxy::ReferenceListProxy::Initializer_t {
     public:
        Initializer(game::Id_t shipId)
            : m_shipId(shipId)
            { }

        virtual void call(game::Session& session, game::ref::ListObserver& obs)
            {
                game::Game* pGame = session.getGame().get();
                game::Root* pRoot = session.getRoot().get();
                if (pGame != 0 && pRoot != 0) {
                    game::ref::List objectList;
                    game::ref::UserList otherList;

                    game::map::Universe& univ = pGame->currentTurn().universe();
                    game::map::Ship* pShip = univ.ships().get(m_shipId);
                    game::map::Point pt;
                    int owner;
                    if (pShip != 0 && pShip->isPlayable(game::map::Object::Playable) && pShip->getOwner(owner) && pShip->getPosition(pt)) {
                        // Add the planet
                        if (game::Id_t pid = univ.getPlanetAt(pt)) {
                            if (game::actions::CargoTransferSetup::fromPlanetShip(univ, pid, m_shipId).isValid()) {
                                objectList.add(game::Reference(game::Reference::Planet, pid));
                            }
                            if (game::actions::CargoTransferSetup::fromShipBeamUp(pGame->currentTurn(), m_shipId, pRoot->hostConfiguration()).isValid()) {
                                otherList.add(game::ref::UserList::OtherItem,
                                              session.translator()("Beam up multiple"),
                                              game::Reference(game::Reference::Special, Special_BeamUpMultiple),
                                              false,
                                              util::SkinColor::Static);
                            }
                        } else {
                            if (game::actions::CargoTransferSetup::fromShipJettison(univ, m_shipId).isValid()) {
                                otherList.add(game::ref::UserList::OtherItem,
                                              session.translator()("Jettison into space"),
                                              game::Reference(game::Reference::Special, Special_Jettison),
                                              false,
                                              util::SkinColor::Static);
                            }
                        }

                        // Add ships
                        game::map::AnyShipType ty(univ);
                        for (game::Id_t sid = ty.findNextIndex(0); sid != 0; sid = ty.findNextIndex(sid)) {
                            if (game::actions::CargoTransferSetup::fromShipShip(univ, m_shipId, sid).isValid()) {
                                objectList.add(game::Reference(game::Reference::Ship, sid));
                            }
                        }
                    }

                    obs.setList(objectList);
                    obs.setExtra(otherList);
                }
            }
        virtual Initializer* clone() const
            { return new Initializer(m_shipId); }
     private:
        game::Id_t m_shipId;
    };

    Downlink link(root);
    client::proxy::ReferenceListProxy proxy(root, gameSender, tx);
    ObjectSelectionDialog dlg(root, proxy);
    proxy.setConfigurationSelection(game::ref::CARGO_TRANSFER);
    proxy.setContentNew(std::auto_ptr<client::proxy::ReferenceListProxy::Initializer_t>(new Initializer(shipId)));
    // FIXME: need to deal with empty list

    // Build a CargoTransferSetup
    game::Reference ref = dlg.run(tx("Transfer cargo to..."));
    client::proxy::CargoTransferSetupProxy setupProxy(gameSender);
    game::actions::CargoTransferSetup setup;
    switch (ref.getType()) {
     case game::Reference::Ship:
        setup = setupProxy.createShipShip(link, shipId, ref.getId());
        break;

     case game::Reference::Planet:
        setup = setupProxy.createPlanetShip(link, ref.getId(), shipId);
        setup.swapSides();
        break;

     case game::Reference::Special:
        switch (ref.getId()){
         case Special_Jettison:
            setup = setupProxy.createShipJettison(link, shipId);
            break;

         case Special_BeamUpMultiple:
            setup = setupProxy.createShipBeamUp(link, shipId);
            break;
        }

     default:
        break;
    }

    doCargoTransfer(root, gameSender, tx, setup);
}

void
client::doPlanetCargoTransfer(ui::Root& root,
                              util::RequestSender<game::Session> gameSender,
                              afl::string::Translator& tx,
                              game::Id_t planetId,
                              bool unload)
{
    // ex doPlanetTransfer, doPlanetTransferFor
    class Initializer : public client::proxy::ReferenceListProxy::Initializer_t {
     public:
        Initializer(game::Id_t planetId)
            : m_planetId(planetId)
            { }

        virtual void call(game::Session& session, game::ref::ListObserver& obs)
            {
                game::Game* pGame = session.getGame().get();
                if (pGame != 0) {
                    game::ref::List objectList;

                    // Add ships
                    game::map::Universe& univ = pGame->currentTurn().universe();
                    game::map::AnyShipType ty(univ);
                    for (game::Id_t sid = ty.findNextIndex(0); sid != 0; sid = ty.findNextIndex(sid)) {
                        if (game::actions::CargoTransferSetup::fromPlanetShip(univ, m_planetId, sid).isValid()) {
                            objectList.add(game::Reference(game::Reference::Ship, sid));
                        }
                    }

                    obs.setList(objectList);
                }
            }
        virtual Initializer* clone() const
            { return new Initializer(m_planetId); }
     private:
        game::Id_t m_planetId;
    };

    Downlink link(root);
    client::proxy::ReferenceListProxy proxy(root, gameSender, tx);
    ObjectSelectionDialog dlg(root, proxy);
    proxy.setConfigurationSelection(game::ref::CARGO_TRANSFER);
    proxy.setContentNew(std::auto_ptr<client::proxy::ReferenceListProxy::Initializer_t>(new Initializer(planetId)));
    // FIXME: need to deal with empty list
    // messageBox(_("There's none of our ships orbiting this planet."),
    //            _("Cargo Transfer"));

    // Build a CargoTransferSetup
    game::Reference ref = dlg.run(unload ? tx("Unload ship...") : tx("Transfer cargo to..."));
    client::proxy::CargoTransferSetupProxy setupProxy(gameSender);
    doCargoTransfer(root, gameSender, tx, setupProxy.createPlanetShip(link, planetId, ref.getId()));
}
