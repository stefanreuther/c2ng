/**
  *  \file test/game/map/rendererlistenertest.cpp
  *  \brief Test for game::map::RendererListener
  */

#include "game/map/rendererlistener.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.map.RendererListener")
{
    using game::map::Point;
    class Tester : public game::map::RendererListener {
     public:
        virtual void drawGridLine(Point /*a*/, Point /*b*/)
            { }
        virtual void drawBorderLine(Point /*a*/, Point /*b*/)
            { }
        virtual void drawBorderCircle(Point /*c*/, int /*radius*/)
            { }
        virtual void drawSelection(Point /*p*/)
            { }
        virtual void drawMessageMarker(Point /*p*/)
            { }
        virtual void drawPlanet(Point /*p*/, int /*id*/, int /*flags*/, String_t /*label*/)
            { }
        virtual void drawShip(Point /*p*/, int /*id*/, Relation_t /*rel*/, int /*flags*/, String_t /*label*/)
            { }
        virtual void drawMinefield(Point /*p*/, int /*id*/, int /*r*/, bool /*isWeb*/, Relation_t /*rel*/, bool /*filled*/)
            { }
        virtual void drawUfo(Point /*p*/, int /*id*/, int /*r*/, int /*colorCode*/, int /*speed*/, int /*heading*/, bool /*filled*/)
            { }
        virtual void drawUfoConnection(Point /*a*/, Point /*b*/, int /*colorCode*/)
            { }
        virtual void drawIonStorm(Point /*p*/, int /*r*/, int /*voltage*/, int /*speed*/, int /*heading*/, bool /*filled*/)
            { }
        virtual void drawUserCircle(Point /*pt*/, int /*r*/, int /*color*/)
            { }
        virtual void drawUserLine(Point /*a*/, Point /*b*/, int /*color*/)
            { }
        virtual void drawUserRectangle(Point /*a*/, Point /*b*/, int /*color*/)
            { }
        virtual void drawUserMarker(Point /*pt*/, int /*shape*/, int /*color*/, String_t /*label*/)
            { }
        virtual void drawExplosion(Point /*p*/)
            { }
        virtual void drawShipTrail(Point /*a*/, Point /*b*/, Relation_t /*rel*/, int /*flags*/, int /*age*/)
            { }
        virtual void drawShipWaypoint(Point /*a*/, Point /*b*/, Relation_t /*rel*/)
            { }
        virtual void drawShipTask(Point /*a*/, Point /*b*/, Relation_t /*rel*/, int /*seq*/)
            { }
        virtual void drawShipVector(Point /*a*/, Point /*b*/, Relation_t /*rel*/)
            { }
        virtual void drawWarpWellEdge(Point /*a*/, Edge /*e*/)
            { }
    };
    Tester t;
}
