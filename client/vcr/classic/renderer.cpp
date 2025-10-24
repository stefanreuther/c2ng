/**
  *  \file client/vcr/classic/renderer.cpp
  *  \brief Class client::vcr::classic::Renderer
  */

#include "client/vcr/classic/renderer.hpp"

#include <cassert>
#include "afl/string/format.hpp"
#include "client/vcr/beamsprite.hpp"
#include "client/vcr/torpedosprite.hpp"
#include "game/vcr/classic/algorithm.hpp"
#include "game/vcr/classic/utils.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "gfx/gen/explosionrenderer.hpp"
#include "gfx/scan.hpp"
#include "ui/colorscheme.hpp"
#include "ui/res/resid.hpp"

namespace gvc = game::vcr::classic;
using afl::string::Format;

namespace {
    /* Z coordinates: highest value drawn last = on top */
    const int Z_TEXT    = 6;
    const int Z_TORP    = 5;
    const int Z_BEAM    = 4;
    const int Z_BANG    = 3;
    const int Z_FTR     = 2;
    const int Z_OBJECTS = 1;
    const int Z_SCENE   = 0;

    String_t makeUnitResource(gvc::Side side, bool isPlanet, int shipPictureNumber)
    {
        if (isPlanet) {
            return "vcr.planet";
        } else {
            return Format("%s.%d", (side==gvc::LeftSide ? ui::res::VCR_LSHIP : ui::res::VCR_RSHIP), shipPictureNumber);
        }
    }

    class ExplosionSprite : public gfx::anim::Sprite {
     public:
        ExplosionSprite(ui::ColorScheme& cs)
            : Sprite(),
              m_state(0),
              m_colors(cs)
            { setExtent(gfx::Rectangle(0, 0, 12, 12)); }

        virtual void draw(gfx::Canvas& can)
            {
                gfx::Context<uint8_t> ctx(can, m_colors);
                if (m_state < 3) {
                    ctx.setColor(ui::Color_White);
                    drawCircle(ctx, getCenter(), 3);
                } else if (m_state < 6) {
                    ctx.setColor(ui::Color_Yellow);
                    drawCircle(ctx, getCenter(), 4);
                } else {
                    ctx.setColor(ui::Color_Red);
                    drawCircle(ctx, getCenter(), 5);
                }
            }

        virtual void tick()
            {
                if (++m_state > 10) {
                    markForDeletion();
                } else {
                    markChanged();
                }
            }

     private:
        int m_state;
        ui::ColorScheme& m_colors;
    };

    class ShieldSprite : public gfx::anim::Sprite {
     public:
        static const int SIZE = 35;
        ShieldSprite(ui::ColorScheme& cs)
            : Sprite(),
              m_state(0),
              m_colors(cs)
            { setExtent(gfx::Rectangle(0, 0, 2*SIZE+1, 2*SIZE+1)); }

        virtual void draw(gfx::Canvas& can)
            {
                gfx::Context<uint8_t> ctx(can, m_colors);
                ctx.setColor(static_cast<uint8_t>(ui::Color_Shield + 15 - m_state));
                drawCircle(ctx, getCenter(), SIZE);
            }

        virtual void tick()
            {
                if (++m_state > 10) {
                    markForDeletion();
                } else {
                    markChanged();
                }
            }

     private:
        int m_state;
        ui::ColorScheme& m_colors;
    };

    class GeneratedExplosionSprite : public gfx::anim::Sprite {
     public:
        GeneratedExplosionSprite()
            : Sprite(),
              m_rng(77),
              m_renderer(gfx::Point(12, 12), 6, 7, m_rng),
              m_canvas()
            { setExtent(gfx::Rectangle(0, 0, 12, 12)); }

        virtual void draw(gfx::Canvas& can)
            {
                if (m_canvas.get() != 0) {
                    can.blit(getExtent().getTopLeft(), *m_canvas, gfx::Rectangle(0, 0, 12, 12));
                }
            }

