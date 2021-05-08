/**
  *  \file u/t_util_layout.cpp
  *  \brief Test for util::Layout
  */

#include "util/layout.hpp"

#include "t_util.hpp"

using util::Label;
using util::Labels_t;
using util::computeLabelPositions;

void
TestUtilLayout::testLabelPos()
{
    // Base case: everything fits at its position
    {
        Labels_t ls;
        ls.push_back(Label(1, 10, 5));
        ls.push_back(Label(2, 20, 7));
        computeLabelPositions(ls, 0, 30);
        TS_ASSERT_EQUALS(ls[0].id,  1);
        TS_ASSERT_EQUALS(ls[0].pos, 10);
        TS_ASSERT_EQUALS(ls[1].id,  2);
        TS_ASSERT_EQUALS(ls[1].pos, 20);
    }

    // Independant of original position
    {
        Labels_t ls;
        ls.push_back(Label(1, 20, 7));
        ls.push_back(Label(2, 10, 5));
        computeLabelPositions(ls, 0, 30);
        TS_ASSERT_EQUALS(ls[0].id,  2);
        TS_ASSERT_EQUALS(ls[0].pos, 10);
        TS_ASSERT_EQUALS(ls[1].id,  1);
        TS_ASSERT_EQUALS(ls[1].pos, 20);
    }

    // Everything cramped to beginning
    {
        Labels_t ls;
        ls.push_back(Label(1, 0, 5));
        ls.push_back(Label(2, 0, 7));
        ls.push_back(Label(3, 0, 3));
        computeLabelPositions(ls, 0, 30);
        TS_ASSERT_EQUALS(ls[0].id,  1);
        TS_ASSERT_EQUALS(ls[0].pos, 0);
        TS_ASSERT_EQUALS(ls[1].id,  2);
        TS_ASSERT_EQUALS(ls[1].pos, 5);
        TS_ASSERT_EQUALS(ls[2].id,  3);
        TS_ASSERT_EQUALS(ls[2].pos, 12);
    }

    // Everything cramped to end
    {
        Labels_t ls;
        ls.push_back(Label(1, 30, 5));
        ls.push_back(Label(2, 30, 7));
        ls.push_back(Label(3, 30, 3));
        computeLabelPositions(ls, 0, 30);
        TS_ASSERT_EQUALS(ls[0].id,  1);
        TS_ASSERT_EQUALS(ls[0].pos, 15);
        TS_ASSERT_EQUALS(ls[1].id,  2);
        TS_ASSERT_EQUALS(ls[1].pos, 20);
        TS_ASSERT_EQUALS(ls[2].id,  3);
        TS_ASSERT_EQUALS(ls[2].pos, 27);
    }

    // Everything cramped to the middle
    {
        Labels_t ls;
        ls.push_back(Label(1, 14, 2));
        ls.push_back(Label(2, 14, 2));
        ls.push_back(Label(3, 14, 2));
        computeLabelPositions(ls, 0, 30);
        TS_ASSERT_EQUALS(ls[0].id,  1);
        TS_ASSERT_EQUALS(ls[0].pos, 12);
        TS_ASSERT_EQUALS(ls[1].id,  2);
        TS_ASSERT_EQUALS(ls[1].pos, 14);
        TS_ASSERT_EQUALS(ls[2].id,  3);
        TS_ASSERT_EQUALS(ls[2].pos, 16);
    }

    // Two bunches
    {
        Labels_t ls;
        ls.push_back(Label(1, 10, 3));
        ls.push_back(Label(2, 10, 3));
        ls.push_back(Label(3, 20, 3));
        ls.push_back(Label(4, 20, 3));
        computeLabelPositions(ls, 0, 30);
        TS_ASSERT_EQUALS(ls[0].id,  1);
        TS_ASSERT_EQUALS(ls[0].pos, 9);
        TS_ASSERT_EQUALS(ls[1].id,  2);
        TS_ASSERT_EQUALS(ls[1].pos, 12);
        TS_ASSERT_EQUALS(ls[2].id,  3);
        TS_ASSERT_EQUALS(ls[2].pos, 19);
        TS_ASSERT_EQUALS(ls[3].id,  4);
        TS_ASSERT_EQUALS(ls[3].pos, 22);
    }

    // Outside
    {
        Labels_t ls;
        ls.push_back(Label(1, 40, 4));
        ls.push_back(Label(2, 50, 4));
        ls.push_back(Label(3, 60, 4));
        ls.push_back(Label(4, 70, 4));
        computeLabelPositions(ls, 0, 30);
        TS_ASSERT_EQUALS(ls[0].id,  1);
        TS_ASSERT_EQUALS(ls[0].pos, 14);
        TS_ASSERT_EQUALS(ls[1].id,  2);
        TS_ASSERT_EQUALS(ls[1].pos, 18);
        TS_ASSERT_EQUALS(ls[2].id,  3);
        TS_ASSERT_EQUALS(ls[2].pos, 22);
        TS_ASSERT_EQUALS(ls[3].id,  4);
        TS_ASSERT_EQUALS(ls[3].pos, 26);
    }
}

