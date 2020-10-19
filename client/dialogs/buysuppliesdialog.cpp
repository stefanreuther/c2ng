/**
  *  \file client/dialogs/buysuppliesdialog.cpp
  */

#include "client/dialogs/buysuppliesdialog.hpp"
#include "afl/string/format.hpp"
#include "client/downlink.hpp"
#include "client/widgets/helpwidget.hpp"
#include "game/proxy/convertsuppliesproxy.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/rich/statictext.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/decimalselector.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"

using afl::string::Format;
using game::proxy::ConvertSuppliesProxy;
using ui::Group;
using ui::widgets::StandardDialogButtons;

namespace {
    class BuySuppliesDialog {
     public:
        BuySuppliesDialog(ui::Root& root, int32_t maxSuppliesToBuy, afl::string::Translator& tx)
            : m_root(root),
              m_loop(root),
              m_value(0),
              m_select(root, m_value, 0, maxSuppliesToBuy, 10),
              m_translator(tx)
            { }

        bool run(util::RequestSender<game::Session> gameSender)
            {
                // ex WUndoSellSuppliesDialog::init
                afl::base::Deleter del;
                ui::Window& win = del.addNew(new ui::Window(m_translator("Undo Supply Sale"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
                win.add(del.addNew(new ui::rich::StaticText(m_translator(Format("You can buy up to %d supplies. (This is the amount of supplies "
                                                                                "you already sold this turn but have not used otherwise.)\n"
                                                                                "Enter amount to buy:",
                                                                                // FIXME: use formatNumber()
                                                                                m_select.getMax())),
                                                            400,
                                                            m_root.provider())));
                win.add(m_select);

                ui::Widget& helper = del.addNew(new client::widgets::HelpWidget(m_root, gameSender, "pcc2:sellsup"));

                StandardDialogButtons& btns = del.addNew(new StandardDialogButtons(m_root));
                btns.addStop(m_loop);
                btns.addHelp(helper);
                win.add(btns);
                win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));
                win.add(helper);
                win.pack();

                m_root.centerWidget(win);
                m_root.add(win);
                m_select.requestFocus();
                return m_loop.run() != 0;
            }

        int32_t getValue() const
            { return m_value.get(); }

     private:
        ui::Root& m_root;
        ui::EventLoop m_loop;
        afl::base::Observable<int32_t> m_value;
        ui::widgets::DecimalSelector m_select;
        afl::string::Translator& m_translator;
    };
}

void
client::dialogs::doBuySuppliesDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, game::Id_t planetId, int32_t reservedMoney, int32_t reservedSupplies, afl::string::Translator& tx)
{
    // ex WUndoSellSuppliesDialog::doDialog
    ConvertSuppliesProxy proxy(gameSender);
    Downlink link(root);

    ConvertSuppliesProxy::Status st = proxy.init(link, planetId, reservedMoney, reservedSupplies);
    if (st.maxSuppliesToBuy == 0) {
        ui::dialogs::MessageBox(tx("You cannot buy supplies. Either you have not yet sold any this turn, "
                                   "or you have already spent the money."),
                                tx("Undo Supply Sale"),
                                root).doOkDialog();
    } else {
        BuySuppliesDialog dlg(root, st.maxSuppliesToBuy, tx);
        if (dlg.run(gameSender)) {
            proxy.buySupplies(dlg.getValue());
        }
    }
}

