/**
  *  \file client/dialogs/ufoinfo.cpp
  *  \brief Ufo information dialog
  */

#include "client/dialogs/ufoinfo.hpp"
#include "afl/base/deleter.hpp"
#include "afl/string/format.hpp"
#include "client/map/circleoverlay.hpp"
#include "client/map/renderer.hpp"
#include "client/map/widget.hpp"
#include "client/si/control.hpp"
#include "client/tiles/selectionheadertile.hpp"
#include "client/widgets/helpwidget.hpp"
#include "game/proxy/ufoproxy.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/icons/colortile.hpp"
#include "ui/invisiblewidget.hpp"
#include "ui/layout/grid.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/basebutton.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/keyforwarder.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/simpletable.hpp"
#include "ui/window.hpp"
#include "util/math.hpp"

using afl::string::Format;
using game::proxy::UfoProxy;

namespace {
    const int NUM_LINES = 9;

    gfx::Point getPreferredMapSize(ui::Root& root)
    {
        int px = root.provider().getFont(gfx::FontRequest())->getLineHeight() * (NUM_LINES + 2);
        return gfx::Point(px, px);
    }

    int getReductionFactor(const gfx::Rectangle& area, int radius)
    {
        int screenRadius = std::min(area.getWidth(), area.getHeight()) / 2;
        if (screenRadius > 0) {
            return util::divideAndRoundUp(radius, screenRadius);
        } else {
            return 1;
        }
    }

    /*
     *  UfoInfoDialog - dialog main class
     */
    class UfoInfoDialog : public client::si::Control, public gfx::KeyEventConsumer {
     public:
        UfoInfoDialog(client::si::UserSide& iface, ui::Root& root, afl::string::Translator& tx, client::si::OutputState& out);

        void run();
        bool handleKey(util::Key_t key, int prefix);

        virtual void handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target target);
        virtual void handleEndDialog(client::si::RequestLink2 link, int code);
        virtual void handlePopupConsole(client::si::RequestLink2 link);
        virtual void handleScanKeyboardMode(client::si::RequestLink2 link);
        virtual void handleSetView(client::si::RequestLink2 link, String_t name, bool withKeymap);
        virtual void handleUseKeymap(client::si::RequestLink2 link, String_t name, int prefix);
        virtual void handleOverlayMessage(client::si::RequestLink2 link, String_t text);
        virtual client::si::ContextProvider* createContextProvider();

     private:
        client::si::UserSide& m_userSide;
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        client::si::OutputState& m_outputState;
        UfoProxy m_proxy;
        ui::EventLoop m_loop;

        // Content widgets
        client::map::Widget m_mapWidget;
        client::map::CircleOverlay m_mapOverlay;
        ui::widgets::SimpleTable m_infoTable;
        ui::widgets::SimpleTable m_configTable;
        ui::widgets::Button m_keepButton;
        ui::widgets::Button m_otherButton;
        ui::icons::ColorTile m_colorTile;
        ui::widgets::BaseButton m_colorButton;

        // Status cache
        game::map::Point m_ufoCenter;

        void initWidgets();

        void onUfoChange(const UfoProxy::UfoInfo& info);
        void onGoto();
    };
}


/*
 *  UfoInfoDialog
 */

UfoInfoDialog::UfoInfoDialog(client::si::UserSide& iface,
                             ui::Root& root,
                             afl::string::Translator& tx,
                             client::si::OutputState& out)
    : Control(iface),
      m_userSide(iface),
      m_root(root),
      m_translator(tx),
      m_outputState(out),
      m_proxy(root.engine().dispatcher(), iface.gameSender()),
      m_loop(root),
      m_mapWidget(iface.gameSender(), root, getPreferredMapSize(root)),
      m_mapOverlay(root.colorScheme()),
      m_infoTable(root, 2, NUM_LINES),
      m_configTable(root, 2, 2),
      m_keepButton("K", 'k', root),
      m_otherButton("X", 'x', root),
      m_colorTile(root, gfx::Point(10, 10), ui::Color_Gray),
      m_colorButton(root, 0),
      m_ufoCenter()
{
    m_proxy.sig_ufoChange.add(this, &UfoInfoDialog::onUfoChange);
    initWidgets();
}

