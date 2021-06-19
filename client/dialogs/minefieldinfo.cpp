/**
  *  \file client/dialogs/minefieldinfo.cpp
  */

#include "client/dialogs/minefieldinfo.hpp"
#include "afl/base/deleter.hpp"
#include "afl/string/format.hpp"
#include "client/downlink.hpp"
#include "client/map/circleoverlay.hpp"
#include "client/map/widget.hpp"
#include "client/si/control.hpp"
#include "client/tiles/selectionheadertile.hpp"
#include "client/widgets/helpwidget.hpp"
#include "game/proxy/configurationproxy.hpp"
#include "game/proxy/minefieldproxy.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/invisiblewidget.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/simpletable.hpp"
#include "ui/window.hpp"
#include "util/math.hpp"
#include "util/numberformatter.hpp"

using game::proxy::MinefieldProxy;

namespace {
    class MinefieldInfoDialog;

    const int NUM_LINES = 7;

    gfx::Point getPreferredMapSize(ui::Root& root)
    {
        int px = root.provider().getFont(gfx::FontRequest())->getLineHeight() * (NUM_LINES + 2);
        return gfx::Point(px, px);
    }

    int getReductionFactor(const gfx::Rectangle& area, int radius)
    {
        int screenRadius = std::min(area.getWidth(), area.getHeight()) / 2;
        if (screenRadius > 0) {
            return util::divideAndRoundUp(radius, screenRadius);
        } else {
            return 1;
        }
    }

    class MinefieldInfoKeyHandler : public ui::InvisibleWidget {
     public:
        MinefieldInfoKeyHandler(MinefieldInfoDialog& parent)
            : InvisibleWidget(), m_parent(parent)
            { }
        virtual bool handleKey(util::Key_t key, int prefix);
     private:
        MinefieldInfoDialog& m_parent;
    };

    class MinefieldInfoDialog : public client::si::Control {
     public:
        MinefieldInfoDialog(client::si::UserSide& iface, ui::Root& root, afl::string::Translator& tx, client::si::OutputState& out);

        void run();
        bool handleKey(util::Key_t key, int prefix);

        virtual void handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target target);
        virtual void handleEndDialog(client::si::RequestLink2 link, int code);
        virtual void handlePopupConsole(client::si::RequestLink2 link);
        virtual void handleSetViewRequest(client::si::RequestLink2 link, String_t name, bool withKeymap);
        virtual void handleUseKeymapRequest(client::si::RequestLink2 link, String_t name, int prefix);
        virtual void handleOverlayMessageRequest(client::si::RequestLink2 link, String_t text);
        virtual client::si::ContextProvider* createContextProvider();

     private:
        client::si::UserSide& m_userSide;
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        client::si::OutputState& m_outputState;
        MinefieldProxy m_proxy;
        ui::EventLoop m_loop;

        // Content widgets
        client::map::Widget m_mapWidget;
        client::map::CircleOverlay m_mapOverlay;
        ui::widgets::SimpleTable m_minefieldTable;
        ui::widgets::SimpleTable m_passageTable;
        ui::widgets::Button m_planetButton;

        // Status cache
        int m_passageDistance;
        game::Id_t m_planetId;
        game::Id_t m_minefieldId;
        game::map::Point m_minefieldCenter;

        void initLabels();

        void onMinefieldChange(const MinefieldProxy::MinefieldInfo& info);
        void onPassageChange(const MinefieldProxy::PassageInfo& info);
        void onGoto();
        void onDelete();

        void setPassageDistance(int newDistance);
        void showSweepInfo();
    };
}

/*
 *  MinefieldInfoKeyHandler
 */

bool
MinefieldInfoKeyHandler::handleKey(util::Key_t key, int prefix)
{
    return m_parent.handleKey(key, prefix);
}

/*
 *  MinefieldInfoDialog
 */

