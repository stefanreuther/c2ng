/**
  *  \file client/dialogs/buildstarbasedialog.cpp
  */

#include "client/dialogs/buildstarbasedialog.hpp"
#include "afl/base/deleter.hpp"
#include "client/proxy/buildstarbaseproxy.hpp"
#include "client/proxy/configurationproxy.hpp"
#include "client/widgets/costdisplay.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"
#include "util/keystring.hpp"

using client::proxy::ConfigurationProxy;
using client::proxy::BuildStarbaseProxy;
using client::widgets::CostDisplay;
using game::spec::Cost;
using ui::widgets::Button;

namespace {
    bool showStarbaseBuildOrder(bool success,
                                ui::Root& root,
                                afl::string::Translator& tx,
                                const BuildStarbaseProxy::Status& st,
                                util::NumberFormatter fmt)
    {
        afl::base::Deleter del;

        // Window
        ui::Window& win = del.addNew(new ui::Window(tx("Build Starbase"), root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
        ui::EventLoop loop(root);

        // Cost display
        CostDisplay& dpy = del.addNew(new CostDisplay(root,
                                                      CostDisplay::Types_t() + Cost::Tritanium + Cost::Duranium + Cost::Molybdenum + Cost::Money + Cost::Supplies,
                                                      fmt));
        dpy.setCost(st.cost);
        dpy.setAvailableAmount(st.available);
        dpy.setRemainingAmount(st.remaining);
        dpy.setMissingAmount(st.missing);
        win.add(dpy);

        // Message/buttons
        ui::widgets::KeyDispatcher& disp = del.addNew(new ui::widgets::KeyDispatcher());
        ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));
        g.add(del.addNew(new ui::Spacer()));
        if (success) {
            win.add(del.addNew(new ui::widgets::StaticText(tx("Build this starbase?"), util::SkinColor::Static, gfx::FontRequest().addSize(1), root.provider(), 1)));

            util::KeyString yes(tx("Yes")), no(tx("No"));
            Button& btnYes = del.addNew(new Button(yes.getString(), yes.getKey(), root));
            Button& btnNo  = del.addNew(new Button(no.getString(),  no.getKey(),  root));
            btnYes.sig_fire.addNewClosure(loop.makeStop(1));
            btnNo.sig_fire.addNewClosure(loop.makeStop(0));

            g.add(btnYes);
            g.add(btnNo);
        } else {
            win.add(del.addNew(new ui::widgets::StaticText(tx("You can't build a starbase here."), util::SkinColor::Static, gfx::FontRequest().addSize(1), root.provider(), 1)));

            Button& btn = del.addNew(new Button(tx("OK"), util::Key_Return, root));
            btn.sig_fire.addNewClosure(loop.makeStop(1));

            g.add(btn);
        }
        g.add(del.addNew(new ui::Spacer()));
        win.add(g);
        win.add(disp);
        win.pack();
        disp.addNewClosure(util::Key_Return, loop.makeStop(1));
        disp.addNewClosure(' ',              loop.makeStop(1));
        disp.addNewClosure(util::Key_Escape, loop.makeStop(0));

        root.centerWidget(win);
        root.add(win);
        int result = loop.run();

        return result != 0;
    }
}

void
client::dialogs::doBuildStarbaseDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx, game::Id_t pid)
{
    // ex client/act-planet.cc:doPlanetBuildBase

    // Proxies
    ConfigurationProxy config(gameSender);
    BuildStarbaseProxy action(gameSender);
    Downlink link(root);

    // What to do?
    BuildStarbaseProxy::Status st;
    action.init(link, pid, st);

    switch (st.mode) {
     case BuildStarbaseProxy::Error:
        ui::dialogs::MessageBox(st.errorMessage, tx("Build Starbase"), root).doOkDialog();
        break;

     case BuildStarbaseProxy::CanBuild:
        if (showStarbaseBuildOrder(true, root, tx, st, config.getNumberFormatter(link))) {
            action.commit(link);
        }
        break;

     case BuildStarbaseProxy::CannotBuild:
        showStarbaseBuildOrder(false, root, tx, st, config.getNumberFormatter(link));
        break;

     case BuildStarbaseProxy::CanCancel:
        if (ui::dialogs::MessageBox(tx("You wanted to build a starbase at this planet. "
                                       "Cancel this order?"),
                                    tx("Build Starbase"), root).doYesNoDialog())
        {
            action.commit(link);
        }
        break;
    }
}
