/**
  *  \file client/cargotransfer.cpp
  */

#include "client/cargotransfer.hpp"
#include "afl/base/signal.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/cargotransferdialog.hpp"
#include "client/downlink.hpp"
#include "client/widgets/referencelistbox.hpp"
#include "game/actions/cargotransfersetup.hpp"
#include "game/game.hpp"
#include "game/map/anyshiptype.hpp"
#include "game/map/universe.hpp"
#include "game/proxy/cargotransferproxy.hpp"
#include "game/proxy/cargotransfersetupproxy.hpp"
#include "game/proxy/referencelistproxy.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"

namespace {
    const int Special_Jettison = 1;
    const int Special_BeamUpMultiple = 2;

    class ObjectSelectionDialog {
     public:
        ObjectSelectionDialog(ui::Root& root, afl::string::Translator& tx, game::proxy::ReferenceListProxy& proxy)
            : m_root(root),
              m_translator(tx),
              m_list(root),
              m_proxy(proxy)
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
                if (m_list.doStandardDialog(title, String_t(), 0, m_root, m_translator)) {
                    result = m_list.getCurrentReference();
                }
                return result;
            }

        bool isEmpty() const
            { return m_list.getNumItems() == 0; }

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        client::widgets::ReferenceListbox m_list;
        game::proxy::ReferenceListProxy& m_proxy;
    };

    bool solveConflicts(game::proxy::WaitIndicator& ind, ui::Root& root, afl::string::Translator& tx, game::proxy::CargoTransferSetupProxy& setupProxy)
    {
        while (const game::proxy::CargoTransferSetupProxy::ConflictInfo* info = setupProxy.getConflictInfo()) {
            if (!ui::dialogs::MessageBox(afl::string::Format(tx("Ship %s (#%d) is currently transferring to %s (#%d). "
                                                                "You can only transfer to one ship at a time.\n"
                                                                "Cancel existing transfer to proceed?"),
                                                             info->fromName,
                                                             info->fromId,
                                                             info->toName,
                                                             info->toId),
                                         tx("Cargo Transfer"), root).doYesNoDialog(tx))
            {
                return false;
            }
            setupProxy.cancelConflictingTransfer(ind);
        }
        return true;
    }
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
    // FIXME: PCC1: IF (pus^.Neutronium=0) AND (NOT ActiveTransfer(pus^.Unload))
    //               AND (NOT ActiveTransfer(pus^.Transfer)) THEN BEGIN  { - no fuel, no transfer }
    //                IF NOT NYesNo('You do not have fuel to use the Cargo Transporter. Continue anyway (cargo will be destroyed)?', 'Cargo Transfer') THEN Exit;
    //              END;
    if (!setup.isValid()) {
        return;
    }

    // Prepare
    game::proxy::CargoTransferProxy proxy(gameSender, root.engine().dispatcher());
    proxy.init(setup);

    // Build dialog
    client::dialogs::CargoTransferDialog dlg(root, tx, proxy);
    if (dlg.run(gameSender)) {
        proxy.commit();
    }
}


