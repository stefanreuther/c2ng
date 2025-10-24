/**
  *  \file client/vcr/classic/renderer.hpp
  *  \brief Class client::vcr::classic::Renderer
  */
#ifndef C2NG_CLIENT_VCR_CLASSIC_RENDERER_HPP
#define C2NG_CLIENT_VCR_CLASSIC_RENDERER_HPP

#include "game/vcr/classic/eventlistener.hpp"
#include "game/vcr/classic/types.hpp"
#include "gfx/anim/controller.hpp"
#include "gfx/anim/pixmapsprite.hpp"
#include "gfx/anim/textsprite.hpp"
#include "ui/root.hpp"

namespace client { namespace vcr { namespace classic {

    /** Classic VCR renderer.
        Provides the rendering (i.e. creation of sprites for a gfx::anim::Controller), but no behaviour/timing.
        Playback is provided by feeding the appropriate ScheduledEvent callbacks into this class.

        Each callback can either update sprites, or create an animation.
        The animation is identified by an ID and can be played asynchronously. */
    class Renderer {
     public:
        typedef game::vcr::classic::Side Side_t;

        struct Range {
            int y, minX, maxX;
        };

        class ObjectSprite {
         public:
            ObjectSprite();
            void moveObject(Side_t side, gfx::Point pos);
            void create(Side_t side, gfx::Point pos, gfx::anim::Controller& ctl, afl::base::Ptr<gfx::Canvas> image);
            bool isInitialized() const;
            gfx::Point getWeaponOrigin(Side_t side, int num, int max) const;
            gfx::Point getWeaponTarget() const;
            gfx::Rectangle getExtent() const;
            void drawOutline(gfx::BaseContext& ctx) const;
         private:
            gfx::anim::PixmapSprite* m_sprite;
            std::vector<Range> m_ranges;
        };

        /** Constructor.
            @param ctl  Controller
            @param root UI root (for colorScheme(), provider())
            @param tx   Translator
            @param animationMode Animation mode (0=simple, 1=normal) */
        Renderer(gfx::anim::Controller& ctl, ui::Root& root, afl::string::Translator& tx, int animationMode);

        /** Destructor. */
        ~Renderer();

        /** Set size of battle arena.
            @param extent Screen coordinates */
        void setExtent(gfx::Rectangle extent);

        /** Place an object.
            Implements ScheduledEventConsumer::placeObject().
            @param side Side
            @param info Object information */
        void placeObject(Side_t side, const game::vcr::classic::EventListener::UnitInfo& info);

        /** Move an object.
            Implements ScheduledEvent::MoveObject.
            @param size Side
            @param pos  Position */
        void moveObject(Side_t side, int32_t pos);

        /** Start a fighter.
            Implements ScheduledEvent::StartFighter.
            @param side     Side
            @param track    Fighter track
            @param pos      Position
            @param distance Distance from base */
        void startFighter(Side_t side, int32_t track, int32_t pos, int32_t distance);

        /** Move a fighter.
            Implements ScheduledEvent::MoveFighter.
            @param side     Side
            @param track    Fighter track
            @param pos      Position
            @param distance Distance from base
            @param state    Fighter state */
        void moveFighter(Side_t side, int32_t track, int32_t pos, int32_t distance, int32_t state);

        /** Update a fighter (move/delete, after discontinuity).
            Implements ScheduledEvent::UpdateFighter.
            @param side     Side
            @param track    Fighter track
            @param pos      Position
            @param distance Distance from base
            @param state    Fighter state */
        void updateFighter(Side_t side, int32_t track, int32_t pos, int32_t distance, int32_t state);

        /** Remove a fighter.
            Implements ScheduledEvent::RemoveFighter.
            @param side     Side
            @param track    Fighter track */
        void removeFighter(Side_t side, int32_t track);

        /** Explode a fighter.
            Implements ScheduledEvent::RemoveFighter.
            @param side     Side
            @param track    Fighter track
            @param id       Animation ID */
        void explodeFighter(Side_t side, int32_t track, int id);

