/**
  *  \file client/vcr/classic/renderer.hpp
  */
#ifndef C2NG_CLIENT_VCR_CLASSIC_RENDERER_HPP
#define C2NG_CLIENT_VCR_CLASSIC_RENDERER_HPP

#include "gfx/anim/controller.hpp"
#include "ui/root.hpp"
#include "game/vcr/classic/types.hpp"
#include "game/vcr/classic/eventlistener.hpp"
#include "gfx/anim/pixmapsprite.hpp"
#include "gfx/anim/textsprite.hpp"

namespace client { namespace vcr { namespace classic {

    class Renderer {
     public:
        typedef game::vcr::classic::Side Side_t;

        Renderer(gfx::anim::Controller& ctl, ui::Root& root, afl::string::Translator& tx);
        ~Renderer();
        void setExtent(gfx::Rectangle extent);
        void placeObject(Side_t side, const game::vcr::classic::EventListener::UnitInfo& info);
        void moveObject(Side_t side, int32_t pos);
        void startFighter(Side_t side, int32_t track, int32_t pos, int32_t distance);
        void moveFighter(Side_t side, int32_t track, int32_t pos, int32_t distance, int32_t state);
        void updateFighter(Side_t side, int32_t track, int32_t pos, int32_t distance, int32_t state);
        void removeFighter(Side_t side, int32_t track);
        void explodeFighter(Side_t side, int32_t track, int id);
        void hitObject(Side_t side, int32_t damageDone, int32_t crewKilled, int32_t shieldLost, int32_t id);
        void fireBeamShipFighter(Side_t side, int32_t track, int32_t beamSlot, int32_t id);
        void fireBeamShipShip(Side_t side, int32_t beamSlot, int32_t id);
        void fireBeamFighterFighter(Side_t side, int32_t track, int32_t targetTrack, int32_t id);
        void fireBeamFighterShip(Side_t side, int32_t track, int32_t id);
        void fireTorpedo(Side_t side, int32_t launcher, int32_t hit, int32_t id, int32_t time);
        void addExplosion(gfx::Point pt, int id);
        gfx::Point convertCoordinates(int x, int y);
        gfx::Point convertFighterPosition(int32_t track, int32_t pos, int32_t distance, game::vcr::classic::FighterStatus state);
        int getTrackY(int track);
        bool isInitialized() const;
        bool hasAnimation(int id) const;
        void updateTime(int32_t t);
        void updateDistance(int32_t d);
        void setResult(game::vcr::classic::BattleResult_t result);
        void setResultVisible(bool flag);
        void removeAnimations(int32_t from, int32_t to);

     private:
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
         private:
            gfx::anim::PixmapSprite* m_sprite;
            std::vector<Range> m_ranges;
        };
        
        gfx::anim::Controller& m_controller;
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        gfx::Rectangle m_extent;

        game::vcr::classic::EventListener::UnitInfo m_objectInfo[2];
        game::vcr::classic::BattleResult_t m_result;

        ObjectSprite m_objects[2];
        std::vector<gfx::anim::PixmapSprite*> m_fighters[2];

        gfx::anim::TextSprite* m_distanceSprite;
        gfx::anim::TextSprite* m_timeSprite;
        gfx::anim::TextSprite* m_resultSprite;

        String_t formatResult() const;
        afl::base::Ptr<gfx::Canvas> getFighterImage(Side_t side, game::vcr::classic::FighterStatus st);
    };

} } }

#endif