        virtual void tick()
            {
                if (m_renderer.hasMoreFrames()) {
                    m_canvas = m_renderer.renderFrame().asPtr();
                    markChanged();
                } else {
                    m_canvas = 0;
                    markForDeletion();
                }
            }

     private:
        util::RandomNumberGenerator m_rng;
        gfx::gen::ExplosionRenderer m_renderer;
        afl::base::Ptr<gfx::Canvas> m_canvas;
    };

    class GeneratedShieldSprite : public gfx::anim::Sprite {
     public:
        static const int SIZE = 35;
        GeneratedShieldSprite(ui::ColorScheme& cs, const client::vcr::classic::Renderer::ObjectSprite& obj)
            : Sprite(),
              m_state(0),
              m_colors(cs),
              m_object(obj)
            { setPosition(); }

        virtual void draw(gfx::Canvas& can)
            {
                gfx::Context<uint8_t> ctx(can, m_colors);
                ctx.setColor(static_cast<uint8_t>(ui::Color_Shield + 15 - m_state));
                m_object.drawOutline(ctx);
            }

        virtual void tick()
            {
                setPosition();
                if (++m_state > 10) {
                    markForDeletion();
                } else {
                    markChanged();
                }
            }

     private:
        int m_state;
        ui::ColorScheme& m_colors;
        const client::vcr::classic::Renderer::ObjectSprite& m_object;

        void setPosition()
            {
                gfx::Rectangle r = m_object.getExtent();
                r.grow(1, 1);
                setExtent(r);
            }
    };
}

/******************************* ObjectSprite ******************************/

client::vcr::classic::Renderer::ObjectSprite::ObjectSprite()
    : m_sprite(0), m_ranges()
{ }

void
client::vcr::classic::Renderer::ObjectSprite::moveObject(Side_t side, gfx::Point pos)
{
    (void) side;
    if (m_sprite != 0) {
        m_sprite->setCenter(pos);
    }
}

void
client::vcr::classic::Renderer::ObjectSprite::create(Side_t side, gfx::Point pos, gfx::anim::Controller& ctl, afl::base::Ptr<gfx::Canvas> image)
{
    // Create and configure sprite
    if (m_sprite == 0) {
        m_sprite = ctl.addNew(new gfx::anim::PixmapSprite(0));
    }
    m_sprite->setPixmap(image);
    m_sprite->setZ(Z_OBJECTS);
    moveObject(side, pos);

    // Determine sprite metrics
    m_ranges.clear();
    if (image.get() != 0) {
        Range r = { 0, 0, 0 };
        while (scanCanvas(*image, r.y, r.minX, r.maxX)) {
            m_ranges.push_back(r);
            ++r.y;
        }
    }
}

inline bool
client::vcr::classic::Renderer::ObjectSprite::isInitialized() const
{
    return m_sprite != 0;
}

gfx::Point
client::vcr::classic::Renderer::ObjectSprite::getWeaponOrigin(Side_t side, int num, int max) const
{
    // ex VcrSpriteVisualizer::findMountpoints, distributeMountpoints
    if (m_sprite == 0) {
        // Uninitialized
        return gfx::Point();
    } else if (num < 0 || max < 0 || num >= max || m_ranges.empty()) {
        // Degenerate cases: inconsistent user request, or pixmap is empty
        return m_sprite->getCenter();
    } else {
        // OK, we can determine an origin
        size_t index = size_t(num) * m_ranges.size() / size_t(max);
        assert(index < m_ranges.size());

        // Use right side of pixmap for LeftSide, left side for RightSide
        gfx::Point anchor = m_sprite->getExtent().getTopLeft();
        if (side == gvc::LeftSide) {
            return anchor + gfx::Point(m_ranges[index].maxX-1, m_ranges[index].y);
        } else {
            return anchor + gfx::Point(m_ranges[index].minX, m_ranges[index].y);
        }
    }
}

gfx::Point
client::vcr::classic::Renderer::ObjectSprite::getWeaponTarget() const
{
    if (m_sprite == 0) {
        return gfx::Point();
    } else {
        return m_sprite->getCenter();
    }
}

