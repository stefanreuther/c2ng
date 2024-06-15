/**
  *  \file client/widgets/scanresult.cpp
  */

#include <cmath>
#include "client/widgets/scanresult.hpp"
#include "afl/string/format.hpp"
#include "game/game.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"
#include "util/request.hpp"
#include "util/skincolor.hpp"

using game::map::Point;
using game::map::Universe;
using util::SkinColor;

namespace {
    const int GAP = 5;

    const int NAME_FLAGS = Universe::NameShips | Universe::NameGravity | Universe::NameVerbose;
}

client::widgets::ScanResult::ScanResult(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx)
    : Widget(),
      m_root(root),
      m_gameSender(gameSender),
      m_reply(root.engine().dispatcher(), *this),
      m_table(root, 2, 2),
      m_valid(false),
      m_origin(),
      m_target()
{
    addChild(m_table, 0);
    m_table.column(0).setColor(SkinColor::Static);
    m_table.column(1).setColor(SkinColor::Green);
    m_table.cell(0, 0).setText(tx("Scan:"));
    m_table.cell(0, 1).setText(tx("Distance:"));
    m_table.setColumnPadding(0, GAP);
}

client::widgets::ScanResult::~ScanResult()
{ }

// Widget:
void
client::widgets::ScanResult::draw(gfx::Canvas& can)
{
    defaultDrawChildren(can);
}

void
client::widgets::ScanResult::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::widgets::ScanResult::requestChildRedraw(Widget& /*child*/, const gfx::Rectangle& area)
{
    requestRedraw(area);
}

void
client::widgets::ScanResult::handleChildAdded(Widget& /*child*/)
{ }

void
client::widgets::ScanResult::handleChildRemove(Widget& /*child*/)
{ }

void
client::widgets::ScanResult::handlePositionChange()
{
    doLayout();
}

void
client::widgets::ScanResult::handleChildPositionChange(Widget& /*child*/, const gfx::Rectangle& /*oldPosition*/)
{ }

ui::layout::Info
client::widgets::ScanResult::getLayoutInfo() const
{
    gfx::Point prefSize;
    for (Widget* p = getFirstChild(); p != 0; p = p->getNextSibling()) {
        ui::layout::Info theirLayout = p->getLayoutInfo();
        prefSize.extendRight(theirLayout.getPreferredSize());
        prefSize.addX(GAP);
    }
    return ui::layout::Info(prefSize, ui::layout::Info::GrowHorizontal);
}

bool
client::widgets::ScanResult::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
client::widgets::ScanResult::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}

void
client::widgets::ScanResult::addButton(Widget& w)
{
    addChild(w, 0);
    doLayout();
}

void
client::widgets::ScanResult::setPositions(game::map::Point origin, game::map::Point target)
{
    class Response : public util::Request<ScanResult> {
     public:
        Response(Point origin, Point target, String_t resultText, String_t distanceText)
            : m_origin(origin), m_target(target), m_resultText(resultText), m_distanceText(distanceText)
            { }
        virtual void handle(ScanResult& w)
            { w.setScanResult(m_origin, m_target, m_resultText, m_distanceText); }
     private:
        Point m_origin;
        Point m_target;
        String_t m_resultText;
        String_t m_distanceText;
    };

    class Query : public util::Request<game::Session> {
     public:
        Query(Point origin, Point target, util::RequestSender<ScanResult> reply)
            : m_origin(origin), m_target(target), m_reply(reply)
            { }
        virtual void handle(game::Session& session)
            {
                // ex WScanResult::update, WScanResult::drawContent (well, sort-of)
                afl::string::Translator& tx = session.translator();
                String_t resultText;

                // Location name. If we don't have a game, just leave it blank.
                game::Game* pGame = session.getGame().get();
                game::Root* pRoot = session.getRoot().get();
                if (pGame != 0 && pRoot != 0) {
                    resultText = pGame->viewpointTurn().universe().findLocationName(m_target, NAME_FLAGS, pGame->mapConfiguration(), pRoot->hostConfiguration(), pRoot->hostVersion(), tx);
                }

                // Distance
                String_t distanceText = afl::string::Format(tx("%.2f ly"), std::sqrt(double(m_origin.getSquaredRawDistance(m_target))));

                // Send response
                m_reply.postNewRequest(new Response(m_origin, m_target, resultText, distanceText));
            }
     private:
        Point m_origin;
        Point m_target;
        util::RequestSender<ScanResult> m_reply;
    };

    // Only call the netherworld if this actually is a change
    if (!m_valid || m_origin != origin || m_target != target) {
        m_valid = true;
        m_origin = origin;
        m_target = target;
        m_gameSender.postNewRequest(new Query(origin, target, m_reply.getSender()));
    }
}

void
client::widgets::ScanResult::clearPositions()
{
    m_valid = false;
    m_table.column(1).setText("...");
}

void
client::widgets::ScanResult::doLayout()
{
    gfx::Rectangle area = getExtent();
    for (Widget* p = getFirstChild(); p != 0; p = p->getNextSibling()) {
        if (p != &m_table) {
            ui::layout::Info theirLayout = p->getLayoutInfo();
            int theirWidth = theirLayout.getPreferredSize().getX();
            int theirHeight = theirLayout.getPreferredSize().getY();

            // Assign widget position along the top, starting from the right
            p->setExtent(gfx::Rectangle(area.getRightX() - theirWidth, area.getTopY(), theirWidth, theirHeight));

            // Reduce remaining space
            area.setWidth(area.getWidth() - GAP - theirLayout.getPreferredSize().getX());
        }
    }

    // Remainder goes to table
    m_table.setExtent(area);
}

void
client::widgets::ScanResult::setScanResult(game::map::Point origin, game::map::Point target, String_t resultText, String_t distanceText)
{
    // Check current state. If we call setPositions quickly in a row, we will receive multiple responses.
    // We only want to display the final, correct one.
    if (m_valid && origin == m_origin && target == m_target) {
        m_table.cell(1, 0).setText(resultText);
        m_table.cell(1, 1).setText(distanceText);
    }
}
