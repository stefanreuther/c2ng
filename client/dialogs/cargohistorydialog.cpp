/**
  *  \file client/dialogs/cargohistorydialog.cpp
  */

#include "client/dialogs/cargohistorydialog.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/quit.hpp"

// Display cargo history.
void
client::dialogs::doCargoHistory(const game::map::ShipCargoInfos_t& info, ui::Root& root, afl::string::Translator& tx)
{
    // ex showCargoHistory
    if (info.empty()) {
        // Nothing known
        ui::dialogs::MessageBox(tx("Nothing is known about this ship's cargo."), tx("Ship History"), root).doOkDialog(tx);
    } else {
        afl::base::Deleter del;

        // Something is known. Render in a document and display that.
        const int em = root.provider().getFont(gfx::FontRequest())->getEmWidth();
        const int width = em * 25 + 5;
        ui::rich::DocumentView& doc = del.addNew(new ui::rich::DocumentView(gfx::Point(width, 100), 0, root.provider()));
        ui::rich::Document& d = doc.getDocument();
        d.setPageWidth(width);
        d.setLeftMargin(5);
        for (size_t i = 0, n = info.size(); i < n; ++i) {
            const game::map::ShipCargoInfo& e = info[i];
            if (e.addSpaceBefore) {
                d.addNewline();
            }
            if (e.isHeading) {
                d.add(util::rich::Text(e.name)
                      .withStyle(util::rich::StyleAttribute::Big)
                      .withStyle(util::rich::StyleAttribute::Underline));
            } else {
                d.add(e.name);
                if (!e.value.empty()) {
                    d.addRight(20*em, e.value);
                    if (!e.unit.empty()) {
                        d.add(" " + e.unit);
                    }
                }
            }
            d.addNewline();
        }
        d.finish();
        doc.adjustToDocumentSize();

        // Dialog
        ui::EventLoop loop(root);
        ui::widgets::Button& btnOK = del.addNew(new ui::widgets::Button(tx("OK"), util::Key_Return, root));
        btnOK.sig_fire.addNewClosure(loop.makeStop(0));

        ui::Window& win = del.addNew(new ui::Window(tx("Ship History"), root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
        win.add(doc);
        ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));
        g.add(del.addNew(new ui::Spacer()));
        g.add(btnOK);
        g.add(del.addNew(new ui::Spacer()));
        win.add(g);

        ui::widgets::KeyDispatcher& disp = del.addNew(new ui::widgets::KeyDispatcher());
        disp.addNewClosure(' ', loop.makeStop(0));
        disp.addNewClosure(util::Key_Escape, loop.makeStop(0));
        win.add(disp);
        win.add(del.addNew(new ui::widgets::Quit(root, loop)));
        win.pack();

        root.centerWidget(win);
        root.add(win);
        loop.run();
    }
}
