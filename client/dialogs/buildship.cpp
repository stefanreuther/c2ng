/**
  *  \file client/dialogs/buildship.cpp
  *  \brief Ship Building Dialog
  */

#include "client/dialogs/buildship.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/buildparts.hpp"
#include "client/dialogs/specbrowserdialog.hpp"   // renderHullInformation
#include "client/downlink.hpp"
#include "client/picturenamer.hpp"
#include "client/widgets/componentlist.hpp"
#include "client/widgets/costsummarylist.hpp"
#include "client/widgets/helpwidget.hpp"
#include "client/widgets/itemcostdisplay.hpp"
#include "game/proxy/basestorageproxy.hpp"
#include "game/proxy/buildshipproxy.hpp"
#include "game/proxy/configurationproxy.hpp"
#include "game/proxy/specbrowserproxy.hpp"
#include "game/proxy/techupgradeproxy.hpp"
#include "ui/cardgroup.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/cardtabbar.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/imagebutton.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"
#include "util/keystring.hpp"
#include "util/rich/parser.hpp"
#include "util/translation.hpp"
#include "util/unicodechars.hpp"

using afl::string::Format;
using client::widgets::CostSummaryList;
using game::TechLevel;
using game::proxy::BaseStorageProxy;
using game::proxy::BuildShipProxy;
using game::proxy::SpecBrowserProxy;
using game::proxy::TechUpgradeProxy;
using ui::dialogs::MessageBox;
using ui::Group;
using ui::rich::DocumentView;
using ui::widgets::Button;
using ui::widgets::StaticText;

namespace gsi = game::spec::info;

namespace {
    gfx::Point getOrderDisplaySize(ui::Root& root)
    {
        // ex WBuildOrderDisplay::getLayoutInfo
        return root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(20, 6);
    }

    gfx::Point getSpecificationDisplaySize(ui::Root& root)
    {
        // ex WHullInfo::getLayoutInfo, WBeamInfo::getLayoutInfo, WTorpInfo::getLayoutInfo, WEngineInfo::getLayoutInfo
        return root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(35, 12);
    }

    /*
     *  BuildShipDialog - Dialog State
     *
     *  We use...
     *  - a BuildShipProxy to set up the build order.
     *  - a BaseStorageProxy to get the list of components.
     *  - a SpecBrowserProxy to obtain the current component's specs.
     *
     *  Lists of components are retrieved once and kept up-to-date.
     *  Whenever focus changes, the new component is selected on the BuildShipProxy and the SpecBrowserProxy;
     *  corresponding updates are received asynchronously.
     */

    class BuildShipDialog {
     public:
        BuildShipDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, game::Id_t planetId, afl::string::Translator& tx);

        void init();
        ui::Window& buildDialog();
        void run(ui::Window& win);

        // UI actions
        void onBuild();
        void onDetailedBill();
        void onBuildParts();
        void onCancelBuild();
        void onToggleUseParts();
        void addBeam();
        void removeBeam();
        void addLauncher();
        void removeLauncher();

        // Updates
        void onStorageUpdate(TechLevel area, const BaseStorageProxy::Parts_t& parts);
        void onOrderUpdate(const BuildShipProxy::Status& st);
        void onSelectionChange();
        void onSpecificationChange(const gsi::PageContent& content);

     private:
        ui::Root& m_root;
        util::RequestSender<game::Session> m_gameSender;
        afl::string::Translator& m_translator;
        ui::EventLoop m_loop;

        BuildShipProxy m_buildProxy;
        BaseStorageProxy m_storageProxy;
        SpecBrowserProxy m_specProxy;
        game::Id_t m_planetId;

        // Widgets:
        client::widgets::ComponentList* m_pComponentList[game::NUM_TECH_AREAS];
        ui::widgets::ImageButton* m_pImageButtons[game::NUM_TECH_AREAS];
        DocumentView* m_pSpecificationDisplay[game::NUM_TECH_AREAS];
        StaticText* m_pInStorage[game::NUM_TECH_AREAS];
        DocumentView m_orderDisplay;
        client::widgets::ItemCostDisplay m_costDisplay;
        StaticText m_numEngines;
        StaticText m_numBeams;
        StaticText m_numLaunchers;
        Button m_moreBeams;
        Button m_fewerBeams;
        Button m_moreLaunchers;
        Button m_fewerLaunchers;
        Button m_usePartsFromStorage;

