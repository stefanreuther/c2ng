/**
  *  \file client/dialogs/shipspeeddialog.cpp
  */

#include "client/dialogs/shipspeeddialog.hpp"
#include "afl/base/deleter.hpp"
#include "client/downlink.hpp"
#include "client/proxy/shipspeedproxy.hpp"
#include "client/widgets/shipspeedwidget.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/rich/statictext.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/window.hpp"
#include "util/translation.hpp"

namespace {
    using client::proxy::ShipSpeedProxy;
    
    class Window {
     public:
        Window(ShipSpeedProxy& proxy, ui::Root& root, const ShipSpeedProxy::Status& st)
            : m_proxy(proxy),
              m_root(root),
              m_loop(root),
              m_value(st.currentSpeed),
              m_originalStatus(st)
            {
                m_value.sig_change.add(this, &Window::onChange);
            }

        bool run()
            {
                // ex WShipSpeedSelector::doStandardDialog
                // Window [VBox]
                //   MultilineStatic
                // HBox
                //   ShipSpeedWidget with buttons
                //   Spacer
                //   Button "OK"
                //   Button "Cancel"
                afl::base::Deleter del;
                ui::Window& win = del.addNew(new ui::Window(_("Set Speed"),
                                                            m_root.provider(),
                                                            m_root.colorScheme(),
                                                            ui::BLUE_WINDOW,
                                                            ui::layout::VBox::instance5));
                win.add(del.addNew(new ui::rich::StaticText(_("Enter new warp factor and confirm with ENTER:"),
                                                            280,    // FIXME?
                                                            m_root.provider())));

                ui::Group& hg = del.addNew(new ui::Group(ui::layout::HBox::instance5));

                client::widgets::ShipSpeedWidget& ssw = del.addNew(new client::widgets::ShipSpeedWidget(m_value, m_originalStatus.maxSpeed, m_originalStatus.hyperSpeedMarker, m_root));
                hg.add(ui::widgets::FrameGroup::wrapWidget(del, m_root.colorScheme(),
                                                           ui::widgets::FrameGroup::LoweredFrame,
                                                           ssw.addButtons(del, m_root)));
                hg.add(del.addNew(new ui::Spacer()));

                ui::widgets::Button& btnOK     = del.addNew(new ui::widgets::Button(_("OK"),     util::Key_Return, m_root));
                ui::widgets::Button& btnCancel = del.addNew(new ui::widgets::Button(_("Cancel"), util::Key_Escape, m_root));
                hg.add(btnOK);
                hg.add(btnCancel);
                win.add(hg);

                btnOK.sig_fire.addNewClosure(m_loop.makeStop(1));
                btnCancel.sig_fire.add(this, &Window::onCancel);

                ssw.requestFocus();
                win.pack();

                // @change/FIXME: lower right, not lower left
                m_root.moveWidgetToEdge(win, 2, 2, 10);
                m_root.add(win);
                return m_loop.run() != 0;
            }

        void onCancel()
            {
                m_proxy.setSpeed(m_originalStatus.currentSpeed);
                m_loop.stop(0);
            }

        void onChange()
            {
                m_proxy.setSpeed(m_value.get());
            }

     private:
        ShipSpeedProxy& m_proxy;
        ui::Root& m_root;
        ui::EventLoop m_loop;
        afl::base::Observable<int32_t> m_value;
        ShipSpeedProxy::Status m_originalStatus;
    };
}


void
client::dialogs::doShipSpeedDialog(game::Id_t shipId, ui::Root& root, util::RequestSender<game::Session> gameSender)
{
    ShipSpeedProxy proxy(gameSender, shipId);
    Downlink link(root);

    // Initialize
    ShipSpeedProxy::Status st = proxy.getStatus(link);

    // Limit=0 means preconditions failed
    if (st.maxSpeed == 0) {
        return;
    }

    // Dialog
    Window dlg(proxy, root, st);
    dlg.run();
}
