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
game::map::RenderList::drawSelection(Point p)
{
    addInstruction(riSelection);
    addPointParameter(p);
}

void
game::map::RenderList::drawPlanet(Point p, int id, int flags)
{
    addInstruction(riPlanet);
    addPointParameter(p);
    addParameter(id);
    addParameter(flags);
}

void
game::map::RenderList::drawShip(Point p, int id, Relation_t rel)
{
    addInstruction(riShip);
    addPointParameter(p);
    addParameter(id);
    addParameter(rel);
}

void
game::map::RenderList::drawFleetLeader(Point p, int id, Relation_t rel)
{
    addInstruction(riFleetLeader);
    addPointParameter(p);
    addParameter(id);
    addParameter(rel);
}

void
game::map::RenderList::drawMinefield(Point p, int id, int r, bool isWeb, Relation_t rel)
{
    addInstruction(riMinefield);
    addPointParameter(p);
    addParameter(id);
    addParameter(r);
    addParameter(isWeb);
    addParameter(rel);
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

         case riPlanet: {
            Point p;
            int32_t id, flags;
            if (it.readPointParameter(p) && it.readParameter(id) && it.readParameter(flags)) {
                listener.drawPlanet(p, id, flags);
            }
            break;
         }

         case riShip: {
            Point p;
            int32_t id, rel;
            if (it.readPointParameter(p) && it.readParameter(id) && it.readParameter(rel)) {
                listener.drawShip(p, id, Relation_t(rel));
            }
            break;
         }

         case riFleetLeader: {
            Point p;
            int32_t id, rel;
            if (it.readPointParameter(p) && it.readParameter(id) && it.readParameter(rel)) {
                listener.drawFleetLeader(p, id, Relation_t(rel));
            }
            break;
         }

         case riMinefield: {
            Point p;
            int32_t id, r, isWeb, rel;
            if (it.readPointParameter(p) && it.readParameter(id) && it.readParameter(r) && it.readParameter(isWeb) && it.readParameter(rel)) {
                listener.drawMinefield(p, id, r, bool(isWeb), Relation_t(rel));
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
        }
    }
}

game::map::RenderList::Iterator
game::map::RenderList::read() const
{
    return Iterator(*this);
}
