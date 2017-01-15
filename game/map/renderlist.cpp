/**
  *  \file game/map/renderlist.cpp
  */

#include <stdexcept>
#include "game/map/renderlist.hpp"
#include "util/translation.hpp"

namespace {
    int16_t pack(game::map::RenderList::Instruction insn, size_t numArgs)
    {
        return 256*insn + numArgs;
    }

    bool unpackInstruction(int value, game::map::RenderList::Instruction& ri)
    {
        if (value >= 0 && value <= game::map::RenderList::MAX_INSTRUCTION) {
            ri = static_cast<game::map::RenderList::Instruction>(value);
            return true;
        } else {
            return false;
        }
    }

    bool unpack(int value, game::map::RenderList::Instruction& insn, size_t& numArgs)
    {
        if (unpackInstruction(value >> 8, insn)) {
            numArgs = (value & 255);
            return true;
        } else {
            return false;
        }
    }
}

game::map::RenderList::Iterator::Iterator(const RenderList& parent)
    : m_parent(parent),
      m_nextInstruction(0),
      m_nextParameter(0),
      m_numParameters(0)
{ }

bool
game::map::RenderList::Iterator::readInstruction(Instruction& insn)
{
    if (m_nextInstruction >= m_parent.m_instructions.size()) {
        return false;
    } else {
        size_t numArgs;
        if (unpack(m_parent.m_instructions[m_nextInstruction++], insn, numArgs)) {
            m_nextParameter = m_nextInstruction;
            m_numParameters = numArgs;
            m_nextInstruction += numArgs;
            return true;
        } else {
            return false;
        }
    }
}

bool
game::map::RenderList::Iterator::readParameter(int& value)
{
    if (m_numParameters <= 0 || m_nextParameter >= m_parent.m_instructions.size()) {
        return false;
    } else {
        value = m_parent.m_instructions[m_nextParameter++];
        --m_numParameters;
        return true;
    }
}

bool
game::map::RenderList::Iterator::readStringParameter(String_t& value)
{
    int index;
    if (readParameter(index)) {
        if (index >= 0 && size_t(index) < m_parent.m_strings.size()) {
            value = m_parent.m_strings[index];
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
    int x, y;
    if (readParameter(x) && readParameter(y)) {
        value = Point(x, y);
        return true;
    } else {
        return false;
    }
}


game::map::RenderList::RenderList()
    : m_instructions(),
      m_strings(),
      m_lastInstruction(-1)
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
    m_lastInstruction = m_instructions.size();
    m_instructions.push_back(pack(ins, 0));
}

void
game::map::RenderList::addParameter(int16_t par)
{
    if (m_instructions.size() > m_lastInstruction) {
        m_instructions.push_back(par);
        m_instructions[m_lastInstruction]++;
    }
}

void
game::map::RenderList::addStringParameter(String_t s)
{
    if (m_strings.size() >= 0x7FFF) {
        // If this is ever hit, convert m_instructions to int32_t.
        throw std::runtime_error(_("Map too complex"));
    }

    int16_t n = static_cast<int16_t>(m_strings.size());
    m_strings.push_back(s);
    addParameter(n);
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
            int id, flags;
            if (it.readPointParameter(p) && it.readParameter(id) && it.readParameter(flags)) {
                listener.drawPlanet(p, id, flags);
            }
            break;
         }

         case riShip: {
            Point p;
            int id, rel;
            if (it.readPointParameter(p) && it.readParameter(id) && it.readParameter(rel)) {
                listener.drawShip(p, id, Relation_t(rel));
            }
            break;
         }

         case riFleetLeader: {
            Point p;
            int id, rel;
            if (it.readPointParameter(p) && it.readParameter(id) && it.readParameter(rel)) {
                listener.drawFleetLeader(p, id, Relation_t(rel));
            }
            break;
         }

         case riMinefield: {
            Point p;
            int id, r, isWeb, rel;
            if (it.readPointParameter(p) && it.readParameter(id) && it.readParameter(r) && it.readParameter(isWeb) && it.readParameter(rel)) {
                listener.drawMinefield(p, id, r, bool(isWeb), Relation_t(rel));
            }
            break;
         }
         case riUserCircle: {
            Point p;
            int r, color;
            if (it.readPointParameter(p) && it.readParameter(r) && it.readParameter(color)) {
                listener.drawUserCircle(p, r, color);
            }
            break;
         }
         case riUserLine: {
            Point a, b;
            int color;
            if (it.readPointParameter(a) && it.readPointParameter(b) && it.readParameter(color)) {
                listener.drawUserLine(a, b, color);
            }
            break;
         }
         case riUserRectangle: {
            Point a, b;
            int color;
            if (it.readPointParameter(a) && it.readPointParameter(b) && it.readParameter(color)) {
                listener.drawUserRectangle(a, b, color);
            }
            break;
         }
         case riUserMarker: {
            Point p;
            int shape, color;
            String_t text;
            if (it.readPointParameter(p) && it.readParameter(shape) && it.readParameter(color) && it.readStringParameter(text)) {
                listener.drawUserMarker(p, shape, color, text);
            }
            break;
         }
        }
    }
}

void
game::map::RenderList::clear()
{
    m_instructions.clear();
    m_strings.clear();
    m_lastInstruction = size_t(-1);
}

size_t
game::map::RenderList::getNumInstructions() const
{
    return m_instructions.size();
}

game::map::RenderList::Iterator
game::map::RenderList::read() const
{
    return Iterator(*this);
}
