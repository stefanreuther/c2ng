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
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"
#include "ui/res/resid.hpp"
#include "util/skincolor.hpp"
#include "game/game.hpp"

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
        game::Game* g = session.getGame().get();
        game::spec::ShipList* pShipList = session.getShipList().get();

        game::NegativeProperty_t level;

        if (obj != 0) {
            result.text[StarchartHeaderTile::Name] = obj->getName(game::PlainName, tx, session.interface());
            result.text[StarchartHeaderTile::Id]   = Format(tx("(Id #%d)"), obj->getId());

            int owner;
            if (obj->getOwner().get(owner) && pRoot != 0) {
                if (owner != 0) {
                    result.text[StarchartHeaderTile::Owner] = pRoot->playerList().getPlayerName(owner, game::Player::ShortName, tx);
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
                result.image = ui::res::makeResourceId(ui::res::PLANET, temp, pl->getId());
            } else {
                result.image = ui::res::PLANET;
            }

            if (g != 0) {
                level = pl->getScore(game::ScoreId_ExpLevel, g->planetScores());
            }
        } else if (Ship* sh = dynamic_cast<Ship*>(obj)) {
            int hullNr;
            game::spec::Hull* pHull = 0;
            if (sh->getHull().get(hullNr) && pShipList != 0) {
                pHull = pShipList->hulls().get(hullNr);
            }
            if (pHull != 0) {
                result.text[StarchartHeaderTile::Type] = pHull->getName(pShipList->componentNamer());
                result.image = ui::res::makeResourceId(ui::res::SHIP, pHull->getInternalPictureNumber(), hullNr);
            } else {
                result.text[StarchartHeaderTile::Type] = tx("Unknown type");
                result.image = RESOURCE_ID("nvc");
            }

            if (g != 0) {
                level = sh->getScore(game::ScoreId_ExpLevel, g->shipScores());
            }

            int mass;
            if (pShipList != 0 && pRoot != 0 && sh->getMass(*pShipList).get(mass)) {
                result.text[StarchartHeaderTile::Mass] = Format(tx("%d kt"), pRoot->userConfiguration().formatNumber(mass));
            }
        } else {
            // Whatever; leave it blank
        }

        int levelValue;
        if (level.get(levelValue) && pRoot != 0) {
            result.text[StarchartHeaderTile::Level] = pRoot->hostConfiguration().getExperienceLevelName(levelValue, session.translator());
        }
    }
}


client::tiles::StarchartHeaderTile::StarchartHeaderTile(ui::Root& root)
    : m_root(root),
      m_content(),
      m_reply(root.engine().dispatcher(), *this),
      conn_imageChange(root.provider().sig_imageChange.add(this, &StarchartHeaderTile::onImageChange)),
      m_isMissingImage(false)
{ }

void
client::tiles::StarchartHeaderTile::draw(gfx::Canvas& can)
{
    afl::base::Ref<gfx::Font> font(m_root.provider().getFont(gfx::FontRequest()));
    int lineHeight = font->getLineHeight();

    gfx::Context<SkinColor::Color> ctx(can, getColorScheme());
    gfx::Context<uint8_t> ctx8(can, m_root.colorScheme());
    ctx.useFont(*font);

    gfx::Rectangle area = getExtent();

    gfx::Rectangle picArea = area.splitX(111);
    picArea.grow(-2, -2);
    ui::drawFrameDown(ctx8, picArea);
    picArea.grow(-1, -1);
    afl::base::Ptr<gfx::Canvas> pix = m_root.provider().getImage(m_content.image);
    if (pix.get() != 0) {
        ctx8.setColor(ui::Color_Black); // blitSized will use the color to fill excess (?)
        drawBackground(ctx8, picArea);
        blitSized(ctx8, picArea, *pix);
        // do NOT reset m_isMissingImage here.
        // This draw might be clipped and not actually cause the image to become visible.
    } else {
        drawSolidBar(ctx8, picArea, 0);
        m_isMissingImage = true;
    }

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
client::tiles::StarchartHeaderTile::handlePositionChange()
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
client::tiles::StarchartHeaderTile::onImageChange()
{
    if (m_isMissingImage) {
        requestRedraw();
        m_isMissingImage = false;
    }
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