void
UfoInfoDialog::run()
{
    // ex WUfoInfoWindow::init
    // VBox
    //   SelectionHeaderTile
    //   HBox
    //     Grid
    //       Color
    //       SimpleTable (Info)
    //       VBox: K, X
    //       SimpleTable (Config)
    //     map::Widget
    //   HBox
    //     Buttons: Help || Goto, Close
    afl::base::Deleter del;

    ui::Window& win = del.addNew(new ui::Window(m_translator("Ufo Information"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    ui::Widget& keys = del.addNew(new ui::widgets::KeyForwarder(*this));

    // Header
    client::tiles::SelectionHeaderTile& header = del.addNew(new client::tiles::SelectionHeaderTile(m_root, keys));
    header.attach(m_proxy);
    win.add(header);

    // Content
    ui::Group& contentGroup = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::Group& textGroup = del.addNew(new ui::Group(del.addNew(new ui::layout::Grid(2))));
    ui::Group& ctlGroup = del.addNew(new ui::Group(del.addNew(new ui::layout::VBox(1))));
    ctlGroup.add(m_keepButton);
    ctlGroup.add(m_otherButton);
    textGroup.add(m_colorButton);
    textGroup.add(m_infoTable);
    textGroup.add(ctlGroup);
    textGroup.add(m_configTable);

    contentGroup.add(textGroup);
    contentGroup.add(ui::widgets::FrameGroup::wrapWidget(del, m_root.colorScheme(), ui::LoweredFrame, m_mapWidget));
    win.add(contentGroup);

    // Buttons
    ui::widgets::Button& btnHelp   = del.addNew(new ui::widgets::Button(m_translator("Help"), 'h', m_root));
    ui::widgets::Button& btnGoto   = del.addNew(new ui::widgets::Button(m_translator("Go to"),  util::Key_Return, m_root));
    ui::widgets::Button& btnClose  = del.addNew(new ui::widgets::Button(m_translator("Close"),  util::Key_Escape, m_root));
    ui::Group& buttonGroup = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    buttonGroup.add(btnHelp);
    buttonGroup.add(del.addNew(new ui::Spacer()));
    buttonGroup.add(btnGoto);
    buttonGroup.add(btnClose);
    win.add(buttonGroup);
    win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));
    win.add(keys);

    client::widgets::HelpWidget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, m_userSide.gameSender(), "pcc2:ufoscreen"));
    win.add(help);

    btnClose.sig_fire.addNewClosure(m_loop.makeStop(0));
    btnGoto.sig_fire.add(this, &UfoInfoDialog::onGoto);
    btnHelp.dispatchKeyTo(help);

    win.pack();

    m_root.centerWidget(win);
    m_root.add(win);
    m_loop.run();
}

bool
UfoInfoDialog::handleKey(util::Key_t key, int /*prefix*/)
{
    // FIXME: this key dispatch appears way too often
    using game::map::ObjectCursor;
    switch (key) {
     case util::Key_PgUp:
     case util::Key_Up:
     case util::Key_WheelUp:
        m_proxy.browse(ObjectCursor::Previous, false);
        return true;

     case util::KeyMod_Ctrl + util::Key_PgUp:
     case util::KeyMod_Ctrl + util::Key_Up:
     case util::KeyMod_Ctrl + util::Key_WheelUp:
        m_proxy.browse(ObjectCursor::Previous, true);
        return true;

     case util::Key_PgDn:
     case util::Key_Down:
     case util::Key_WheelDown:
        m_proxy.browse(ObjectCursor::Next, false);
        return true;

     case util::KeyMod_Ctrl + util::Key_PgDn:
     case util::KeyMod_Ctrl + util::Key_Down:
     case util::KeyMod_Ctrl + util::Key_WheelDown:
        m_proxy.browse(ObjectCursor::Next, true);
        return true;

     case util::Key_Home:
        m_proxy.browse(ObjectCursor::First, false);
        return true;

     case util::KeyMod_Ctrl + util::Key_Home:
        m_proxy.browse(ObjectCursor::First, true);
        return true;

     case util::Key_End:
        m_proxy.browse(ObjectCursor::Last, false);
        return true;

     case util::Key_Tab:
        m_proxy.browse(ObjectCursor::NextHere, false);
        return true;

     case util::Key_Tab + util::KeyMod_Shift:
        m_proxy.browse(ObjectCursor::PreviousHere, false);
        return true;

     case util::KeyMod_Ctrl + util::Key_End:
        m_proxy.browse(ObjectCursor::Last, true);
        return true;

     default:
        return false;
    }
}

void
UfoInfoDialog::handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target target)
{
    dialogHandleStateChange(link, target, m_outputState, m_loop, 0);
}

void
UfoInfoDialog::handleEndDialog(client::si::RequestLink2 link, int code)
{
    dialogHandleEndDialog(link, code, m_outputState, m_loop, 0);
}

void
UfoInfoDialog::handlePopupConsole(client::si::RequestLink2 link)
{
    defaultHandlePopupConsole(link);
}

void
UfoInfoDialog::handleScanKeyboardMode(client::si::RequestLink2 link)
{
    defaultHandleScanKeyboardMode(link);
}

void
UfoInfoDialog::handleSetView(client::si::RequestLink2 link, String_t name, bool withKeymap)
{
    defaultHandleSetView(link, name, withKeymap);
}

void
UfoInfoDialog::handleUseKeymap(client::si::RequestLink2 link, String_t name, int prefix)
{
    defaultHandleUseKeymap(link, name, prefix);
}

void
UfoInfoDialog::handleOverlayMessage(client::si::RequestLink2 link, String_t text)
{
    defaultHandleOverlayMessage(link, text);
}

