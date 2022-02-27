/**
  *  \file client/dialogs/navchartdialog.cpp
  */

#include "client/dialogs/navchartdialog.hpp"
#include "client/dialogs/planetinfodialog.hpp"
#include "client/dialogs/visualscandialog.hpp"
#include "client/downlink.hpp"
#include "client/map/movementoverlay.hpp"
#include "client/map/overlay.hpp"
#include "client/map/scanneroverlay.hpp"
#include "client/map/widget.hpp"
#include "client/widgets/helpwidget.hpp"
#include "client/widgets/referencelistbox.hpp"
#include "client/widgets/scanresult.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/proxy/chunnelproxy.hpp"
#include "game/proxy/searchproxy.hpp"
#include "game/root.hpp"
#include "game/searchquery.hpp"
#include "game/turn.hpp"
#include "gfx/complex.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/prefixargument.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"
#include "util/request.hpp"

using client::dialogs::NavChartState;
using client::dialogs::NavChartResult;
using game::SearchQuery;

namespace {
    const size_t MAX_OLD_POS = 20;

    class NavChartDialog;

    /*
     *  NavChartOverlay: Implementation of additional keystrokes and map symbols
     */
    class NavChartOverlay : public client::map::Overlay {
     public:
        NavChartOverlay(NavChartDialog& parent);

        virtual void drawBefore(gfx::Canvas& can, const client::map::Renderer& ren);
        virtual void drawAfter(gfx::Canvas& can, const client::map::Renderer& ren);
        virtual bool drawCursor(gfx::Canvas& can, const client::map::Renderer& ren);
        virtual bool handleKey(util::Key_t key, int prefix, const client::map::Renderer& ren);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons, const client::map::Renderer& ren);

     private:
        void drawCircles(gfx::Canvas& can, const client::map::Renderer& ren, game::map::Point pt);
        void onCandidateListUpdate(const game::proxy::ChunnelProxy::CandidateList& data);

        NavChartDialog& m_parent;
        afl::base::Optional<game::map::Point> m_altCenter;  ///< Alternative circle center position.
        std::vector<game::map::Point> m_oldCenters;         ///< Old center positions.

