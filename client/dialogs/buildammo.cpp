/**
  *  \file client/dialogs/buildammo.cpp
  *  \brief Ammo Building Dialog
  */

#include "client/dialogs/buildammo.hpp"
#include "client/dialogs/specbrowserdialog.hpp"
#include "client/dialogs/techupgradedialog.hpp"
#include "client/downlink.hpp"
#include "client/picturenamer.hpp"
#include "client/widgets/componentlist.hpp"
#include "client/widgets/helpwidget.hpp"
#include "client/widgets/itemcostdisplay.hpp"
#include "game/proxy/buildammoproxy.hpp"
#include "game/proxy/configurationproxy.hpp"
#include "game/proxy/specbrowserproxy.hpp"
#include "game/proxy/techupgradeproxy.hpp"
#include "gfx/keyeventconsumer.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/prefixargument.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/keyforwarder.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"

namespace gsi = game::spec::info;

using afl::string::Format;
using client::widgets::ComponentList;
using client::widgets::ItemCostDisplay;
using game::proxy::BuildAmmoProxy;
using game::proxy::SpecBrowserProxy;
using ui::Group;
using ui::widgets::Button;
using ui::widgets::StaticText;
using util::SkinColor;

namespace {
    /*
     *  Build Ammo Dialog
     *
     *  Controls a BuildAmmoProxy.
     *  In addition, a SpecBrowserProxy provides information.
     */
    class Dialog : public gfx::KeyEventConsumer {
     public:
        Dialog(ui::Root& root, util::RequestSender<game::Session> gameSender, game::Id_t planetId, afl::string::Translator& tx, BuildAmmoProxy& proxy);

        void init();
        void run();

        bool handleKey(util::Key_t key, int prefix);

     private:
        /*
         *  Integration/Proxies
         */
        ui::Root& m_root;
        util::RequestSender<game::Session> m_gameSender;
        afl::string::Translator& m_translator;
        const game::Id_t m_planetId;
        BuildAmmoProxy& m_proxy;
        SpecBrowserProxy m_specProxy;

        /*
         *  State
         */

        // Last requested page/id from SpecBrowserProxy, to avoid duplicate requess
        gsi::Page m_specPage;
        game::Id_t m_specId;

        // Guard: do not send a request do SpecBrowserProxy when m_infoView is not visible;
        // otherwise, response will be rendered wrong.
        bool m_specActive;

        // NumberFormatter
        util::NumberFormatter m_formatter;

        // Last status from BuildAmmoProxy
        BuildAmmoProxy::Status m_status;

        /*
         *  UI/Widgets
         */

        ui::EventLoop m_loop;
        ComponentList m_componentList;
        ItemCostDisplay m_costDisplay;
        StaticText m_targetName;             // Target name (heading)
        StaticText m_targetAmount;           // "You have 20 torpedoes"
        StaticText m_targetRoom;             // "You have room for 15 more"
        ui::rich::DocumentView m_infoView;   // Weapon specification

        afl::base::SignalConnection conn_proxyUpdate;

        const BuildAmmoProxy::Part* getCurrentPart();
        void build(int amount);
        void render();
        void renderCurrent();

        void onOK();
        void onSelectionChange();
        void onSpecificationChange(const gsi::PageContent& content);
        void onProxyUpdate(const BuildAmmoProxy::Status& st);
    };
}

Dialog::Dialog(ui::Root& root, util::RequestSender<game::Session> gameSender, game::Id_t planetId, afl::string::Translator& tx, BuildAmmoProxy& proxy)
    : m_root(root),
      m_gameSender(gameSender),
      m_translator(tx),
      m_planetId(planetId),
      m_proxy(proxy),
      m_specProxy(gameSender, root.engine().dispatcher(), std::auto_ptr<gsi::PictureNamer>(new client::PictureNamer())),
      m_specPage(),
      m_specId(),
      m_specActive(false),
      m_formatter(false, false),
      m_status(),
      m_loop(root),
      m_componentList(root, 11, 18),
      m_costDisplay(root, tx),
      m_targetName(String_t(), SkinColor::Heading, "+", m_root.provider()),
      m_targetAmount(String_t(), SkinColor::Heading, gfx::FontRequest(), m_root.provider()),
      m_targetRoom(String_t(), SkinColor::Heading, gfx::FontRequest(), m_root.provider()),
      m_infoView(root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(25, 10), 0, root.provider()),
      conn_proxyUpdate(proxy.sig_update.add(this, &Dialog::onProxyUpdate))
{
    // Do not show costs on info page; they are on the CostDisplay
    m_specProxy.setWithCost(false);

    // Widgets
    m_targetName.setIsFlexible(true);
    m_targetAmount.setIsFlexible(true);
    m_targetRoom.setIsFlexible(true);
    m_costDisplay.setHighlightingMode(ItemCostDisplay::TotalMode);

    // Connect events for stuff we created
    m_specProxy.sig_pageChange.add(this, &Dialog::onSpecificationChange);
    m_componentList.sig_change.add(this, &Dialog::onSelectionChange);
}