        // State
        util::NumberFormatter m_formatter;
        gsi::Page m_specPage;
        game::Id_t m_specId;
        bool m_isNew;
        int m_currentHull;
        game::spec::Cost m_availableAmount;

        // Deleter (must be last because it contains widgets that emit events during destruction)
        afl::base::Deleter m_deleter;

        // Widget building
        ui::Widget& wrapComponentList(TechLevel area);
        ui::Widget& makeStorageColumn(TechLevel area);
        ui::Widget& makeWeaponInfoGroup(DocumentView& specDisplay, StaticText& num, Button& more, Button& fewer);

        // UI helpers
        void setCursors(const BuildShipProxy::Status& st);
        bool checkClone(game::proxy::WaitIndicator& ind);
        bool checkChange();
        bool checkTechUpgrade(game::proxy::WaitIndicator& ind, game::TechLevel area, int level);
        void renderUsePartsFromStorage(const BuildShipProxy::Status& st);
        void renderBuildOrder(const BuildShipProxy::Status& st);
        void renderSpecification(TechLevel area, const gsi::PageContent& content);
        void updateBuildOrder();

        TechLevel getCurrentArea() const;
        gsi::Page getCurrentPage() const;
    };
}

BuildShipDialog::BuildShipDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, game::Id_t planetId, afl::string::Translator& tx)
    : m_root(root),
      m_gameSender(gameSender),
      m_translator(tx),
      m_loop(root),
      m_buildProxy(gameSender, root.engine().dispatcher(), planetId),
      m_storageProxy(gameSender, root.engine().dispatcher(), planetId),
      m_specProxy(gameSender, root.engine().dispatcher(), std::auto_ptr<gsi::PictureNamer>(new client::PictureNamer())),
      m_planetId(planetId),
      m_orderDisplay(getOrderDisplaySize(root), 0, root.provider()),
      m_costDisplay(root, tx),
      m_numEngines(String_t(), util::SkinColor::Contrast, "+", root.provider()),
      m_numBeams(String_t(), util::SkinColor::Contrast, "+", root.provider()),
      m_numLaunchers(String_t(), util::SkinColor::Contrast, "+", root.provider()),
      m_moreBeams("+", '+', root),
      m_fewerBeams("-", '-', root),
      m_moreLaunchers("+", '+', root),
      m_fewerLaunchers("-", '-', root),
      m_usePartsFromStorage("U", 'u', root),
      m_formatter(false, false),
      m_specPage(gsi::PlayerPage),
      m_specId(0),
      m_isNew(false),
      m_currentHull(),
      m_availableAmount(),
      m_deleter()
{
    // Do not show costs on info page; they are on the CostDisplay
    m_specProxy.setWithCost(false);

    // Set to its (hopefully final) size so initial text uses correct layout
    m_orderDisplay.setExtent(gfx::Rectangle(gfx::Point(), getOrderDisplaySize(root)));

    // Connect events for stuff we created
    m_specProxy.sig_pageChange.add(this, &BuildShipDialog::onSpecificationChange);
    m_numEngines.setIsFlexible(true);
    m_numBeams.setIsFlexible(true);
    m_numLaunchers.setIsFlexible(true);
    m_moreBeams.sig_fire.add(this, &BuildShipDialog::addBeam);
    m_fewerBeams.sig_fire.add(this, &BuildShipDialog::removeBeam);
    m_moreLaunchers.sig_fire.add(this, &BuildShipDialog::addLauncher);
    m_fewerLaunchers.sig_fire.add(this, &BuildShipDialog::removeLauncher);
    m_usePartsFromStorage.sig_fire.add(this, &BuildShipDialog::onToggleUseParts);
}

