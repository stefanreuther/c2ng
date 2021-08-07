/**
  *  \file client/dialogs/classicvcrobject.cpp
  *  \brief Classic VCR Object Information Dialog
  */

#include "client/dialogs/classicvcrobject.hpp"
#include "afl/base/deleter.hpp"
#include "client/dialogs/hullspecification.hpp"
#include "client/downlink.hpp"
#include "client/widgets/helpwidget.hpp"
#include "client/widgets/vcrobjectinfo.hpp"
#include "game/proxy/configurationproxy.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/group.hpp"
#include "ui/invisiblewidget.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/imagebutton.hpp"
#include "ui/widgets/keyforwarder.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "ui/window.hpp"

using game::proxy::VcrDatabaseProxy;
using ui::widgets::Button;
using ui::Group;
using ui::widgets::FrameGroup;

namespace {
    class ClassicVcrObjectDialog : public gfx::KeyEventConsumer {
     public:
        ClassicVcrObjectDialog(ui::Root& root, util::NumberFormatter fmt, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender, VcrDatabaseProxy& proxy, size_t side, game::proxy::WaitIndicator& ind);

        bool run(ui::Root& root);
        void onSideUpdate(const VcrDatabaseProxy::SideInfo& info);
        void onHullUpdate(const VcrDatabaseProxy::HullInfo& info);
        void onTab();
        void onListScroll();
        void onGoTo();
        void onHullSpecification();

        bool handleKey(util::Key_t key, int prefix);
        void setSide(size_t side);
        void addToSimulation(bool after);

        game::Reference getReference() const;

     private:
        ui::widgets::StaticText m_nameWidget;
        ui::widgets::StaticText m_subtitleWidget;
        ui::widgets::StringListbox m_hullList;
        ui::widgets::ImageButton m_image;
        client::widgets::VcrObjectInfo m_info;
        Button m_gotoButton;
        Button m_specButton;
        ui::EventLoop m_loop;

        VcrDatabaseProxy& m_proxy;
        game::proxy::WaitIndicator& m_indicator;
        afl::string::Translator& m_translator;
        ui::Root& m_root;
        util::RequestSender<game::Session> m_gameSender;

        game::Reference m_reference;
        afl::base::Optional<game::ShipQuery> m_shipQuery;

        size_t m_side;

        afl::base::SignalConnection conn_sideUpdate;
        afl::base::SignalConnection conn_hullUpdate;
    };
}

ClassicVcrObjectDialog::ClassicVcrObjectDialog(ui::Root& root, util::NumberFormatter fmt, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender, VcrDatabaseProxy& proxy, size_t side, game::proxy::WaitIndicator& ind)
    : m_nameWidget(String_t(), util::SkinColor::Static, "+", root.provider()),
      m_subtitleWidget(String_t(), util::SkinColor::Static, gfx::FontRequest(), root.provider()),
      m_hullList(root.provider(), root.colorScheme()),
      m_image(String_t(), 0, root, gfx::Point(105, 93)),
      m_info(false, fmt, tx, root.provider()),
      m_gotoButton(tx("Go to"), util::Key_Return, root),
      m_specButton("S", 's', root),
      m_loop(root),
      m_proxy(proxy),
      m_indicator(ind),
      m_translator(tx),
      m_root(root),
      m_gameSender(gameSender),
      m_reference(),
      m_shipQuery(),
      m_side(side),
      conn_sideUpdate(proxy.sig_sideUpdate.add(this, &ClassicVcrObjectDialog::onSideUpdate)),
      conn_hullUpdate(proxy.sig_hullUpdate.add(this, &ClassicVcrObjectDialog::onHullUpdate))
{
    m_nameWidget.setIsFlexible(true);
    m_subtitleWidget.setIsFlexible(true);

    // WVcrHullList::WVcrHullList
    m_hullList.setPreferredWidth(20, false);
    m_hullList.setPreferredHeight(3);
    m_hullList.sig_change.add(this, &ClassicVcrObjectDialog::onListScroll);
    m_gotoButton.sig_fire.add(this, &ClassicVcrObjectDialog::onGoTo);
    m_specButton.sig_fire.add(this, &ClassicVcrObjectDialog::onHullSpecification);
}

