/**
  *  \file client/dialogs/buildship.cpp
  *  \brief Ship Building Dialog
  */

#include "client/dialogs/buildship.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/buildshipmain.hpp"
#include "client/downlink.hpp"
#include "game/proxy/basestorageproxy.hpp"
#include "game/proxy/buildshipproxy.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"
#include "util/keystring.hpp"
#include "util/rich/parser.hpp"

using afl::string::Format;
using client::dialogs::BuildShipMain;
using game::proxy::BaseStorageProxy;
using game::proxy::BuildShipProxy;
using ui::Group;
using ui::dialogs::MessageBox;
using ui::widgets::Button;
using ui::widgets::StaticText;

namespace {
    class BuildShipDialog {
     public:
        BuildShipDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, game::Id_t planetId, afl::string::Translator& tx)
            : m_buildProxy(gameSender, root.engine().dispatcher(), planetId),
              m_storageProxy(gameSender, root.engine().dispatcher(), planetId),
              m_usePartsFromStorage("U", 'u', root),
              m_widget(root, m_buildProxy, m_storageProxy, gameSender, planetId, tx),
              m_loop(root),
              m_isNew(false)
            {
                m_usePartsFromStorage.sig_fire.add(this, &BuildShipDialog::onToggleUseParts);
                m_widget.sig_change.add(this, &BuildShipDialog::onBuildOrderChange);
            }
        void run();
        void onBuild();
        void onCancelBuild();
        void onToggleUseParts();
        void onBuildOrderChange(const BuildShipProxy::Status& st);

     private:
        BuildShipProxy m_buildProxy;
        BaseStorageProxy m_storageProxy;
        Button m_usePartsFromStorage;
        BuildShipMain m_widget;
        ui::EventLoop m_loop;
        bool m_isNew;

        bool checkClone(game::proxy::WaitIndicator& ind);
        bool checkChange();
    };
}

void
BuildShipDialog::run()
{
    // ex WBaseShipBuildDialog::init
    afl::base::Deleter del;
    afl::string::Translator& tx = m_widget.translator();
    ui::Root& root = m_widget.root();

    // Main widget
    m_widget.init(del);
    ui::Window& win = m_widget.buildDialog(del, tx("Build Ship"));

    // Option buttons
    Group& optionGroup = del.addNew(new Group(ui::layout::HBox::instance5));
    optionGroup.add(m_usePartsFromStorage);
    optionGroup.add(del.addNew(new StaticText(tx("Use parts from storage"), util::SkinColor::White, "+", root.provider())));
    optionGroup.add(del.addNew(new ui::Spacer()));
    win.add(optionGroup);

    // Main buttons
    Group& buttonGroup = del.addNew(new Group(ui::layout::HBox::instance5));
    Button& btnBuild  = del.addNew(new Button(tx("Enter - Build"), util::Key_Return, root));
    Button& btnClose  = del.addNew(new Button(tx("Close"), util::Key_Escape, root));
    Button& btnHelp   = del.addNew(new Button(tx("Help"), 'h', root));
    buttonGroup.add(btnHelp);
    buttonGroup.add(del.addNew(new ui::Spacer()));
    buttonGroup.add(m_widget.makeDetailedBillButton(del));
    if (!m_isNew) {
        Button& btnCancel = del.addNew(new Button(tx("C - Cancel Build"), 'c', root));
        buttonGroup.add(btnCancel);
        btnCancel.sig_fire.add(this, &BuildShipDialog::onCancelBuild);
    }
    buttonGroup.add(btnBuild);
    buttonGroup.add(btnClose);
    win.add(buttonGroup);

    // Administrative
    ui::Widget& help = m_widget.makeHelpWidget(del, "pcc2:buildship");
    win.add(help);
    win.add(del.addNew(new ui::widgets::Quit(root, m_loop)));

    // Events
    btnBuild.sig_fire.add(this, &BuildShipDialog::onBuild);
    btnClose.sig_fire.addNewClosure(m_loop.makeStop(0));
    btnHelp.dispatchKeyTo(help);

    // Do it
    win.pack();
    root.centerWidget(win);
    root.add(win);
    m_loop.run();
}

/*
 *  UI actions
 */

