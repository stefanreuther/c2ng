/**
  *  \file client/dialogs/techupgradedialog.cpp
  *  \brief Tech Upgrade Dialog
  */

#include "client/dialogs/techupgradedialog.hpp"
#include "afl/base/deleter.hpp"
#include "afl/string/format.hpp"
#include "client/downlink.hpp"
#include "client/widgets/costdisplay.hpp"
#include "client/widgets/techbar.hpp"
#include "game/proxy/configurationproxy.hpp"
#include "game/proxy/techupgradeproxy.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/eventloop.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/widgets/focusablegroup.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/window.hpp"
#include "util/requestsender.hpp"
#include "util/translation.hpp"

using afl::string::Format;
using client::widgets::CostDisplay;
using game::NUM_TECH_AREAS;
using game::actions::TechUpgrade;
using game::proxy::TechUpgradeProxy;
using game::spec::Cost;
using ui::dialogs::MessageBox;

namespace {
    /*
     *  Dialog
     */
    class TechUpgradeDialog {
        friend class Peer;
     public:
        TechUpgradeDialog(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender, game::Id_t pid);

        void run();

     private:
        void setStatus(const TechUpgradeProxy::Status& st);
        void onChange();
        void onOK();

        /* Plumbing */
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        ui::EventLoop m_loop;
        client::Downlink m_link;

        /* Target tech levels */
        afl::base::Observable<int32_t> m_techLevels[game::NUM_TECH_AREAS];

        /* Widgets */
        ui::widgets::StandardDialogButtons m_buttons;
        client::widgets::CostDisplay m_costDisplay;

        /* Communication */
        TechUpgradeProxy m_proxy;
    };

}

/*
 *  TechUpgradeDialog
 */

TechUpgradeDialog::TechUpgradeDialog(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender, game::Id_t pid)
    : m_root(root),
      m_translator(tx),
      m_loop(root),
      m_link(root, tx),
      m_techLevels(),
      m_buttons(root, tx),
      m_costDisplay(root, tx, CostDisplay::Types_t(Cost::Money), game::proxy::ConfigurationProxy(gameSender).getNumberFormatter(m_link)),
      m_proxy(gameSender, root.engine().dispatcher(), pid)
{
    m_proxy.sig_change.add(this, &TechUpgradeDialog::setStatus);
}

