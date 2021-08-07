/**
  *  \file client/dialogs/ionstormforecast.cpp
  *  \brief Ion Storm Forecast Dialog
  */

#include <cmath>
#include "client/dialogs/ionstormforecast.hpp"
#include "client/map/ionstormforecastoverlay.hpp"
#include "client/map/movementoverlay.hpp"
#include "client/map/scanneroverlay.hpp"
#include "client/map/widget.hpp"
#include "client/widgets/helpwidget.hpp"
#include "client/widgets/scanresult.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/prefixargument.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/window.hpp"
#include "util/math.hpp"

namespace {
    class IonStormForecastDialog {
     public:
        IonStormForecastDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx);

        void setOrigin(game::map::Point pt);
        void setForecast(int voltage, const game::map::IonStorm::Forecast_t& pred);
        void setPositions();
        void onMove(game::map::Point pt);
        void onTab();

        void run();

     private:
        ui::Root& m_root;
        ui::EventLoop m_loop;
        util::RequestSender<game::Session> m_gameSender;
        afl::string::Translator& m_translator;

        client::map::Widget m_mapWidget;
        client::map::ScannerOverlay m_scannerOverlay;
        client::map::MovementOverlay m_movementOverlay;
        client::map::IonStormForecastOverlay m_forecastOverlay;
        client::widgets::ScanResult m_scanResult;

        game::map::Point m_center;
        game::map::Point m_origin;
        game::map::Point m_target;
    };
}

IonStormForecastDialog::IonStormForecastDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx)
    : m_root(root),
      m_loop(root),
      m_gameSender(gameSender),
      m_translator(tx),
      m_mapWidget(gameSender, root, gfx::Point(450, 450)),  // FIXME: size
      m_scannerOverlay(root.colorScheme()),
      m_movementOverlay(root.engine().dispatcher(), gameSender, m_mapWidget, tx),
      m_forecastOverlay(root.colorScheme()),
      m_scanResult(root, gameSender, tx)
{ }

void
IonStormForecastDialog::setOrigin(game::map::Point pt)
{
    m_center = m_origin = m_target = pt;
    setPositions();
}

void
IonStormForecastDialog::setForecast(int voltage, const game::map::IonStorm::Forecast_t& pred)
{
    m_forecastOverlay.setForecast(voltage, pred);
}

void
IonStormForecastDialog::setPositions()
{
    m_mapWidget.setCenter(m_center);
    m_movementOverlay.setPosition(m_target);
    m_movementOverlay.setLockOrigin(m_origin, false);
    m_scannerOverlay.setPositions(m_origin, m_target);
    m_scanResult.setPositions(m_origin, m_target);
}

void
IonStormForecastDialog::onMove(game::map::Point pt)
{
    m_target = pt;
    setPositions();
}

void
IonStormForecastDialog::onTab()
{
    m_center = m_target;
    setPositions();
}

void
IonStormForecastDialog::run()
{
    afl::string::Translator& tx = m_translator;
    afl::base::Deleter del;

    // Build window. Modeled largely after NavChartDialog.
    // VBox
    //   UIFrameGroup
    //     WIonForecastChart
    //   HBox
    //     WScanResult
    //     VBox
    //       UISpacer
    //       HBox
    //         "OK"
    //         "Help"
    // FIXME: needs to be a BLUE_DARK_WINDOW because ScanResult is not currently skinnable
    ui::Window& win = del.addNew(new ui::Window(tx("Ion Storm Forecast"), m_root.provider(), m_root.colorScheme(), ui::BLUE_DARK_WINDOW, ui::layout::VBox::instance5));

    ui::Group& g2   = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::Group& g22  = del.addNew(new ui::Group(ui::layout::VBox::instance5));
    ui::Group& g222 = del.addNew(new ui::Group(ui::layout::HBox::instance5));

    // Map widget
    m_movementOverlay.setMode(client::map::MovementOverlay::AcceptMovementKeys, true);
    m_movementOverlay.setMode(client::map::MovementOverlay::AcceptConfigKeys, true);
    m_movementOverlay.setMode(client::map::MovementOverlay::AcceptZoomKeys, true);
    m_movementOverlay.sig_move.add(this, &IonStormForecastDialog::onMove);
    m_mapWidget.addOverlay(m_forecastOverlay);
    m_mapWidget.addOverlay(m_movementOverlay);
    m_mapWidget.addOverlay(m_scannerOverlay);
    m_mapWidget.setZoom(1, 2);         // PCC 1.x lets initial zoom always start at 1:2, which seems to work well.
    win.add(ui::widgets::FrameGroup::wrapWidget(del, m_root.colorScheme(), ui::LoweredFrame, m_mapWidget));

    g2.add(m_scanResult);
    g2.add(g22);
    g22.add(del.addNew(new ui::Spacer()));
    g22.add(g222);

    // Buttons
    ui::Widget& helper = del.addNew(new client::widgets::HelpWidget(m_root, tx, m_gameSender, "pcc2:ionforecast"));
    ui::widgets::Button& btnClose = del.addNew(new ui::widgets::Button(m_translator("Close"), util::Key_Escape, m_root));
    ui::widgets::Button& btnHelp = del.addNew(new ui::widgets::Button(m_translator("H"), 'h', m_root));
    g222.add(del.addNew(new ui::Spacer()));
    g222.add(btnClose);
    g222.add(btnHelp);

    btnClose.sig_fire.addNewClosure(m_loop.makeStop(0));
    btnHelp.dispatchKeyTo(helper);

    win.add(g2);
    win.add(helper);
    win.add(del.addNew(new ui::PrefixArgument(m_root)));
    win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));

    // Extra keys
    ui::widgets::KeyDispatcher& disp = del.addNew(new ui::widgets::KeyDispatcher());
    disp.add(util::Key_Tab, this, &IonStormForecastDialog::onTab);
    win.add(disp);

    int size = m_root.getExtent().getHeight();
    win.setExtent(gfx::Rectangle(0, 0, size - 50, size - 10));

    m_root.centerWidget(win);
    m_root.add(win);
    m_loop.run();
}


void
client::dialogs::doIonStormForecastDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx, const game::proxy::IonStormProxy::IonStormInfo& info)
{
    // ex doIonStormPrediction
    if (!info.forecast.empty()) {
        // Compute warp arrow as map center
        const int offset = util::squareInteger(info.speed) + info.radius;
        const double angle = info.heading * (util::PI/180.0);
        const game::map::Point origin = info.center +
            game::map::Point(util::roundToInt(std::sin(angle) * offset),
                             util::roundToInt(std::cos(angle) * offset));

        // Build dialog
        IonStormForecastDialog dlg(root, gameSender, tx);
        dlg.setOrigin(origin);
        dlg.setForecast(info.voltage, info.forecast);
        dlg.run();
    }
}