void
BuildShipDialog::init()
{
    client::Downlink link(m_root, m_translator);

    // NumberFormatter
    m_formatter = game::proxy::ConfigurationProxy(m_gameSender).getNumberFormatter(link);
    m_costDisplay.setNumberFormatter(m_formatter);

    // Load list content and construct list widgets
    for (size_t i = 0; i < game::NUM_TECH_AREAS; ++i) {
        // Fetch items
        BaseStorageProxy::Parts_t parts;
        m_storageProxy.getParts(link, TechLevel(i), parts);

        // Make widget
        int numLines = std::max(3, std::min(10, int(parts.size())));
        int widthInEms = (i == game::HullTech ? 15 : 12);
        m_pComponentList[i] = &m_deleter.addNew(new client::widgets::ComponentList(m_root, numLines, widthInEms));
        m_pComponentList[i]->setContent(parts);

        // More widgets
        m_pImageButtons[i] = &m_deleter.addNew(new ui::widgets::ImageButton(String_t(), 0, m_root, gfx::Point(105, 93)));
        m_pSpecificationDisplay[i] = &m_deleter.addNew(new DocumentView(getSpecificationDisplaySize(m_root), 0, m_root.provider()));
        m_pInStorage[i] = &m_deleter.addNew(new StaticText(String_t(), util::SkinColor::Static, gfx::FontRequest(), m_root.provider()));
    }
    m_storageProxy.sig_update.add(this, &BuildShipDialog::onStorageUpdate);

    // Load build order
    BuildShipProxy::Status status;
    m_buildProxy.getStatus(link, status);
    m_buildProxy.sig_change.add(this, &BuildShipDialog::onOrderUpdate);
    m_isNew = status.isNew;
    setCursors(status);
    onOrderUpdate(status);

    // Connect events
    for (size_t i = 0; i < game::NUM_TECH_AREAS; ++i) {
        m_pComponentList[i]->sig_change.add(this, &BuildShipDialog::onSelectionChange);
    }
}

ui::Window&
BuildShipDialog::buildDialog()
{
    // WShipBuildDialog::init
    // Window (VBox)
    //   CardTabBar
    //   CardGroup
    //     HBox (component page)
    //       ComponentList
    //       VBox
    //         Image
    //         Text: in storage
    //         Spacer
    //         Button "Spc-Build"
    //       VBox
    //         Info page; for ships, incl "S" button
    //         Count display/control
    //   HBox
    //     Cost display
    //     Order display
    //   HBox
    //     Option buttons (Use part from storage)
    //   HBox
    //     Enter, Exit, Detailed, Cancel, Help

    afl::base::Deleter& del = m_deleter;
    ui::Window& win = del.addNew(new ui::Window(m_translator("Build Ship"), m_root.provider(), m_root.colorScheme(), ui::BLUE_DARK_WINDOW, ui::layout::VBox::instance5));

    ui::CardGroup& cards = del.addNew(new ui::CardGroup());
    ui::widgets::CardTabBar& tabs = del.addNew(new ui::widgets::CardTabBar(m_root, cards));
    cards.sig_handleFocusChange.add(this, &BuildShipDialog::onSelectionChange);
    tabs.setKeys(ui::widgets::TabBar::Tab | ui::widgets::TabBar::CtrlTab | ui::widgets::TabBar::F6 | ui::widgets::TabBar::Arrows);

    // Hulls
    Group& hullGroup = del.addNew(new Group(ui::layout::HBox::instance5));
    Group& hullInfoGroup = del.addNew(new Group(ui::layout::HBox::instance0));
    hullGroup.add(wrapComponentList(game::HullTech));
    hullGroup.add(makeStorageColumn(game::HullTech));
    hullInfoGroup.add(*m_pSpecificationDisplay[game::HullTech]);
    hullGroup.add(hullInfoGroup);
    cards.add(hullGroup);
    tabs.addPage(util::KeyString(m_translator("Starship Hulls")), hullGroup);

    // Engines
    Group& engineGroup = del.addNew(new Group(ui::layout::HBox::instance5));
    Group& engineInfoGroup = del.addNew(new Group(ui::layout::VBox::instance0));
    engineGroup.add(wrapComponentList(game::EngineTech));
    engineGroup.add(makeStorageColumn(game::EngineTech));
    engineInfoGroup.add(*m_pSpecificationDisplay[game::EngineTech]);
    engineInfoGroup.add(m_numEngines);
    engineGroup.add(engineInfoGroup);
    cards.add(engineGroup);
    tabs.addPage(util::KeyString(m_translator("Engines")), engineGroup);

    // Beams
    Group& beamGroup = del.addNew(new Group(ui::layout::HBox::instance5));
    beamGroup.add(wrapComponentList(game::BeamTech));
    beamGroup.add(makeStorageColumn(game::BeamTech));
    beamGroup.add(makeWeaponInfoGroup(*m_pSpecificationDisplay[game::BeamTech], m_numBeams, m_moreBeams, m_fewerBeams));
    cards.add(beamGroup);
    tabs.addPage(util::KeyString(m_translator("Beams")), beamGroup);

    // Torps
    Group& torpedoGroup = del.addNew(new Group(ui::layout::HBox::instance5));
    torpedoGroup.add(wrapComponentList(game::TorpedoTech));
    torpedoGroup.add(makeStorageColumn(game::TorpedoTech));
    torpedoGroup.add(makeWeaponInfoGroup(*m_pSpecificationDisplay[game::TorpedoTech], m_numLaunchers, m_moreLaunchers, m_fewerLaunchers));
    cards.add(torpedoGroup);
    tabs.addPage(util::KeyString(m_translator("Torpedoes")), torpedoGroup);

    // All the cards/tabs
    win.add(tabs);
    win.add(cards);

    // Bill
    Group& billGroup = del.addNew(new Group(ui::layout::HBox::instance5));
    billGroup.add(m_costDisplay);
    billGroup.add(m_orderDisplay);
    win.add(billGroup);

    // Return half-made window
    return win;
}