/* Initialisation that takes time (Downlink) */
void
Dialog::init()
{
    // Number Formatter
    client::Downlink link(m_root, m_translator);
    m_formatter = game::proxy::ConfigurationProxy(m_gameSender).getNumberFormatter(link);
    m_costDisplay.setNumberFormatter(m_formatter);

    // Initial state
    m_proxy.getStatus(link, m_status);
    m_specActive = true;
    render();
}

/* Show the dialog */
void
Dialog::run()
{
    // ex WAmmoWindow::init()
    // VBox
    //   HBox 'topGroup'
    //     VBox 'leftGroup'
    //       "Weapon:"
    //       FrameGroup > AmmoList
    //     VBox 'rightGroup'
    //       HBox 'infoGroup'
    //         VBox 'labelGroup': Target, Spacer, Amount, Room
    //         VBox 'amountGroup': "+", "-"
    //       Info
    //   "Total Cost:"
    //   CostDisplay
    //   StandardDialogButtons
    const int em = m_root.provider().getFont(gfx::FontRequest())->getEmWidth();
    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(m_translator("Build Torpedoes/Fighters"), m_root.provider(), m_root.colorScheme(), ui::BLUE_DARK_WINDOW, ui::layout::VBox::instance5));

    Group& topGroup = del.addNew(new Group(ui::layout::HBox::instance5));
    Group& leftGroup = del.addNew(new Group(ui::layout::VBox::instance5));
    Group& rightGroup = del.addNew(new Group(ui::layout::VBox::instance5));
    Group& infoGroup = del.addNew(new Group(ui::layout::HBox::instance5));
    Group& labelGroup = del.addNew(new Group(ui::layout::VBox::instance0));
    Group& amountGroup = del.addNew(new Group(ui::layout::VBox::instance5));

    leftGroup.add(del.addNew(new StaticText(m_translator("Weapon"), SkinColor::Heading, "+", m_root.provider())));
    leftGroup.add(ui::widgets::FrameGroup::wrapWidget(del, m_root.colorScheme(), ui::LoweredFrame, del.addNew(new ui::widgets::ScrollbarContainer(m_componentList, m_root))));
    leftGroup.add(del.addNew(new ui::Spacer()));

    labelGroup.add(m_targetName);
    labelGroup.add(del.addNew(new ui::Spacer(ui::layout::Info(gfx::Point(20*em, 5), gfx::Point(20*em, 5), ui::layout::Info::GrowHorizontal))));
    labelGroup.add(m_targetAmount);
    labelGroup.add(m_targetRoom);

    Button& btnPlus  = del.addNew(new Button("+", '+', m_root));
    Button& btnMinus = del.addNew(new Button("-", '-', m_root));
    amountGroup.add(btnPlus);
    amountGroup.add(btnMinus);
    amountGroup.add(del.addNew(new ui::Spacer()));
    btnPlus.dispatchKeyTo(*this);
    btnMinus.dispatchKeyTo(*this);

    infoGroup.add(labelGroup);
    infoGroup.add(amountGroup);

    rightGroup.add(infoGroup);
    rightGroup.add(m_infoView);
    rightGroup.add(del.addNew(new ui::Spacer()));

    topGroup.add(leftGroup);
    topGroup.add(rightGroup);

    ui::widgets::StandardDialogButtons& btn = del.addNew(new ui::widgets::StandardDialogButtons(m_root, m_translator));
    // FIXME: help: ui::Widget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, m_gameSender, "?");

    win.add(topGroup);
    win.add(del.addNew(new StaticText(m_translator("Total Cost"), SkinColor::Heading, "+", m_root.provider())));
    win.add(m_costDisplay);
    win.add(btn);

    win.add(del.addNew(new ui::PrefixArgument(m_root)));
    win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));
    win.add(del.addNew(new ui::widgets::KeyForwarder(*this)));

    btn.cancel().sig_fire.addNewClosure(m_loop.makeStop(0));
    btn.ok().sig_fire.add(this, &Dialog::onOK);

    win.pack();
    m_root.centerWidget(win);
    m_root.add(win);
    m_loop.run();
}

/* Key handler */
bool
Dialog::handleKey(util::Key_t key, int prefix)
{
    // ex WAmmoWindow::handleEvent(const UIEvent& e, bool second_pass)
    switch (key) {
     case '+':
        build(prefix != 0 ? prefix : 1);
        return true;
     case '-':
        build(prefix != 0 ? -prefix : -1);
        return true;
     case util::KeyMod_Ctrl + '+':
        build(100);
        return true;
     case util::KeyMod_Ctrl + '-':
        build(-100);
        return true;
     case util::KeyMod_Alt + '+':
        build(10000);
        return true;
     case util::KeyMod_Alt + '-':
        build(-10000);
        return true;
     default:
        return false;
    }
}

