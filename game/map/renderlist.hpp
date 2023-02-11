/**
  *  \file game/map/renderlist.hpp
  *  \brief Class game::map::RenderList
  */
#ifndef C2NG_GAME_MAP_RENDERLIST_HPP
#define C2NG_GAME_MAP_RENDERLIST_HPP

#include "afl/base/types.hpp"
#include "afl/string/string.hpp"
#include "game/map/point.hpp"
#include "game/map/rendererlistener.hpp"
#include "util/stringinstructionlist.hpp"

namespace game { namespace map {

    /** Replayable starchart renderer.
        Implements RendererListener to store a list of calls, and allows replaying them as needed.
        The RenderList object does not contain any references and can be passed around. */
    class RenderList : public RendererListener, public util::StringInstructionList {
     public:
        /** Constructor.
            Makes an empty list. */
        RenderList();

        /** Destructor. */
        ~RenderList();

        // RendererListener
        virtual void drawGridLine(Point a, Point b);
        virtual void drawBorderLine(Point a, Point b);
        virtual void drawBorderCircle(Point c, int radius);
        virtual void drawSelection(Point p);
        virtual void drawMessageMarker(Point p);
        virtual void drawPlanet(Point p, int id, int flags, String_t label);
        virtual void drawShip(Point p, int id, Relation_t rel, int flags, String_t label);
        virtual void drawMinefield(Point p, int id, int r, bool isWeb, Relation_t rel, bool filled);
        virtual void drawUfo(Point p, int id, int r, int colorCode, int speed, int heading, bool filled);
        virtual void drawUfoConnection(Point a, Point b, int colorCode);
        virtual void drawIonStorm(Point p, int r, int voltage, int speed, int heading, bool filled);
        virtual void drawUserCircle(Point pt, int r, int color);
        virtual void drawUserLine(Point a, Point b, int color);
        virtual void drawUserRectangle(Point a, Point b, int color);
        virtual void drawUserMarker(Point pt, int shape, int color, String_t label);
        virtual void drawExplosion(Point p);
        virtual void drawShipTrail(Point a, Point b, Relation_t rel, int flags, int age);
        virtual void drawShipWaypoint(Point a, Point b, Relation_t rel);
        virtual void drawShipVector(Point a, Point b, Relation_t rel);
        virtual void drawWarpWellEdge(Point a, Edge e);

        /** Replay.
            Reproduces the calls that were made to this object, on the given listener.
            @param listener Listener */
        void replay(RendererListener& listener) const;

     private:
        enum Instruction {
            riGridBorderLine,           // x1,y1,x2,y2 [inclusive]
            riGridBorderCircle,         // x,y,r
            riGridLine,                 // x1,y1,x2,y2 [inclusive]
            riSelection,                // x,y
            riMessageMarker,            // x,y
            riPlanet,                   // x,y,id,flags
            riShip,                     // x,y,id,rel,flags
            riMinefield,                // x,y,id,r,isWeb,rel
            riUfo,                      // x,y,id,r,color,speed,heading,fill
            riUfoConnection,            // x1,y1,x2,y2,color
            riIonStorm,                 // x,y,r,voltage,speed,heading,fill
            riUserCircle,               // x,y,r,color
            riUserLine,                 // x,y,x,y,color
            riUserRectangle,            // x,y,x,y,color
            riUserMarker,               // x,y,shape,color,text
            riExplosion,                // x,y
            riShipTrail,                // x1,y1,x2,y2,rel,flags,age
            riShipWaypoint,             // x1,y1,x2,y2,rel
            riShipVector,               // x1,y1,x2,y2,rel
            riWarpWellEdge              // x,y,edge
        };
        static const uint16_t MAX_INSTRUCTION = riWarpWellEdge;

        class Iterator : public StringInstructionList::Iterator {
         public:
            Iterator(const RenderList& parent);

            bool readInstruction(Instruction& insn);
            bool readPointParameter(Point& value);
        };

        void addInstruction(Instruction ins);
        void addPointParameter(Point pt);

        Iterator read() const;
    };

} }

#endif