void
BuildShipDialog::run(ui::Window& win)
{
    // ex WBaseShipBuildDialog::init
    afl::base::Deleter& del = m_deleter;

    // Option buttons
    Group& optionGroup = del.addNew(new Group(ui::layout::HBox::instance5));
    optionGroup.add(m_usePartsFromStorage);
    optionGroup.add(del.addNew(new StaticText(m_translator("Use parts from storage"), util::SkinColor::White, "+", m_root.provider())));
    optionGroup.add(del.addNew(new ui::Spacer()));
    win.add(optionGroup);

    // Main buttons
    Group& buttonGroup = del.addNew(new Group(ui::layout::HBox::instance5));
    Button& btnBuild  = del.addNew(new Button(m_translator("Enter - Build"), util::Key_Return, m_root));
    Button& btnClose  = del.addNew(new Button(m_translator("Close"), util::Key_Escape, m_root));
    Button& btnDetail = del.addNew(new Button(m_translator("D - Detailed Bill"), 'd', m_root));
    Button& btnHelp   = del.addNew(new Button(m_translator("Help"), 'h', m_root));
    buttonGroup.add(btnBuild);
    buttonGroup.add(btnClose);
    buttonGroup.add(btnDetail);
    if (!m_isNew) {
        Button& btnCancel = del.addNew(new Button(m_translator("C - Cancel Build"), 'c', m_root));
        buttonGroup.add(btnCancel);
        btnCancel.sig_fire.add(this, &BuildShipDialog::onCancelBuild);
    }
    buttonGroup.add(del.addNew(new ui::Spacer()));
    buttonGroup.add(btnHelp);
    win.add(buttonGroup);

    // Administrative
    client::widgets::HelpWidget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, m_gameSender, "pcc2:buildship"));
    win.add(help);
    win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));

    // Events
    btnBuild.sig_fire.add(this, &BuildShipDialog::onBuild);
    btnDetail.sig_fire.add(this, &BuildShipDialog::onDetailedBill);
    btnClose.sig_fire.addNewClosure(m_loop.makeStop(0));
    btnHelp.dispatchKeyTo(help);

    // Do it
    win.pack();
    m_root.centerWidget(win);
    m_root.add(win);
    m_loop.run();
}

