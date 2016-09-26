/**
  *  \file client/tiles/shipoverviewtile.cpp
  */

#include "client/tiles/shipoverviewtile.hpp"
#include "gfx/context.hpp"
#include "gfx/complex.hpp"
#include "afl/string/format.hpp"
#include "util/translation.hpp"
#include "util/skincolor.hpp"
#include "game/map/ship.hpp"
#include "client/objectlistener.hpp"
#include "game/root.hpp"
#include "game/game.hpp"
#include "game/spec/hull.hpp"
#include "game/turn.hpp"

namespace {
    void showNumber(gfx::Context& ctx, gfx::Rectangle& area, game::IntegerProperty_t value, const char* label)
    {
        // FIXME: magic number
        int n;
        gfx::Rectangle numberArea = area.splitX(80);
        if (value.get(n)) {
            // FIXME: numToString
            outTextF(ctx, numberArea, afl::string::Format("%d%s", n, label));
        }
    }
}

client::tiles::ShipOverviewTile::ShipOverviewTile(ui::Root& root)
    : SimpleWidget(),
      m_root(root),
      m_receiver(root.engine().dispatcher(), *this)
{
    m_strings[0] = "zero";
    m_strings[1] = "one";
    m_strings[2] = "two";
    m_strings[3] = "three";
    m_strings[4] = "four";
    m_strings[5] = "five";
    m_strings[6] = "six";
    m_strings[7] = "seven";
    m_ints[0] = 11;
    m_ints[1] = 22;
    m_ints[2] = 33;
    m_ints[3] = 44;
}

client::tiles::ShipOverviewTile::~ShipOverviewTile()
{ }

void
client::tiles::ShipOverviewTile::draw(gfx::Canvas& can)
{
    gfx::Context ctx(can);
    gfx::Rectangle area(getExtent());
    ctx.useColorScheme(getColorScheme());
    drawBackground(ctx, area);
    ctx.setColor(util::SkinColor::Static);

    // Get font
    afl::base::Ptr<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest());
    if (!font.get()) {
        return;
    }
    const int h = font->getLineHeight();
    ctx.useFont(*font);

    // Line 1: Controller + Level + Hull
    // Line 2: "Location: LOC"
    // Line 3: "Waypoint: WAYP"
    // Line 4: "(DISTANCE, SPEED)"
    for (int i = 0; i < 4; ++i) {
        outTextF(ctx, area.splitY(h), m_strings[i]);
    }
    area.consumeY(h/3);

    // Line 5: "Mission: ...."
    // Line 6: "Primary enemy: ...."
    for (int i = 4; i < 6; ++i) {
        outTextF(ctx, area.splitY(h), m_strings[i]);
    }
    area.consumeY(h/3);

    // Line 7: "Cargo:    N T D M"
    gfx::Rectangle cargoArea = area.splitY(h);
    // FIXME: magic numbers
    outTextF(ctx, cargoArea.splitX(50), _("Cargo:"));
    ctx.setTextAlign(2, 0);
    showNumber(ctx, cargoArea, m_ints[0], "N");
    showNumber(ctx, cargoArea, m_ints[1], "T");
    showNumber(ctx, cargoArea, m_ints[2], "D");
    showNumber(ctx, cargoArea, m_ints[3], "M");
    ctx.setTextAlign(0, 0);

    // Line 8: "N colonists, N mc, N supplies"
    outTextF(ctx, area.splitY(h), m_strings[6]);
    area.consumeY(h/3);

    // Line 9: "FCode: XXX"
    for (int i = 7; i < 9; ++i) {
        outTextF(ctx, area.splitY(h), m_strings[i]);
    }
}

void
client::tiles::ShipOverviewTile::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::tiles::ShipOverviewTile::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{ }

ui::layout::Info
client::tiles::ShipOverviewTile::getLayoutInfo() const
{
    afl::base::Ptr<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest().addWeight(1));
    if (font.get() != 0) {
        // magic numbers from WShipOverviewTile::WShipOverviewTile
        return font->getCellSize().scaledBy(30, 11);
    } else {
        return ui::layout::Info();
    }
}

bool
client::tiles::ShipOverviewTile::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
client::tiles::ShipOverviewTile::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}

void
client::tiles::ShipOverviewTile::setStatus(afl::base::Memory<String_t> strings, afl::base::Memory<game::IntegerProperty_t> ints)
{
    afl::base::Memory<String_t>(m_strings).copyFrom(strings);
    afl::base::Memory<game::IntegerProperty_t>(m_ints).copyFrom(ints);
    requestRedraw();
}