        game::proxy::ChunnelProxy m_chunnelProxy;
        game::proxy::ChunnelProxy::CandidateList m_chunnelData;
    };

    /*
     *  Dialog
     */
    class NavChartDialog {
     public:
        friend class NavChartOverlay;  // FIXME: merge? Give them a NavChartState only?

        NavChartDialog(ui::Root& root, client::si::UserSide& us, afl::string::Translator& tx, NavChartState& state, NavChartResult& result);

        void run();
        void setInitialZoom();
        void setPositions();
        void doListShips();
        void doSearchShips();
        void onToggleChunnel();
        void updateChunnelButton();
        void onOK();
        void onDoubleClick(game::map::Point pt);
        void onMove(game::map::Point pt);
        void onShipSelect(game::proxy::WaitIndicator& ind, game::Id_t id);
        game::Id_t chooseChunnelMate();

     private:
        ui::Root& m_root;
        client::si::UserSide& m_userSide;
        ui::EventLoop m_loop;
        afl::string::Translator& m_translator;
        NavChartState& m_state;
        NavChartResult& m_result;

        client::map::Widget m_mapWidget;
        client::map::ScannerOverlay m_scannerOverlay;
        client::map::MovementOverlay m_movementOverlay;
        client::widgets::ScanResult m_scanResult;
        NavChartOverlay m_navChartOverlay;
        ui::widgets::Button* m_chunnelButton;
    };

    /*
     *  Synchronous Wrapper for SearchProxy
     *
     *  SearchProxy is entirely asynchronous.
     *  We do not display a search result, so we need a synchronous version.
     */
    class SyncSearchProxy {
     public:
        SyncSearchProxy(util::RequestSender<game::Session> gameSender, util::RequestDispatcher& reply, game::proxy::WaitIndicator& ind)
            : m_proxy(gameSender, reply), m_waitIndicator(ind)
            {
                m_proxy.sig_success.add(this, &SyncSearchProxy::onSuccess);
                m_proxy.sig_error.add(this, &SyncSearchProxy::onError);
            }

        bool search(const SearchQuery& q, bool saveQuery)
            {
                m_proxy.search(q, saveQuery);
                return m_waitIndicator.wait();
            }

        const game::ref::List& result() const
            { return m_result; }

     private:
        void onSuccess(const game::ref::List& result)
            {
                m_result = result;
                m_waitIndicator.post(true);
            }
        void onError(String_t /*err*/)
            { m_waitIndicator.post(false); }

        game::proxy::SearchProxy m_proxy;
        game::proxy::WaitIndicator& m_waitIndicator;
        game::ref::List m_result;
    };

    /*
     *  Simple one-trick-pony to query a ship's status
     */
    class ShipStatusProxy {
     public:
        ShipStatusProxy()
            : m_owner(), m_position()
            { }

        void getShipStatus(game::proxy::WaitIndicator& ind, util::RequestSender<game::Session> gameSender, game::Id_t id)
            {
                // Clear status
                m_owner = 0;
                m_position = game::map::Point();

                // Task
                class Task : public util::Request<game::Session> {
                 public:
                    Task(ShipStatusProxy& parent, game::Id_t id)
                        : m_parent(parent), m_id(id)
                        { }
                    void handle(game::Session& session)
                        {
                            if (const game::Game* g = session.getGame().get()) {
                                if (const game::Turn* t = g->getViewpointTurn().get()) {
                                    if (const game::map::Ship* s = t->universe().ships().get(m_id)) {
                                        s->getOwner(m_parent.m_owner);
                                        s->getPosition(m_parent.m_position);
                                    }
                                }
                            }
                        }
                 private:
                    ShipStatusProxy& m_parent;
                    game::Id_t m_id;
                };
                Task t(*this, id);
                ind.call(gameSender, t);
            }

        int getOwner() const
            { return m_owner; }
        game::map::Point getPosition() const
            { return m_position; }
     private:
        int m_owner;
        game::map::Point m_position;
    };
}


/*
 *  NavChartOverlay
 */

NavChartOverlay::NavChartOverlay(NavChartDialog& parent)
    : m_parent(parent), m_altCenter(), m_oldCenters(),
      m_chunnelProxy(parent.m_userSide.gameSender(), parent.m_root.engine().dispatcher()),
      m_chunnelData()
{
    m_chunnelProxy.sig_candidateListUpdate.add(this, &NavChartOverlay::onCandidateListUpdate);
    if (m_parent.m_state.acceptChunnel && m_parent.m_state.shipId != 0) {
        m_chunnelProxy.postCandidateRequest(m_parent.m_state.shipId);
    }
}

void
NavChartOverlay::drawBefore(gfx::Canvas& can, const client::map::Renderer& ren)
{
    // ex WShipNavigationChart::drawPre, CAimChart.PreDraw
    if (m_parent.m_state.chunnelMode) {
        // Chunnel mode
        gfx::Context<uint8_t> ctx(can, m_parent.m_root.colorScheme());

        // Forbidden area
        if (m_chunnelData.minDistance != 0) {
            gfx::Point pt = ren.scale(m_parent.m_state.origin);
            int r = ren.scale(m_chunnelData.minDistance);

            ctx.setFillPattern(gfx::FillPattern::LTSLASH);
            ctx.setColor(ui::Color_Fire + 4);

            drawFilledCircle(ctx, pt, r);
            drawCircle(ctx, pt, r);
        }

        // Possible targets
        ctx.setColor(ui::Color_BrightCyan); /* PCC 1.x uses GREENSCALE+15 */
        const int r = std::min(20, std::max(5, ren.scale(10)));
        for (size_t i = 0, n = m_chunnelData.candidates.size(); i < n; ++i) {
            const gfx::Point pt = ren.scale(m_chunnelData.candidates[i].pos);
            ctx.setCursor(pt - gfx::Point(r, 0));
            drawLineRel(ctx, r, -r);
            drawLineRel(ctx, r, r);
            drawLineRel(ctx, -r, r);
            drawLineRel(ctx, -r, -r);
        }
    } else {
        // Regular mode, warp circles
        drawCircles(can, ren, m_parent.m_state.origin);
        if (const game::map::Point* pt = m_altCenter.get()) {
            drawCircles(can, ren, *pt);
        }
    }
}