// UI actions

/* "Build" button. Decide action and perform it. */
void
BuildShipDialog::onBuild()
{
    // ex WBaseShipBuildDialog::onOk
    client::Downlink link(m_root, m_translator);
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
        MessageBox(m_translator("You cannot build this ship."),
                   m_translator("Build Order Rejected"),
                   m_root).doOkDialog(m_translator);
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

/* "Detailed bill" button */
void
BuildShipDialog::onDetailedBill()
{
    // ex doBuildDetailedBill
    client::Downlink link(m_root, m_translator);
    game::spec::CostSummary result;
    m_buildProxy.getCostSummary(link, result);
    if (result.getNumItems() == 0) {
        return;
    }

    // Dialog:
    //   Window [VBox]
    //     CostSummaryList
    //     HBox
    //       "Help", Spacer, "Export", "Close"
    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(m_translator("Cost for building that starship"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    CostSummaryList& list = del.addNew(new CostSummaryList(int(result.getNumItems()), false, CostSummaryList::ComparisonFooter, m_root.provider(), m_root.colorScheme(), m_translator));
    list.setContent(result);
    list.setAvailableAmount(m_availableAmount);
    win.add(list);

    ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    Button& btnHelp = del.addNew(new Button(m_translator("Help"), 'h', m_root));
    // FIXME: Button& btnExport = del.addNew(new Button(m_translator("E - Export"), 'e', m_root));
    Button& btnClose = del.addNew(new Button(m_translator("Close"), util::Key_Escape, m_root));
    g.add(btnHelp);
    g.add(del.addNew(new ui::Spacer()));
    // g.add(btnExport);
    g.add(btnClose);
    win.add(g);

    ui::EventLoop loop(m_root);
    ui::Widget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, m_gameSender, "pcc2:buildship"));
    ui::widgets::KeyDispatcher& disp = del.addNew(new ui::widgets::KeyDispatcher());
    btnHelp.dispatchKeyTo(help);
    btnClose.sig_fire.addNewClosure(loop.makeStop(0));
    disp.addNewClosure(' ', loop.makeStop(0));
    disp.addNewClosure(util::Key_Return, loop.makeStop(0));
    win.add(help);
    win.add(disp);
    win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));

    win.pack();
    m_root.centerWidget(win);
    m_root.add(win);
    loop.run();
}