void
TechUpgradeDialog::run()
{
    // ex WTechUpgradeWindow::doWindow, bdata.pas:UpgradeTech
    // Initialize and query initial state.
    // Caller has checked preconditions; if they didn't, this will create a dialog that's not useful.
    // However, that's easy to detect because max=0.
    TechUpgradeProxy::Status st;
    m_proxy.getStatus(m_link, st);
    if (st.max[0] == 0) {
        return;
    }
    setStatus(st);

    // Build dialog
    static const char*const TECH_NAMES[] = {
        N_("Engines:"),
        N_("Hulls:"),
        N_("Beam Weapons:"),
        N_("Torpedoes:")
    };

    // Window [VBox]
    //   FocusableGroup  \ 4x
    //     TechBar       /
    //   CostDisplay
    //   StandardDialogButtons
    afl::base::Deleter del;
    ui::Window& dlg(del.addNew(new ui::Window(m_translator("Tech Upgrade"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5)));

    ui::widgets::FocusIterator& it(del.addNew(new ui::widgets::FocusIterator(ui::widgets::FocusIterator::Vertical | ui::widgets::FocusIterator::Tab)));
    for (size_t i = 0; i < NUM_TECH_AREAS; ++i) {
        m_techLevels[i].set(st.current[i]);
        m_techLevels[i].sig_change.add(this, &TechUpgradeDialog::onChange);
        client::widgets::TechBar& bar(del.addNew(new client::widgets::TechBar(m_root, m_techLevels[i], st.min[i], st.max[i], m_translator(TECH_NAMES[i]))));
        ui::Widget& w = ui::widgets::FocusableGroup::wrapWidget(del, bar);
        dlg.add(w);
        it.add(w);
    }

    dlg.add(m_costDisplay);
    dlg.add(m_buttons);
    dlg.add(it);
    m_buttons.cancel().sig_fire.addNewClosure(m_loop.makeStop(0));
    m_buttons.ok().sig_fire.add(this, &TechUpgradeDialog::onOK);

    // Run
    dlg.pack();
    m_root.centerWidget(dlg);
    m_root.add(dlg);

    m_loop.run();
}

void
TechUpgradeDialog::setStatus(const TechUpgradeProxy::Status& st)
{
    // Update display
    m_costDisplay.setCost(st.cost);
    m_costDisplay.setAvailableAmount(st.available);
    m_costDisplay.setRemainingAmount(st.remaining);
    m_costDisplay.setMissingAmount(st.missing);

    // Update button
    m_buttons.ok().setState(ui::Widget::DisabledState, st.status != TechUpgrade::Success);
}

void
TechUpgradeDialog::onChange()
{
    // Submit all at once so we don't need to determine what changed.
    TechUpgradeProxy::Order o;
    for (size_t i = 0; i < NUM_TECH_AREAS; ++i) {
        o.values[i] = m_techLevels[i].get();
    }
    m_proxy.setAll(o);
}

void
TechUpgradeDialog::onOK()
{
    // Verify that request is valid.
    // The original code (with custom Peer, not TechUpgradeProxy) checked whether there are any in-flight requests
    // which would mean that our known status is not equal to game-side status. We now explicitly sync here.
    TechUpgradeProxy::Status st;
    m_proxy.getStatus(m_link, st);
    if (st.status == TechUpgrade::Success) {
        m_proxy.commit();
        m_loop.stop(0);
    }
}

void
client::dialogs::doTechUpgradeDialog(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender, game::Id_t pid)
{
    TechUpgradeDialog(root, tx, gameSender, pid).run();
}

bool
client::dialogs::checkTechUpgrade(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender, game::Id_t pid,
                                  game::proxy::WaitIndicator& ind, game::TechLevel area, int level, game::spec::Cost reservedAmount,
                                  String_t introFormat, String_t title)
{
    // Try to achieve correct tech level
    TechUpgradeProxy techProxy(gameSender, root.engine().dispatcher(), pid);
    techProxy.setReservedAmount(reservedAmount);
    techProxy.upgradeTechLevel(area, level);
    TechUpgradeProxy::Status techStatus;
    techProxy.getStatus(ind, techStatus);

    // TechUpgradeProxy will never set a disallowed tech level, leaving us with a "all fine, 0$" transaction.
    // Convert to failure report.
    if (techStatus.current[area] < level) {
        techStatus.status = game::actions::TechUpgrade::DisallowedTech;
    }

    String_t message = Format(introFormat, level);
    switch (techStatus.status) {
     case game::actions::TechUpgrade::Success:
        if (techStatus.cost.get(Cost::Money) != 0) {
            message += " ";
            message += Format(tx("Do you want to upgrade for %d mc?"), techStatus.cost.get(Cost::Money));
            if (!MessageBox(message, title, root).doYesNoDialog(tx)) {
                return false;
            }
            techProxy.commit();
        }
        break;

     case game::actions::TechUpgrade::MissingResources:
        message += " ";
        message += Format(tx("You do not have the required %d megacredits required to upgrade to the required level."), techStatus.cost.get(Cost::Money));
        MessageBox(message, title, root).doOkDialog(tx);
        return false;

     case game::actions::TechUpgrade::DisallowedTech:
     case game::actions::TechUpgrade::DisabledTech:    // cannot happen
     case game::actions::TechUpgrade::ForeignHull:     // cannot happen
        message += " ";
        message += tx("You cannot buy this tech level.");
        MessageBox(message, title, root).doOkDialog(tx);
        return false;
    }
    return true;
}
