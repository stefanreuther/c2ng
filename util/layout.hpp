/**
  *  \file util/layout.hpp
  *  \brief Layout Utilities
  */
#ifndef C2NG_UTIL_LAYOUT_HPP
#define C2NG_UTIL_LAYOUT_HPP

#include <vector>

namespace util {

    struct Label {
        int id;
        int pos;
        int size;
        Label(int id, int pos, int size)
            : id(id), pos(pos), size(size)
            { }
    };

    typedef std::vector<Label> Labels_t;

    /** Compute positions of labels on an axis on a diagram.

        Given a list of label positions and sizes, moves them around to not overlap.
        For example, given two labels of size 10 to be placed at position 100, this will move one to the left, one to the right.

        For each labels, you specify
        - id (this function will re-order the label list)
        - pos (position)
        - size (size; item occupies [pos,pos+size))

        \param [in,out] labels   Labels to place
        \param [in]     minPos   Minimum position
        \param [in]     maxPos   Maximum position */
    void computeLabelPositions(Labels_t& labels, int minPos, int maxPos);

}

#endif
