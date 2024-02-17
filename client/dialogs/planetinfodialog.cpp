/**
  *  \file client/dialogs/planetinfodialog.cpp
  */

#include "client/dialogs/planetinfodialog.hpp"
#include "afl/base/deleter.hpp"
#include "afl/base/signalconnection.hpp"
#include "afl/io/xml/nodereader.hpp"
#include "afl/io/xml/tagnode.hpp"
#include "afl/io/xml/textnode.hpp"
#include "client/dialogs/grounddefensedialog.hpp"
#include "client/downlink.hpp"
#include "client/widgets/helpwidget.hpp"
#include "client/widgets/planetmineralinfo.hpp"
#include "game/game.hpp"
#include "game/map/universe.hpp"
#include "game/proxy/configurationproxy.hpp"
#include "game/proxy/planetinfoproxy.hpp"
#include "game/proxy/referenceproxy.hpp"
#include "game/turn.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/rich/documentparser.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/decimalselector.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"

using client::widgets::PlanetMineralInfo;
using game::Reference;
using game::proxy::PlanetInfoProxy;
using game::proxy::ReferenceProxy;

namespace {
    void addNodes(afl::io::xml::NodeReader& rdr, const afl::io::xml::Nodes_t& nodes)
    {
        for (size_t i = 0; i < nodes.size(); ++i) {
            rdr.addNode(nodes[i]);
        }
    }


    class PlanetInfoDialog {
     public:
        PlanetInfoDialog(ui::Root& root,
                         util::RequestSender<game::Session> gameSender,
                         const util::NumberFormatter& fmt,
                         afl::string::Translator& tx,
                         PlanetInfoProxy& proxy)
            : m_proxy(proxy),
              m_gameSender(gameSender),
              m_formatter(fmt),
              m_translator(tx),
              m_root(root),
              m_loop(root),
              m_del(),
              conn_update()
            {
                // ex WPlanetScanWindow::WPlanetScanWindow
                // ex WPlanetScanWindow::init
                m_docView = &m_del.addNew(new ui::rich::DocumentView(gfx::Point(315, 400), // <- FIXME
                                                                     ui::rich::DocumentView::fl_Scroll | ui::rich::DocumentView::fl_ScrollMark,
                                                                     root.provider()));
                for (size_t i = 0; i < 4; ++i) {
                    m_info[i] = &m_del.addNew(new PlanetMineralInfo(root, fmt, tx));
                }

                conn_update = proxy.sig_change.add(this, &PlanetInfoDialog::update);
            }

        ~PlanetInfoDialog()
            { }

        void run(const String_t& title)
            {
                // HBox
                //   VBox
                //     DocView
                //     HBox
                //       "Close", "C", Spacer, "H"
                //   VBox
                //     4x PlanetMineralInfo

                ui::Window win(title, m_root.provider(), m_root.colorScheme(), ui::BLUE_DARK_WINDOW, ui::layout::HBox::instance5);

                ui::Group& lgroup = m_del.addNew(new ui::Group(ui::layout::VBox::instance5));
                ui::Group& rgroup = m_del.addNew(new ui::Group(ui::layout::VBox::instance5));
                ui::Group& bgroup = m_del.addNew(new ui::Group(ui::layout::HBox::instance5));

                lgroup.add(*m_docView);
                lgroup.add(bgroup);

                rgroup.add(m_del.addNew(new ui::widgets::StaticText(m_translator("Minerals"), util::SkinColor::Heading, gfx::FontRequest().addSize(1).addWeight(1), m_root.provider(), gfx::LeftAlign)));
                for (size_t i = 0; i < 4; ++i) {
                    rgroup.add(*m_info[i]);
                }

                ui::Widget& helper = m_del.addNew(new client::widgets::HelpWidget(m_root, m_translator, m_gameSender, "pcc2:envscreen"));

                ui::widgets::Button& btnClose = m_del.addNew(new ui::widgets::Button(m_translator("Close"), util::Key_Return, m_root));
                ui::widgets::Button& btnCombat = m_del.addNew(new ui::widgets::Button("C", 'c', m_root));
                ui::widgets::Button& btnHelp = m_del.addNew(new ui::widgets::Button("H", 'h', m_root));

                bgroup.add(btnClose);
                bgroup.add(btnCombat);
                bgroup.add(m_del.addNew(new ui::Spacer()));
                bgroup.add(btnHelp);

                win.add(lgroup);
                win.add(rgroup);

                ui::widgets::KeyDispatcher& disp = m_del.addNew(new ui::widgets::KeyDispatcher());
                win.add(disp);
                win.add(helper);
                win.add(m_del.addNew(new ui::widgets::Quit(m_root, m_loop)));

                btnClose.sig_fire.addNewClosure(m_loop.makeStop(1));
                btnCombat.sig_fire.add(this, &PlanetInfoDialog::onGroundCombat);
                btnHelp.dispatchKeyTo(helper);

                disp.addNewClosure(' ', m_loop.makeStop(1));
                disp.addNewClosure(util::Key_Escape, m_loop.makeStop(1));
                disp.addNewClosure(util::Key_F5, m_loop.makeStop(1));
                disp.addNewClosure(util::Key_F5 + util::KeyMod_Shift, m_loop.makeStop(1));
                disp.addNewClosure(util::Key_F5 + util::KeyMod_Ctrl, m_loop.makeStop(1));

                win.pack();
                m_root.centerWidget(win);
                m_root.add(win);
                m_loop.run();
            }


