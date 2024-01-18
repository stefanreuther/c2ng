/**
  *  \file test/util/layouttest.cpp
  *  \brief Test for util::Layout
  */

#include "util/layout.hpp"
#include "afl/test/testrunner.hpp"

using util::Label;
using util::Labels_t;
using util::computeLabelPositions;

// Base case: everything fits at its position
AFL_TEST("util.Layout:computeLabelPositions:normal", a)
{
    Labels_t ls;
    ls.push_back(Label(1, 10, 5));
    ls.push_back(Label(2, 20, 7));
    computeLabelPositions(ls, 0, 30);
    a.checkEqual("01", ls[0].id,  1);
    a.checkEqual("02", ls[0].pos, 10);
    a.checkEqual("03", ls[1].id,  2);
    a.checkEqual("04", ls[1].pos, 20);
}

// Independant of original position
AFL_TEST("util.Layout:computeLabelPositions:order", a)
{
    Labels_t ls;
    ls.push_back(Label(1, 20, 7));
    ls.push_back(Label(2, 10, 5));
    computeLabelPositions(ls, 0, 30);
    a.checkEqual("01", ls[0].id,  2);
    a.checkEqual("02", ls[0].pos, 10);
    a.checkEqual("03", ls[1].id,  1);
    a.checkEqual("04", ls[1].pos, 20);
}

// Everything cramped to beginning
AFL_TEST("util.Layout:computeLabelPositions:at-beginning", a)
{
    Labels_t ls;
    ls.push_back(Label(1, 0, 5));
    ls.push_back(Label(2, 0, 7));
    ls.push_back(Label(3, 0, 3));
    computeLabelPositions(ls, 0, 30);
    a.checkEqual("01", ls[0].id,  1);
    a.checkEqual("02", ls[0].pos, 0);
    a.checkEqual("03", ls[1].id,  2);
    a.checkEqual("04", ls[1].pos, 5);
    a.checkEqual("05", ls[2].id,  3);
    a.checkEqual("06", ls[2].pos, 12);
}

// Everything cramped to end
AFL_TEST("util.Layout:computeLabelPositions:at-end", a)
{
    Labels_t ls;
    ls.push_back(Label(1, 30, 5));
    ls.push_back(Label(2, 30, 7));
    ls.push_back(Label(3, 30, 3));
    computeLabelPositions(ls, 0, 30);
    a.checkEqual("01", ls[0].id,  1);
    a.checkEqual("02", ls[0].pos, 15);
    a.checkEqual("03", ls[1].id,  2);
    a.checkEqual("04", ls[1].pos, 20);
    a.checkEqual("05", ls[2].id,  3);
    a.checkEqual("06", ls[2].pos, 27);
}

// Everything cramped to the middle
AFL_TEST("util.Layout:computeLabelPositions:at-center", a)
{
    Labels_t ls;
    ls.push_back(Label(1, 14, 2));
    ls.push_back(Label(2, 14, 2));
    ls.push_back(Label(3, 14, 2));
    computeLabelPositions(ls, 0, 30);
    a.checkEqual("01", ls[0].id,  1);
    a.checkEqual("02", ls[0].pos, 12);
    a.checkEqual("03", ls[1].id,  2);
    a.checkEqual("04", ls[1].pos, 14);
    a.checkEqual("05", ls[2].id,  3);
    a.checkEqual("06", ls[2].pos, 16);
}

// Two bunches
AFL_TEST("util.Layout:computeLabelPositions:two-bunches", a)
{
    Labels_t ls;
    ls.push_back(Label(1, 10, 3));
    ls.push_back(Label(2, 10, 3));
    ls.push_back(Label(3, 20, 3));
    ls.push_back(Label(4, 20, 3));
    computeLabelPositions(ls, 0, 30);
    a.checkEqual("01", ls[0].id,  1);
    a.checkEqual("02", ls[0].pos, 9);
    a.checkEqual("03", ls[1].id,  2);
    a.checkEqual("04", ls[1].pos, 12);
    a.checkEqual("05", ls[2].id,  3);
    a.checkEqual("06", ls[2].pos, 19);
    a.checkEqual("07", ls[3].id,  4);
    a.checkEqual("08", ls[3].pos, 22);
}

// Outside
AFL_TEST("util.Layout:computeLabelPositions:outside", a)
{
    Labels_t ls;
    ls.push_back(Label(1, 40, 4));
    ls.push_back(Label(2, 50, 4));
    ls.push_back(Label(3, 60, 4));
    ls.push_back(Label(4, 70, 4));
    computeLabelPositions(ls, 0, 30);
    a.checkEqual("01", ls[0].id,  1);
    a.checkEqual("02", ls[0].pos, 14);
    a.checkEqual("03", ls[1].id,  2);
    a.checkEqual("04", ls[1].pos, 18);
    a.checkEqual("05", ls[2].id,  3);
    a.checkEqual("06", ls[2].pos, 22);
    a.checkEqual("07", ls[3].id,  4);
    a.checkEqual("08", ls[3].pos, 26);
}
