/**
  *  \file client/tiles/starchartheadertile.cpp
  */

#include "client/tiles/starchartheadertile.hpp"
#include "afl/string/format.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/proxy/objectlistener.hpp"
#include "game/root.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/shiplist.hpp"
#include "game/tables/temperaturename.hpp"
#include "gfx/context.hpp"
#include "util/skincolor.hpp"

using afl::string::Format;
using game::map::Ship;
using game::map::Planet;
using game::map::Object;
using util::SkinColor;

namespace {
    void prepareContent(game::Session& session, Object* obj, client::tiles::StarchartHeaderTile::Content& result)
    {
        // ex WNarrowHeaderTile::drawData
        using client::tiles::StarchartHeaderTile;
        afl::string::Translator& tx = session.translator();
        game::Root* pRoot = session.getRoot().get();
        game::spec::ShipList* pShipList = session.getShipList().get();

        // FIXME-> ResId image;
        if (obj != 0) {
            result.text[StarchartHeaderTile::Name] = obj->getName(game::PlainName, tx, session.interface());
            result.text[StarchartHeaderTile::Id]   = Format(tx("(Id #%d)"), obj->getId());

            int owner;
            if (obj->getOwner(owner) && pRoot != 0) {
                if (owner != 0) {
                    result.text[StarchartHeaderTile::Owner] = pRoot->playerList().getPlayerName(owner, game::Player::ShortName);
                } else {
                    result.text[StarchartHeaderTile::Owner] = tx("unowned");
                }
            }
        }

        if (Planet* pl = dynamic_cast<Planet*>(obj)) {
            int temp;
            if (pl->getTemperature().get(temp)) {
                // Note: xgettext will parse the following thing wrong:
                result.text[StarchartHeaderTile::Type] = Format(tx("%d" "\xC2\xB0" "F, %s"), temp, game::tables::TemperatureName(tx).get(temp));
                // FIXME: image = ResId(res::planet, pl->getTemperature(), pl->getId());
            } else {
                // FIXME: image = ResId(res::planet);
            }
            //     level = pl->unit_scores.getScore(planet_score_definitions.lookupScore(ScoreId_ExpLevel));
        } else if (Ship* sh = dynamic_cast<Ship*>(obj)) {
            int hullNr;
            game::spec::Hull* pHull = 0;
            if (sh->getHull().get(hullNr) && pShipList != 0) {
                pHull = pShipList->hulls().get(hullNr);
            }
            if (pHull != 0) {
                result.text[StarchartHeaderTile::Type] = pHull->getName(pShipList->componentNamer());
                // FIXME: image = ResId(res::ship, getHull(h).getInternalPictureNumber(), h);
            } else {
                result.text[StarchartHeaderTile::Type] = tx("Unknown type");
                // FIXME: image = ResId("nvc");
            }
            // FIXME: guessed position, controller!!!!!!!
            // FIXME: level = sh->unit_scores.getScore(ship_score_definitions.lookupScore(ScoreId_ExpLevel));

            int mass;
            if (pShipList != 0 && pRoot != 0 && sh->getMass(*pShipList).get(mass)) {
                result.text[StarchartHeaderTile::Mass] = Format(tx("%d kt"), pRoot->userConfiguration().formatNumber(mass));
            }
        } else {
            //     return;
        }

        // if (owner == 0)
        //     outTextF(ctx, x, y, w, _("unowned"));
        // else if (owner > 0)
        //     outTextF(ctx, x, y, w, player_racenames.getShortName(owner));
        // y += dy;

        // if (level >= 0)
        //     outTextF(ctx, x, y, w, getExperienceLevelName(level));
        // y += dy;
    }
}


client::tiles::StarchartHeaderTile::StarchartHeaderTile(ui::Root& root)
    : m_root(root),
      m_content(),
      m_reply(root.engine().dispatcher(), *this)
{ }

void
client::tiles::StarchartHeaderTile::draw(gfx::Canvas& can)
{
    afl::base::Ref<gfx::Font> font(m_root.provider().getFont(gfx::FontRequest()));
    int lineHeight = font->getLineHeight();

    gfx::Context<SkinColor::Color> ctx(can, getColorScheme());
    ctx.useFont(*font);

    // FIXME: image
    gfx::Rectangle area = getExtent();

    // drawFrameDown(can, GfxRect(in.x + 2, in.y + 2, 107, 95));

    // Ptr<GfxPixmap> pix = ResManager::getInstance().getPixmap(image);
    // if (pix)
    //     pix->blitSized(can, in.x + 3, in.y + 3, 105, 93);
    // else
    //     drawSolidBar(can, GfxRect(in.x + 3, in.y + 3, 105, 93), standard_colors[COLOR_BLACK]);

    ctx.setColor(SkinColor::White);
    outTextF(ctx, area.splitY(lineHeight), m_content.text[Name]);

    ctx.setColor(SkinColor::Yellow);
    outTextF(ctx, area.splitY(lineHeight), m_content.text[Id]);

    ctx.setColor(SkinColor::Static);
    for (size_t i = Type; i < NUM_LINES; ++i) {
        outTextF(ctx, area.splitY(lineHeight), m_content.text[i]);
    }
}

void
client::tiles::StarchartHeaderTile::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::tiles::StarchartHeaderTile::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{ }

ui::layout::Info
client::tiles::StarchartHeaderTile::getLayoutInfo() const
{
    return m_root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(20, NUM_LINES);
}

bool
client::tiles::StarchartHeaderTile::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
client::tiles::StarchartHeaderTile::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}

void
client::tiles::StarchartHeaderTile::setContent(const Content& content)
{
    m_content = content;
    requestRedraw();
}

void
client::tiles::StarchartHeaderTile::attach(game::proxy::ObjectObserver& oop)
{
    class Updater : public util::Request<StarchartHeaderTile> {
     public:
        Updater(const Content& content)
            : m_content(content)
            { }
        virtual void handle(StarchartHeaderTile& tile)
            { tile.setContent(m_content); }
     private:
        Content m_content;
    };

    class Listener : public game::proxy::ObjectListener {
     public:
        Listener(util::RequestSender<StarchartHeaderTile> reply)
            : m_reply(reply)
            { }
        virtual void handle(game::Session& session, game::map::Object* obj)
            {
                Content result;
                prepareContent(session, obj, result);
                m_reply.postNewRequest(new Updater(result));
            }
     private:
        util::RequestSender<StarchartHeaderTile> m_reply;
    };

    oop.addNewListener(new Listener(m_reply.getSender()));
}
