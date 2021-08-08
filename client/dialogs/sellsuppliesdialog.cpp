/**
  *  \file client/dialogs/sellsuppliesdialog.cpp
  */

#include "client/dialogs/sellsuppliesdialog.hpp"
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

using afl::string::Format;
using game::proxy::ConvertSuppliesProxy;
using ui::Group;
using ui::widgets::Button;

namespace {
    class SellSuppliesDialog {
     public:
        SellSuppliesDialog(ui::Root& root, int32_t maxSuppliesToSell, ConvertSuppliesProxy& proxy, afl::string::Translator& tx)
            : m_root(root),
              m_loop(root),
              m_value(0),
              m_select(root, tx, m_value, 0, maxSuppliesToSell, 10),
              m_proxy(proxy),
              m_translator(tx)
            { }

        void run(util::RequestSender<game::Session> gameSender)
            {
                // ex WSellSuppliesDialog::init
                afl::base::Deleter del;
                ui::Window& win = del.addNew(new ui::Window(m_translator("Sell Supplies"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
                win.add(del.addNew(new ui::rich::StaticText(String_t(Format(m_translator("You have %d kt supplies. You'll get 1 mc per kiloton. "
                                                                                         "Remember that PCC automatically sells supplies when needed.\n"
                                                                                         "Enter amount to sell:"),
                                                                            // FIXME: use formatNumber()
                                                                            m_select.getMax())),
                                                            400,
                                                            m_root.provider())));
                win.add(m_select);

                ui::Widget& helper = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, gameSender, "pcc2:sellsup"));

                Button& btnOK     = del.addNew(new Button(m_translator("OK"),         util::Key_Return, m_root));
                Button& btnAllBut = del.addNew(new Button(m_translator("All but..."), 'a',              m_root));
                Button& btnCancel = del.addNew(new Button(m_translator("Cancel"),     util::Key_Escape, m_root));
                Button& btnHelp   = del.addNew(new Button(m_translator("Help"),       'h',              m_root));
                btnOK.sig_fire.add(this, &SellSuppliesDialog::onOK);
                btnAllBut.sig_fire.add(this, &SellSuppliesDialog::onAllBut);
                btnCancel.sig_fire.addNewClosure(m_loop.makeStop(0));
                btnHelp.dispatchKeyTo(helper);

                Group& g = del.addNew(new Group(ui::layout::HBox::instance5));
                g.add(btnHelp);
                g.add(del.addNew(new ui::Spacer()));
                g.add(btnAllBut);
                g.add(btnOK);
                g.add(btnCancel);
                win.add(g);
                win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));
                win.add(helper);
                win.pack();

                m_root.centerWidget(win);
                m_root.add(win);
                m_select.requestFocus();
                m_loop.run();
            }

        void onOK()
            {
                m_proxy.sellSupplies(m_value.get());
                m_loop.stop(1);
            }

        void onAllBut()
            {
                // ex WSellSuppliesDialog::onAll
                if (m_value.get() == 0 && m_select.getMax() != 0) {
                    if (!ui::dialogs::MessageBox(m_translator("Do you really want to sell all supplies?"),
                                                 m_translator("Sell Supplies"),
                                                 m_root).doYesNoDialog(m_translator))
                    {
                        return;
                    }
                }

                m_proxy.sellSupplies(m_select.getMax() - m_value.get());
                m_loop.stop(1);
            }

     private:
        ui::Root& m_root;
        ui::EventLoop m_loop;
        afl::base::Observable<int32_t> m_value;
        ui::widgets::DecimalSelector m_select;
        ConvertSuppliesProxy& m_proxy;
        afl::string::Translator& m_translator;
    };
}

void
client::dialogs::doSellSuppliesDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, game::Id_t planetId, int32_t reservedSupplies, int32_t reservedMoney, afl::string::Translator& tx)
{
    // ex WSellSuppliesDialog::doDialog, pdata.pas:SellSupplies
    ConvertSuppliesProxy proxy(gameSender);
    Downlink link(root, tx);

    ConvertSuppliesProxy::Status st = proxy.init(link, planetId, reservedSupplies, reservedMoney);
    if (st.maxSuppliesToSell == 0) {
        ui::dialogs::MessageBox(tx("You do not have any supplies on this planet."),
                                tx("Sell Supplies"),
                                root).doOkDialog(tx);
    } else {
        SellSuppliesDialog(root, st.maxSuppliesToSell, proxy, tx).run(gameSender);
    }
}