void
NavChartOverlay::drawAfter(gfx::Canvas& /*can*/, const client::map::Renderer& /*ren*/)
{ }

bool
NavChartOverlay::drawCursor(gfx::Canvas& /*can*/, const client::map::Renderer& /*ren*/)
{
    return false;
}

bool
NavChartOverlay::handleKey(util::Key_t key, int /*prefix*/, const client::map::Renderer& /*ren*/)
{
    // ex WShipNavigationChart::handleEvent, CAimChart.Handle
    NavChartState& st = m_parent.m_state;
    switch (key) {
     case util::Key_Tab:
        if (st.target != st.center) {
            // Remember old position
            m_oldCenters.push_back(st.center);
            if (m_oldCenters.size() > MAX_OLD_POS) {
                m_oldCenters.erase(m_oldCenters.begin());
            }

            // Update
            st.center = st.target;
            m_parent.setPositions();
        }
        return true;

     case util::Key_Backspace:
        if (m_oldCenters.empty()) {
            st.center = st.origin;
        } else {
            st.center = m_oldCenters.back();
            m_oldCenters.pop_back();
        }
        m_parent.setPositions();
        return true;

     case util::Key_Insert:
     case 'y':
        // Add circle
        if (st.chunnelMode) {
            m_altCenter = st.target;
            requestRedraw();
        }
        return true;

     case util::Key_Delete:
        // Remove circle
        if (st.chunnelMode) {
            m_altCenter = afl::base::Nothing;
            requestRedraw();
        }
        return true;

     case 'l':
     case 'L':
        // List ships
        m_parent.doListShips();
        return true;

     case 's':
     case util::Key_F7:
        // Search
        m_parent.doSearchShips();
        return true;

     case util::Key_F5:
     case util::Key_F5 + util::KeyMod_Ctrl:
        // Planet info
        client::dialogs::doPlanetInfoDialog(m_parent.m_root, m_parent.m_userSide.gameSender(), st.target, m_parent.m_translator);
        return true;

     case util::Key_F5 + util::KeyMod_Shift:
        // Planet info here
        client::dialogs::doPlanetInfoDialog(m_parent.m_root, m_parent.m_userSide.gameSender(), st.origin, m_parent.m_translator);
        return true;

     default:
        return false;
    }
}

bool
NavChartOverlay::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/, const client::map::Renderer& /*ren*/)
{
    return false;
}

void
NavChartOverlay::drawCircles(gfx::Canvas& can, const client::map::Renderer& ren, game::map::Point pt)
{
    // WShipNavigationChart::drawCircles
    gfx::Context<uint8_t> ctx(can, m_parent.m_root.colorScheme());
    ctx.setColor(ui::Color_Dark);

    gfx::Point p = ren.scale(pt);
    if (m_parent.m_state.hyperjumping) {
        // HYP circle
        drawCircle(ctx, p, ren.scale(340));
        drawCircle(ctx, p, ren.scale(360));
    } else if (int r = m_parent.m_state.speed) {
        // Warp circles
        drawCircle(ctx, p, ren.scale(r));
        drawCircle(ctx, p, ren.scale(2*r));
        drawCircle(ctx, p, ren.scale(3*r));
    } else {
        // No circles
    }
}

void
NavChartOverlay::onCandidateListUpdate(const game::proxy::ChunnelProxy::CandidateList& data)
{
    if (data != m_chunnelData) {
        m_chunnelData = data;
        requestRedraw();
    }
}