MinefieldInfoDialog::MinefieldInfoDialog(client::si::UserSide& iface,
                                         ui::Root& root,
                                         afl::string::Translator& tx,
                                         client::si::OutputState& out)
    : Control(iface, root, tx),
      m_userSide(iface),
      m_root(root),
      m_translator(tx),
      m_outputState(out),
      m_proxy(root.engine().dispatcher(), iface.gameSender()),
      m_loop(root),
      m_mapWidget(iface.gameSender(), root, getPreferredMapSize(root)),
      m_mapOverlay(root.colorScheme()),
      m_minefieldTable(root, 2, NUM_LINES),
      m_passageTable(root, 2, 2),
      m_planetButton("P", 'p', root),
      m_passageDistance(0),
      m_planetId(0),
      m_minefieldId(0),
      m_minefieldCenter()
{
    m_proxy.sig_minefieldChange.add(this, &MinefieldInfoDialog::onMinefieldChange);
    m_proxy.sig_passageChange.add(this, &MinefieldInfoDialog::onPassageChange);
    initLabels();
}

void
MinefieldInfoDialog::run()
{
    // ex WMinefieldInfoWindow::init, chartdlg.pas:MineInfo
    // VBox
    //   SelectionHeaderTile
    //   HBox
    //     VBox cg1
    //       HBox cg11
    //         SimpleTable (Minefield Info)
    //         Spacer (required to avoid that buttons grow)
    //         VBox: P, S, I, Spacer
    //       HBox cg12
    //         SimpleTable (Passage)
    //         VBox cg121
    //           HBox: -, +
    //           Spacer
    //     map::Widget
    //   HBox
    //     Buttons: Goto, Close, Delete || Help
    afl::base::Deleter del;

    ui::Window& win = del.addNew(new ui::Window(m_translator("Minefield Information"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    ui::Widget& keys = del.addNew(new MinefieldInfoKeyHandler(*this));

    // Header
    client::tiles::SelectionHeaderTile& header = del.addNew(new client::tiles::SelectionHeaderTile(m_root, keys));
    header.attach(m_proxy);
    win.add(header);

    // Content: Minefield Info
    ui::widgets::Button& btnP = m_planetButton;
    ui::widgets::Button& btnS = del.addNew(new ui::widgets::Button("S", 's', m_root));
    // ui::widgets::Button& btnI = del.addNew(new ui::widgets::Button("I", 'i', m_root));
    btnP.setFont(""); btnP.dispatchKeyTo(keys);
    btnS.setFont(""); btnS.dispatchKeyTo(keys);
    // btnI.setFont(""); btnI.dispatchKeyTo(keys);

    ui::Group& cg = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::Group& cg1 = del.addNew(new ui::Group(ui::layout::VBox::instance5));
    ui::Group& cg11 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::Group& cg111 = del.addNew(new ui::Group(del.addNew(new ui::layout::VBox(1))));
    cg111.add(btnP);
    cg111.add(btnS);
    // cg111.add(btnI);
    cg111.add(del.addNew(new ui::Spacer()));
    cg11.add(m_minefieldTable);
    cg11.add(del.addNew(new ui::Spacer()));
    cg11.add(cg111);

    // Content: Passage Info
    ui::widgets::Button& btnPlus = del.addNew(new ui::widgets::Button("+", '+', m_root));
    ui::widgets::Button& btnMinus = del.addNew(new ui::widgets::Button("-", '-', m_root));
    btnPlus.setFont(""); btnPlus.dispatchKeyTo(keys);
    btnMinus.setFont(""); btnMinus.dispatchKeyTo(keys);

    ui::Group& cg12 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::Group& cg121 = del.addNew(new ui::Group(ui::layout::VBox::instance0));
    ui::Group& cg1211 = del.addNew(new ui::Group(del.addNew(new ui::layout::HBox(1))));
    cg1211.add(btnPlus);
    cg1211.add(btnMinus);
    cg121.add(cg1211);
    cg121.add(del.addNew(new ui::Spacer()));
    cg12.add(m_passageTable);
    cg12.add(cg121);
    cg1.add(cg11);
    cg1.add(cg12);
    cg.add(cg1);
    cg.add(ui::widgets::FrameGroup::wrapWidget(del, m_root.colorScheme(), ui::LoweredFrame, m_mapWidget));
    win.add(cg);

    // Buttons
    ui::widgets::Button& btnClose  = del.addNew(new ui::widgets::Button(m_translator("Close"),  util::Key_Escape, m_root));
    ui::widgets::Button& btnGoto   = del.addNew(new ui::widgets::Button(m_translator("Go to"),  util::Key_Return, m_root));
    ui::widgets::Button& btnDelete = del.addNew(new ui::widgets::Button(m_translator("Delete"), util::Key_Delete, m_root));
    ui::widgets::Button& btnHelp   = del.addNew(new ui::widgets::Button(m_translator("Help"), 'h', m_root));
    ui::Group& buttonGroup = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    buttonGroup.add(btnClose);
    buttonGroup.add(btnGoto);
    buttonGroup.add(btnDelete);
    buttonGroup.add(del.addNew(new ui::Spacer()));
    buttonGroup.add(btnHelp);
    win.add(buttonGroup);
    win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));
    win.add(keys);

    client::widgets::HelpWidget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, m_userSide.gameSender(), "pcc2:minescreen"));
    win.add(help);

    btnClose.sig_fire.addNewClosure(m_loop.makeStop(0));
    btnGoto.sig_fire.add(this, &MinefieldInfoDialog::onGoto);
    btnDelete.sig_fire.add(this, &MinefieldInfoDialog::onDelete);
    btnHelp.dispatchKeyTo(help);

    win.pack();

    m_root.centerWidget(win);
    m_root.add(win);
    m_loop.run();
}

