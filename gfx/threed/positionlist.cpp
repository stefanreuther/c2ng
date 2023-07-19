/**
  *  \file gfx/threed/positionlist.cpp
  *  \brief Class gfx::threed::PositionList
  */

#include "gfx/threed/positionlist.hpp"

using gfx::threed::Vec3f;

namespace {
    struct Node {
        bool isRange;
        Vec3f first;
        Vec3f last;

        Node(Vec3f a)
            : isRange(false), first(a), last(a)
            { }
        Node(Vec3f a, Vec3f b)
            : isRange(true), first(a), last(b)
            { }
    };

    void interpolateRange(std::vector<Vec3f>& result, const Vec3f& first, const Vec3f& last, size_t itemsPerRange)
    {
        switch (itemsPerRange) {
         case 0:
            break;

         case 1:
            result.push_back((first + last) * 0.5);
            break;

         default:
            result.push_back(first);
            for (size_t i = 1; i < itemsPerRange-1; ++i) {
                result.push_back(((first * float(itemsPerRange-1-i)) + (last * float(i))) * (1.0f / float(itemsPerRange-1)));
            }
            result.push_back(last);
            break;
        }
    }
}

gfx::threed::PositionList::PositionList()
    : m_items()
{ }

gfx::threed::PositionList::~PositionList()
{ }

void
gfx::threed::PositionList::add(uint16_t id, const Vec3f& pos)
{
    m_items.push_back(Item(id, pos));
}

size_t
gfx::threed::PositionList::getNumPositions() const
{
    return m_items.size();
}

uint16_t
gfx::threed::PositionList::getIdByIndex(size_t index) const
{
    return (index < m_items.size()
            ? m_items[index].id
            : 0);
}

gfx::threed::Vec3f
gfx::threed::PositionList::getPositionByIndex(size_t index) const
{
    return (index < m_items.size()
            ? m_items[index].pos
            : Vec3f(0,0,0));
}

afl::base::Optional<size_t>
gfx::threed::PositionList::findId(uint16_t id, size_t startAt) const
{
    for (size_t i = startAt; i < m_items.size(); ++i) {
        if (m_items[i].id == id) {
            return i;
        }
    }
    return afl::base::Nothing;
}

std::vector<gfx::threed::Vec3f>
gfx::threed::PositionList::findPoints(const Definition def, size_t numPoints) const
{
    // Locate input
    std::vector<Node> nodes;
    Vec3f first(0,0,0);
    bool hasFirst = false;
    size_t numRanges = 0;

    for (size_t i = 0; i < m_items.size(); ++i) {
        const Item& me = m_items[i];
        if (me.id == def.itemId) {
            nodes.push_back(Node(me.pos));
        } else if (m_items[i].id == def.firstId) {
            first = me.pos;
            hasFirst = true;
        } else if (m_items[i].id == def.lastId) {
            if (hasFirst) {
                nodes.push_back(Node(first, me.pos));
                ++numRanges;
                hasFirst = false;
            }
        } else {
            // skip
        }
    }

    // Do we have any ranges to expand?
    size_t itemsPerRange = 0;
    if (numRanges > 0) {
        size_t numFixed = nodes.size() - numRanges;
        if (numPoints > numFixed) {
            itemsPerRange = ((numPoints - numFixed) + (numRanges-1)) / numRanges;
        }
    }

    // Generate output
    std::vector<Vec3f> result;
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (nodes[i].isRange) {
            interpolateRange(result, nodes[i].first, nodes[i].last, itemsPerRange);
        } else {
            result.push_back(nodes[i].first);
        }
    }

    // Too many?
    if (result.size() > numPoints) {
        size_t tooMany = result.size() - numPoints;
        if (tooMany % 2 == 0 || numPoints == 1) {
            // Even excess: remove from sides
            // Same thing if we only want one result; the "odd excess" rule would leave an item on the side
            // For example, given a ship with 3 mountpoints but only one beam, this will preserve the middle one.
            result.erase(result.begin(), result.begin() + tooMany/2);
            result.erase(result.begin() + numPoints, result.end());
        } else {
            // Odd excess: remove from middle
            // For example, given a ship with 3 mountpoints but only two beam, this will preserve the outer ones.
            size_t pos = numPoints/2;
            result.erase(result.begin() + pos,
                         result.begin() + pos + tooMany);
        }
    }

    return result;
}
