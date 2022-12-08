/**
  *  \file client/dialogs/ionstorminfo.cpp
  *  \brief Ion Storm Information Dialog
  */

#include <cmath>
#include "client/dialogs/ionstorminfo.hpp"
#include "afl/base/deleter.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/ionstormforecast.hpp"
#include "client/map/circleoverlay.hpp"
#include "client/map/widget.hpp"
#include "client/si/control.hpp"
#include "client/tiles/selectionheadertile.hpp"
#include "client/widgets/helpwidget.hpp"
#include "game/proxy/ionstormproxy.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/invisiblewidget.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/keyforwarder.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/simpletable.hpp"
#include "ui/window.hpp"
#include "util/math.hpp"
#include "util/string.hpp"

using game::proxy::IonStormProxy;

namespace {
    const int NUM_LINES = 5;

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

    class IonStormInfoDialog : public client::si::Control, public gfx::KeyEventConsumer {
     public:
        IonStormInfoDialog(client::si::UserSide& iface, ui::Root& root, afl::string::Translator& tx, client::si::OutputState& out);

        void run();
        bool handleKey(util::Key_t key, int prefix);

        virtual void handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target target);
        virtual void handleEndDialog(client::si::RequestLink2 link, int code);
        virtual void handlePopupConsole(client::si::RequestLink2 link);
        virtual void handleScanKeyboardMode(client::si::RequestLink2 link);
        virtual void handleSetView(client::si::RequestLink2 link, String_t name, bool withKeymap);
        virtual void handleUseKeymap(client::si::RequestLink2 link, String_t name, int prefix);
        virtual void handleOverlayMessage(client::si::RequestLink2 link, String_t text);
        virtual game::interface::ContextProvider* createContextProvider();

     private:
        client::si::UserSide& m_userSide;
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        client::si::OutputState& m_outputState;
        IonStormProxy m_proxy;
        ui::EventLoop m_loop;

        // Content widgets
        client::map::Widget m_mapWidget;
        client::map::CircleOverlay m_mapOverlay;
        ui::widgets::SimpleTable m_infoTable;

        // Status cache
        IonStormProxy::IonStormInfo m_info;

        void initLabels();

        void onStormChange(const IonStormProxy::IonStormInfo& info);
        void onGoto();
        void onForecast();
    };
}


/*
 *  IonStormInfoDialog
 */

IonStormInfoDialog::IonStormInfoDialog(client::si::UserSide& iface,
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
      m_info()
{
    m_proxy.sig_stormChange.add(this, &IonStormInfoDialog::onStormChange);
    initLabels();
}

void
IonStormInfoDialog::run()
{
    // ex WIonStormInfoWindow::WIonStormInfoWindow
    // VBox
    //   SelectionHeaderTile
    //   HBox
    //     SimpleTable (IonStorm Info)
    //     map::Widget
    //   HBox
    //     Buttons: Help || Forecast, Goto, Close
    afl::base::Deleter del;

    ui::Window& win = del.addNew(new ui::Window(m_translator("Ion Storm Information"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    ui::Widget& keys = del.addNew(new ui::widgets::KeyForwarder(*this));

    // Header
    client::tiles::SelectionHeaderTile& header = del.addNew(new client::tiles::SelectionHeaderTile(m_root, keys));
    header.attach(m_proxy);
    win.add(header);

    // Content
    ui::Group& g2 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    g2.add(m_infoTable);
    g2.add(ui::widgets::FrameGroup::wrapWidget(del, m_root.colorScheme(), ui::LoweredFrame, m_mapWidget));
    win.add(g2);

    // Buttons
    ui::widgets::Button& btnClose    = del.addNew(new ui::widgets::Button(m_translator("Close"),  util::Key_Escape, m_root));
    ui::widgets::Button& btnGoto     = del.addNew(new ui::widgets::Button(m_translator("Go to"),  util::Key_Return, m_root));
    ui::widgets::Button& btnForecast = del.addNew(new ui::widgets::Button(m_translator("F - Forecast"), 'f', m_root));
    ui::widgets::Button& btnHelp   = del.addNew(new ui::widgets::Button(m_translator("Help"), 'h', m_root));
    ui::Group& buttonGroup = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    buttonGroup.add(btnHelp);
    buttonGroup.add(del.addNew(new ui::Spacer()));
    buttonGroup.add(btnForecast);
    buttonGroup.add(btnGoto);
    buttonGroup.add(btnClose);
    win.add(buttonGroup);
    win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));
    win.add(keys);

    client::widgets::HelpWidget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, m_userSide.gameSender(), "pcc2:ionscreen"));
    win.add(help);

    btnClose.sig_fire.addNewClosure(m_loop.makeStop(0));
    btnGoto.sig_fire.add(this, &IonStormInfoDialog::onGoto);
    btnForecast.sig_fire.add(this, &IonStormInfoDialog::onForecast);
    btnHelp.dispatchKeyTo(help);

    win.pack();

    m_root.centerWidget(win);
    m_root.add(win);
    m_loop.run();
}