gfx::Rectangle
client::vcr::classic::Renderer::ObjectSprite::getExtent() const
{
    if (m_sprite == 0) {
        return gfx::Rectangle();
    } else {
        return m_sprite->getExtent();
    }
}

void
client::vcr::classic::Renderer::ObjectSprite::drawOutline(gfx::BaseContext& ctx) const
{
    if (m_sprite != 0) {
        const int origX = m_sprite->getExtent().getLeftX();
        const int origY = m_sprite->getExtent().getTopY();
        for (size_t i = 0; i < m_ranges.size(); ++i) {
            const Range& me = m_ranges[i];
            bool isFirst = (i == 0 || me.y != m_ranges[i-1].y+1);
            bool isLast = (i+1 >= m_ranges.size() || me.y+1 != m_ranges[i+1].y);

            // First in a bunch
            if (isFirst) {
                drawHLine(ctx, me.minX + origX, me.y + origY-1, me.maxX + origX);
            }

            // Sides
            int minX = me.minX;
            int maxX = me.maxX;
            if (!isFirst) {
                minX = std::min(minX, m_ranges[i-1].minX);
                maxX = std::max(maxX, m_ranges[i-1].maxX);
            }
            if (!isLast) {
                minX = std::min(minX, m_ranges[i+1].minX);
                maxX = std::max(maxX, m_ranges[i+1].maxX);
            }
            drawHLine(ctx, minX + origX-1, me.y + origY, me.minX + origX-1);
            drawHLine(ctx, maxX + origX+1, me.y + origY, me.maxX + origX+1);

            // Last in a bunch
            if (isLast) {
                drawHLine(ctx, me.minX + origX, me.y + origY+1, me.maxX + origX);
            }
        }
    }
}


/******************************** Renderer *******************************/

client::vcr::classic::Renderer::Renderer(gfx::anim::Controller& ctl, ui::Root& root, afl::string::Translator& tx, int animationMode)
    : m_controller(ctl),
      m_root(root),
      m_translator(tx),
      m_extent(),
      m_animationMode(animationMode),
      m_result(),
      m_distanceSprite(m_controller.addNew(new gfx::anim::TextSprite(root.provider()))),
      m_timeSprite(m_controller.addNew(new gfx::anim::TextSprite(root.provider()))),
      m_resultSprite()
{
    m_distanceSprite->setColor(root.colorScheme().getColor(ui::Color_White));
    m_timeSprite->setColor(root.colorScheme().getColor(ui::Color_White));

    m_distanceSprite->setFont(gfx::FontRequest().setStyle(1));
    m_timeSprite->setFont(gfx::FontRequest().setStyle(1));

    m_distanceSprite->setTextAlign(gfx::LeftAlign, gfx::BottomAlign);
    m_timeSprite->setTextAlign(gfx::RightAlign, gfx::BottomAlign);

    m_distanceSprite->setZ(Z_TEXT);
    m_timeSprite->setZ(Z_TEXT);
}

client::vcr::classic::Renderer::~Renderer()
{ }

void
client::vcr::classic::Renderer::setExtent(gfx::Rectangle extent)
{
    m_extent = extent;
    m_distanceSprite->setPosition(gfx::Point(extent.getLeftX(), extent.getBottomY()));
    m_timeSprite->setPosition(gfx::Point(extent.getRightX(), extent.getBottomY()));
}

void
client::vcr::classic::Renderer::placeObject(Side_t side, const game::vcr::classic::EventListener::UnitInfo& info)
{
    m_objectInfo[side] = info;
    m_objects[side].create(side, convertCoordinates(info.position, 0), m_controller, m_root.provider().getImage(makeUnitResource(side, m_objectInfo[side].object.isPlanet(), m_objectInfo[side].object.getPicture())));
}

void
client::vcr::classic::Renderer::moveObject(Side_t side, int32_t pos)
{
    m_objects[side].moveObject(side, convertCoordinates(pos, 0));
}

