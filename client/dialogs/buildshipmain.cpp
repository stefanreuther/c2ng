/**
  *  \file client/dialogs/buildshipmain.cpp
  *  \brief Class client::dialogs::BuildShipMain
  */

#include "client/dialogs/buildshipmain.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/buildparts.hpp"
#include "client/dialogs/hullspecification.hpp"
#include "client/dialogs/specbrowserdialog.hpp"
#include "client/dialogs/techupgradedialog.hpp"
#include "client/downlink.hpp"
#include "client/picturenamer.hpp"
#include "client/widgets/costsummarylist.hpp"
#include "client/widgets/helpwidget.hpp"
#include "game/proxy/configurationproxy.hpp"
#include "ui/cardgroup.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/cardtabbar.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "util/translation.hpp"

using afl::string::Format;
using client::widgets::ComponentList;
using client::widgets::CostSummaryList;
using client::widgets::HelpWidget;
using game::TechLevel;
using game::proxy::BaseStorageProxy;
using game::proxy::BuildShipProxy;
using ui::Group;
using ui::rich::DocumentView;
using ui::widgets::Button;
using ui::widgets::StaticText;
using ui::widgets::TabBar;
using util::SkinColor;

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

    ComponentList::Parts_t convertParts(const BaseStorageProxy::Parts_t& in)
    {
        ComponentList::Parts_t out;
        for (size_t i = 0; i < in.size(); ++i) {
            const BaseStorageProxy::Part& pt = in[i];
            out.push_back(ComponentList::Part(pt.id, pt.name, pt.numParts, true, pt.techStatus));
        }
        return out;
    }
}

client::dialogs::BuildShipMain::BuildShipMain(ui::Root& root,
                                              game::proxy::BuildShipProxy& buildProxy,
                                              game::proxy::BaseStorageProxy& storageProxy,
                                              util::RequestSender<game::Session> gameSender,
                                              game::Id_t planetId,
                                              afl::string::Translator& tx)
    : m_root(root),
      m_gameSender(gameSender),
      m_translator(tx),
      m_buildProxy(buildProxy),
      m_storageProxy(storageProxy),
      m_specProxy(gameSender, root.engine().dispatcher(), std::auto_ptr<gsi::PictureNamer>(new PictureNamer())),
      m_planetId(planetId),
      m_orderDisplay(getOrderDisplaySize(root), 0, root.provider()),
      m_costDisplay(root, tx),
      m_numEngines(String_t(), SkinColor::Contrast, "+", root.provider()),
      m_numBeams(String_t(), SkinColor::Contrast, "+", root.provider()),
      m_numLaunchers(String_t(), SkinColor::Contrast, "+", root.provider()),
      m_moreBeams("+", '+', root),
      m_fewerBeams("-", '-', root),
      m_moreLaunchers("+", '+', root),
      m_fewerLaunchers("-", '-', root),
      m_formatter(false, false),
      m_specPage(gsi::PlayerPage),
      m_specId(0),
      m_currentHull(),
      m_availableAmount()
{
    // Do not show costs on info page; they are on the CostDisplay
    m_specProxy.setWithCost(false);

    // Set to its (hopefully final) size so initial text uses correct layout
    m_orderDisplay.setExtent(gfx::Rectangle(gfx::Point(), getOrderDisplaySize(root)));

    // Connect events for stuff we created
    m_specProxy.sig_pageChange.add(this, &BuildShipMain::onSpecificationChange);
    m_numEngines.setIsFlexible(true);
    m_numBeams.setIsFlexible(true);
    m_numLaunchers.setIsFlexible(true);
    m_moreBeams.sig_fire.add(this, &BuildShipMain::addBeam);
    m_fewerBeams.sig_fire.add(this, &BuildShipMain::removeBeam);
    m_moreLaunchers.sig_fire.add(this, &BuildShipMain::addLauncher);
    m_fewerLaunchers.sig_fire.add(this, &BuildShipMain::removeLauncher);
}

