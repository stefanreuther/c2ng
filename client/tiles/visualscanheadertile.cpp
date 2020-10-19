/**
  *  \file client/tiles/visualscanheadertile.cpp
  */

#include "client/tiles/visualscanheadertile.hpp"
#include "afl/string/format.hpp"
#include "game/game.hpp"
#include "game/map/object.hpp"
#include "game/map/ship.hpp"
#include "game/playerlist.hpp"
#include "game/proxy/objectlistener.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "game/spec/hull.hpp"
#include "gfx/context.hpp"
#include "util/translation.hpp"

using afl::string::Format;
using game::map::Ship;
using game::map::Object;
using util::SkinColor;

namespace {
    const int GAP = 3;

    void prepareContent(game::Session& session, Object* obj, client::tiles::VisualScanHeaderTile::Content& result)
    {
        afl::string::Translator& tx = session.translator();
        game::Root* pRoot = session.getRoot().get();
        game::Game* pGame = session.getGame().get();
        game::spec::ShipList* pShipList = session.getShipList().get();
        Ship* s = dynamic_cast<Ship*>(obj);

        if (pRoot != 0 && pShipList != 0 && pGame != 0 && s != 0) {
            // It's a ship
            // Line 1: Ship #1: FOOBAR
            //     or: Ship #1 (guessed position!)
            if (s->getShipKind() == Ship::GuessedShip) {
                result.title = Format(tx.translateString("Ship #%d (guessed position!)").c_str(), s->getId());
            } else {
                result.title = s->getName(game::LongName, tx, session.interface());
            }

            // Line 2: (our <ship>)
            //         (our <ship> under X control)
            //         (X <ship>)
            //         (X <ship> under Y control)
            //         (X <ship> under our control)
            // where <ship> is either 'freighter', 'starship', or 'ship'
            enum { Freighter, Starship, Ship } kind;
            int hullId = 0;
            if (s->getHull().get(hullId)) {
                if (s->getShipKind() == Ship::CurrentShip) {
                    if (s->getNumBeams().orElse(0) != 0 || s->getNumBays().orElse(0) != 0 || s->getNumLaunchers().orElse(0) != 0) {
                        kind = Starship;
                    } else {
                        kind = Freighter;
                    }
                } else if (const game::spec::Hull* h = pShipList->hulls().get(hullId)) {
                    if (h->getMaxBeams() != 0 || h->getNumBays() != 0 || h->getMaxLaunchers() != 0) {
                        kind = Starship;
                    } else {
                        kind = Freighter;
                    }
                } else {
                    kind = Ship;
                }
            } else {
                kind = Ship;
            }

            const int shipOwner = s->getRealOwner().orElse(0);
            const int viewpoint = pGame->getViewpointPlayer();

            if (shipOwner == viewpoint) {
                static const char*const table[] = {
                    N_("our freighter"),
                    N_("our starship"),
                    N_("our ship")
                };
                result.subtitle = tx.translateString(table[kind]);
            } else {
                static const char*const table[] = {
                    N_("%s freighter"),
                    N_("%s starship"),
                    N_("%s ship")
                };
                result.subtitle = Format(tx.translateString(table[kind]).c_str(),
                                         pRoot->playerList().getPlayerName(shipOwner, game::Player::AdjectiveName));
            }

            int perceivedOwner = 0;
            s->getOwner(perceivedOwner);

            if (shipOwner != perceivedOwner) {
                if (perceivedOwner == viewpoint) {
                    result.subtitle += tx.translateString(" under our control");
                } else {
                    result.subtitle += Format(tx.translateString(" under %s control").c_str(),
                                              pRoot->playerList().getPlayerName(perceivedOwner, game::Player::AdjectiveName));
                }
            }

            result.subtitleColor = pGame->teamSettings().getPlayerColor(perceivedOwner);

            // Line 3: Unknown type
            //         HULL CLASS
            //         Experienced HULL CLASS
            if (const game::spec::Hull* h = pShipList->hulls().get(hullId)) {
                // FIXME: experience levels
                result.type = h->getName(pShipList->componentNamer());
            } else {
                result.type = tx.translateString("Unknown type");
            }
        } else if (obj != 0) {
            // Something else
            result.title = obj->getName(game::PlainName, tx, session.interface());
        } else {
            // Nothing. Leave result default.initialized.
        }
    }
}


client::tiles::VisualScanHeaderTile::VisualScanHeaderTile(ui::Root& root)
    : m_root(root),
      m_content(),
      m_reply(root.engine().dispatcher(), *this)
{
    // ex WVisualScanHeaderTile::WVisualScanHeaderTile
}

client::tiles::VisualScanHeaderTile::~VisualScanHeaderTile()
{ }

void
client::tiles::VisualScanHeaderTile::draw(gfx::Canvas& can)
{
    // ex WVisualScanHeaderTile::drawData
    gfx::Context<SkinColor::Color> ctx(can, getColorScheme());
    ctx.useFont(*m_root.provider().getFont(gfx::FontRequest().addWeight(1)));
    ctx.setSolidBackground();

    const int lineHeight = ctx.getFont()->getCellSize().getY();
    gfx::Rectangle area = getExtent();

    // First line
    ctx.setColor(SkinColor::Static);
    outTextF(ctx, area.splitY(lineHeight), m_content.title);

    // Second line
    ctx.setColor(m_content.subtitleColor);
    ctx.useFont(*m_root.provider().getFont(gfx::FontRequest()));
    outTextF(ctx, area.splitY(lineHeight), m_content.subtitle);

    // Last line
    ctx.setColor(SkinColor::Static);
    area.consumeY(GAP);
    outTextF(ctx, area, m_content.type);
}

void
client::tiles::VisualScanHeaderTile::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::tiles::VisualScanHeaderTile::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{ }

ui::layout::Info
client::tiles::VisualScanHeaderTile::getLayoutInfo() const
{
    // ex WVisualScanHeaderTile::WVisualScanHeaderTile
    gfx::Point size = m_root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(20, 3);
    size.addY(GAP);
    return size;
}

bool
client::tiles::VisualScanHeaderTile::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
client::tiles::VisualScanHeaderTile::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}

void
client::tiles::VisualScanHeaderTile::setContent(const Content& content)
{
    m_content = content;
    requestRedraw();
}

void
client::tiles::VisualScanHeaderTile::attach(game::proxy::ObjectObserver& oop)
{
    class Updater : public util::Request<VisualScanHeaderTile> {
     public:
        Updater(const Content& content)
            : m_content(content)
            { }
        virtual void handle(VisualScanHeaderTile& tile)
            { tile.setContent(m_content); }
     private:
        Content m_content;
    };

    class Listener : public game::proxy::ObjectListener {
     public:
        Listener(util::RequestSender<VisualScanHeaderTile> reply)
            : m_reply(reply)
            { }
        virtual void handle(game::Session& session, game::map::Object* obj)
            {
                Content result;
                prepareContent(session, obj, result);
                m_reply.postNewRequest(new Updater(result));
            }
     private:
        util::RequestSender<VisualScanHeaderTile> m_reply;
    };

    oop.addNewListener(new Listener(m_reply.getSender()));
}