/*
 *  NavChartDialog
 */

NavChartDialog::NavChartDialog(ui::Root& root,
                               client::si::UserSide& us,
                               afl::string::Translator& tx,
                               NavChartState& state,
                               NavChartResult& result)
    : m_root(root), m_userSide(us), m_loop(root), m_translator(tx), m_state(state),
      m_result(result),
      m_mapWidget(us.gameSender(), root, gfx::Point(450, 450)),  // FIXME: size
      m_scannerOverlay(root.colorScheme()),
      m_movementOverlay(root.engine().dispatcher(), us.gameSender(), m_mapWidget, tx),
      m_scanResult(root, us.gameSender(), tx),
      m_navChartOverlay(*this),
      m_chunnelButton(0)
{ }

void
NavChartDialog::run()
{
    afl::string::Translator& tx = m_translator;
    afl::base::Deleter del;

    // VBox
    //   UIFrameGroup
    //     WShipNavigationChart
    //   HBox
    //     WScanResult
    //     VBox
    //       UISpacer
    //       HBox
    //         "OK"
    //         "ESC"
    //         "Help"
    // FIXME: needs to be a BLUE_DARK_WINDOW because ScanResult is not currently skinnable
    ui::Window& win = del.addNew(new ui::Window(m_state.title, m_root.provider(), m_root.colorScheme(), ui::BLUE_DARK_WINDOW, ui::layout::VBox::instance5));

    ui::Group& g2   = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::Group& g22  = del.addNew(new ui::Group(ui::layout::VBox::instance5));
    ui::Group& g222 = del.addNew(new ui::Group(ui::layout::HBox::instance5));

    m_movementOverlay.setMode(client::map::MovementOverlay::AcceptMovementKeys, true);
    m_movementOverlay.setMode(client::map::MovementOverlay::AcceptConfigKeys, true);
    m_movementOverlay.setMode(client::map::MovementOverlay::AcceptZoomKeys, true);
    m_movementOverlay.sig_doubleClick.add(this, &NavChartDialog::onDoubleClick);
    win.add(ui::widgets::FrameGroup::wrapWidget(del, m_root.colorScheme(), ui::LoweredFrame, m_mapWidget));
    m_mapWidget.addOverlay(m_movementOverlay);
    m_mapWidget.addOverlay(m_scannerOverlay);
    m_mapWidget.addOverlay(m_navChartOverlay);
    m_movementOverlay.sig_move.add(this, &NavChartDialog::onMove);

    g2.add(m_scanResult);
    g2.add(g22);
    if (m_state.acceptChunnel) {
        ui::Group& g221 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
        ui::widgets::Button& btn = del.addNew(new ui::widgets::Button("C", 'c', m_root));
        btn.setFont(gfx::FontRequest());

        g221.add(del.addNew(new ui::Spacer()));
        g221.add(del.addNew(new ui::widgets::StaticText(m_translator("Make Chunnel"), util::SkinColor::Static, gfx::FontRequest(), m_root.provider())));
        g221.add(btn);
        g22.add(g221);
        btn.sig_fire.add(this, &NavChartDialog::onToggleChunnel);
        m_chunnelButton = &btn;
        updateChunnelButton();
    } else {
        g22.add(del.addNew(new ui::Spacer()));
    }
    g22.add(g222);

    ui::Widget& helper = del.addNew(new client::widgets::HelpWidget(m_root, tx, m_userSide.gameSender(), "pcc2:navchart"));
    if (m_state.acceptShip) {
        ui::widgets::Button& btnSearch = del.addNew(new ui::widgets::Button("S", 's', m_root));
        g222.add(btnSearch);
        btnSearch.dispatchKeyTo(m_mapWidget);   // will be handled by NavChartOverlay
    }
    ui::widgets::Button& btnOK     = del.addNew(new ui::widgets::Button(tx("F10 - OK"), util::Key_F10,    m_root));
    ui::widgets::Button& btnCancel = del.addNew(new ui::widgets::Button(tx("ESC"),      util::Key_Escape, m_root));
    ui::widgets::Button& btnHelp   = del.addNew(new ui::widgets::Button("H",            'h',              m_root));
    g222.add(btnOK);
    g222.add(btnCancel);
    g222.add(btnHelp);
    btnOK.sig_fire.add(this, &NavChartDialog::onOK);
    btnCancel.sig_fire.addNewClosure(m_loop.makeStop(0));
    btnHelp.dispatchKeyTo(helper);

    win.add(g2);
    win.add(del.addNew(new ui::PrefixArgument(m_root)));
    win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));
    win.add(helper);
    win.pack();

    setPositions();
    setInitialZoom();

    m_root.centerWidget(win);
    m_root.add(win);
    m_loop.run();

    m_chunnelButton = 0;
}