/* Get current part definition; null if none */
const BuildAmmoProxy::Part*
Dialog::getCurrentPart()
{
    size_t index = m_componentList.getCurrentItem();
    return (index < m_status.parts.size() ? &m_status.parts[index] : 0);
}

/* Build or scrap a number of parts */
void
Dialog::build(int amount)
{
    // ex WAmmoWindow::build(int32 amount)
    // Preconditions
    const BuildAmmoProxy::Part* p = getCurrentPart();
    if (p == 0 || amount == 0) {
        return;
    }

    // If we're trying to build, and don't have sufficient tech, we have to upgrade.
    if (amount > 0 && p->techStatus != game::AvailableTech) {
        client::Downlink link(m_root, m_translator);
        client::dialogs::checkTechUpgrade(m_root, m_translator, m_gameSender, m_planetId,
                                          link, game::TorpedoTech, p->techLevel, m_translator("To build this torpedo, you need tech %d."),
                                          m_translator("Build Torpedoes"));

        // Traditionally, PCC doesn't build immediately after upgrading tech.
        // FIXME: the tech upgrade may invalidate the build-ammo order!
    } else {
        // Okay, we're building stuff
        m_proxy.addLimitCash(p->type, amount);
    }
}

/* Render entire status. This could change everything. */
void
Dialog::render()
{
    // ex WAmmoInfo::drawContent (part)
    m_targetName.setText(m_status.targetName);

    // List content. List will deal with cursor updates itself.
    ComponentList::Parts_t parts;
    for (size_t i = 0; i < m_status.parts.size(); ++i) {
        const BuildAmmoProxy::Part& pt = m_status.parts[i];
        parts.push_back(ComponentList::Part(int(pt.type), pt.name, pt.amount, pt.isAccessible, pt.techStatus));
    }
    m_componentList.setContent(parts);

    // Cost
    m_costDisplay.setAvailableAmount(m_status.available);
    m_costDisplay.setTotalCost(m_status.cost);

    // Current item
    renderCurrent();
}

/* Render current item. Status didn't change but user scrolled. */
void
Dialog::renderCurrent()
{
    // ex WAmmoInfo::drawContent (part)
    const BuildAmmoProxy::Part* p = getCurrentPart();

    // Amount
    if (p == 0) {
        m_targetAmount.setText(String_t());
    } else if (p->page == gsi::FighterPage) {
        m_targetAmount.setText(Format(m_translator("You have %d fighter%!1{s%}."), m_formatter.formatNumber(p->amount)));
    } else {
        m_targetAmount.setText(Format(m_translator("You have %d torpedo%!1{es%}."), m_formatter.formatNumber(p->amount)));
    }

    // Room
    if (p == 0) {
        m_targetRoom.setText(String_t());
    } else if (p->amount >= p->maxAmount) {
        m_targetRoom.setText(m_translator("This is the maximum amount."));
    } else if (p->maxAmount - p->amount <= 5000) {
        m_targetRoom.setText(Format(m_translator("You have room for %d more."), m_formatter.formatNumber(p->maxAmount - p->amount)));
    } else {
        m_targetRoom.setText(String_t());
    }

    // Cost
    if (p != 0) {
        m_costDisplay.setPartCost(p->cost);
        m_costDisplay.setPartTechLevel(m_status.availableTech, p->techLevel);
    }

    // Select part on SpecBrowserProxy to get current specs
    // (but avoid sending duplicate requests as this is triggered by many no-changes)
    if (p != 0) {
        if (m_specActive && (m_specPage != p->page || m_specId != p->id)) {
            m_specProxy.setPageId(p->page, p->id);
            m_specPage = p->page;
            m_specId = p->id;
        }
    }
}

/* "OK" button */
void
Dialog::onOK()
{
    m_proxy.commit();
    m_loop.stop(1);
}

/* User scrolled in list */
void
Dialog::onSelectionChange()
{
    renderCurrent();
}

/* Specification report from SpecBrowserProxy */
void
Dialog::onSpecificationChange(const gsi::PageContent& content)
{
    // FIXME: show an image?
    ui::rich::Document& doc = m_infoView.getDocument();
    doc.clear();
    doc.add(util::rich::Text(content.title).withStyle(util::rich::StyleAttribute::Big).withColor(SkinColor::Heading));
    doc.addParagraph();
    client::dialogs::renderHullInformation(doc, m_root, content, m_translator);
    doc.finish();
    m_infoView.handleDocumentUpdate();
}

/* Status update from BuildAmmoProxy */
void
Dialog::onProxyUpdate(const BuildAmmoProxy::Status& st)
{
    m_status = st;
    render();
}



/*
 *  Entry Point
 */

void
client::dialogs::doBuildAmmo(ui::Root& root,
                             game::proxy::BuildAmmoProxy& proxy,
                             util::RequestSender<game::Session> gameSender,
                             game::Id_t planetId,
                             afl::string::Translator& tx)
{
    Dialog dlg(root, gameSender, planetId, tx, proxy);
    dlg.init();
    dlg.run();
}
