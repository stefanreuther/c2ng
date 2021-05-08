/**
  *  \file client/dialogs/buildparts.cpp
  *  \brief Starship Part Building
  */

#include "client/dialogs/buildparts.hpp"
#include "afl/base/deleter.hpp"
#include "afl/string/format.hpp"
#include "client/downlink.hpp"
#include "client/widgets/costdisplay.hpp"
#include "client/widgets/helpwidget.hpp"
#include "game/proxy/buildpartsproxy.hpp"
#include "game/proxy/configurationproxy.hpp"
#include "ui/eventloop.hpp"
#include "ui/invisiblewidget.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/prefixargument.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"
#include "util/translation.hpp"

using afl::string::Format;
using client::widgets::CostDisplay;
using game::actions::BuildParts;
using game::proxy::BuildPartsProxy;
using game::spec::Cost;
using ui::widgets::Button;

namespace {
    /*
     *  BuildPartsKeyHandler
     */

    class BuildPartsKeyHandler : public ui::InvisibleWidget {
     public:
        BuildPartsKeyHandler(BuildPartsProxy& proxy)
            : m_proxy(proxy)
            { }
        virtual bool handleKey(util::Key_t key, int prefix)
            {
                // ex WBuildComponentWindow::handleEvent
                switch (key) {
                 case '+':
                    m_proxy.add(prefix != 0 ? prefix : 1);
                    return true;

                 case util::KeyMod_Ctrl + '+':
                 case util::KeyMod_Alt + '+':
                    m_proxy.add(100);
                    return true;

                 case '-':
                    m_proxy.add(prefix != 0 ? -prefix : -1);
                    return true;

                 case util::KeyMod_Ctrl + '-':
                 case util::KeyMod_Alt + '-':
                    m_proxy.add(-100);
                    return true;

                 default:
                    return false;
                }
            }

     private:
        BuildPartsProxy& m_proxy;
    };


    /*
     *  BuildPartsDialog
     */

    class BuildPartsDialog {
     public:
        BuildPartsDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, BuildPartsProxy& proxy, afl::string::Translator& tx, util::NumberFormatter fmt, game::proxy::WaitIndicator& ind)
            : m_root(root),
              m_gameSender(gameSender),
              m_proxy(proxy),
              m_translator(tx),
              m_formatter(fmt),
              m_waitIndicator(ind),
              m_loop(root),
              m_costDisplay(root, tx, CostDisplay::Types_t() + Cost::Money + Cost::Tritanium + Cost::Duranium + Cost::Molybdenum, fmt),
              m_dialogButtons(root, tx),
              m_countText(String_t(), util::SkinColor::Static, "+", root.provider())
            {
                // WBuildComponentWindow::WBuildComponentWindow
                m_countText.setIsFlexible(true);
                m_proxy.sig_change.add(this, &BuildPartsDialog::setStatus);
            }

        void run(String_t introText);
        void setStatus(const BuildPartsProxy::Status& st);
        void onOK();

     private:
        // Related objects
        ui::Root& m_root;
        util::RequestSender<game::Session> m_gameSender;
        BuildPartsProxy& m_proxy;
        afl::string::Translator& m_translator;
        util::NumberFormatter m_formatter;
        game::proxy::WaitIndicator& m_waitIndicator;
        ui::EventLoop m_loop;

        // Widgets
        CostDisplay m_costDisplay;
        ui::widgets::StandardDialogButtons m_dialogButtons;
        ui::widgets::StaticText m_countText;
    };
}

void
BuildPartsDialog::run(String_t introText)
{
    // WBuildComponentWindow::init
    // Window [VBox]
    //   MultilineStatic "Use this window to build..."
    //   HBox: Count, "+", "-"
    //   CostDisplay
    //   Buttons

    afl::base::Deleter del;

    ui::Widget& keyHandler = del.addNew(new BuildPartsKeyHandler(m_proxy));
    ui::Window& win = del.addNew(new ui::Window(m_translator("Build Components"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

    int introWidth = m_costDisplay.getLayoutInfo().getPreferredSize().getX();
    ui::rich::DocumentView& introDoc = del.addNew(new ui::rich::DocumentView(gfx::Point(introWidth, 10), 0, m_root.provider()));
    introDoc.setExtent(gfx::Rectangle(0, 0, introWidth, 10));
    introDoc.getDocument().add(introText);
    introDoc.getDocument().finish();
    introDoc.adjustToDocumentSize();
    win.add(introDoc);

    ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    Button& btnFewer = del.addNew(new Button("-", '-', m_root));
    Button& btnMore  = del.addNew(new Button("+", '+', m_root));
    g.add(m_countText);
    g.add(btnFewer);
    g.add(btnMore);
    win.add(g);
    win.add(m_costDisplay);
    win.add(m_dialogButtons);
    win.add(keyHandler);
    win.add(del.addNew(new ui::PrefixArgument(m_root)));
    win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));

    ui::Widget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, m_gameSender, "pcc2:buildship"));
    win.add(help);

    // Events
    m_dialogButtons.cancel().sig_fire.addNewClosure(m_loop.makeStop(0));
    m_dialogButtons.ok().sig_fire.add(this, &BuildPartsDialog::onOK);
    m_dialogButtons.addHelp(help);
    btnFewer.dispatchKeyTo(keyHandler);
    btnMore.dispatchKeyTo(keyHandler);

    // Run
    win.pack();
    m_root.centerWidget(win);
    m_root.add(win);
    m_loop.run();
}

void
BuildPartsDialog::setStatus(const BuildPartsProxy::Status& st)
{
    m_countText.setText(Format(m_translator("In storage: %d"), m_formatter.formatNumber(st.numParts)));

    m_costDisplay.setCost(st.cost);
    m_costDisplay.setAvailableAmount(st.available);
    m_costDisplay.setRemainingAmount(st.remaining);
    m_costDisplay.setMissingAmount(st.missing);

    m_dialogButtons.ok().setState(ui::Widget::DisabledState, st.status != BuildParts::Success);
}

void
BuildPartsDialog::onOK()
{
    // Update status to guarantee we're current
    BuildPartsProxy::Status st;
    m_proxy.getStatus(m_waitIndicator, st);
    setStatus(st);

    // Commit if successful
    if (st.status == BuildParts::Success) {
        m_proxy.commit();
        m_loop.stop(0);
    }
}


/*
 *  Entry Point
 */

void
client::dialogs::doBuildShipParts(ui::Root& root, util::RequestSender<game::Session> gameSender, game::Id_t planetId, game::TechLevel area, int partId, afl::string::Translator& tx)
{
    // ex doBuildComponent
    // Initialize
    Downlink link(root, tx);
    BuildPartsProxy proxy(gameSender, root.engine().dispatcher(), planetId);
    proxy.selectPart(area, partId);

    BuildPartsProxy::Status st;
    proxy.getStatus(link, st);

    // Dialog
    BuildPartsDialog dlg(root, gameSender, proxy, tx, game::proxy::ConfigurationProxy(gameSender).getNumberFormatter(link), link);
    dlg.setStatus(st);

    static const char*const MESSAGES[] = {
        N_("Use this window to build %s engines and put them into starbase storage for later use."),
        N_("Use this window to build %s hulls and put them into starbase storage for later use."),
        N_("Use this window to build %s beams and put them into starbase storage for later use."),
        N_("Use this window to build %s launchers and put them into starbase storage for later use."),
    };
    dlg.run(Format(tx(MESSAGES[area]), st.name));
}