bool
MinefieldInfoDialog::handleKey(util::Key_t key, int /*prefix*/)
{
    // ex WMinefieldPassageTile::handleEvent
    using game::map::ObjectCursor;
    switch (key) {
     case util::Key_PgUp:
     case util::Key_Up:
     case util::Key_WheelUp:
        m_proxy.browse(ObjectCursor::Previous, false);
        return true;

     case util::KeyMod_Ctrl + util::Key_PgUp:
     case util::KeyMod_Ctrl + util::Key_Up:
     case util::KeyMod_Ctrl + util::Key_WheelUp:
        m_proxy.browse(ObjectCursor::Previous, true);
        return true;

     case util::Key_PgDn:
     case util::Key_Down:
     case util::Key_WheelDown:
        m_proxy.browse(ObjectCursor::Next, false);
        return true;

     case util::KeyMod_Ctrl + util::Key_PgDn:
     case util::KeyMod_Ctrl + util::Key_Down:
     case util::KeyMod_Ctrl + util::Key_WheelDown:
        m_proxy.browse(ObjectCursor::Next, true);
        return true;

     case util::Key_Home:
        m_proxy.browse(ObjectCursor::First, false);
        return true;

     case util::KeyMod_Ctrl + util::Key_Home:
        m_proxy.browse(ObjectCursor::First, true);
        return true;

     case util::Key_End:
        m_proxy.browse(ObjectCursor::Last, false);
        return true;

     case util::Key_Tab:
        m_proxy.browse(ObjectCursor::NextHere, false);
        return true;

     case util::Key_Tab + util::KeyMod_Shift:
        m_proxy.browse(ObjectCursor::PreviousHere, false);
        return true;

     case util::KeyMod_Ctrl + util::Key_End:
        m_proxy.browse(ObjectCursor::Last, true);
        return true;

     case '+':
        setPassageDistance(m_passageDistance + 10);
        return true;

     case util::KeyMod_Shift + '+':
        setPassageDistance(m_passageDistance + 1);
        return true;

     case '-':
        setPassageDistance(m_passageDistance - 10);
        return true;

     case util::KeyMod_Shift + '-':
        setPassageDistance(m_passageDistance - 1);
        return true;

     case 'p':
        if (m_planetId != 0) {
            executeGoToReference("(Controlling Planet)", game::Reference(game::Reference::Planet, m_planetId));
        }
        return true;

     case 's':
        showSweepInfo();
        return true;

     default:
        return false;
    }
}