void
client::tiles::ShipOverviewTile::attach(ObjectObserverProxy& oop)
{
    class Job : public util::Request<ShipOverviewTile> {
     public:
        Job(game::Session& s, game::map::Object* obj//,
            // game::Root* root,
            // game::Game* g,
            // game::spec::ShipList* sl,
            // afl::string::Translator& tx
            )
            {
                game::Root* root = s.getRoot().get();
                game::Game* g = s.getGame().get();
                game::spec::ShipList* sl = s.getShipList().get();
                if (game::map::Ship* sh = dynamic_cast<game::map::Ship*>(obj)) {
                    // Line 1: Hull etc.
                    {
                        int owner;
                        int realOwner;
                        if (sh->getOwner(owner)
                            && sh->getRealOwner().get(realOwner)
                            && realOwner != owner
                            && root != 0)
                        {
                            if (game::Player* pl = root->playerList().get(realOwner)) {
                                m_strings[0] += pl->getName(game::Player::AdjectiveName);
                                m_strings[0] += " ";
                            }
                        }

                        if (g != 0) {
                            game::UnitScoreList::Index_t index;
                            int16_t value, turn;
                            if (g->shipScores().lookup(game::ScoreId_ExpLevel, index)
                                && sh->unitScores().get(index, value, turn))
                            {
                                m_strings[0] += root->hostConfiguration().getExperienceLevelName(value, s.translator());
                                m_strings[0] += " ";
                            }
                        }

                        int hull;
                        if (sh->getHull().get(hull) && sl != 0) {
                            if (const game::spec::Hull* h = sl->hulls().get(hull)) {
                                m_strings[0] += h->getName(sl->componentNamer());
                            }
                        }
                    }

                    // Line 2: Location
                    {
                        using game::map::Universe;
                        game::map::Point pt;
                        if (sh->getPosition(pt) && g != 0) {
                            m_strings[1] = afl::string::Format(s.translator().translateString("Location: %s").c_str(),
                                                               g->currentTurn().universe().getLocationName(pt,
                                                                                                           Universe::NameGravity | Universe::NameOrbit | Universe::NameVerbose,
                                                                                                           root->hostConfiguration(),
                                                                                                           root->hostVersion(),
                                                                                                           s.translator(),
                                                                                                           s.interface()));
                        }
                    }

                    // Line 3: Waypoint
                    {
                        using game::map::Universe;
                        game::map::Point pt;
                        if (sh->getWaypoint().get(pt) && g != 0) {
                            m_strings[1] = afl::string::Format(s.translator().translateString("Waypoint: %s").c_str(),
                                                               g->currentTurn().universe().getLocationName(pt,
                                                                                                           Universe::NameGravity | Universe::NameVerbose,
                                                                                                           root->hostConfiguration(),
                                                                                                           root->hostVersion(),
                                                                                                           s.translator(),
                                                                                                           s.interface()));
                        }
                    }

                    // // Line 4: Movement, speed
                    // FIXME: port this
                    // if (ps.getWaypointDX() == 0 && ps.getWaypointDY() == 0) {
                    //     line = _("at waypoint");
                    // } else {
                    //     line = format(_("%.2f ly"), distFromDX(ps.getWaypointDX(), ps.getWaypointDY()));
                    //     line += ", ";
                    //     if (ps.isHyperdriving()) {
                    //         line += _("Hyperdrive");
                    //     } else if (ps.getWarp() == 0) {
                    //         line += _("not moving");
                    //     } else {
                    //         line += format(_("warp %d"), ps.getWarp());
                    //     }
                    // }

                    // // Line 5: Mission
                    // FIXME: port this
                    // if (const GMission* msn = GMissionList::getInstance().getMissionByShip(ps)) {
                    //     line = msn->getLabel(ps);
                    // } else {
                    //     line = format("M%d I%d T%d", ps.getMission(), ps.getInterceptId(), ps.getTowId());
                    // }

                    // Line 6: PE
                    {
                        int pe;
                        if (sh->getPrimaryEnemy().get(pe) && root != 0) {
                            if (pe == 0) {
                                m_strings[5] = afl::string::Format(_("Primary Enemy: %s").c_str(), _("none"));
                            } else if (const game::Player* pl = root->playerList().get(pe)) {
                                m_strings[5] = afl::string::Format(_("Primary Enemy: %s").c_str(), pl->getName(game::Player::ShortName));
                            } else {
                                // unknown
                            }
                        }
                    }

                    // Line 7: Cargo
                    m_ints[0] = sh->getCargo(game::Element::Neutronium);
                    m_ints[1] = sh->getCargo(game::Element::Tritanium);
                    m_ints[2] = sh->getCargo(game::Element::Duranium);
                    m_ints[3] = sh->getCargo(game::Element::Molybdenum);

                    // Line 8: More cargo
                    {
                        int col, mc, sup;
                        if (sh->getCargo(game::Element::Colonists).get(col)
                            && sh->getCargo(game::Element::Money).get(mc)
                            && sh->getCargo(game::Element::Supplies).get(sup))
                        {
                            m_strings[6] = afl::string::Format(_("%d colonist%!1{s%}, %d mc, %d suppl%1{y%|ies%}").c_str(),
                                                               /*FIXME:clansToString*/(col),
                                                               /*FIXME:numToString*/(mc),
                                                               /*FIXME:numToString*/(sup));
                        }
                    }

                    // Line 9: FCode
                    {
                        String_t fc;
                        if (sh->getFriendlyCode().get(fc)) {
                            m_strings[7] = afl::string::Format(_("FCode: %s").c_str(), fc);
                        }
                    }

                    // // Line 11: Misc
                    // FIXME: port this
                    // if (ps.isFleetLeader()) {
                    //     line = format(_("Leader of fleet #%d"), ps.getFleetNumber());
                    // } else if (ps.isFleetMember()) {
                    //     line = format(_("Member of fleet #%d"), ps.getFleetNumber());
                    // } else if (ps.getRealOwner() != ps.getOwner()) {
                    //     line = format(_("a %s ship"), player_racenames.getAdjName(ps.getRealOwner()));
                    // } else {
                    //     line.clear();
                    // }
                }
            }
        void handle(ShipOverviewTile& t)
            { t.setStatus(m_strings, m_ints); }
     private:
        String_t m_strings[9];
        game::IntegerProperty_t m_ints[4];
    };
    class Listener : public ObjectListener {
     public:
        Listener(util::RequestSender<ShipOverviewTile> reply)
            : m_reply(reply)
            { }
        virtual void handle(game::Session& s, game::map::Object* obj)
            {
                m_reply.postNewRequest(new Job(s, obj));
            }
     private:
        util::RequestSender<ShipOverviewTile> m_reply;
    };

    oop.addNewListener(new Listener(m_receiver.getSender()));
}