void
client::doShipCargoTransfer(ui::Root& root,
                            util::RequestSender<game::Session> gameSender,
                            afl::string::Translator& tx,
                            game::Id_t shipId)
{
    // ex doShipCargoTransfer, doShipTransferFor
    class Initializer : public game::proxy::ReferenceListProxy::Initializer_t {
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

                    game::map::Universe& univ = pGame->viewpointTurn().universe();
                    game::map::Ship* pShip = univ.ships().get(m_shipId);
                    game::map::Point pt;
                    int owner;
                    if (pShip != 0 && pShip->isPlayable(game::map::Object::Playable) && pShip->getOwner().get(owner) && pShip->getPosition().get(pt)) {
                        // Add the planet
                        if (game::Id_t pid = univ.findPlanetAt(pt)) {
                            if (game::actions::CargoTransferSetup::fromPlanetShip(univ, pid, m_shipId).isValid()) {
                                objectList.add(game::Reference(game::Reference::Planet, pid));
                            }
                            if (game::actions::CargoTransferSetup::fromShipBeamUp(pGame->viewpointTurn(), m_shipId, pRoot->hostConfiguration()).isValid()) {
                                otherList.add(game::ref::UserList::OtherItem,
                                              session.translator()("Beam up multiple"),
                                              game::Reference(game::Reference::Special, Special_BeamUpMultiple),
                                              false,
                                              game::map::Object::Playable,
                                              util::SkinColor::Static);
                            }
                        } else {
                            if (game::actions::CargoTransferSetup::fromShipJettison(univ, m_shipId).isValid()) {
                                otherList.add(game::ref::UserList::OtherItem,
                                              session.translator()("Jettison into space"),
                                              game::Reference(game::Reference::Special, Special_Jettison),
                                              false,
                                              game::map::Object::Playable,
                                              util::SkinColor::Static);
                            }
                        }

                        // Add ships
                        const game::map::AnyShipType& ty(univ.allShips());
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
     private:
        game::Id_t m_shipId;
    };

    Downlink link(root, tx);
    game::proxy::ReferenceListProxy proxy(gameSender, root.engine().dispatcher());
    ObjectSelectionDialog dlg(root, tx, proxy);
    proxy.setConfigurationSelection(game::ref::CARGO_TRANSFER);
    proxy.setContentNew(std::auto_ptr<game::proxy::ReferenceListProxy::Initializer_t>(new Initializer(shipId)));
    proxy.waitIdle(link);

    if (dlg.isEmpty()) {
        ui::dialogs::MessageBox(tx("There's no other unit here we could transfer to or from."),
                                tx("Cargo Transfer"),
                                root).doOkDialog(tx);
    } else {
        // Build a CargoTransferSetup
        game::Reference ref = dlg.run(tx("Transfer cargo to..."));
        game::proxy::CargoTransferSetupProxy setupProxy(gameSender);
        switch (ref.getType()) {
         case game::Reference::Ship:
            setupProxy.createShipShip(link, shipId, ref.getId());
            break;

         case game::Reference::Planet:
            setupProxy.createPlanetShip(link, ref.getId(), shipId);
            setupProxy.swapSides();
            break;

         case game::Reference::Special:
            switch (ref.getId()){
             case Special_Jettison:
                setupProxy.createShipJettison(link, shipId);
                break;

             case Special_BeamUpMultiple:
                setupProxy.createShipBeamUp(link, shipId);
                break;
            }

         default:
            break;
        }

        if (!solveConflicts(link, root, tx, setupProxy)) {
            return;
        }

        doCargoTransfer(root, gameSender, tx, setupProxy.get());
    }
}

void
client::doPlanetCargoTransfer(ui::Root& root,
                              util::RequestSender<game::Session> gameSender,
                              afl::string::Translator& tx,
                              game::Id_t planetId,
                              bool unload)
{
    // ex doPlanetTransfer, doPlanetTransferFor, pdata.pas:PlanetTransferCargo
    class Initializer : public game::proxy::ReferenceListProxy::Initializer_t {
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
                    game::map::Universe& univ = pGame->viewpointTurn().universe();
                    const game::map::AnyShipType& ty(univ.allShips());
                    for (game::Id_t sid = ty.findNextIndex(0); sid != 0; sid = ty.findNextIndex(sid)) {
                        if (game::actions::CargoTransferSetup::fromPlanetShip(univ, m_planetId, sid).isValid()) {
                            objectList.add(game::Reference(game::Reference::Ship, sid));
                        }
                    }

                    obs.setList(objectList);
                }
            }
     private:
        game::Id_t m_planetId;
    };

    Downlink link(root, tx);
    game::proxy::ReferenceListProxy proxy(gameSender, root.engine().dispatcher());
    ObjectSelectionDialog dlg(root, tx, proxy);
    proxy.setConfigurationSelection(game::ref::CARGO_TRANSFER);
    proxy.setContentNew(std::auto_ptr<game::proxy::ReferenceListProxy::Initializer_t>(new Initializer(planetId)));
    proxy.waitIdle(link);

    if (dlg.isEmpty()) {
        ui::dialogs::MessageBox(tx("There's none of our ships orbiting this planet."),
                                tx("Cargo Transfer"),
                                root).doOkDialog(tx);
    } else {
        // Build a CargoTransferSetup
        game::Reference ref = dlg.run(unload ? tx("Unload ship...") : tx("Transfer cargo to..."));
        game::proxy::CargoTransferSetupProxy setupProxy(gameSender);
        setupProxy.createPlanetShip(link, planetId, ref.getId());
        doCargoTransfer(root, gameSender, tx, setupProxy.get());
    }
}