void
NavChartDialog::setInitialZoom()
{
    // ex WShipNavigationChart::setInitialZoom
    m_mapWidget.setZoomToInclude(m_state.target);
    if (m_state.hyperjumping) {
        m_mapWidget.setZoomToInclude(m_state.origin + game::map::Point(360, 360));
    }
}

void
NavChartDialog::setPositions()
{
    m_mapWidget.setCenter(m_state.center);
    m_movementOverlay.setPosition(m_state.target);
    m_movementOverlay.setLockOrigin(m_state.origin, m_state.hyperjumping);
    m_scannerOverlay.setPositions(m_state.origin, m_state.target);
    m_scanResult.setPositions(m_state.origin, m_state.target);
}

void
NavChartDialog::doListShips()
{
    client::Downlink link(m_root, m_translator);
    client::dialogs::VisualScanDialog dlg(m_userSide, m_root, m_translator);
    dlg.setTitle(m_translator("List Ships"));
    dlg.setOkName(m_translator("OK"));
    dlg.setAllowForeignShips(!m_state.requireOwnShip);

    if (dlg.loadCurrent(link, m_state.target, game::ref::List::Options_t(game::ref::List::IncludeForeignShips), m_state.excludeShip)) {
        game::Reference ref = dlg.run();
        m_result.outputState = dlg.outputState();
        if (ref.isSet()) {
            onShipSelect(link, ref.getId());
        }

        // If the dialog caused a script-side context change, but the above didn't yet confirm the dialog, cancel it.
        if (m_result.result == NavChartResult::Canceled && m_result.outputState.isValid()) {
            m_loop.stop(0);
        }
    }
}

void
NavChartDialog::doSearchShips()
{
    // ex mission.pas:SearchForShip
    if (m_state.acceptShip) {
        ui::widgets::InputLine input(30, 20, m_root);
        if (!input.doStandardDialog(m_translator("Search for ship"), m_translator("Enter ship name or Id#:"), m_translator)) {
            return;
        }

        String_t qStr = afl::string::strTrim(input.getText());
        if (qStr.empty()) {
            return;
        }

        // Make a search query
        SearchQuery q(SearchQuery::MatchName, SearchQuery::SearchObjects_t(SearchQuery::SearchShips), qStr);
        q.setPlayedOnly(m_state.requireOwnShip);

        // Search and pick first result
        client::Downlink link(m_root, m_translator);
        SyncSearchProxy proxy(m_userSide.gameSender(), m_root.engine().dispatcher(), link);
        bool ok = false;
        if (proxy.search(q, false)) {
            const game::ref::List& list = proxy.result();
            for (size_t i = 0, n = list.size(); i < n; ++i) {
                if (list[i].getType() == game::Reference::Ship && list[i].getId() != m_state.excludeShip) {
                    ok = true;
                    onShipSelect(link, list[i].getId());
                }
            }
        }

        if (!ok) {
            ui::dialogs::MessageBox(m_translator("No matching ship found."), m_translator("Search for ship"), m_root)
                .doOkDialog(m_translator);
        }
    }
}

