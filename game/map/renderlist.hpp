/**
  *  \file game/map/renderlist.hpp
  */
#ifndef C2NG_GAME_MAP_RENDERLIST_HPP
#define C2NG_GAME_MAP_RENDERLIST_HPP

#include "afl/base/types.hpp"
#include "afl/string/string.hpp"
#include "game/map/point.hpp"
#include "game/map/rendererlistener.hpp"
#include "util/stringinstructionlist.hpp"

namespace game { namespace map {

    class RenderList : public RendererListener, public util::StringInstructionList {
     public:
        enum Instruction {
            riGridBorderLine,           // x1,y1,x2,y2 [inclusive]
            riGridLine,                 // x1,y1,x2,y2 [inclusive]
            riSelection,                // x,y
            riPlanet,                   // x,y,id,flags
            riShip,                     // x,y,id,rel
            riFleetLeader,              // x,y,id,rel
            riMinefield,                // x,y,id,r,isWeb,rel
            riUserCircle,               // x,y,r,color
            riUserLine,                 // x,y,x,y,color
            riUserRectangle,            // x,y,x,y,color
            riUserMarker                // x,y,shape,color,text
        };
        static const uint16_t MAX_INSTRUCTION = riUserMarker;

        class Iterator : public StringInstructionList::Iterator {
         public:
            Iterator(const RenderList& parent);

            bool readInstruction(Instruction& insn);
            bool readPointParameter(Point& value);
        };

        RenderList();
        ~RenderList();

        virtual void drawGridLine(Point a, Point b);
        virtual void drawBorderLine(Point a, Point b);
        virtual void drawSelection(Point p);
        virtual void drawPlanet(Point p, int id, int flags);
        virtual void drawShip(Point p, int id, Relation_t rel);
        virtual void drawFleetLeader(Point p, int id, Relation_t rel);
        virtual void drawMinefield(Point p, int id, int r, bool isWeb, Relation_t rel);
        virtual void drawUserCircle(Point pt, int r, int color);
        virtual void drawUserLine(Point a, Point b, int color);
        virtual void drawUserRectangle(Point a, Point b, int color);
        virtual void drawUserMarker(Point pt, int shape, int color, String_t label);

        void addInstruction(Instruction ins);
        void addPointParameter(Point pt);

        void replay(RendererListener& listener) const;

        Iterator read() const;
    };

} }

#endif