void
client::vcr::classic::Renderer::startFighter(Side_t side, int32_t track, int32_t pos, int32_t distance)
{
    while (m_fighters[side].size() <= size_t(track)) {
        m_fighters[side].push_back(0);
    }
    if (m_fighters[side][track] != 0) {
        m_fighters[side][track]->markForDeletion();
    }
    m_fighters[side][track] = new gfx::anim::PixmapSprite(0);
    m_controller.addNewSprite(m_fighters[side][track]);
    m_fighters[side][track]->setPixmap(getFighterImage(side, gvc::FighterAttacks));
    m_fighters[side][track]->setCenter(convertFighterPosition(track, pos, distance, gvc::FighterAttacks));
    m_fighters[side][track]->setZ(Z_FTR);
}

void
client::vcr::classic::Renderer::moveFighter(Side_t side, int32_t track, int32_t pos, int32_t distance, int32_t state)
{
    gvc::FighterStatus fs = gvc::FighterStatus(state);
    if (m_fighters[side].size() > size_t(track) && m_fighters[side][track] != 0) {
        m_fighters[side][track]->setCenter(convertFighterPosition(track, pos, distance, fs));
        m_fighters[side][track]->setPixmap(getFighterImage(side, fs));
    }
}

void
client::vcr::classic::Renderer::updateFighter(Side_t side, int32_t track, int32_t pos, int32_t distance, int32_t state)
{
    if (state == game::vcr::classic::FighterIdle) {
        // Fighter is gone; same as removeFighter
        removeFighter(side, track);
    } else {
        // Fighter exists; create or move
        startFighter(side, track, pos, distance);
        moveFighter(side, track, pos, distance, state);
    }
}

void
client::vcr::classic::Renderer::removeFighter(Side_t side, int32_t track)
{
    if (m_fighters[side].size() > size_t(track) && m_fighters[side][track] != 0) {
        m_fighters[side][track]->markForDeletion();
        m_fighters[side][track] = 0;
    }
}

void
client::vcr::classic::Renderer::explodeFighter(Side_t side, int32_t track, int id)
{
    if (m_fighters[side].size() > size_t(track) && m_fighters[side][track] != 0) {
        addExplosion(m_fighters[side][track]->getCenter(), id);
    }
}

void
client::vcr::classic::Renderer::hitObject(Side_t side, int32_t damageDone, int32_t crewKilled, int32_t shieldLost, int32_t id)
{
    if (m_objects[side].isInitialized() != 0) {
        if (damageDone > 0 || crewKilled > 0) {
            addExplosion(m_objects[side].getWeaponTarget(), id);
        } else if (shieldLost > 0) {
            addShield(m_objects[side], m_objects[side].getWeaponTarget(), id);
        } else {
        }
    }
}

void
client::vcr::classic::Renderer::fireBeamShipFighter(Side_t side, int32_t track, int32_t beamSlot, int32_t id)
{
    Side_t opp = flipSide(side);
    if (m_fighters[opp].size() > size_t(track) && m_fighters[opp][track] != 0 && m_objects[side].isInitialized() != 0) {
        gfx::anim::Sprite* p = new BeamSprite(m_root.colorScheme(),
                                              m_objects[side].getWeaponOrigin(side, beamSlot, m_objectInfo[side].object.getNumBeams()),
                                              m_fighters[opp][track]->getCenter());
        m_controller.addNewSprite(p);
        p->setId(id);
        p->setZ(Z_BEAM);
    }
}

void
client::vcr::classic::Renderer::fireBeamShipShip(Side_t side, int32_t beamSlot, int32_t id)
{
    Side_t opp = flipSide(side);
    if (m_objects[side].isInitialized() && m_objects[opp].isInitialized()) {
        gfx::anim::Sprite* p = new BeamSprite(m_root.colorScheme(),
                                              m_objects[side].getWeaponOrigin(side, beamSlot, m_objectInfo[side].object.getNumBeams()),
                                              m_objects[opp].getWeaponTarget());
        m_controller.addNewSprite(p);
        p->setId(id);
        p->setZ(Z_BEAM);
    }
}

