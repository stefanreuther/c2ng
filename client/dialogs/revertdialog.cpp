/**
  *  \file client/dialogs/revertdialog.cpp
  *  \brief "Reset Location" dialog
  */

#include <memory>
#include "client/dialogs/revertdialog.hpp"
#include "afl/functional/stringtable.hpp"
#include "client/downlink.hpp"
#include "client/widgets/helpwidget.hpp"
#include "client/widgets/referencelistbox.hpp"
#include "game/proxy/reverterproxy.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/rich/statictext.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/optiongrid.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "util/rich/parser.hpp"
#include "util/translation.hpp"

using afl::functional::createStringTable;
using game::map::LocationReverter;
using game::proxy::ReverterProxy;
using ui::widgets::Button;
using ui::widgets::OptionGrid;

namespace {
    const char*const VALUES[] = {
        N_("keep"),
        N_("reset"),
    };

    class RevertDialog {
     public:
        RevertDialog(ui::Root& root, const ReverterProxy::Status& status, afl::string::Translator& tx)
            : m_root(root),
              m_status(status),
              m_translator(tx),
              m_modes(),
              m_loop(root),
              m_grid(0, 0, root),
              m_okButton(tx("OK"), util::Key_Return, root)
            { }

        bool run(util::RequestSender<game::Session> gameSender)
            {
                // VBox
                //   OptionGrid
                //   Rich Static
                //   HBox
                //     Button "Help"
                //     Button "List"
                //     Spacer
                //     Button "OK"
                //     Button "Cancel"
                afl::base::Deleter del;
                ui::Window& win = del.addNew(new ui::Window(m_translator("Reset Location"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

                // Options
                m_grid.addItem(LocationReverter::Cargo,    'c', m_translator("Cargo, structures, builds"))
                    .addPossibleValues(createStringTable(VALUES).map(m_translator));
                m_grid.addItem(LocationReverter::Missions, 'm', m_translator("Missions, orders, names"))
                    .addPossibleValues(createStringTable(VALUES).map(m_translator));
                update();

                // Build the dialog
                win.add(m_grid);

                const char*const TEXT = N_("This will reset all units at this place to their state at the beginning of the turn. "
                                           "Please choose which areas you want to reset. "
                                           "<b>Cargo</b> will reset everything that interacts with cargo (buildings, transfers, etc.), "
                                           "<b>Missions</b> will reset everything else.");
                win.add(del.addNew(new ui::rich::StaticText(util::rich::Parser::parseXml(m_translator(TEXT)),
                                                            30 * m_root.provider().getFont("")->getEmWidth(),
                                                            m_root.provider())));

                ui::Widget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, gameSender, "pcc2:reset"));
                ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));
                Button& btnHelp   = del.addNew(new Button(m_translator("Help"), 'h', m_root));
                Button& btnList   = del.addNew(new Button(m_translator("List units..."), 'l', m_root));
                Button& btnCancel = del.addNew(new Button(m_translator("Cancel"), util::Key_Escape, m_root));
                g.add(btnHelp);
                g.add(btnList);
                g.add(del.addNew(new ui::Spacer()));
                g.add(m_okButton);
                g.add(btnCancel);
                win.add(g);
                win.add(help);
                win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));

                btnCancel.sig_fire.addNewClosure(m_loop.makeStop(0));
                m_okButton.sig_fire.add(this, &RevertDialog::onOK);
                m_grid.sig_click.add(this, &RevertDialog::onOptionClick);
                btnList.sig_fire.add(this, &RevertDialog::onList);
                btnHelp.dispatchKeyTo(help);

                win.pack();
                m_root.centerWidget(win);
                m_root.add(win);
                return m_loop.run();
            }

        LocationReverter::Modes_t getModes() const
            {
                return m_modes;
            }

     private:
        void update();
        void updateMode(LocationReverter::Mode m);

        void onOK();
        void onOptionClick(int n);
        void onList();

        ui::Root& m_root;
        const ReverterProxy::Status& m_status;
        afl::string::Translator& m_translator;
        LocationReverter::Modes_t m_modes;
        ui::EventLoop m_loop;
        OptionGrid m_grid;
        Button m_okButton;
    };
}

void
RevertDialog::update()
{
    updateMode(LocationReverter::Cargo);
    updateMode(LocationReverter::Missions);
    m_okButton.setState(Button::DisabledState, m_modes.empty());
}

void
RevertDialog::updateMode(LocationReverter::Mode m)
{
    m_grid.findItem(m)
        .setValue(m_translator(VALUES[m_modes.contains(m)]))
        .setEnabled(m_status.modes.contains(m));
}

void
RevertDialog::onOK()
{
    if (!m_modes.empty()) {
        m_loop.stop(1);
    }
}

void
RevertDialog::onOptionClick(int n)
{
    LocationReverter::Mode m = LocationReverter::Mode(n);
    if (m_status.modes.contains(m)) {
        m_modes ^= m;
        update();
    }
}

void
RevertDialog::onList()
{
    client::widgets::ReferenceListbox box(m_root);
    box.setNumLines(15);
    box.setContent(m_status.list);
    box.doStandardDialog(m_translator("Reset Location"), m_translator("Affected units:"), 0, m_root, m_translator);
}

void
client::dialogs::doRevertLocation(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx, game::map::Point pos)
{
    // ex doResetDialog(GReset& reset) (sort-of), ship.pas:ResetXY

    // Set up
    Downlink link(root, tx);
    ReverterProxy proxy(gameSender);
    ReverterProxy::Status status;
    proxy.init(link, pos, status);

    // Possible?
    if (status.modes.empty() || status.list.empty()) {
        ui::dialogs::MessageBox(tx("There is nothing that can be reset at this location."),
                                tx("Reset Location"),
                                root).doOkDialog(tx);
    } else {
        RevertDialog dlg(root, status, tx);
        if (dlg.run(gameSender)) {
            proxy.commit(dlg.getModes());
        }
    }
}
