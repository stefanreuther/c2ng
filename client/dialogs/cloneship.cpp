/**
  *  \file client/dialogs/cloneship.cpp
  *  \brief Ship cloning dialog
  */

#include "client/dialogs/cloneship.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/buildship.hpp"
#include "client/downlink.hpp"
#include "client/widgets/costdisplay.hpp"
#include "game/proxy/cloneshipproxy.hpp"
#include "game/proxy/configurationproxy.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/rich/statictext.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/widgets/quit.hpp"

using afl::string::Format;
using client::widgets::CostDisplay;
using game::actions::CloneShip;
using game::proxy::CloneShipProxy;
using game::spec::Cost;
using ui::dialogs::MessageBox;
using ui::widgets::Button;
using ui::widgets::KeyDispatcher;

namespace {
    /* Warn for possible conflict. Return true to proceed. */
    bool warnConflict(const CloneShipProxy::Status& st, bool isBuild, ui::Root& root, afl::string::Translator& tx)
    {
        switch (st.conflictStatus) {
         case CloneShip::NoConflict:
            break;

         case CloneShip::IsBuilding:
            if (!MessageBox(Format(isBuild
                                   ? tx("This base is already building a %s. Proceed anyway and replace this order?")
                                   : tx("This base is already building a %s. Proceed anyway and delay this order?"),
                                   st.conflict.name), tx("Clone Ship"), root).doYesNoDialog(tx))
            {
                return false;
            }
            break;

         case CloneShip::IsCloning:
            if (!MessageBox(Format(tx("This base is already cloning %s. Proceed anyway and replace this order?"),
                                   st.conflict.name), tx("Clone Ship"), root).doYesNoDialog(tx))
            {
                return false;
            }
            break;
        }
        return true;
    }