void
client::dialogs::BuildShipMain::init(afl::base::Deleter& del)
{
    Downlink link(m_root, m_translator);

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
        m_pComponentList[i] = &del.addNew(new ComponentList(m_root, numLines, widthInEms));
        m_pComponentList[i]->setContent(convertParts(parts));

        // More widgets
        m_pImageButtons[i] = &del.addNew(new ui::widgets::ImageButton(String_t(), 0, m_root, gfx::Point(105, 93)));
        m_pSpecificationDisplay[i] = &del.addNew(new DocumentView(getSpecificationDisplaySize(m_root), 0, m_root.provider()));
        m_pInStorage[i] = &del.addNew(new StaticText(String_t(), SkinColor::Static, gfx::FontRequest(), m_root.provider()));
    }
    m_storageProxy.sig_update.add(this, &BuildShipMain::onStorageUpdate);

    // Load build order
    BuildShipProxy::Status status;
    m_buildProxy.getStatus(link, status);
    m_buildProxy.sig_change.add(this, &BuildShipMain::onOrderUpdate);
    setCursors(status);
    onOrderUpdate(status);

    // Connect events
    for (size_t i = 0; i < game::NUM_TECH_AREAS; ++i) {
        conn_componentSelectionChange[i] = m_pComponentList[i]->sig_change.add(this, &BuildShipMain::onSelectionChange);
    }
}

ui::Window&
client::dialogs::BuildShipMain::buildDialog(afl::base::Deleter& del, String_t title)
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

    ui::Window& win = del.addNew(new ui::Window(title, m_root.provider(), m_root.colorScheme(), ui::BLUE_DARK_WINDOW, ui::layout::VBox::instance5));

    ui::CardGroup& cards = del.addNew(new ui::CardGroup());
    ui::widgets::CardTabBar& tabs = del.addNew(new ui::widgets::CardTabBar(m_root, cards));
    conn_mainSelectionChange = cards.sig_handleFocusChange.add(this, &BuildShipMain::onSelectionChange);
    tabs.setKeys(TabBar::Tab | TabBar::CtrlTab | TabBar::F6 | TabBar::Arrows);

    // Hulls
    Group& hullGroup = del.addNew(new Group(ui::layout::HBox::instance5));
    Group& hullInfoGroup = del.addNew(new Group(ui::layout::HBox::instance0));
    Group& hullButtonGroup = del.addNew(new Group(ui::layout::VBox::instance5));
    hullGroup.add(wrapComponentList(del, game::HullTech));
    hullGroup.add(makeStorageColumn(del, game::HullTech));
    hullInfoGroup.add(*m_pSpecificationDisplay[game::HullTech]);
    hullInfoGroup.add(hullButtonGroup);
    hullGroup.add(hullInfoGroup);
    cards.add(hullGroup);
    tabs.addPage(util::KeyString(m_translator("Starship Hulls")), hullGroup);

    Button& btnHullSpec = del.addNew(new Button("S", 's', m_root));
    hullButtonGroup.add(btnHullSpec);
    hullButtonGroup.add(del.addNew(new ui::Spacer()));
    btnHullSpec.sig_fire.add(this, &BuildShipMain::onHullSpecification);

    // Engines
    Group& engineGroup = del.addNew(new Group(ui::layout::HBox::instance5));
    Group& engineInfoGroup = del.addNew(new Group(ui::layout::VBox::instance0));
    engineGroup.add(wrapComponentList(del, game::EngineTech));
    engineGroup.add(makeStorageColumn(del, game::EngineTech));
    engineInfoGroup.add(*m_pSpecificationDisplay[game::EngineTech]);
    engineInfoGroup.add(m_numEngines);
    engineGroup.add(engineInfoGroup);
    cards.add(engineGroup);
    tabs.addPage(util::KeyString(m_translator("Engines")), engineGroup);

    // Beams
    Group& beamGroup = del.addNew(new Group(ui::layout::HBox::instance5));
    beamGroup.add(wrapComponentList(del, game::BeamTech));
    beamGroup.add(makeStorageColumn(del, game::BeamTech));
    beamGroup.add(makeWeaponInfoGroup(del, *m_pSpecificationDisplay[game::BeamTech], m_numBeams, m_moreBeams, m_fewerBeams));
    cards.add(beamGroup);
    tabs.addPage(util::KeyString(m_translator("Beams")), beamGroup);

    // Torps
    Group& torpedoGroup = del.addNew(new Group(ui::layout::HBox::instance5));
    torpedoGroup.add(wrapComponentList(del, game::TorpedoTech));
    torpedoGroup.add(makeStorageColumn(del, game::TorpedoTech));
    torpedoGroup.add(makeWeaponInfoGroup(del, *m_pSpecificationDisplay[game::TorpedoTech], m_numLaunchers, m_moreLaunchers, m_fewerLaunchers));
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