bool
IonStormInfoDialog::handleKey(util::Key_t key, int /*prefix*/)
{
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
IonStormInfoDialog::handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target target)
{
    dialogHandleStateChange(link, target, m_outputState, m_loop, 0);
}

void
IonStormInfoDialog::handleEndDialog(client::si::RequestLink2 link, int code)
{
    dialogHandleEndDialog(link, code, m_outputState, m_loop, 0);
}

void
IonStormInfoDialog::handlePopupConsole(client::si::RequestLink2 link)
{
    defaultHandlePopupConsole(link);
}

void
IonStormInfoDialog::handleScanKeyboardMode(client::si::RequestLink2 link)
{
    defaultHandleScanKeyboardMode(link);
}

void
IonStormInfoDialog::handleSetView(client::si::RequestLink2 link, String_t name, bool withKeymap)
{
    defaultHandleSetView(link, name, withKeymap);
}

void
IonStormInfoDialog::handleUseKeymap(client::si::RequestLink2 link, String_t name, int prefix)
{
    defaultHandleUseKeymap(link, name, prefix);
}

void
IonStormInfoDialog::handleOverlayMessage(client::si::RequestLink2 link, String_t text)
{
    defaultHandleOverlayMessage(link, text);
}

game::interface::ContextProvider*
IonStormInfoDialog::createContextProvider()
{
    return 0;
}

void
IonStormInfoDialog::initLabels()
{
    afl::string::Translator& tx = m_translator;
    const int em = m_root.provider().getFont(gfx::FontRequest())->getEmWidth();

    // PCC2 uses a 9 em/16 em split.
    // We can easily have the labels computed automatically, so we only set the right column.
    const int RIGHT = 16*em;

    // ex WIonStormInfoTile::drawData (part), CIonWindow.DrawInterior
    m_infoTable.all().setColor(ui::Color_Black);
    m_infoTable.cell(0, 0).setText(tx("Centered at:"));
    m_infoTable.cell(0, 1).setText(tx("Radius:"));
    m_infoTable.cell(0, 3).setText(tx("Movement:"));
    m_infoTable.cell(0, 4).setText(tx("Voltage:"));
    m_infoTable.setColumnWidth(1, RIGHT);
    m_infoTable.setColumnPadding(0, 5);

    // ex WObjectSelectionChartWidget::drawData (sort-of)
    m_mapWidget.addOverlay(m_mapOverlay);
    m_mapOverlay.setColor(ui::Color_Gray);
}

void
IonStormInfoDialog::onStormChange(const IonStormProxy::IonStormInfo& info)
{
    // If .stormId is zero, no more storms remain
    if (info.stormId == 0) {
        m_loop.stop(0);
    } else {
        // ex WIonStormInfoTile::drawData (part), CIonWindow.DrawData
        m_infoTable.cell(1, 0).setText(info.center.toString());
        m_infoTable.cell(1, 1).setText(info.text[IonStormProxy::Radius]);

        {
            String_t s = info.text[IonStormProxy::Heading];
            util::addListItem(s, ", ", info.text[IonStormProxy::Speed]);
            m_infoTable.cell(1, 2).setText(s);
        }
        {
            String_t s = info.text[IonStormProxy::Voltage];
            util::addListItem(s, ", ", info.text[IonStormProxy::Status]);
            m_infoTable.cell(1, 3).setText(s);
        }
        m_infoTable.cell(1, 4).setText(info.text[IonStormProxy::ClassName]);

        m_mapWidget.setCenter(info.center);
        m_mapWidget.setZoom(1, getReductionFactor(m_mapWidget.getExtent(), info.radius));
        m_mapOverlay.setPosition(info.center, info.radius);

        // Save state for use by user input
        m_info = info;
    }
}

void
IonStormInfoDialog::onGoto()
{
    if (m_info.center.getX() != 0) {
        executeGoToReferenceWait("(Ion Storm)", m_info.center);
    }
}

void
IonStormInfoDialog::onForecast()
{
    client::dialogs::doIonStormForecastDialog(m_root, interface().gameSender(), m_translator, m_info);
}


/*
 *  Main Entry Point
 */

void
client::dialogs::doIonStormInfoDialog(client::si::UserSide& iface,
                                       ui::Root& root,
                                       afl::string::Translator& tx,
                                       client::si::OutputState& out)
{
    IonStormInfoDialog(iface, root, tx, out)
        .run();
}