client::si::ContextProvider*
UfoInfoDialog::createContextProvider()
{
    return 0;
}

void
UfoInfoDialog::initWidgets()
{
    afl::string::Translator& tx = m_translator;
    const int em = m_root.provider().getFont(gfx::FontRequest())->getEmWidth();

    // The 12 em/16 em split is also in PCC2, and makes sure the widgets are lined up.
    const int LEFT = 12*em;
    const int RIGHT = 16*em;

    // ex WUfoInfoTile::drawData (part)
    m_infoTable.all().setColor(ui::Color_Black);
    m_infoTable.cell(0, 0).setExtraColumns(1);
    m_infoTable.cell(0, 1).setExtraColumns(1);
    m_infoTable.cell(0, 2).setText(tx("Location:"));
    m_infoTable.cell(0, 3).setText(tx("Radius:"));
    m_infoTable.cell(0, 4).setText(tx("Speed:"));
    m_infoTable.cell(0, 5).setText(tx("Heading:"));
    m_infoTable.cell(0, 6).setText(tx("Visible at:"));
    m_infoTable.cell(0, 8).setText(tx("Last info:"));
    m_infoTable.setColumnWidth(0, LEFT);
    m_infoTable.setColumnWidth(1, RIGHT);

    // ex WUfoSettingsTile::drawData (part)
    m_configTable.all().setColor(ui::Color_Black);
    m_configTable.cell(0, 0).setText(tx("Keep:"));
    m_configTable.cell(0, 1).setText(tx("Other end:"));
    m_configTable.setColumnWidth(0, LEFT);
    m_configTable.setColumnWidth(1, RIGHT);

    // ex WObjectSelectionChartWidget::drawData (sort-of)
    m_mapWidget.addOverlay(m_mapOverlay);
    m_mapOverlay.setColor(ui::Color_Gray);

    // Buttons
    m_keepButton.setFont("-");
    m_keepButton.sig_fire.add(&m_proxy, &UfoProxy::toggleStoredInHistory);
    m_otherButton.setFont("-");
    m_otherButton.setState(ui::Widget::DisabledState, true);
    m_otherButton.sig_fire.add(&m_proxy, &UfoProxy::browseToOtherEnd);

    // Color
    m_colorTile.setFrameType(ui::LoweredFrame);
    m_colorButton.setIcon(m_colorTile);
    m_colorButton.setGrowthBehaviour(ui::layout::Info::GrowBoth);
}

void
UfoInfoDialog::onUfoChange(const UfoProxy::UfoInfo& info)
{
    // If .ufoId is zero, no more ufos remain
    if (info.ufoId == 0) {
        m_loop.stop(0);
    } else {
        // ex WUfoInfoTile::drawData (part)
        afl::string::Translator& tx = m_translator;
        m_infoTable.cell(0, 0).setText(info.text[UfoProxy::Info1]);
        m_infoTable.cell(0, 1).setText(info.text[UfoProxy::Info2]);
        m_infoTable.cell(1, 2).setText(info.center.toString());
        m_infoTable.cell(1, 3).setText(info.text[UfoProxy::Radius]);
        m_infoTable.cell(1, 4).setText(info.text[UfoProxy::Speed]);
        m_infoTable.cell(1, 5).setText(info.text[UfoProxy::Heading]);
        m_infoTable.cell(1, 6).setText(info.text[UfoProxy::PlanetRange] + tx(" (from planet)"));
        m_infoTable.cell(1, 7).setText(info.text[UfoProxy::ShipRange]   + tx(" (from ship)"));
        m_infoTable.cell(1, 8).setText(info.text[UfoProxy::LastInfo]);

        m_configTable.cell(1, 0).setText(info.isStoredInHistory ? tx("yes") : tx("no"));
        m_configTable.cell(1, 1).setText(info.text[UfoProxy::OtherEndName]);

        m_otherButton.setState(ui::Widget::DisabledState, !info.hasOtherEnd);

        m_mapWidget.setCenter(info.center);
        m_mapWidget.setZoom(1, getReductionFactor(m_mapWidget.getExtent(), info.radius));
        m_mapOverlay.setPosition(info.center, info.radius);

        if (m_colorTile.setColor(client::map::getUfoColor(info.colorCode))) {
            m_colorButton.requestRedraw();
        }

        // Save state for use by user input
        m_ufoCenter = info.center;
    }
}


void
UfoInfoDialog::onGoto()
{
    if (m_ufoCenter.getX() != 0) {
        executeGoToReferenceWait("(Ufo)", m_ufoCenter);
    }
}


/*
 *  Main Entry Point
 */

void
client::dialogs::doUfoInfoDialog(client::si::UserSide& iface,
                                 ui::Root& root,
                                 afl::string::Translator& tx,
                                 client::si::OutputState& out)
{
    UfoInfoDialog(iface, root, tx, out)
        .run();
}