    /* Confirm clone request. Return true to proceed (user confirmed and status allows proceeding). */
    bool confirmClone(const CloneShipProxy::Status& st, ui::Root& root, afl::string::Translator& tx, util::NumberFormatter fmt)
    {
        // ex doCloneShip (part), CCloneShipWindow.DrawInterior
        // Dialog [VBox]
        //   CostDisplay
        //   StaticText
        //   HBox
        //     Spacer
        //     "OK" or "Yes"/"No"
        //     Spacer
        afl::base::Deleter del;
        ui::Window& win = del.addNew(new ui::Window(tx("Clone Ship"), root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

        CostDisplay& costdpy = del.addNew(new CostDisplay(root, tx, CostDisplay::Types_t() + Cost::Tritanium + Cost::Duranium + Cost::Molybdenum + Cost::Supplies + Cost::Money, fmt));
        costdpy.setCost(st.cost);
        costdpy.setAvailableAmount(st.available);
        costdpy.setRemainingAmount(st.remaining);
        costdpy.setMissingAmount(st.missing);
        win.add(costdpy);

        bool ok = false;
        String_t message;
        switch (st.paymentStatus) {
         case CloneShip::CannotPayTech:
            // Not enough cash to upgrate tech
            ok = false;
            message = tx("You cannot clone this ship, because you do not have enough money to upgrade your technology.");
            break;

         case CloneShip::CannotPayComponents:
            // Can upgrade tech, but not build ship
            ok = true;
            message = tx("You do not have enough resources to clone this ship. Try anyway?");
            if (!st.techCost.isZero()) {
                message += tx(" At this point, PCC2 will buy only the required tech upgrades.");
            }
            break;

         case CloneShip::CanPay:
            // Everything fine
            ok = true;
            message = tx("Build this ship?");
            break;
        }

        // Add information text, same width as the CostDisplay
        util::rich::Text text;
        if (int32_t techCost = st.techCost.get(Cost::Money)) {
            text += Format(tx("Cost includes %d mc for tech upgrades."), fmt.formatNumber(techCost));
            text += "\n\n";
        }

        text += util::rich::Text(message).withStyle(util::rich::StyleAttribute::Bold);
        if (ok && st.isCloneOnce) {
            text += "\n\n";
            text += tx("Note: this ship can be cloned only once; it will be unclonable after the clone.");
        }
        win.add(del.addNew(new ui::rich::StaticText(text, costdpy.getLayoutInfo().getPreferredSize().getX(), root.provider())));

        ui::EventLoop loop(root);
        ui::Group& g3 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
        g3.add(del.addNew(new ui::Spacer()));
        if (ok) {
            Button& btnYes = del.addNew(new Button(util::KeyString(tx("Yes")), root));
            btnYes.sig_fire.addNewClosure(loop.makeStop(1));
            g3.add(btnYes);

            Button& btnNo = del.addNew(new Button(util::KeyString(tx("No")), root));
            btnNo.sig_fire.addNewClosure(loop.makeStop(0));
            g3.add(btnNo);
        } else {
            Button& btnOK = del.addNew(new Button(util::KeyString(tx("OK")), root));
            btnOK.sig_fire.addNewClosure(loop.makeStop(0));
            g3.add(btnOK);
        }
        g3.add(del.addNew(new ui::Spacer()));
        win.add(g3);

        KeyDispatcher& disp = del.addNew(new KeyDispatcher());
        disp.addNewClosure(util::Key_Return, loop.makeStop(1));
        disp.addNewClosure(util::Key_Escape, loop.makeStop(0));
        disp.addNewClosure(' ', loop.makeStop(1));
        win.add(disp);
        win.add(del.addNew(new ui::widgets::Quit(root, loop)));
        win.pack();
        root.centerWidget(win);
        root.add(win);

        // Display dialog.
        bool result = loop.run();
        return result && ok;
    }

    /* Confirm leaving fleet. Return true to proceed (user confirmed if needed). */
    bool confirmFleet(const CloneShipProxy::Status& st, ui::Root& root, afl::string::Translator& tx)
    {
        if (st.isInFleet) {
            if (!MessageBox(tx("This ship is member of a fleet. To clone, it must leave the fleet and stay here. "
                               "Leave the fleet?"),
                            tx("Clone Ship"), root)
                .doYesNoDialog(tx))
            {
                return false;
            }
        }
        return true;
    }
}

void
client::dialogs::doCloneShip(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender, game::Id_t shipId)
{
    // ex doCloneShip(GShip& sh, GPlanet& pl)
    // Initialize
    Downlink link(root, tx);
    CloneShipProxy proxy(gameSender, shipId);
    CloneShipProxy::Status st;
    proxy.getStatus(link, st);

    // If the proxy reports an invalid status, caller didn't properly check preconditions
    if (!st.valid) {
        return;
    }

    // Determine order status
    switch (st.orderStatus) {
     case CloneShip::CanClone:
        if (warnConflict(st, false, root, tx)) {
            if (confirmClone(st, root, tx, game::proxy::ConfigurationProxy(gameSender).getNumberFormatter(link))) {
                if (confirmFleet(st, root, tx)) {
                    proxy.commit();
                }
            }
        }
        break;

     case CloneShip::CanBuild:
        if (warnConflict(st, true, root, tx)) {
            doBuildShip(root, gameSender, st.planetId, st.buildOrder, tx);
        }
        break;

     case CloneShip::PlayerCannotClone:
        MessageBox(tx("You cannot clone ships."), tx("Clone Ship"), root).doOkDialog(tx);
        break;

     case CloneShip::ShipIsUnclonable:
        MessageBox(tx("This ship is unclonable."), tx("Clone Ship"), root).doOkDialog(tx);
        break;

     case CloneShip::RemoteOwnerCanBuild:
        MessageBox(tx("This ship cannot be cloned here, because its real owner "
                      "cannot clone it. You have to own it yourself to be able "
                      "to clone it."),
                   tx("Clone Ship"), root)
            .doOkDialog(tx);
        break;

     case CloneShip::TechLimitExceeded:
        MessageBox(tx("To clone this ship, you need to upgrade your technology over Tech 6. "
                      "As a shareware player, you can't do this."),
                   tx("Build order rejected"), root)
            .doOkDialog(tx);
        break;
    }
}
