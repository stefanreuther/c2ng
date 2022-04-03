/**
  *  \file client/dialogs/buildshiporder.cpp
  *  \brief Starbase Ship Build Order Editor
  */

#include "client/dialogs/buildshiporder.hpp"
#include "client/dialogs/buildshipmain.hpp"
#include "client/downlink.hpp"
#include "game/proxy/basestorageproxy.hpp"
#include "game/proxy/buildshipproxy.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/window.hpp"

using game::proxy::BaseStorageProxy;
using game::proxy::BuildShipProxy;
using ui::Group;
using ui::widgets::Button;

bool
client::dialogs::doEditShipBuildOrder(ui::Root& root,
                                      game::ShipBuildOrder& order,
                                      util::RequestSender<game::proxy::StarbaseAdaptor> adaptorSender,
                                      util::RequestSender<game::Session> gameSender,
                                      game::Id_t planetId,
                                      afl::string::Translator& tx)
{
    // ex WBaseTaskShipBuildDialog::init, WBaseTaskShipBuildDialog::runDialog
    BuildShipProxy buildProxy(adaptorSender, root.engine().dispatcher());
    BaseStorageProxy storageProxy(adaptorSender, root.engine().dispatcher());
    buildProxy.setUsePartsFromStorage(false);
    if (order.getHullIndex() != 0) {
        buildProxy.setBuildOrder(order);
    }

    // Build dialog. Deleter must be after proxies.
    afl::base::Deleter del;
    BuildShipMain dlg(root, buildProxy, storageProxy, gameSender, planetId, tx);
    dlg.init(del);

    ui::EventLoop loop(root);
    ui::Window& win = dlg.buildDialog(del, tx("Ship Build Order"));

    Group& buttonGroup = del.addNew(new Group(ui::layout::HBox::instance5));
    Button& btnOK     = del.addNew(new Button(tx("OK"),     util::Key_Return, root));
    Button& btnCancel = del.addNew(new Button(tx("Cancel"), util::Key_Escape, root));
    Button& btnHelp   = del.addNew(new Button(tx("Help"),   'h', root));
    buttonGroup.add(btnOK);
    buttonGroup.add(btnCancel);
    buttonGroup.add(dlg.makeDetailedBillButton(del));
    buttonGroup.add(del.addNew(new ui::Spacer()));
    buttonGroup.add(btnHelp);
    win.add(buttonGroup);

    // Administrative
    ui::Widget& help = dlg.makeHelpWidget(del, "pcc2:basetaskscreen");
    win.add(help);
    win.add(del.addNew(new ui::widgets::Quit(root, loop)));

    // Events
    btnOK.sig_fire.addNewClosure(loop.makeStop(1));
    btnCancel.sig_fire.addNewClosure(loop.makeStop(0));
    btnHelp.dispatchKeyTo(help);

    // Do it
    win.pack();
    root.centerWidget(win);
    root.add(win);

    bool ok = loop.run();
    if (ok) {
        Downlink link(root, tx);
        BuildShipProxy::Status st;
        buildProxy.getStatus(link, st);
        order = st.order;
    }
    return ok;
}