void
client::vcr::classic::Renderer::fireBeamFighterFighter(Side_t side, int32_t track, int32_t targetTrack, int32_t id)
{
    Side_t opp = flipSide(side);
    if (m_fighters[side].size() > size_t(track) && m_fighters[side][track] != 0) {
        if (m_fighters[opp].size() > size_t(targetTrack) && m_fighters[opp][targetTrack] != 0) {
            gfx::anim::Sprite* p = new BeamSprite(m_root.colorScheme(),
                                                  m_fighters[side][track]->getCenter(),
                                                  m_fighters[opp][targetTrack]->getCenter());
            m_controller.addNewSprite(p);
            p->setId(id);
            p->setZ(Z_BEAM);
        }
    }
}

void
client::vcr::classic::Renderer::fireBeamFighterShip(Side_t side, int32_t track, int32_t id)
{
    Side_t opp = flipSide(side);
    if (m_fighters[side].size() > size_t(track) && m_fighters[side][track] != 0 && m_objects[opp].isInitialized()) {
        gfx::anim::Sprite* p = new BeamSprite(m_root.colorScheme(),
                                              m_fighters[side][track]->getCenter(),
                                              m_objects[opp].getWeaponTarget());
        m_controller.addNewSprite(p);
        p->setId(id);
        p->setZ(Z_BEAM);
    }
}

void
client::vcr::classic::Renderer::fireTorpedo(Side_t side, int32_t launcher, int32_t /*hit*/, int32_t id, int32_t time)
{
    if (m_objects[side].isInitialized() && m_objects[!side].isInitialized()) {
        gfx::Point a = m_objects[side].getWeaponOrigin(side, launcher, m_objectInfo[side].object.getNumLaunchers());
        gfx::Point b = m_objects[flipSide(side)].getWeaponTarget();
        gfx::anim::Sprite* p = new TorpedoSprite(m_root.colorScheme(), a, b, time);
        m_controller.addNewSprite(p);
        p->setId(id);
        p->setZ(Z_TORP);
    }
}

void
client::vcr::classic::Renderer::addExplosion(gfx::Point pt, int id)
{
    gfx::anim::Sprite* p;
    if (m_animationMode == 0) {
        p = new ExplosionSprite(m_root.colorScheme());
    } else {
        p = new GeneratedExplosionSprite();
    }
    m_controller.addNewSprite(p);
    p->setCenter(pt);
    p->setId(id);
    p->setZ(Z_BANG);
}

void
client::vcr::classic::Renderer::addShield(const ObjectSprite& obj, gfx::Point pt, int id)
{
    gfx::anim::Sprite* p;
    if (m_animationMode == 0) {
        p = new ShieldSprite(m_root.colorScheme());
    } else {
        p = new GeneratedShieldSprite(m_root.colorScheme(), obj);
    }
    m_controller.addNewSprite(p);
    p->setCenter(pt);
    p->setId(id);
    p->setZ(Z_BANG);
}

gfx::Point
client::vcr::classic::Renderer::convertCoordinates(int x, int y)
{
    return gfx::Point(m_extent.getLeftX() + x*m_extent.getWidth() / gvc::Algorithm::MAX_COORDINATE,
                      m_extent.getTopY() + (100+y)*m_extent.getHeight() / 200);
}

gfx::Point
client::vcr::classic::Renderer::convertFighterPosition(int32_t track, int32_t pos, int32_t distance, game::vcr::classic::FighterStatus state)
{
    /* The limit defines the release angle of fighters.
       The "distance" parameter reports a fighter's distance to their base.
       "limit = distance * 2" provides a game-like appearance, with fighters being quickly placed next to their base; this is similar to PCC2.
       For comparison,
       - vcr.exe, PVCR use no limit, placing fighters next to their base with no animation which looks pretty dull.
       - PCC2 Web uses a sinoid curve, which looks more life-like than game-like, but needs unit locations as input
       - "limit = distance/2" looks pretty half-baked.

       It is important to compute the position only from data that can be provided by the player (model).
       PCC2 produced the release angle by limiting movement using previous tick's sprite position,
       which would need a elaborate view-side state tracking for FF/REW. */
    int y = getTrackY(track);
    int limit = distance * 2;
    if (state == gvc::FighterReturns) {
        limit += 10;
    }
    if (y < -limit) {
        y = -limit;
    }
    if (y > limit) {
        y = limit;
    }
    return convertCoordinates(pos, y);
}

