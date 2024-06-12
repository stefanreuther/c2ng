/**
  *  \file client/tiles/shipmovementtile.cpp
  */

#include <cmath>
#include "client/tiles/shipmovementtile.hpp"
#include "afl/base/countof.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/string/format.hpp"
#include "game/game.hpp"
#include "game/map/chunnelmission.hpp"
#include "game/map/ship.hpp"
#include "game/map/shippredictor.hpp"
#include "game/proxy/objectlistener.hpp"
#include "game/turn.hpp"
#include "ui/draw.hpp"
#include "ui/layout/hbox.hpp"
#include "util/math.hpp"
#include "util/translation.hpp"

using gfx::Point;
using gfx::Rectangle;
using ui::widgets::FrameGroup;
using util::SkinColor;
using game::spec::BasicHullFunction;
using game::map::ChunnelMission;

namespace {
    enum {
        LabelColumn,
        ValueColumn
    };
    const size_t NumColumns = 2;
    const size_t NumLines = 7;

    int computeCloakFuel(const game::map::Ship& ship,
                         const game::config::HostConfiguration& config,
                         const game::UnitScoreDefinitionList& scoreDefinitions,
                         const game::spec::ShipList& shipList,
                         int eta)
    {
        // FIXME: similar function in ShipPredictor.
        // FIXME: use Hull::getCloakFuelUsage, make separate function
        if (shipList.missions().isMissionCloaking(ship.getMission().orElse(0), ship.getRealOwner().orElse(0), config)
            && (ship.hasSpecialFunction(BasicHullFunction::Cloak, scoreDefinitions, shipList, config)
                || ship.hasSpecialFunction(BasicHullFunction::HardenedCloak, scoreDefinitions, shipList, config))
            && !ship.hasSpecialFunction(BasicHullFunction::AdvancedCloak, scoreDefinitions, shipList, config))
        {
            if (game::spec::Hull* h = shipList.hulls().get(ship.getHull().orElse(0))) {
                // Regular cloaking that burns fuel
                // PHost/HOST 3.22.20 formula
                const int cfb = config[config.CloakFuelBurn](ship.getRealOwner().orElse(0));
                int fuel = h->getMass() * cfb / 100;
                if (fuel < cfb) {
                    fuel = cfb;
                }
                if (eta != 0) {
                    fuel *= eta;
                }
                return fuel;
            } else {
                return 0;
            }
        } else {
            return 0;
        }
    }

    int computeTurnFuel(const game::map::Ship& ship,
                        const game::config::HostConfiguration& config,
                        const game::spec::ShipList& shipList,
                        int eta)
    {
        // FIXME: use Hull::getTurnFuelUsage
        const game::spec::Hull* hull = shipList.hulls().get(ship.getHull().orElse(0));
        int fuel = (hull != 0
                    ? (int32_t(config[config.FuelUsagePerTurnFor100KT](ship.getRealOwner().orElse(0))) * hull->getMass() + 99) / 100
                    : 0);
        if (eta != 0) {
            fuel *= eta;
        }
        return fuel;
    }

}

class client::tiles::ShipMovementTile::Job : public util::Request<ShipMovementTile> {
 public:
    Data data;

    void handle(ShipMovementTile& t)
        { t.setData(data); }
};


client::tiles::ShipMovementTile::ShipMovementTile(ui::Root& root, afl::string::Translator& tx, gfx::KeyEventConsumer& kmw)
    : CollapsibleDataView(root),
      m_translator(tx),
      m_table(root, NumColumns, NumLines),
      m_warpButton("W", 'w', root),
      m_chartButton("A", 'a', root),
      m_queryButton("Q", 'q', root),
      m_fleetButton("F10", util::Key_F10, root),
      m_fleetFrame(ui::layout::HBox::instance0, root.colorScheme(), ui::NoFrame),
      m_receiver(root.engine().dispatcher(), *this)
{
    init(kmw);
}

