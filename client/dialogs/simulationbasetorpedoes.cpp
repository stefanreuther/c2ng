/**
  *  \file client/dialogs/simulationbasetorpedoes.cpp
  */

#include "client/dialogs/simulationbasetorpedoes.hpp"
#include "afl/base/observable.hpp"
#include "afl/container/ptrvector.hpp"
#include "client/widgets/helpwidget.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "ui/widgets/listlikedecimalselector.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"

using ui::widgets::ListLikeDecimalSelector;

bool
client::dialogs::editSimulationBaseTorpedoes(ui::Root& root,
                                             util::RequestSender<game::Session> gameSender,
                                             size_t initialFocus,
                                             game::proxy::SimulationSetupProxy::Elements_t& list,
                                             afl::string::Translator& tx)
{
    // ex editSimulatorPlanetTorps, ccsim.pas:EditTorpedoes
    typedef afl::base::Observable<int32_t> Value_t;
    afl::container::PtrVector<Value_t> values;
    afl::base::Deleter del;

    ui::Window& win = del.addNew(new ui::Window(tx("Starbase Torpedoes"), root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    ui::widgets::FocusIterator& it = del.addNew(new ui::widgets::FocusIterator(ui::widgets::FocusIterator::Vertical | ui::widgets::FocusIterator::Tab));
    ui::Widget* toFocus = 0;
    ui::Group& g = del.addNew(new ui::Group(ui::layout::VBox::instance0));
    for (size_t i = 0; i < list.size(); ++i) {
        Value_t& thisValue = *values.pushBackNew(new Value_t(list[i].first));
        ListLikeDecimalSelector& thisInput = del.addNew(new ListLikeDecimalSelector(root, list[i].second, thisValue, 0, 10000, 10));
        it.add(thisInput);
        if (i == 0 || i == initialFocus) {
            toFocus = &thisInput;
        }
        g.add(thisInput);
    }
    win.add(g);

    ui::EventLoop loop(root);
    ui::widgets::StandardDialogButtons& btn = del.addNew(new ui::widgets::StandardDialogButtons(root, tx));
    ui::Widget& help = del.addNew(new client::widgets::HelpWidget(root, tx, gameSender, "pcc2:simplanet"));
    btn.addStop(loop);
    btn.addHelp(help);
    win.add(btn);
    win.add(del.addNew(new ui::widgets::Quit(root, loop)));
    win.add(it);
    win.pack();
    if (toFocus) {
        toFocus->requestFocus();
    }

    root.moveWidgetToEdge(win, gfx::RightAlign, gfx::BottomAlign, 10);
    root.add(win);

    if (loop.run() != 0) {
        for (size_t i = 0; i < list.size(); ++i) {
            list[i].first = values[i]->get();
        }
        return true;
    } else {
        return false;
    }
}
