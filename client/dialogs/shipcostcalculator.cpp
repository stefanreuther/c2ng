/**
  *  \file client/dialogs/shipcostcalculator.cpp
  *  \brief Starship cost calculator
  */

#include "client/dialogs/shipcostcalculator.hpp"
#include "client/dialogs/buildshipmain.hpp"
#include "game/proxy/basestorageproxy.hpp"
#include "game/proxy/buildshipproxy.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/group.hpp"
#include "ui/widgets/button.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/statictext.hpp"

using client::dialogs::BuildShipMain;
using game::proxy::BaseStorageProxy;
using game::proxy::BuildShipProxy;
using ui::Group;
using ui::widgets::Button;
using ui::widgets::StaticText;

namespace {
    class ShipCostCalcDialog {
     public:
        ShipCostCalcDialog(ui::Root& root,
                           util::RequestSender<game::proxy::StarbaseAdaptor> adaptorSender,
                           util::RequestSender<game::Session> gameSender,
                           bool useStorage,
                           afl::string::Translator& tx)
            : m_buildProxy(adaptorSender, root.engine().dispatcher()),
              m_storageProxy(adaptorSender, root.engine().dispatcher(), true),
              m_main(root, m_buildProxy, m_storageProxy, gameSender, 0 /* no part building */, tx),
              m_loop(root),
              m_deleter(),
              m_pUsePartsFromStorageButton(),
              m_pUseTechUpgradeButton()
            {
                m_pUseTechUpgradeButton = &m_deleter.addNew(new Button("I", 'i', root));
                m_pUseTechUpgradeButton->sig_fire.add(this, &ShipCostCalcDialog::onToggleUseTechUpgrade);
                if (useStorage) {
                    m_pUsePartsFromStorageButton = &m_deleter.addNew(new Button("U", 'u', root));
                    m_pUsePartsFromStorageButton->sig_fire.add(this, &ShipCostCalcDialog::onToggleUsePartsFromStorage);
                }
                m_main.sig_change.add(this, &ShipCostCalcDialog::onBuildOrderChange);
            }

        void run()
            {
                // ex WShipCostCalcDialog::init
                afl::string::Translator& tx = m_main.translator();
                ui::Root& root = m_main.root();

                m_main.init(m_deleter);
                ui::Window& win = m_main.buildDialog(m_deleter, tx("Starship Cost Calculator"));

                Group& g1 = m_deleter.addNew(new Group(ui::layout::HBox::instance5));
                if (m_pUseTechUpgradeButton != 0) {
                    g1.add(*m_pUseTechUpgradeButton);
                    g1.add(m_deleter.addNew(new StaticText(tx("Include tech upgrades"), util::SkinColor::White, "+", root.provider())));
                    g1.add(m_deleter.addNew(new ui::Spacer(gfx::Point(10, 10))));
                }
                if (m_pUsePartsFromStorageButton != 0) {
                    g1.add(*m_pUsePartsFromStorageButton);
                    g1.add(m_deleter.addNew(new StaticText(tx("Use parts from storage"), util::SkinColor::White, "+", root.provider())));
                }
                g1.add(m_deleter.addNew(new ui::Spacer()));
                win.add(g1);

                Group& g2 = m_deleter.addNew(new Group(ui::layout::HBox::instance5));
                Button& btnHelp = m_deleter.addNew(new Button(tx("Help"), 'h', root));
                Button& btnClose = m_deleter.addNew(new Button(tx("Close"), util::Key_Escape, root));
                g2.add(btnHelp);
                g2.add(m_deleter.addNew(new ui::Spacer()));
                g2.add(m_main.makeDetailedBillButton(m_deleter));
                g2.add(btnClose);
                win.add(g2);

                ui::Widget& help = m_main.makeHelpWidget(m_deleter, "pcc2:buildship");
                win.add(help);
                win.add(m_deleter.addNew(new ui::widgets::Quit(root, m_loop)));

                btnHelp.dispatchKeyTo(help);
                btnClose.sig_fire.addNewClosure(m_loop.makeStop(0));

                win.pack();
                root.centerWidget(win);
                root.add(win);
                m_loop.run();
            }

        void onToggleUsePartsFromStorage()
            {
                // ex WShipCostCalcDialog::onToggleUseParts()
                if (m_pUsePartsFromStorageButton != 0) {
                    m_buildProxy.setUsePartsFromStorage(!m_pUsePartsFromStorageButton->getFlags().contains(ui::HighlightedButton));
                }
            }

        void onToggleUseTechUpgrade()
            {
                // ex WShipCostCalcDialog::onToggleTechUpgrade()
                if (m_pUseTechUpgradeButton != 0) {
                    m_buildProxy.setUseTechUpgrade(!m_pUseTechUpgradeButton->getFlags().contains(ui::HighlightedButton));
                }
            }

        void onBuildOrderChange(const BuildShipProxy::Status& st)
            {
                if (m_pUsePartsFromStorageButton != 0) {
                    m_pUsePartsFromStorageButton->setFlag(ui::HighlightedButton, st.isUsePartsFromStorage);
                }
                if (m_pUseTechUpgradeButton != 0) {
                    m_pUseTechUpgradeButton->setFlag(ui::HighlightedButton, st.isUseTechUpgrade);
                }
            }

     private:
        BuildShipProxy m_buildProxy;
        BaseStorageProxy m_storageProxy;
        BuildShipMain m_main;
        ui::EventLoop m_loop;
        afl::base::Deleter m_deleter;

        Button* m_pUsePartsFromStorageButton;
        Button* m_pUseTechUpgradeButton;
    };
}


void
client::dialogs::doShipCostCalculator(ui::Root& root,
                                      util::RequestSender<game::proxy::StarbaseAdaptor> adaptorSender,
                                      util::RequestSender<game::Session> gameSender,
                                      bool useStorage,
                                      afl::string::Translator& tx)
{
    ShipCostCalcDialog(root, adaptorSender, gameSender, useStorage, tx).run();
}