        /** Hit an object.
            Implements ScheduledEvent::HitObject.
            @param side        Side (object that was hit)
            @param damageDone  Damage done
            @param crewKilled  Crew killed
            @param shieldLost  Shield lost
            @param id          Animation ID */
        void hitObject(Side_t side, int32_t damageDone, int32_t crewKilled, int32_t shieldLost, int32_t id);

        /** Fire beam from ship at fighter.
            Implements ScheduledEvent::FireBeamShipFighter.
            @param side        Side (firing ship)
            @param track       Victim fighter track
            @param beamSlot    Beam slot (0-based)
            @param id          Animation ID */
        void fireBeamShipFighter(Side_t side, int32_t track, int32_t beamSlot, int32_t id);

        /** Fire beam from ship at fighter.
            Implements ScheduledEvent::FireBeamShipShip.
            @param side        Side (firing ship)
            @param beamSlot    Beam slot (0-based)
            @param id          Animation ID */
        void fireBeamShipShip(Side_t side, int32_t beamSlot, int32_t id);

        /** Fire beam from fighter at fighter.
            Implements ScheduledEvent::FireBeamFighterFighter.
            @param side        Side (firing fighter)
            @param track       Firing fighter track
            @param targetTrack Victim fighter track
            @param id          Animation ID */
        void fireBeamFighterFighter(Side_t side, int32_t track, int32_t targetTrack, int32_t id);

        /** Fire beam from fighter at ship.
            Implements ScheduledEvent::FireBeamFighterShip.
            @param side        Side (firing fighter)
            @param track       Firing fighter track
            @param id          Animation ID */
        void fireBeamFighterShip(Side_t side, int32_t track, int32_t id);

        /** Fire torpedo.
            Implements ScheduledEvent::FireTorpedo.
            @param side        Side (firing ship)
            @param launcher    Launcher (0-based)
            @param hit         Negative if torpedo misses, non-negative if hits
            @param id          Animation ID
            @param time        Travel time */
        void fireTorpedo(Side_t side, int32_t launcher, int32_t hit, int32_t id, int32_t time);

        /** Check initialisation status.
            @return true if left and right side have been placed (placeObject()) */
        bool isInitialized() const;

        /** Check whether animation is running.
            @param id Animation ID to check
            @return true if animation is running (call ctl.tick() to advance it), false if animation is not running or complete */
        bool hasAnimation(int id) const;

        /** Update time.
            Implements ScheduledEvent::UpdateTime.
            @param t Time */
        void updateTime(int32_t t);

        /** Update distance.
            Implements ScheduledEvent::UpdateDistance.
            @param d Distance */
        void updateDistance(int32_t d);

        /** Set result.
            Implements ScheduledEvent::SetResult.
            @param result Result */
        void setResult(game::vcr::classic::BattleResult_t result);

        /** Set visibility of result.
            @param flag true if result shall be visible */
        void setResultVisible(bool flag);

        /** Remove animations.
            After this, hasAnimation() will return false for all IDs between from and to, inclusive.
            @param from First Animation ID to remove
            @param to   Last Animation ID to remove */
        void removeAnimations(int32_t from, int32_t to);

     private:
        gfx::anim::Controller& m_controller;
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        gfx::Rectangle m_extent;
        int m_animationMode;

        game::vcr::classic::EventListener::UnitInfo m_objectInfo[2];
        game::vcr::classic::BattleResult_t m_result;

        ObjectSprite m_objects[2];
        std::vector<gfx::anim::PixmapSprite*> m_fighters[2];

        gfx::anim::TextSprite* m_distanceSprite;
        gfx::anim::TextSprite* m_timeSprite;
        gfx::anim::TextSprite* m_resultSprite;

        void addExplosion(gfx::Point pt, int id);
        void addShield(const ObjectSprite& obj, gfx::Point pt, int id);
        gfx::Point convertCoordinates(int x, int y);
        gfx::Point convertFighterPosition(int32_t track, int32_t pos, int32_t distance, game::vcr::classic::FighterStatus state);
        int getTrackY(int track);
        String_t formatResult() const;
        afl::base::Ptr<gfx::Canvas> getFighterImage(Side_t side, game::vcr::classic::FighterStatus st);
    };

} } }

#endif