void
client::tiles::ShipMovementTile::attach(game::proxy::ObjectObserver& oop)
{
    class Listener : public game::proxy::ObjectListener {
     public:
        Listener(util::RequestSender<ShipMovementTile> reply)
            : m_reply(reply)
            { }
        virtual void handle(game::Session& s, game::map::Object* obj)
            {
                // ex WShipMovementTile::drawShipMovementTile [sort-of]
                std::auto_ptr<Job> job(new Job());
                game::map::Ship* sh = dynamic_cast<game::map::Ship*>(obj);
                game::Game* g = s.getGame().get();
                game::Root* root = s.getRoot().get();
                game::spec::ShipList* shipList = s.getShipList().get();
                afl::string::Translator& tx = s.translator();
                if (sh != 0 && root != 0 && shipList != 0 && g != 0 && sh->getShipKind() == game::map::Ship::CurrentShip) {
                    game::map::ShipPredictor crystal_ball(g->viewpointTurn().universe(),
                                                          sh->getId(),
                                                          g->shipScores(),
                                                          *shipList,
                                                          g->mapConfiguration(),
                                                          root->hostConfiguration(),
                                                          root->hostVersion(),
                                                          root->registrationKey());
                    crystal_ball.addTowee();
                    crystal_ball.computeMovement();

                    ChunnelMission chd;
                    chd.check(*sh, g->viewpointTurn().universe(), g->mapConfiguration(), g->shipScores(), g->teamSettings(), *shipList, *root);
                    const bool is_chunnel = chd.getTargetId() != 0
                        && sh->getWaypointDX().orElse(0) == 0
                        && sh->getWaypointDY().orElse(0) == 0;
                    const bool is_training = shipList->missions().isExtendedMission(sh->getMission().orElse(0), game::spec::Mission::pmsn_Training, root->hostConfiguration());
                    const bool is_hyper = !is_training && sh->isHyperdriving(g->shipScores(), *shipList, root->hostConfiguration());

                    game::map::Point pos = sh->getPosition().orElse(game::map::Point());

                    // Location
                    job->data.text[Data::Location] = g->viewpointTurn().universe().findLocationName(pos,
                                                                                                    game::map::Universe::NameGravity | game::map::Universe::NameVerbose,
                                                                                                    g->mapConfiguration(),
                                                                                                    root->hostConfiguration(),
                                                                                                    root->hostVersion(),
                                                                                                    s.translator());
                    job->data.colors[Data::Location] = SkinColor::Green;

                    // Waypoint
                    game::map::Ship* otherShip = 0;
                    if (sh->getMission().orElse(0) == game::spec::Mission::msn_Intercept
                        && (otherShip = g->viewpointTurn().universe().ships().get(sh->getMissionParameter(game::InterceptParameter).orElse(0))) != 0)
                    {
                        job->data.text[Data::Waypoint] = otherShip->getName();
                        job->data.colors[Data::Waypoint] = SkinColor::Green;
                    } else if (is_chunnel && (otherShip = g->viewpointTurn().universe().ships().get(chd.getTargetId())) != 0) {
                        job->data.text[Data::Waypoint] = afl::string::Format(tx("chunnel to %s"), otherShip->getName());
                        job->data.colors[Data::Waypoint] = (chd.getFailureReasons() & ~(ChunnelMission::chf_Fuel | ChunnelMission::chf_Distance)) != 0
                            ? SkinColor::Yellow
                            : SkinColor::Green;
                    } else {
                        game::map::Point wp;
                        sh->getWaypoint().get(wp);
                        job->data.text[Data::Waypoint] = g->viewpointTurn().universe().findLocationName(wp,
                                                                                                        game::map::Universe::NameGravity | game::map::Universe::NameVerbose | game::map::Universe::NameShips,
                                                                                                        g->mapConfiguration(),
                                                                                                        root->hostConfiguration(),
                                                                                                        root->hostVersion(),
                                                                                                        s.translator());
                        job->data.colors[Data::Waypoint] = SkinColor::Green;
                    }

                    // Distance
                    int32_t dist2 = util::getDistance2FromDX(sh->getWaypointDX().orElse(0), sh->getWaypointDY().orElse(0));
                    job->data.text[Data::Distance] = afl::string::Format(tx("%.2f ly"), std::sqrt(double(dist2)));
                    job->data.colors[Data::Distance] = is_chunnel
                        ? ((chd.getFailureReasons() & chd.chf_Distance) != 0
                           ? SkinColor::Yellow
                           : SkinColor::Green)
                        : !is_chunnel
                        ? SkinColor::Green
                        : 20*20 > dist2                    // yoda conditionals because otherwise emacs thinks this is a template.
                        ? SkinColor::Red
                        : !root->hostVersion().isExactHyperjumpDistance2(dist2)
                        ? SkinColor::Yellow
                        : SkinColor::Green;

                    // Warp
                    int warpFactor = sh->getWarpFactor().orElse(0);
                    if (warpFactor == 0 && dist2 != 0) {
                        job->data.text[Data::WarpFactor] = tx("not moving");
                        job->data.colors[Data::WarpFactor] = SkinColor::Red;
                    } else {
                        if (is_hyper) {
                            job->data.text[Data::WarpFactor] = afl::string::Format(tx("Hyperdrive (warp %d)"), warpFactor);
                            job->data.colors[Data::WarpFactor] = SkinColor::Green;
                        } else if (warpFactor == 0 && !is_chunnel) {
                            job->data.text[Data::WarpFactor] = tx("not moving");
                            job->data.colors[Data::WarpFactor] = SkinColor::Green;
                        } else {
                            job->data.text[Data::WarpFactor] = afl::string::Format(tx("Warp %d"), warpFactor);
                            job->data.colors[Data::WarpFactor] = (is_chunnel && warpFactor > 0 ? SkinColor::Yellow : SkinColor::Green);
                        }
                    }

                    // E.T.A.
                    if (is_chunnel && chd.getFailureReasons() == 0) {
                        job->data.text[Data::Eta] = tx("chunnel");
                        job->data.colors[Data::Eta] = SkinColor::Green;
                    } else if ((sh->getWaypointDX().orElse(0) == 0 && sh->getWaypointDY().orElse(0) == 0)
                               && (sh->getWarpFactor().orElse(0) > 0
                                   || sh->getMission().orElse(0) != game::spec::Mission::msn_Intercept))
                    {
                        job->data.text[Data::Eta] = tx("at waypoint");
                        job->data.colors[Data::Eta] = SkinColor::Green;
                    } else if (is_hyper) {
                        job->data.text[Data::Eta] = tx("hyperjump");
                        job->data.colors[Data::Eta] = SkinColor::Green;
                    } else if (sh->getWarpFactor().orElse(0) == 0) {
                        job->data.text[Data::Eta] = tx("not moving");
                        job->data.colors[Data::Eta] = SkinColor::Red;
                    } else if (is_training) {
                        job->data.text[Data::Eta] = tx("training, won't move");
                        job->data.colors[Data::Eta] = SkinColor::Red;
                    } else if (crystal_ball.isAtTurnLimit()) {
                        job->data.text[Data::Eta] = tx("too long");
                        job->data.colors[Data::Eta] = SkinColor::Green;
                    } else {
                        job->data.text[Data::Eta] = afl::string::Format(tx("%d turn%!1{s%}"), crystal_ball.getNumTurns());
                        job->data.colors[Data::Eta] = SkinColor::Green;
                    }

                    // Fuel usage
                    /* Recompute turn/cloak fuel usage anew from eta.
                       This is how PCC1 does it.
                       - also show one turn usage for stationary ships
                       - also show full usage if predictor turned off cloaking */
                    bool footnotes = !crystal_ball.getUsedProperties().empty() || chd.getFailureReasons() != 0;
                    int eta        = crystal_ball.getNumTurns();
                    int move_fuel  = crystal_ball.getMovementFuelUsed();
                    int cloak_fuel = computeCloakFuel(*sh, root->hostConfiguration(), g->shipScores(), *shipList, eta);
                    int turn_fuel  = computeTurnFuel(*sh, root->hostConfiguration(), *shipList, eta);
                    int have_fuel  = sh->getCargo(game::Element::Neutronium).orElse(0);

                    if (is_chunnel) {
                        move_fuel = 50;
                        job->data.text[Data::FuelUsage] = afl::string::Format(tx("chunnel, %d kt"), root->userConfiguration().formatNumber(move_fuel));
                    } else {
                        job->data.text[Data::FuelUsage] = afl::string::Format(tx("%d kt"), root->userConfiguration().formatNumber(move_fuel));
                    }
                    if (cloak_fuel > 0 || turn_fuel > 0) {
                        if (turn_fuel == 0) {
                            job->data.text[Data::FuelUsage] += afl::string::Format(tx(" (+%d kt cloak)"), cloak_fuel);
                        } else {
                            job->data.text[Data::FuelUsage] += afl::string::Format(tx(" (+%d kt)"), cloak_fuel+turn_fuel);
                        }
                    }

                    if (is_chunnel) {
                        job->data.colors[Data::FuelUsage] = (chd.getFailureReasons() & ChunnelMission::chf_Fuel) != 0 ? SkinColor::Red : SkinColor::Green;
                    } else if (move_fuel > have_fuel || (have_fuel == 0 && eta > 0 && !root->hostConfiguration()[game::config::HostConfiguration::AllowNoFuelMovement]())) {
                        job->data.colors[Data::FuelUsage] = SkinColor::Red;
                    } else if (crystal_ball.isAtTurnLimit() || move_fuel+cloak_fuel+turn_fuel > have_fuel) {
                        job->data.colors[Data::FuelUsage] = SkinColor::Yellow;
                    } else {
                        job->data.colors[Data::FuelUsage] = SkinColor::Green;
                    }
                    if (footnotes) {
                        job->data.text[Data::FuelUsage] += " *";
                    }

                    // Load
                    int towee_mass = 0;
                    if (sh->getMission().orElse(0) == game::spec::Mission::msn_Tow) {
                        // FIXME: self-tow?
                        if (game::map::Ship* towee = g->viewpointTurn().universe().ships().get(sh->getMissionParameter(game::TowParameter).orElse(0))) {
                            towee->getMass(*shipList).get(towee_mass);
                        }
                    }
                    job->data.text[Data::EngineLoad] = afl::string::Format(tx("%d kt"), root->userConfiguration().formatNumber(sh->getMass(*shipList).orElse(0) + towee_mass));
                    job->data.colors[Data::EngineLoad] = SkinColor::Green;

                    // Fleet status
                    if (sh->getFleetNumber() == 0) {
                        job->data.fleetStatus = ui::NoFrame;
                    } else if (sh->isFleetLeader()) {
                        job->data.fleetStatus = ui::GreenFrame;
                    } else {
                        job->data.fleetStatus = ui::RedFrame;
                    }

                    // Prediction color
                    job->data.hasExplanation = footnotes;
                }
                m_reply.postNewRequest(job.release());
            }
     private:
        util::RequestSender<ShipMovementTile> m_reply;
    };

    oop.addNewListener(new Listener(m_receiver.getSender()));
}