     private:
        void update()
            {
                // Mines
                m_info[0]->setContent(m_translator("Neutronium"), m_proxy.getMineralInfo(PlanetInfoProxy::Neutronium), PlanetMineralInfo::First);
                m_info[1]->setContent(m_translator("Tritanium"),  m_proxy.getMineralInfo(PlanetInfoProxy::Tritanium),  PlanetMineralInfo::Second);
                m_info[2]->setContent(m_translator("Duranium"),   m_proxy.getMineralInfo(PlanetInfoProxy::Duranium),   PlanetMineralInfo::Second);
                m_info[3]->setContent(m_translator("Molybdenum"), m_proxy.getMineralInfo(PlanetInfoProxy::Molybdenum), PlanetMineralInfo::Second);

                // Free-text
                // - prepare XML document
                afl::io::xml::NodeReader rdr;
                afl::io::xml::TagNode climateHeader("h1");
                climateHeader.addNewChild(new afl::io::xml::TextNode(m_translator("Climate")));
                rdr.addNode(&climateHeader);
                addNodes(rdr, m_proxy.getClimateInfo());

                afl::io::xml::TagNode nativeHeader("h1");
                nativeHeader.addNewChild(new afl::io::xml::TextNode(m_translator("Natives")));
                rdr.addNode(&nativeHeader);
                addNodes(rdr, m_proxy.getNativeInfo());

                afl::io::xml::TagNode colonyHeader("h1");
                colonyHeader.addNewChild(new afl::io::xml::TextNode(m_translator("Colony")));
                rdr.addNode(&colonyHeader);
                addNodes(rdr, m_proxy.getColonyInfo());

                // - render document
                ui::rich::Document& doc = m_docView->getDocument();
                doc.clear();
                ui::rich::DocumentParser(doc, rdr).parseDocument();
                doc.finish();
                m_docView->handleDocumentUpdate();
            }

        void onGroundCombat()
            {
                if (m_proxy.getGroundDefenseInfo().isPlayable) {
                    // ex WPlanetScanWindow::onGroundCombat
                    client::dialogs::doGroundDefenseDialog(m_root, m_proxy.getGroundDefenseInfo(), m_formatter, m_translator);
                } else {
                    afl::base::Observable<int32_t> value(m_proxy.getUnloadInfo().hostileUnload);
                    ui::widgets::DecimalSelector sel(m_root, m_translator, value, 0, 10000, 10);
                    if (doStandardDialog(m_translator("Ground Combat"), m_translator("Clans to attack with:"), sel, false, m_root, m_translator)) {
                        m_proxy.setAttackingClansOverride(value.get());
                    }
                }
            }

        PlanetInfoProxy& m_proxy;
        util::RequestSender<game::Session> m_gameSender;
        util::NumberFormatter m_formatter;
        afl::string::Translator& m_translator;
        ui::Root& m_root;
        ui::EventLoop m_loop;
        afl::base::Deleter m_del;

        // Left side
        ui::rich::DocumentView* m_docView;

        // Right side
        PlanetMineralInfo* m_info[4];

        afl::base::SignalConnection conn_update;
    };
}



void
client::dialogs::doPlanetInfoDialog(ui::Root& root,
                                    util::RequestSender<game::Session> gameSender,
                                    game::Id_t planetId,

                                    afl::string::Translator& tx)
{
    // ex doPlanetScan
    // ex envscan.pas:ScanPlanet, DoEnvScan
    // Determine planet name, synchronously
    String_t planetName;
    Downlink link(root, tx);
    if (!ReferenceProxy(gameSender).getReferenceName(link, Reference(Reference::Planet, planetId), game::LongName /* Planet #x: nnn */).get(planetName)) {
        planetName = tx("Planet");
    }

    // NumberFormatter
    util::NumberFormatter fmt = game::proxy::ConfigurationProxy(gameSender).getNumberFormatter(link);

    // Set up PlanetInfoProxy to retrieve data asynchronously.
    // This must be after the synchronous wait so that the window is already open when the data arrives, and word-wrap works correctly.
    PlanetInfoProxy proxy(gameSender, root.engine().dispatcher());
    PlanetInfoDialog dlg(root, gameSender, fmt, tx, proxy);
    proxy.setPlanet(planetId);

    dlg.run(planetName);
}

void
client::dialogs::doPlanetInfoDialog(ui::Root& root,
                                    util::RequestSender<game::Session> gameSender,
                                    game::map::Point pos,
                                    afl::string::Translator& tx)
{
    class Init : public util::Request<game::Session> {
     public:
        Init(game::map::Point pos)
            : m_pos(pos), m_id(0)
            { }
        virtual void handle(game::Session& s)
            {
                game::Game* g = s.getGame().get();
                game::Root* r = s.getRoot().get();
                if (g != 0 && r != 0) {
                    m_id = g->viewpointTurn().universe().findPlanetAt(m_pos, true, g->mapConfiguration(), r->hostConfiguration(), r->hostVersion());
                }
            }
        game::Id_t getId() const
            { return m_id; }
     private:
        game::map::Point m_pos;
        game::Id_t m_id;
    };

    Init i(pos);
    Downlink link(root, tx);
    link.call(gameSender, i);

    if (game::Id_t id = i.getId()) {
        doPlanetInfoDialog(root, gameSender, id, tx);
    }
}