/* "Build parts" button for current part */
void
BuildShipDialog::onBuildParts()
{
    // ex doBuildComponent (part)
    // Determine what we need
    const TechLevel area = getCurrentArea();
    const int id = m_pComponentList[area]->getCurrentId();

    // Synchronize current part
    client::Downlink link(m_root, m_translator);
    BuildShipProxy::Status shipStatus;
    m_buildProxy.getStatus(link, shipStatus);

    // Try to achieve correct tech level
    if (!checkTechUpgrade(link, area, shipStatus.partTech)) {
        return;
    }

    // Dialog
    client::dialogs::doBuildShipParts(m_root, m_gameSender, m_planetId, area, id, m_translator);
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

/* "+" button for beams. */
void
BuildShipDialog::addBeam()
{
    m_buildProxy.addParts(game::actions::BuildShip::BeamWeapon, +1);
}

/* "-" button for beams. */
void
BuildShipDialog::removeBeam()
{
    m_buildProxy.addParts(game::actions::BuildShip::BeamWeapon, -1);
}

/* "+" button for torpedo launchers. */
void
BuildShipDialog::addLauncher()
{
    m_buildProxy.addParts(game::actions::BuildShip::TorpedoWeapon, +1);
}

/* "-" button for torpedo launchers. */
void
BuildShipDialog::removeLauncher()
{
    m_buildProxy.addParts(game::actions::BuildShip::TorpedoWeapon, -1);
}

// Actions

/* BaseStorageProxy update. Just update the list. */
void
BuildShipDialog::onStorageUpdate(TechLevel area, const BaseStorageProxy::Parts_t& parts)
{
    m_pComponentList[area]->setContent(parts);
}

/* Order update. Render everything.
   However, do NOT update the list cursors.
   That happens only once during initialisation (setCursors), to avoid queued updates overriding user scrolling. */
void
BuildShipDialog::onOrderUpdate(const BuildShipProxy::Status& st)
{
    // Render build order
    renderBuildOrder(st);

    // Render costs
    m_costDisplay.setAvailableAmount(st.available);
    m_costDisplay.setPartCost(st.partCost);
    m_costDisplay.setPartTechLevel(st.availableTech, st.partTech);
    m_costDisplay.setTotalCost(st.totalCost);
    m_availableAmount = st.available;

    // Engines
    m_numEngines.setText(Format(m_translator("Ship requires %d engine%!1{s%}."), st.numEngines));

    // Beams
    bool noBeams = (st.maxBeams == 0);
    if (noBeams) {
        m_numBeams.setText(m_translator("Ship cannot have beams."));
    } else {
        m_numBeams.setText(Format(m_translator("Beams on ship: %d (max %d)"), st.order.getNumBeams(), st.maxBeams));
    }
    m_moreBeams.setState(ui::Widget::DisabledState, noBeams);
    m_fewerBeams.setState(ui::Widget::DisabledState, noBeams);

    // Launchers
    bool noLaunchers = (st.maxLaunchers == 0);
    if (noLaunchers) {
        m_numLaunchers.setText(m_translator("Ship cannot have torpedo launchers."));
    } else {
        m_numLaunchers.setText(Format(m_translator("Launchers on ship: %d (max %d)"), st.order.getNumLaunchers(), st.maxLaunchers));
    }
    m_moreLaunchers.setState(ui::Widget::DisabledState, noLaunchers);
    m_fewerLaunchers.setState(ui::Widget::DisabledState, noLaunchers);

    // Render current amount
    TechLevel area = getCurrentArea();
    m_pInStorage[area]->setText(Format(m_translator("In storage: %d"), m_formatter.formatNumber(m_pComponentList[area]->getCurrentAmount())));

    // Use-parts-from-storage flag
    renderUsePartsFromStorage(st);
}

/* User focus changed, i.e. new list item or page. Update build order and current part. */
void
BuildShipDialog::onSelectionChange()
{
    updateBuildOrder();
}

/* Specification update from SpecBrowserProxy. */
void
BuildShipDialog::onSpecificationChange(const gsi::PageContent& content)
{
    TechLevel area = getCurrentArea();
    m_pImageButtons[area]->setImage(content.pictureName);

    renderSpecification(area, content);
}

// Widget building

/* Wrap an area's ComponentList into its frames */
ui::Widget&
BuildShipDialog::wrapComponentList(TechLevel area)
{
    return ui::widgets::FrameGroup::wrapWidget(m_deleter, m_root.colorScheme(), ui::LoweredFrame,
                                               m_deleter.addNew(new ui::widgets::ScrollbarContainer(*m_pComponentList[area], m_root)));
}

/* Make storage column (image, "build" button, "in storage" display) for one area. */
ui::Widget&
BuildShipDialog::makeStorageColumn(TechLevel area)
{
    Button& btnBuild = m_deleter.addNew(new Button(m_translator("Spc-Build"), ' ', m_root));
    Group& g = m_deleter.addNew(new Group(ui::layout::VBox::instance5));
    g.add(ui::widgets::FrameGroup::wrapWidget(m_deleter, m_root.colorScheme(), ui::LoweredFrame, *m_pImageButtons[area]));
    g.add(*m_pInStorage[area]);
    g.add(m_deleter.addNew(new ui::Spacer()));
    g.add(btnBuild);
    btnBuild.sig_fire.add(this, &BuildShipDialog::onBuildParts);
    return g;
}

/* Make information column */
ui::Widget&
BuildShipDialog::makeWeaponInfoGroup(DocumentView& specDisplay, StaticText& num, Button& more, Button& fewer)
{
    Group& g = m_deleter.addNew(new Group(ui::layout::VBox::instance5));
    Group& g1 = m_deleter.addNew(new Group(ui::layout::HBox::instance5));
    g.add(specDisplay);
    g1.add(num);
    g1.add(fewer);
    g1.add(more);
    g.add(g1);
    return g;
}

// UI helpers

/* Set cursors in ComponentList's according to build order */
void
BuildShipDialog::setCursors(const BuildShipProxy::Status& st)
{
    // Position cursors
    m_pComponentList[game::HullTech]->setCurrentId(st.order.getHullIndex());
    m_pComponentList[game::EngineTech]->setCurrentId(st.order.getEngineType());
    m_pComponentList[game::BeamTech]->setCurrentId(st.order.getBeamType());
    m_pComponentList[game::TorpedoTech]->setCurrentId(st.order.getLauncherType());

    // Remember initial hull
    m_currentHull = st.order.getHullIndex();
}

/* Check for conflicting clone order.
   Return true to proceed, false to stop processing. */
bool
BuildShipDialog::checkClone(game::proxy::WaitIndicator& ind)
{
    // Are we cloning?
    game::Id_t shipId;
    String_t shipName;
    if (!m_buildProxy.findShipCloningHere(ind, shipId, shipName)) {
        return true;
    }

    // OK, we are cloning. Ask user.
    MessageBox box(Format(m_translator("This base is already cloning %s (#%d). Do you want to cancel that order? "
                                       "If you say \"No\", this ship will be built after the clone completed."),
                                       shipName, shipId),
                   m_translator("Build Ship"), m_root);
    enum { YES, NO, CANCEL };
    box.addButton(YES,    util::KeyString(m_translator("Yes")));
    box.addButton(NO,     util::KeyString(m_translator("No")));
    box.addButton(CANCEL, m_translator("Cancel"), util::Key_Escape);
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
    // Do the dialog by hand. We don't want ENTER to confirm the build.
    MessageBox box(util::rich::Parser::parseXml(m_translator("This starbase already has a build order. Do you want to change that order?\n\n"
                                                             "<small>To exit the ship build screen without changing the build order, use \"Exit\" (ESC). "
                                                             "To replace the existing order with your new choice, use <kbd>Y</kbd>.</small>")),
                   m_translator("Build Ship"), m_root);
    enum { YES, NO };
    box.addButton(YES, util::KeyString(m_translator("Yes")));
    box.addButton(NO,  util::KeyString(m_translator("No")));
    box.ignoreKey(util::Key_Return);          // We don't want users to confirm this dialog accidentally.
    int reply = box.run();

    return (reply == YES);
}

bool
BuildShipDialog::checkTechUpgrade(game::proxy::WaitIndicator& ind, game::TechLevel area, int level)
{
    // Try to achieve correct tech level
    TechUpgradeProxy techProxy(m_gameSender, m_root.engine().dispatcher(), m_planetId);
    techProxy.upgradeTechLevel(area, level);
    TechUpgradeProxy::Status techStatus;
    techProxy.getStatus(ind, techStatus);

    static const char*const MESSAGES[] = {
        N_("To build this engine, you need tech %d."),
        N_("To build this hull, you need tech %d."),
        N_("To build this beam, you need tech %d."),
        N_("To build this torpedo launcher, you need tech %d.")
    };
    String_t message = Format(m_translator(MESSAGES[area]), level);
    switch (techStatus.status) {
     case game::actions::TechUpgrade::Success:
        if (techStatus.cost.get(game::spec::Cost::Money) != 0) {
            message += " ";
            message += Format(m_translator("Do you want to upgrade for %d mc?"), techStatus.cost.get(game::spec::Cost::Money));
            if (!MessageBox(message, m_translator("Build Components"), m_root).doYesNoDialog(m_translator)) {
                return false;
            }
            techProxy.commit();
        }
        break;

     case game::actions::TechUpgrade::MissingResources:
        message += " ";
        message += Format(m_translator("You do not have the required %d megacredits required to upgrade to the required level."), techStatus.cost.get(game::spec::Cost::Money));
        MessageBox(message, m_translator("Build Components"), m_root).doOkDialog(m_translator);
        return false;

     case game::actions::TechUpgrade::DisallowedTech:
     case game::actions::TechUpgrade::DisabledTech:    // cannot happen
     case game::actions::TechUpgrade::ForeignHull:     // cannot happen
        message += " ";
        message += m_translator("You cannot buy this tech level.");
        MessageBox(message, m_translator("Build Components"), m_root).doOkDialog(m_translator);
        return false;
    }
    return true;
}

/* Render "use parts from storage" flag. */
void
BuildShipDialog::renderUsePartsFromStorage(const BuildShipProxy::Status& st)
{
    // ex WBaseShipBuildDialog::updateUseParts
    m_usePartsFromStorage.setFlag(ui::HighlightedButton, st.isUsePartsFromStorage);
}

/* Render build order summary. */
void
BuildShipDialog::renderBuildOrder(const BuildShipProxy::Status& st)
{
    // ex WBuildOrderDisplay::drawContent
    ui::rich::Document& doc = m_orderDisplay.getDocument();
    doc.clear();
    doc.add(util::rich::Text(m_translator("Your order:")).withColor(util::SkinColor::Heading));
    doc.addNewline();
    for (size_t i = 0, n = st.description.size(); i < n; ++i) {
        doc.add(UTF_BULLET " ");
        doc.add(st.description[i]);
        doc.addNewline();
    }
    doc.finish();
    m_orderDisplay.handleDocumentUpdate();
}

/* Render component specification. */
void
BuildShipDialog::renderSpecification(TechLevel area, const gsi::PageContent& content)
{
    DocumentView& docView = *m_pSpecificationDisplay[area];
    ui::rich::Document& doc = docView.getDocument();
    doc.clear();
    client::dialogs::renderHullInformation(doc, m_root, content, m_translator);
    doc.finish();
    docView.handleDocumentUpdate();
}

/* Update build order. */
void
BuildShipDialog::updateBuildOrder()
{
    // Determine active page
    TechLevel area = getCurrentArea();
    gsi::Page page = getCurrentPage();

    // Select part on BuildShipProxy to get current part cost
    // As a special case, avoid setting the same hull again because that will lose custom weapon counts.
    const int id = m_pComponentList[area]->getCurrentId();
    if (area != game::HullTech || id != m_currentHull) {
        m_buildProxy.setPart(area, id);
    }
    m_buildProxy.selectPart(area, id);
    if (area == game::HullTech) {
        m_currentHull = id;
    }

    // Select part on SpecBrowserProxy to get current specs
    // (but avoid sending duplicate requests as this is triggered by many no-changes)
    if (m_specPage != page || m_specId != id) {
        m_specProxy.setPageId(page, id);
        m_specPage = page;
        m_specId = id;
    }
}

TechLevel
BuildShipDialog::getCurrentArea() const
{
    if (m_pComponentList[game::EngineTech]->hasState(ui::Widget::FocusedState)) {
        return game::EngineTech;
    } else if (m_pComponentList[game::BeamTech]->hasState(ui::Widget::FocusedState)) {
        return game::BeamTech;
    } else if (m_pComponentList[game::TorpedoTech]->hasState(ui::Widget::FocusedState)) {
        return game::TorpedoTech;
    } else {
        return game::HullTech;
    }
}

gsi::Page
BuildShipDialog::getCurrentPage() const
{
    if (m_pComponentList[game::EngineTech]->hasState(ui::Widget::FocusedState)) {
        return gsi::EnginePage;
    } else if (m_pComponentList[game::BeamTech]->hasState(ui::Widget::FocusedState)) {
        return gsi::BeamPage;
    } else if (m_pComponentList[game::TorpedoTech]->hasState(ui::Widget::FocusedState)) {
        return gsi::TorpedoPage;
    } else {
        return gsi::HullPage;
    }
}


/*
 *  Main Entry Point
 */

void
client::dialogs::doBuildShip(ui::Root& root, util::RequestSender<game::Session> gameSender, game::Id_t planetId, afl::string::Translator& tx)
{
    BuildShipDialog dlg(root, gameSender, planetId, tx);
    dlg.init();
    dlg.run(dlg.buildDialog());
}