bool
ClassicVcrObjectDialog::run(ui::Root& root)
{
    // ex WVcrInfo::WVcrInfo, WVcrInfo::init
    afl::string::Translator& tx = m_translator;
    afl::base::Deleter del;

    // VBox
    //   HBox
    //     VBox (Title, Subtitle, Spacer, Hull List)
    //     Image
    //     Spacer
    //   Info
    //   HBox (Help, Spacer, Ins, Tab, Goto, Close)
    ui::Window& win = del.addNew(new ui::Window(tx("Combat Information"), root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    Group& headGroup = del.addNew(new Group(ui::layout::HBox::instance5));
    Group& leftGroup = del.addNew(new Group(ui::layout::VBox::instance0));
    leftGroup.add(m_nameWidget);
    leftGroup.add(m_subtitleWidget);
    leftGroup.add(del.addNew(new ui::Spacer()));
    leftGroup.add(FrameGroup::wrapWidget(del, root.colorScheme(), ui::LoweredFrame, m_hullList));
    headGroup.add(leftGroup);
    headGroup.add(FrameGroup::wrapWidget(del, root.colorScheme(), ui::LoweredFrame, m_image));
    headGroup.add(del.addNew(new ui::Spacer()));
    win.add(headGroup);

    Group& infoGroup = del.addNew(new Group(ui::layout::HBox::instance5));
    Group& rightGroup = del.addNew(new Group(ui::layout::VBox::instance5));
    rightGroup.add(del.addNew(new ui::Spacer()));
    rightGroup.add(m_specButton);
    infoGroup.add(m_info);
    infoGroup.add(rightGroup);
    win.add(infoGroup);

    ui::Widget& disp = del.addNew(new ui::widgets::KeyForwarder(*this));
    ui::Widget& help = del.addNew(new client::widgets::HelpWidget(root, tx, m_gameSender, "pcc2:vcrinfo"));
    Group& buttons = del.addNew(new Group(ui::layout::HBox::instance5));
    Button& btnHelp  = del.addNew(new Button(tx("Help"), 'h', root));
    Button& btnAdd   = del.addNew(new Button(tx("Ins - Sim"), util::Key_Insert, root));
    Button& btnTab   = del.addNew(new Button(tx("Tab - Other Side"), util::Key_Tab, root));
    Button& btnClose = del.addNew(new Button(tx("Close"), util::Key_Escape, root));
    buttons.add(btnHelp);
    buttons.add(del.addNew(new ui::Spacer()));
    buttons.add(btnAdd);
    buttons.add(btnTab);
    buttons.add(m_gotoButton);
    buttons.add(btnClose);
    win.add(buttons);
    win.add(del.addNew(new ui::widgets::Quit(root, m_loop)));
    win.add(disp);

    btnAdd.dispatchKeyTo(disp); // to capture Shift-Ins
    btnHelp.dispatchKeyTo(help);
    btnTab.sig_fire.add(this, &ClassicVcrObjectDialog::onTab);
    btnClose.sig_fire.addNewClosure(m_loop.makeStop(0));

    m_proxy.setSide(m_side, true);
    m_hullList.requestFocus();

    win.pack();
    root.centerWidget(win);
    root.add(win);
    return m_loop.run() != 0;
}

void
ClassicVcrObjectDialog::onSideUpdate(const VcrDatabaseProxy::SideInfo& info)
{
    m_nameWidget.setText(info.name);
    m_subtitleWidget.setText(info.subtitle);
    m_hullList.setItems(info.typeChoices);
    m_hullList.setCurrentItem(0);
    m_reference = info.reference;
    m_gotoButton.setState(ui::Widget::DisabledState, !m_reference.isSet());
}

void
ClassicVcrObjectDialog::onHullUpdate(const VcrDatabaseProxy::HullInfo& info)
{
    // ex WVcrInfo::onMove
    m_image.setImage(info.imageName);
    if (const game::vcr::PlanetInfo* pi = info.planetInfo.get()) {
        m_info.setPlanetInfo(*pi);
    } else if (const game::vcr::ShipInfo* si = info.shipInfo.get()) {
        m_info.setShipInfo(*si);
    } else {
        m_info.clear();
    }

    m_shipQuery = info.shipQuery;
    m_specButton.setState(ui::Widget::DisabledState, !m_shipQuery.isValid());
}

void
ClassicVcrObjectDialog::onTab()
{
    m_reference = game::Reference();
    setSide(m_side ^ 1);
}

void
ClassicVcrObjectDialog::onListScroll()
{
    int32_t n;
    if (m_hullList.getCurrentKey(n)) {
        m_proxy.setHullType(n);
    }
}

void
ClassicVcrObjectDialog::onGoTo()
{
    if (m_reference.isSet()) {
        m_loop.stop(1);
    }
}

void
ClassicVcrObjectDialog::onHullSpecification()
{
    if (const game::ShipQuery* q = m_shipQuery.get()) {
        client::dialogs::showHullSpecification(*q, m_root, m_translator, m_gameSender);
    }
}

bool
ClassicVcrObjectDialog::handleKey(util::Key_t key, int /*prefix*/)
{
    // ex WVcrInfo::handleEvent(const UIEvent& event, bool second_pass)
    switch (key) {
     case util::Key_Tab:
     case util::Key_Tab + util::KeyMod_Shift:
        onTab();
        return true;

     case util::Key_Left:
        setSide(0);
        return true;

     case util::Key_Right:
        setSide(1);
        return true;

     case util::Key_Insert:
        addToSimulation(true);
        return true;

     case util::Key_Insert + util::KeyMod_Shift:
        addToSimulation(false);
        return true;

     default:
        return false;
    }
}

void
ClassicVcrObjectDialog::setSide(size_t side)
{
    if (side != m_side) {
        m_side = side;
        m_proxy.setSide(m_side, true);
    }
}

void
ClassicVcrObjectDialog::addToSimulation(bool after)
{
    // ex WVcrInfoMain::addToSim
    // Proxy needs the hull type
    int32_t n;
    if (!m_hullList.getCurrentKey(n)) {
        return;
    }

    client::dialogs::addToSimulation(m_indicator, m_proxy, n, after, m_root, m_translator);
}

game::Reference
ClassicVcrObjectDialog::getReference() const
{
    return m_reference;
}



/*
 *  Entry Points
 */

game::Reference
client::dialogs::doClassicVcrObjectInfoDialog(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender, game::proxy::VcrDatabaseProxy& proxy, size_t side)
{
    // ex doVcrInfo
    game::proxy::ConfigurationProxy configProxy(gameSender);
    Downlink link(root, tx);

    ClassicVcrObjectDialog dlg(root, configProxy.getNumberFormatter(link), tx, gameSender, proxy, side, link);
    bool ok = dlg.run(root);
    return ok ? dlg.getReference() : game::Reference();
}

void
client::dialogs::addToSimulation(game::proxy::WaitIndicator& ind,
                                 game::proxy::VcrDatabaseProxy& proxy,
                                 int hullNr,
                                 bool after,
                                 ui::Root& root,
                                 afl::string::Translator& tx)
{
    VcrDatabaseProxy::AddResult result = proxy.addToSimulation(ind, hullNr, after);
    String_t message;
    switch (result) {
     case VcrDatabaseProxy::Success:
        break;
     case VcrDatabaseProxy::Error:
        message = tx("Unit cannot be added to simulation.");
        break;
     case VcrDatabaseProxy::NotPlayable:
        message = tx("This fight could not be played. You can use [Shift+Ins] to use this unit's status before the fight for simulation.");
        break;
     case VcrDatabaseProxy::NotParseable:
        message = tx("This unit cannot be added to simulation because PCC2 cannot interpret its data correctly.");
        break;
     case VcrDatabaseProxy::UnitDied:
        message = tx("This unit did not survive the fight. You can use [Shift+Ins] to use this unit's status before the fight for simulation.");
        break;
    }
    if (!message.empty()) {
        ui::dialogs::MessageBox(message, tx("Add to Simulation"), root).doOkDialog(tx);
    }
}