void
client::tiles::ShipMovementTile::setData(const Data& data)
{
    static_assert(countof(data.text) == NumLines, "countof(text)");
    static_assert(countof(data.colors) == NumLines, "countof(colors)");

    for (size_t i = 0; i < NumLines; ++i) {
        m_table.cell(ValueColumn, i).
            setText(data.text[i]).
            setColor(ui::DARK_COLOR_SET[data.colors[i]]);
    }

    m_fleetFrame.setType(data.fleetStatus);
    m_queryButton.setState(DisabledState, !data.hasExplanation);
}

void
client::tiles::ShipMovementTile::setChildPositions()
{
    Point anchor = getAnchorPoint(LeftAligned | DataAligned);
    Rectangle area = getExtent();
    m_table.setExtent(Rectangle(anchor.getX(), anchor.getY(),
                                area.getRightX() - anchor.getX(),
                                area.getBottomY() - anchor.getY()));

    const int GRID = root().provider().getFont(gfx::FontRequest().addSize(1))->getTextHeight("Tp") * 9/8 - 4;
    m_chartButton.setExtent(Rectangle(area.getRightX() -    GRID-2, area.getBottomY() -   GRID-2, GRID, GRID));
    m_queryButton.setExtent(Rectangle(area.getRightX() -  2*GRID-5, area.getBottomY() -   GRID-2, GRID, GRID));
    m_warpButton.setExtent(Rectangle(area.getRightX() -     GRID-2, area.getBottomY() - 2*GRID-5, GRID, GRID));
    m_fleetFrame.setExtent(Rectangle(area.getRightX() - GRID*7/4-4, area.getBottomY() - 3*GRID-10, GRID*7/4+4, GRID+4));
}