ui::Widget&
client::dialogs::BuildShipMain::makeDetailedBillButton(afl::base::Deleter& del)
{
    Button& btnDetail = del.addNew(new Button(m_translator("D - Detailed Bill"), 'd', m_root));
    btnDetail.sig_fire.add(this, &BuildShipMain::onDetailedBill);
    return btnDetail;
}

ui::Widget&
client::dialogs::BuildShipMain::makeHelpWidget(afl::base::Deleter& del, String_t helpId)
{
    return del.addNew(new HelpWidget(m_root, m_translator, m_gameSender, helpId));
}

// UI actions
void
client::dialogs::BuildShipMain::onDetailedBill()
{
    // ex doBuildDetailedBill
    Downlink link(m_root, m_translator);
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

    Group& g = del.addNew(new Group(ui::layout::HBox::instance5));
    Button& btnHelp = del.addNew(new Button(m_translator("Help"), 'h', m_root));
    // FIXME: Button& btnExport = del.addNew(new Button(m_translator("E - Export"), 'e', m_root));
    Button& btnClose = del.addNew(new Button(m_translator("Close"), util::Key_Escape, m_root));
    g.add(btnHelp);
    g.add(del.addNew(new ui::Spacer()));
    // g.add(btnExport);
    g.add(btnClose);
    win.add(g);

    ui::EventLoop loop(m_root);
    ui::Widget& help = del.addNew(new HelpWidget(m_root, m_translator, m_gameSender, "pcc2:buildship"));
    ui::widgets::KeyDispatcher& disp = del.addNew(new ui::widgets::KeyDispatcher());
    btnHelp.dispatchKeyTo(help);
    btnClose.sig_fire.addNewClosure(loop.makeStop(0));
    disp.addNewClosure(' ', loop.makeStop(0));
    disp.addNewClosure(util::Key_Return, loop.makeStop(0));
    win.add(help);
    win.add(disp);
    win.add(del.addNew(new ui::widgets::Quit(m_root, loop)));

    win.pack();
    m_root.centerWidget(win);
    m_root.add(win);
    loop.run();
}

void
client::dialogs::BuildShipMain::onHullSpecification()
{
    // Get status
    Downlink link(m_root, m_translator);
    game::ShipQuery q = m_buildProxy.getQuery(link);

    // Show it
    client::dialogs::showHullSpecification(q, m_root, m_translator, m_gameSender);
}
void
client::dialogs::BuildShipMain::onBuildParts()
{
    // ex doBuildComponent (part)
    // Planet Id 0 means we do not have an actual planet
    if (m_planetId == 0) {
        return;
    }

    // Determine what we need
    const TechLevel area = getCurrentArea();
    const int id = m_pComponentList[area]->getCurrentId();

    // Synchronize current part
    Downlink link(m_root, m_translator);
    BuildShipProxy::Status shipStatus;
    m_buildProxy.getStatus(link, shipStatus);

    // Try to achieve correct tech level
    if (!checkTechUpgrade(link, area, shipStatus.partTech)) {
        return;
    }

    // Dialog
    doBuildShipParts(m_root, m_gameSender, m_planetId, area, id, m_translator);
}

/* "+" button for beams. */
void
client::dialogs::BuildShipMain::addBeam()
{
    m_buildProxy.addParts(game::actions::BuildShip::BeamWeapon, +1);
}

/* "-" button for beams. */
void
client::dialogs::BuildShipMain::removeBeam()
{
    m_buildProxy.addParts(game::actions::BuildShip::BeamWeapon, -1);
}

/* "+" button for torpedo launchers. */
void
client::dialogs::BuildShipMain::addLauncher()
{
    m_buildProxy.addParts(game::actions::BuildShip::TorpedoWeapon, +1);
}

/* "-" button for torpedo launchers. */
void
client::dialogs::BuildShipMain::removeLauncher()
{
    m_buildProxy.addParts(game::actions::BuildShip::TorpedoWeapon, -1);
}

// Updates

/* BaseStorageProxy update. Just update the list. */
void
client::dialogs::BuildShipMain::onStorageUpdate(game::TechLevel area, const game::proxy::BaseStorageProxy::Parts_t& parts)
{
    m_pComponentList[area]->setContent(convertParts(parts));
}