/* "Build" button. Decide action and perform it. */
void
BuildShipDialog::onBuild()
{
    // ex WBaseShipBuildDialog::onOk
    afl::string::Translator& tx = m_widget.translator();
    ui::Root& root = m_widget.root();
    client::Downlink link(root, tx);
    BuildShipProxy::Status st;
    m_buildProxy.getStatus(link, st);

    switch (st.status) {
     case game::actions::BaseBuildAction::MissingResources:
        // FIXME: special handling for missing resources:
        //         bool add =
        //             UIMessageBox(_("You do not have enough resources to build this ship now. "
        //                            "Do you want to add this build order to this base's Auto Task, "
        //                            "to build it as soon as resources are available?"),
        //                          _("Build order rejected")).doYesNoDialog();
        //         ta.doCancel();
        //         transfer.doCancel();
        //         if (add && addToAutoTask(getAutoTaskForObject(planet, IntExecutionContext::pkBaseTask, true),
        //                                  makeBuildOrderCommand("EnqueueShip", action.getBuildOrder())))
        //         {
        //             action.getPlanet().setBaseBuildOrder(blankBuildOrder);
        //             stop(1);
        //         }
     case game::actions::BaseBuildAction::DisallowedTech:
     case game::actions::BaseBuildAction::ForeignHull:
     case game::actions::BaseBuildAction::DisabledTech:
        MessageBox(tx("You cannot build this ship."), tx("Build Order Rejected"), root).doOkDialog(tx);
        break;
     case game::actions::BaseBuildAction::Success:
        if (!checkClone(link)) {
            return;
        }
        if (st.isChange && !checkChange()) {
            return;
        }
        m_buildProxy.commit();
        m_loop.stop(0);
        break;
    }
}

/* "Cancel build" button */
void
BuildShipDialog::onCancelBuild()
{
    // WBaseShipBuildDialog::onCancelBuild
    m_buildProxy.cancel();
    m_loop.stop(0);
}

/* "Use parts from storage" toggle.
   We use the button's HighlightedButton flag to store the current state. */
void
BuildShipDialog::onToggleUseParts()
{
    // WBaseShipBuildDialog::onToggleUseParts
    m_buildProxy.setUsePartsFromStorage(!m_usePartsFromStorage.getFlags().contains(ui::HighlightedButton));
}

/* Check for conflicting clone order.
   Return true to proceed, false to stop processing. */
bool
BuildShipDialog::checkClone(game::proxy::WaitIndicator& ind)
{
    afl::string::Translator& tx = m_widget.translator();
    ui::Root& root = m_widget.root();

    // Are we cloning?
    game::Id_t shipId;
    String_t shipName;
    if (!m_buildProxy.findShipCloningHere(ind, shipId, shipName)) {
        return true;
    }

    // OK, we are cloning. Ask user.
    MessageBox box(Format(tx("This base is already cloning %s (#%d). Do you want to cancel that order? "
                             "If you say \"No\", this ship will be built after the clone completed."),
                          shipName, shipId),
                   tx("Build Ship"), root);
    enum { YES, NO, CANCEL };
    box.addButton(YES,    util::KeyString(tx("Yes")));
    box.addButton(NO,     util::KeyString(tx("No")));
    box.addButton(CANCEL, tx("Cancel"), util::Key_Escape);
    box.addKey(YES, ' ');
    int reply = box.run();

    if (reply == CANCEL) {
        return false;
    }
    if (reply == YES) {
        m_buildProxy.cancelAllCloneOrders();
    }
    return true;
}

/* Warn for changed build order.
   Return true to proceed, false to stop processing. */
bool
BuildShipDialog::checkChange()
{
    afl::string::Translator& tx = m_widget.translator();
    ui::Root& root = m_widget.root();

    // Do the dialog by hand. We don't want ENTER to confirm the build.
    MessageBox box(util::rich::Parser::parseXml(tx("This starbase already has a build order. Do you want to change that order?\n\n"
                                                   "<small>To exit the ship build screen without changing the build order, use \"Exit\" (ESC). "
                                                   "To replace the existing order with your new choice, use <kbd>Y</kbd>.</small>")),
                   tx("Build Ship"), root);
    enum { YES, NO };
    box.addButton(YES, util::KeyString(tx("Yes")));
    box.addButton(NO,  util::KeyString(tx("No")));
    box.ignoreKey(util::Key_Return);          // We don't want users to confirm this dialog accidentally.
    int reply = box.run();

    return (reply == YES);
}

/* Render "use parts from storage" flag. */
void
BuildShipDialog::onBuildOrderChange(const BuildShipProxy::Status& st)
{
    // ex WBaseShipBuildDialog::updateUseParts
    m_isNew = st.isNew;
    m_usePartsFromStorage.setFlag(ui::HighlightedButton, st.isUsePartsFromStorage);
}


/*
 *  Main Entry Point
 */

void
client::dialogs::doBuildShip(ui::Root& root, util::RequestSender<game::Session> gameSender, game::Id_t planetId, afl::string::Translator& tx)
{
    BuildShipDialog(root, gameSender, planetId, tx).run();
}
