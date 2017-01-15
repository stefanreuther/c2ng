/**
  *  \file game/map/renderlist.hpp
  */
#ifndef C2NG_GAME_MAP_RENDERLIST_HPP
#define C2NG_GAME_MAP_RENDERLIST_HPP

#include <vector>
#include "afl/base/types.hpp"
#include "afl/string/string.hpp"
#include "game/map/point.hpp"
#include "game/map/rendererlistener.hpp"

namespace game { namespace map {

    class RenderList : public RendererListener {
     public:
        // FIXME: this is two classes in one: a replayable RendererListener and a serialized instruction storage.
        // It makes sense to separate the two for testability.
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
        static const int MAX_INSTRUCTION = riUserMarker;

        class Iterator {
         public:
            Iterator(const RenderList& parent);

            bool readInstruction(Instruction& insn);
            bool readParameter(int& value);
            bool readStringParameter(String_t& value);
            bool readPointParameter(Point& value);

         private:
            const RenderList& m_parent;
            size_t m_nextInstruction;
            size_t m_nextParameter;
            size_t m_numParameters;
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
        void addParameter(int16_t par);
        void addStringParameter(String_t s);
        void addPointParameter(Point pt);

        void replay(RendererListener& listener) const;

        size_t getNumInstructions() const;

        void clear();

        Iterator read() const;

     private:
        std::vector<int16_t> m_instructions;
        std::vector<String_t> m_strings;
        size_t m_lastInstruction;
    };

} }

#endif