/* Order update. Render everything.
   However, do NOT update the list cursors.
   That happens only once during initialisation (setCursors), to avoid queued updates overriding user scrolling. */
void
client::dialogs::BuildShipMain::onOrderUpdate(const game::proxy::BuildShipProxy::Status& st)
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

    // Forward to derived class
    sig_change.raise(st);
}

/* User focus changed, i.e. new list item or page. Update build order and current part. */
void
client::dialogs::BuildShipMain::onSelectionChange()
{
    updateBuildOrder();
}

/* Specification update from SpecBrowserProxy. */
void
client::dialogs::BuildShipMain::onSpecificationChange(const game::spec::info::PageContent& content)
{
    TechLevel area = getCurrentArea();
    m_pImageButtons[area]->setImage(content.pictureName);

    renderSpecification(area, content);
}

// Widget building

/* Wrap an area's ComponentList into its frames */
ui::Widget&
client::dialogs::BuildShipMain::wrapComponentList(afl::base::Deleter& del, game::TechLevel area)
{
    return ui::widgets::FrameGroup::wrapWidget(del, m_root.colorScheme(), ui::LoweredFrame,
                                               del.addNew(new ui::widgets::ScrollbarContainer(*m_pComponentList[area], m_root)));
}

/* Make storage column (image, "build" button, "in storage" display) for one area. */
ui::Widget&
client::dialogs::BuildShipMain::makeStorageColumn(afl::base::Deleter& del, game::TechLevel area)
{
    Group& g = del.addNew(new Group(ui::layout::VBox::instance5));
    g.add(ui::widgets::FrameGroup::wrapWidget(del, m_root.colorScheme(), ui::LoweredFrame, *m_pImageButtons[area]));
    if (m_planetId != 0) {
        // We have a planet
        Button& btnBuild = del.addNew(new Button(m_translator("Spc-Build"), ' ', m_root));
        g.add(*m_pInStorage[area]);
        g.add(del.addNew(new ui::Spacer()));
        g.add(btnBuild);
        btnBuild.sig_fire.add(this, &BuildShipMain::onBuildParts);
    } else {
        // We do not have a planet: just a spacer
        g.add(del.addNew(new ui::Spacer()));
    }
    return g;
}

/* Make information column */
ui::Widget&
client::dialogs::BuildShipMain::makeWeaponInfoGroup(afl::base::Deleter& del, ui::rich::DocumentView& specDisplay, ui::widgets::StaticText& num, ui::widgets::Button& more, ui::widgets::Button& fewer)
{
    Group& g = del.addNew(new Group(ui::layout::VBox::instance5));
    Group& g1 = del.addNew(new Group(ui::layout::HBox::instance5));
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
client::dialogs::BuildShipMain::setCursors(const game::proxy::BuildShipProxy::Status& st)
{
    // Position cursors
    m_pComponentList[game::HullTech]->setCurrentId(st.order.getHullIndex());
    m_pComponentList[game::EngineTech]->setCurrentId(st.order.getEngineType());
    m_pComponentList[game::BeamTech]->setCurrentId(st.order.getBeamType());
    m_pComponentList[game::TorpedoTech]->setCurrentId(st.order.getLauncherType());

    // Remember initial hull
    m_currentHull = st.order.getHullIndex();
}

bool
client::dialogs::BuildShipMain::checkTechUpgrade(game::proxy::WaitIndicator& ind, game::TechLevel area, int level)
{
    static const char*const MESSAGES[] = {
        N_("To build this engine, you need tech %d."),
        N_("To build this hull, you need tech %d."),
        N_("To build this beam, you need tech %d."),
        N_("To build this torpedo launcher, you need tech %d.")
    };
    return client::dialogs::checkTechUpgrade(m_root, m_translator, m_gameSender, m_planetId,
                                             ind, area, level, game::spec::Cost(),
                                             m_translator(MESSAGES[area]), m_translator("Build Components"));
}

/* Render build order summary. */
void
client::dialogs::BuildShipMain::renderBuildOrder(const game::proxy::BuildShipProxy::Status& st)
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
client::dialogs::BuildShipMain::renderSpecification(game::TechLevel area, const game::spec::info::PageContent& content)
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
client::dialogs::BuildShipMain::updateBuildOrder()
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

game::TechLevel
client::dialogs::BuildShipMain::getCurrentArea() const
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

game::spec::info::Page
client::dialogs::BuildShipMain::getCurrentPage() const
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