void
MinefieldInfoDialog::handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target target)
{
    dialogHandleStateChange(link, target, m_outputState, m_loop, 0);
}

void
MinefieldInfoDialog::handleEndDialog(client::si::RequestLink2 link, int code)
{
    dialogHandleEndDialog(link, code, m_outputState, m_loop, 0);
}

void
MinefieldInfoDialog::handlePopupConsole(client::si::RequestLink2 link)
{
    defaultHandlePopupConsole(link);
}

void
MinefieldInfoDialog::handleSetViewRequest(client::si::RequestLink2 link, String_t name, bool withKeymap)
{
    defaultHandleSetViewRequest(link, name, withKeymap);
}

void
MinefieldInfoDialog::handleUseKeymapRequest(client::si::RequestLink2 link, String_t name, int prefix)
{
    defaultHandleUseKeymapRequest(link, name, prefix);
}

void
MinefieldInfoDialog::handleOverlayMessageRequest(client::si::RequestLink2 link, String_t text)
{
    defaultHandleOverlayMessageRequest(link, text);
}

client::si::ContextProvider*
MinefieldInfoDialog::createContextProvider()
{
    return 0;
}

void
MinefieldInfoDialog::initLabels()
{
    afl::string::Translator& tx = m_translator;
    const int em = m_root.provider().getFont(gfx::FontRequest())->getEmWidth();

    // The 12 em/16 em split is also in PCC2, and makes sure the widgets are lined up.
    const int LEFT = 12*em;
    const int RIGHT = 16*em;

    // ex WMinefieldInfoTile::drawData (part)
    m_minefieldTable.all().setColor(ui::Color_Black);
    m_minefieldTable.cell(0, 0).setText(tx("Owner:"));
    m_minefieldTable.cell(0, 1).setText(tx("Size:"));
    m_minefieldTable.cell(0, 3).setText(tx("After decay:"));
    m_minefieldTable.cell(0, 4).setText(tx("Last info:"));
    m_minefieldTable.cell(0, 5).setText(tx("Controlled by:"));
    m_minefieldTable.setColumnWidth(0, LEFT);
    m_minefieldTable.setColumnWidth(1, RIGHT);

    // ex WMinefieldPassageTile::drawData (part)
    m_passageTable.all().setColor(ui::Color_Black);
    m_passageTable.cell(0, 0).setText(tx("Successful passage:"));
    m_passageTable.setColumnWidth(0, LEFT);
    m_passageTable.setColumnWidth(1, RIGHT);

    // ex WObjectSelectionChartWidget::drawData (sort-of)
    m_mapWidget.addOverlay(m_mapOverlay);
    m_mapOverlay.setColor(ui::Color_Gray);
}

void
MinefieldInfoDialog::onMinefieldChange(const MinefieldProxy::MinefieldInfo& info)
{
    // If .minefieldId is zero, no more minefields remain
    if (info.minefieldId == 0) {
        m_loop.stop(0);
    } else {
        // ex WMinefieldInfoTile::drawData (part)
        m_minefieldTable.cell(1, 0).setText(info.text[MinefieldProxy::Owner]);
        m_minefieldTable.cell(1, 1).setText(info.text[MinefieldProxy::Radius]);
        m_minefieldTable.cell(1, 2).setText(info.text[MinefieldProxy::Units]);
        m_minefieldTable.cell(1, 3).setText(info.text[MinefieldProxy::AfterDecay]);
        m_minefieldTable.cell(1, 4).setText(info.text[MinefieldProxy::LastInfo]);
        m_minefieldTable.cell(1, 5).setText(info.text[MinefieldProxy::ControlPlanet]);
        m_minefieldTable.cell(1, 6).setText(info.text[MinefieldProxy::ControlPlayer]);
        m_planetButton.setState(ui::Widget::DisabledState, info.controllingPlanetId == 0);

        m_mapWidget.setCenter(info.center);
        m_mapWidget.setZoom(1, getReductionFactor(m_mapWidget.getExtent(), info.radius));
        m_mapOverlay.setPosition(info.center, info.radius);

        // Save state for use by user input
        m_planetId = info.controllingPlanetId;
        m_minefieldId = info.minefieldId;
        m_minefieldCenter = info.center;
    }
}