gfx::Point
client::tiles::ShipMovementTile::getPreferredChildSize() const
{
    return root().provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(30, NumLines);
}

void
client::tiles::ShipMovementTile::init(gfx::KeyEventConsumer& kmw)
{
    static const char*const LABELS[] = {
        N_("Location:"),
        N_("Waypoint:"),
        N_("Distance:"),
        N_("Warp Factor:"),
        N_("E.T.A.:"),
        N_("Fuel Usage:"),
        N_("Engine Load:"),
    };
    static_assert(countof(LABELS) == NumLines, "countof(LABELS)");
    for (size_t i = 0; i < NumLines; ++i) {
        m_table.cell(LabelColumn, i).setText(m_translator(LABELS[i]));
    }
    m_table.column(LabelColumn).setColor(ui::DARK_COLOR_SET[SkinColor::Static]);
    m_table.setColumnPadding(LabelColumn, 5);

    // Add everything
    m_fleetFrame.addChild(m_fleetButton, 0);
    m_fleetFrame.setFrameWidth(2);
    addChild(m_table, 0);
    addChild(m_warpButton, 0);
    addChild(m_chartButton, 0);
    addChild(m_queryButton, 0);
    addChild(m_fleetFrame, 0);

    // Register buttons
    m_warpButton.dispatchKeyTo(kmw);
    m_chartButton.dispatchKeyTo(kmw);
    m_queryButton.dispatchKeyTo(kmw);
    m_fleetButton.dispatchKeyTo(kmw);
}