int
client::vcr::classic::Renderer::getTrackY(int track)
{
    // ex VcrFighterSprite::getTrackY
    int t19 = track % 19;
    if (t19 & 1) {
        t19 = -1 - t19/2;        // 1 .. 17 -> -1 .. -9
    } else {
        t19 = t19/2;             // 0 .. 18 -> 0 .. 9
    }

    return 9*t19 + 2*(track / 19);
}

bool
client::vcr::classic::Renderer::isInitialized() const
{
    return m_objects[0].isInitialized() && m_objects[1].isInitialized();
}

bool
client::vcr::classic::Renderer::hasAnimation(int id) const
{
    return m_controller.findSpriteById(id) != 0;
}

void
client::vcr::classic::Renderer::updateTime(int32_t t)
{
    // ex VcrTimeSprite::draw (sort-of)
    m_timeSprite->setText(Format(m_translator.translateString("Time: %3d:%02d").c_str(), t/60, t%60));
}

void
client::vcr::classic::Renderer::updateDistance(int32_t d)
{
    // ex VcrDistanceSprite::draw (sort-of)
    m_distanceSprite->setText(Format(m_translator.translateString("Distance: %5d m").c_str(), d));
}

void
client::vcr::classic::Renderer::setResult(game::vcr::classic::BattleResult_t result)
{
    m_result = result;
    if (m_resultSprite != 0) {
        m_resultSprite->setText(formatResult());
    }
}

void
client::vcr::classic::Renderer::setResultVisible(bool flag)
{
    if (flag) {
        if (m_resultSprite == 0) {
            m_resultSprite = new gfx::anim::TextSprite(m_root.provider());
            m_controller.addNewSprite(m_resultSprite);
            m_resultSprite->setFont(gfx::FontRequest().addSize(1));
            m_resultSprite->setPosition(gfx::Point(m_extent.getCenter().getX(), m_extent.getTopY() + 10));
            m_resultSprite->setTextAlign(gfx::CenterAlign, gfx::TopAlign);
            m_resultSprite->setColor(m_root.colorScheme().getColor(ui::Color_White));
            m_resultSprite->setText(formatResult());
            m_resultSprite->setZ(Z_TEXT);
        }
    } else {
        if (m_resultSprite != 0) {
            m_resultSprite->markForDeletion();
            m_resultSprite = 0;
        }
    }
}

void
client::vcr::classic::Renderer::removeAnimations(int32_t from, int32_t to)
{
    // Our sprites have ID 0. Bad things happen if someone deletes them, so refuse that.
    m_controller.deleteSpritesById(std::max(1, from), to);
}

String_t
client::vcr::classic::Renderer::formatResult() const
{
    return game::vcr::classic::formatBattleResult(m_result,
                                                  m_objectInfo[0].object.getNonEmptyName(m_translator), m_objectInfo[0].relation,
                                                  m_objectInfo[1].object.getNonEmptyName(m_translator), m_objectInfo[1].relation,
                                                  String_t(), m_translator);
}

afl::base::Ptr<gfx::Canvas>
client::vcr::classic::Renderer::getFighterImage(Side_t side, game::vcr::classic::FighterStatus st)
{
    const char* tpl = (st == (side == gvc::LeftSide ? gvc::FighterAttacks : gvc::FighterReturns)
                       ? "vcr.lftr%d"
                       : "vcr.rftr%d");
    return m_root.provider().getImage(afl::string::Format(tpl, m_objectInfo[side].object.getRace()));
}