void
MinefieldInfoDialog::onPassageChange(const MinefieldProxy::PassageInfo& info)
{
    afl::string::Translator& tx = m_translator;

    // ex WMinefieldPassageTile::drawData (part)
    m_passageTable.cell(1, 0).setText(afl::string::Format(tx("%d ly - %.1f%%"),           info.distance, info.normalPassageRate * 100.0));
    m_passageTable.cell(1, 1).setText(afl::string::Format(tx("%d ly - %.1f%% (cloaked)"), info.distance, info.cloakedPassageRate * 100.0));

    // Save distance for use by user input
    m_passageDistance = info.distance;
}

void
MinefieldInfoDialog::onGoto()
{
    if (m_minefieldCenter.getX() != 0) {
        executeGoToReference("(Minefield)", m_minefieldCenter);
    }
}

void
MinefieldInfoDialog::onDelete()
{
    if (m_minefieldId != 0) {
        if (ui::dialogs::MessageBox(m_translator("Do you want to remove this minefield from the starchart? "
                                                 "Note that this will not sweep the field, you just won't "
                                                 "see it any longer."),
                                    m_translator("Delete Minefield"),
                                    m_root).doYesNoDialog(m_translator))
        {
            m_proxy.erase(m_minefieldId);
        }
    }
}

void
MinefieldInfoDialog::setPassageDistance(int newDistance)
{
    m_proxy.setPassageDistance(std::max(0, std::min(1000, newDistance)));
}

void
MinefieldInfoDialog::showSweepInfo()
{
    // ex showSweepInfo
    // Retrieve mine sweep information
    client::Downlink link(m_root, m_translator);
    MinefieldProxy::SweepInfo info;
    m_proxy.getSweepInfo(link, info);
    if (info.weapons.empty()) {
        ui::dialogs::MessageBox(m_translator("Mine sweep information not available."), m_translator("Minefield Information"), m_root).doOkDialog(m_translator);
        return;
    }

    // Retrieve formatter
    util::NumberFormatter fmt(game::proxy::ConfigurationProxy(m_userSide.gameSender()).getNumberFormatter(link));

    // Build document
    const int em = m_root.provider().getFont(gfx::FontRequest())->getEmWidth();
    ui::rich::DocumentView docView(gfx::Point(20*em, 1), 0, m_root.provider());
    ui::rich::Document& doc = docView.getDocument();
    doc.setPageWidth(20 * em);
    doc.add(String_t(afl::string::Format(info.isWeb ? m_translator("To sweep %d web mines, use...") : m_translator("To sweep %d mines, use..."), fmt.formatNumber(info.units))));
    doc.addParagraph();
    for (size_t i = 0; i < info.weapons.size(); ++i) {
        doc.addRight(5*em, fmt.formatNumber(info.weapons[i].needed));
        doc.add(" " + info.weapons[i].name);
        doc.addNewline();
    }
    doc.finish();
    docView.adjustToDocumentSize();

    // Show window
    ui::dialogs::MessageBox(docView, m_translator("Minefield Information"), m_root)
        .doOkDialog(m_translator);
}


/*
 *  Main Entry Point
 */

void
client::dialogs::doMinefieldInfoDialog(client::si::UserSide& iface,
                                       ui::Root& root,
                                       afl::string::Translator& tx,
                                       client::si::OutputState& out)
{
    MinefieldInfoDialog(iface, root, tx, out)
        .run();
}