void
NavChartDialog::onToggleChunnel()
{
    // Toggle chunnel
    if (m_state.acceptChunnel) {
        m_state.chunnelMode = !m_state.chunnelMode;
        m_mapWidget.requestRedraw();
        updateChunnelButton();
    }
}

void
NavChartDialog::updateChunnelButton()
{
    // ex WShipNavigationChart::setChunnelButton
    if (m_chunnelButton != 0) {
        m_chunnelButton->setFlag(ui::HighlightedButton, m_state.chunnelMode);
    }
}

void
NavChartDialog::onOK()
{
    if (m_state.chunnelMode) {
        // Chunnel mode: find potential mates and pick one
        // ex chooseChunnelMate
        game::Id_t id = chooseChunnelMate();
        if (id != 0) {
            m_result.result = NavChartResult::Chunnel;
            m_result.position = m_state.target;
            m_result.shipId = id;
            m_loop.stop(0);
        }
    } else if (m_state.acceptLocation) {
        // Just accept this location
        m_result.result = NavChartResult::Location;
        m_result.position = m_state.target;
        m_result.shipId = 0;
        m_loop.stop(0);
    } else if (m_state.acceptShip) {
        // Accept ship
        doListShips();
    } else {
        // nix
    }
}

void
NavChartDialog::onDoubleClick(game::map::Point /*pt*/)
{
    onOK();
}

void
NavChartDialog::onMove(game::map::Point pt)
{
    m_state.target = pt;
    setPositions();
}

void
NavChartDialog::onShipSelect(game::proxy::WaitIndicator& ind, game::Id_t id)
{
    // ex WShipArgWindow::onShipSelect
    // FIXME: handle exceptions
    // FIXME: handle chunnelMode?
    if (m_state.acceptShip) {
        if (id == m_state.excludeShip) {
            // Invalid: this ship selected, but not allowed
            return;
        }

        // Query ship status
        ShipStatusProxy proxy;
        proxy.getShipStatus(ind, m_userSide.gameSender(), id);
        game::map::Point target = proxy.getPosition();
        int owner = proxy.getOwner();

        // Verify owner
        if (m_state.requireOwnShip) {
            proxy.getShipStatus(ind, m_userSide.gameSender(), m_state.shipId);
            if (proxy.getOwner() != owner) {
                // Invalid: foreign ship selected, but not allowed
                return;
            }
        }

        // Preconditions passed
        m_result.result = NavChartResult::Ship;
        m_result.position = target;
        m_result.shipId = id;
        m_loop.stop(0);
    }
}

game::Id_t
NavChartDialog::chooseChunnelMate()
{
    // ex CAimChart.PickFirecloud (sort-of)
    game::ref::UserList list;
    client::Downlink link(m_root, m_translator);
    game::proxy::ChunnelProxy(m_userSide.gameSender(), m_root.engine().dispatcher()).getCandidates(link, m_state.shipId, m_state.target, list);

    if (list.empty()) {
        ui::dialogs::MessageBox(m_translator("There are no potential chunnel mates at the current position."), m_translator("Chunnel"), m_root).doOkDialog(m_translator);
        return 0;
    } else if (list.size() == 1) {
        return list.get(0)->reference.getId();
    } else {
        client::widgets::ReferenceListbox box(m_root);
        box.setContent(list);
        box.setNumLines(10);
        box.setWidth(m_root.provider().getFont(gfx::FontRequest())->getEmWidth() * 20);
        if (ui::widgets::doStandardDialog(m_translator("Chunnel"), String_t(), box, false, m_root, m_translator)) {
            return box.getCurrentReference().getId();
        } else {
            return 0;
        }
    }
}

/*
 *  Main Entry Point
 */

void
client::dialogs::doNavigationChart(NavChartResult& result,
                                   NavChartState& in,
                                   client::si::UserSide& us,
                                   ui::Root& root,
                                   afl::string::Translator& tx)
{
    // ex doShipNavigationChart, doGenericNavigationChart
    NavChartDialog(root, us, tx, in, result).run();
}
