/**
  *  \file game/map/renderlist.cpp
  */

#include "game/map/renderlist.hpp"

game::map::RenderList::Iterator::Iterator(const RenderList& parent)
    : StringInstructionList::Iterator(parent)
{ }

bool
game::map::RenderList::Iterator::readInstruction(Instruction& insn)
{
    StringInstructionList::Instruction_t rawInsn;
    if (StringInstructionList::Iterator::readInstruction(rawInsn)) {
        if (/*rawInsn >= 0 &&*/ rawInsn <= MAX_INSTRUCTION) {
            insn = static_cast<Instruction>(rawInsn);
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool
game::map::RenderList::Iterator::readPointParameter(Point& value)
{
    int32_t x, y;
    if (readParameter(x) && readParameter(y)) {
        value = Point(x, y);
        return true;
    } else {
        return false;
    }
}


game::map::RenderList::RenderList()
{ }

game::map::RenderList::~RenderList()
{ }

void
game::map::RenderList::drawGridLine(Point a, Point b)
{
    addInstruction(riGridLine);
    addPointParameter(a);
    addPointParameter(b);
}

void
game::map::RenderList::drawBorderLine(Point a, Point b)
{
    addInstruction(riGridBorderLine);
    addPointParameter(a);
    addPointParameter(b);
}

void
game::map::RenderList::drawBorderCircle(Point c, int radius)
{
    addInstruction(riGridBorderCircle);
    addPointParameter(c);
    addParameter(radius);
}

void
game::map::RenderList::drawSelection(Point p)
{
    addInstruction(riSelection);
    addPointParameter(p);
}

void
game::map::RenderList::drawMessageMarker(Point p)
{
    addInstruction(riMessageMarker);
    addPointParameter(p);
}

void
game::map::RenderList::drawPlanet(Point p, int id, int flags, String_t label)
{
    addInstruction(riPlanet);
    addPointParameter(p);
    addParameter(id);
    addParameter(flags);
    addStringParameter(label);
}

void
game::map::RenderList::drawShip(Point p, int id, Relation_t rel, int flags, String_t label)
{
    addInstruction(riShip);
    addPointParameter(p);
    addParameter(id);
    addParameter(rel);
    addParameter(flags);
    addStringParameter(label);
}

void
game::map::RenderList::drawMinefield(Point p, int id, int r, bool isWeb, Relation_t rel, bool filled)
{
    addInstruction(riMinefield);
    addPointParameter(p);
    addParameter(id);
    addParameter(r);
    addParameter(isWeb);
    addParameter(rel);
    addParameter(filled);
}

void
game::map::RenderList::drawUfo(Point p, int id, int r, int colorCode, int speed, int heading, bool filled)
{
    addInstruction(riUfo);
    addPointParameter(p);
    addParameter(id);
    addParameter(r);
    addParameter(colorCode);
    addParameter(speed);
    addParameter(heading);
    addParameter(filled);
}

void
game::map::RenderList::drawUfoConnection(Point a, Point b, int colorCode)
{
    addInstruction(riUfoConnection);
    addPointParameter(a);
    addPointParameter(b);
    addParameter(colorCode);
}

void
game::map::RenderList::drawIonStorm(Point p, int r, int voltage, int speed, int heading, bool filled)
{
    addInstruction(riIonStorm);
    addPointParameter(p);
    addParameter(r);
    addParameter(voltage);
    addParameter(speed);
    addParameter(heading);
    addParameter(filled);
}

void
game::map::RenderList::drawUserCircle(Point pt, int r, int color)
{
    addInstruction(riUserCircle);
    addPointParameter(pt);
    addParameter(r);
    addParameter(color);
}

void
game::map::RenderList::drawUserLine(Point a, Point b, int color)
{
    addInstruction(riUserLine);
    addPointParameter(a);
    addPointParameter(b);
    addParameter(color);
}

void
game::map::RenderList::drawUserRectangle(Point a, Point b, int color)
{
    addInstruction(riUserRectangle);
    addPointParameter(a);
    addPointParameter(b);
    addParameter(color);
}

void
game::map::RenderList::drawUserMarker(Point pt, int shape, int color, String_t label)
{
    addInstruction(riUserMarker);
    addPointParameter(pt);
    addParameter(shape);
    addParameter(color);
    addStringParameter(label);
}

void
game::map::RenderList::drawExplosion(Point p)
{
    addInstruction(riExplosion);
    addPointParameter(p);
}

void
game::map::RenderList::drawShipTrail(Point a, Point b, Relation_t rel, int flags, int age)
{
    addInstruction(riShipTrail);
    addPointParameter(a);
    addPointParameter(b);
    addParameter(rel);
    addParameter(flags);
    addParameter(age);
}

void
game::map::RenderList::drawShipWaypoint(Point a, Point b, Relation_t rel)
{
    addInstruction(riShipWaypoint);
    addPointParameter(a);
    addPointParameter(b);
    addParameter(rel);
}

void
game::map::RenderList::drawShipVector(Point a, Point b, Relation_t rel)
{
    addInstruction(riShipVector);
    addPointParameter(a);
    addPointParameter(b);
    addParameter(rel);
}

void
game::map::RenderList::drawWarpWellEdge(Point a, Edge e)
{
    addInstruction(riWarpWellEdge);
    addPointParameter(a);
    addParameter(e);
}

void
game::map::RenderList::addInstruction(Instruction ins)
{
    StringInstructionList::addInstruction(static_cast<Instruction_t>(ins));
}

void
game::map::RenderList::addPointParameter(Point pt)
{
    addParameter(pt.getX());
    addParameter(pt.getY());
}

void
game::map::RenderList::replay(RendererListener& listener) const
{
    Iterator it(read());
    Instruction insn;
    while (it.readInstruction(insn)) {
        switch (insn) {
         case riGridBorderLine: {
            Point a, b;
            if (it.readPointParameter(a) && it.readPointParameter(b)) {
                listener.drawBorderLine(a, b);
            }
            break;
         }

         case riGridBorderCircle: {
            Point c;
            int r;
            if (it.readPointParameter(c) && it.readParameter(r)) {
                listener.drawBorderCircle(c, r);
            }
            break;
         }

         case riGridLine: {
            Point a, b;
            if (it.readPointParameter(a) && it.readPointParameter(b)) {
                listener.drawGridLine(a, b);
            }
            break;
         }

         case riSelection: {
            Point p;
            if (it.readPointParameter(p)) {
                listener.drawSelection(p);
            }
            break;
         }

         case riMessageMarker: {
            Point p;
            if (it.readPointParameter(p)) {
                listener.drawMessageMarker(p);
            }
            break;
         }

         case riPlanet: {
            Point p;
            int32_t id, flags;
            String_t label;
            if (it.readPointParameter(p) && it.readParameter(id) && it.readParameter(flags) && it.readStringParameter(label)) {
                listener.drawPlanet(p, id, flags, label);
            }
            break;
         }

         case riShip: {
            Point p;
            int32_t id, rel, flags;
            String_t label;
            if (it.readPointParameter(p) && it.readParameter(id) && it.readParameter(rel) && it.readParameter(flags) && it.readStringParameter(label)) {
                listener.drawShip(p, id, Relation_t(rel), flags, label);
            }
            break;
         }

         case riMinefield: {
            Point p;
            int32_t id, r, isWeb, rel, filled;
            if (it.readPointParameter(p) && it.readParameter(id) && it.readParameter(r) && it.readParameter(isWeb) && it.readParameter(rel) && it.readParameter(filled)) {
                listener.drawMinefield(p, id, r, bool(isWeb), Relation_t(rel), bool(filled));
            }
            break;
         }

         case riUfo: {
            Point p;
            int32_t id, r, colorCode, speed, heading, filled;
            if (it.readPointParameter(p) && it.readParameter(id) && it.readParameter(r)
                && it.readParameter(colorCode) && it.readParameter(speed) && it.readParameter(heading)
                && it.readParameter(filled))
            {
                listener.drawUfo(p, id, r, colorCode, speed, heading, bool(filled));
            }
            break;
         }

         case riUfoConnection: {
            Point a, b;
            int32_t colorCode;
            if (it.readPointParameter(a) && it.readPointParameter(b) && it.readParameter(colorCode)) {
                listener.drawUfoConnection(a, b, colorCode);
            }
            break;
         }

         case riIonStorm: {
            Point p;
            int32_t r, voltage, speed, heading, filled;
            if (it.readPointParameter(p) && it.readParameter(r) && it.readParameter(voltage) && it.readParameter(speed) && it.readParameter(heading) && it.readParameter(filled)) {
                listener.drawIonStorm(p, r, voltage, speed, heading, bool(filled));
            }
            break;
         }

         case riUserCircle: {
            Point p;
            int32_t r, color;
            if (it.readPointParameter(p) && it.readParameter(r) && it.readParameter(color)) {
                listener.drawUserCircle(p, r, color);
            }
            break;
         }
         case riUserLine: {
            Point a, b;
            int32_t color;
            if (it.readPointParameter(a) && it.readPointParameter(b) && it.readParameter(color)) {
                listener.drawUserLine(a, b, color);
            }
            break;
         }
         case riUserRectangle: {
            Point a, b;
            int32_t color;
            if (it.readPointParameter(a) && it.readPointParameter(b) && it.readParameter(color)) {
                listener.drawUserRectangle(a, b, color);
            }
            break;
         }
         case riUserMarker: {
            Point p;
            int32_t shape, color;
            String_t text;
            if (it.readPointParameter(p) && it.readParameter(shape) && it.readParameter(color) && it.readStringParameter(text)) {
                listener.drawUserMarker(p, shape, color, text);
            }
            break;
         }

         case riExplosion: {
            Point p;
            if (it.readPointParameter(p)) {
                listener.drawExplosion(p);
            }
            break;
         }

         case riShipTrail: {
            Point a, b;
            int32_t rel, flags, age;
            if (it.readPointParameter(a) && it.readPointParameter(b) && it.readParameter(rel) && it.readParameter(flags) && it.readParameter(age)) {
                listener.drawShipTrail(a, b, Relation_t(rel), flags, age);
            }
            break;
         }

         case riShipWaypoint: {
            Point a, b;
            int32_t rel;
            if (it.readPointParameter(a) && it.readPointParameter(b) && it.readParameter(rel)) {
                listener.drawShipWaypoint(a, b, Relation_t(rel));
            }
            break;
         }

         case riShipVector: {
            Point a, b;
            int32_t rel;
            if (it.readPointParameter(a) && it.readPointParameter(b) && it.readParameter(rel)) {
                listener.drawShipVector(a, b, Relation_t(rel));
            }
            break;
         }

         case riWarpWellEdge: {
            Point a;
            int32_t edge;
            if (it.readPointParameter(a) && it.readParameter(edge)) {
                listener.drawWarpWellEdge(a, Edge(edge));
            }
            break;
         }
        }
    }
}

game::map::RenderList::Iterator
game::map::RenderList::read() const
{
    return Iterator(*this);
}
