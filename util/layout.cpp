/**
  *  \file util/layout.cpp
  *  \brief Layout Utilities
  */

#include <algorithm>
#include "util/layout.hpp"

namespace {
    struct ComparePositions {
        bool operator()(const util::Label& a, const util::Label& b) const
            { return a.pos < b.pos; }
    };

    int computeTotalSize(const util::Labels_t& labels, size_t start, size_t count)
    {
        int result = 0;
        for (size_t i = 0; i < count; ++i) {
            result += labels[start++].size;
        }
        return result;
    }
}

void
util::computeLabelPositions(Labels_t& labels, int minPos, int maxPos)
{
    // ex host/scores.cgi:drawRightLabels
    std::stable_sort(labels.begin(), labels.end(), ComparePositions());

    // Force all positions into range
    for (size_t i = 0, n = labels.size(); i < n; ++i) {
        labels[i].pos = std::max(minPos, std::min(maxPos - labels[i].size, labels[i].pos));
    }

    // Compute new locations
    size_t i = 0;
    while (i < labels.size()) {
        // Try to build a group.
        // If items [i, i+num) need more room than there is between i and i+num,
        // we need to include num in the group.
        // Start with the maximum-size group and reduce its size as long as possible.
        size_t num = labels.size() - i;
        const int firstY = labels[i].pos;
        while (num > 1 && firstY + computeTotalSize(labels, i, num-1) <= labels[i+num-1].pos) {
            --num;
        }

        // Total size of all these items
        const int neededSpace = computeTotalSize(labels, i, num);

        // Space currently in use
        const int usedSpace = labels[i+num-1].pos + labels[i+num-1].size - labels[i].pos;

        // New position of first item: move up by half of the excess size (but force into range)
        int y = firstY - (neededSpace - usedSpace) / 2;
        y = std::max(minPos, y);
        y = std::min(maxPos - neededSpace, y);

        // Assign all positions
        while (num > 0) {
            labels[i].pos = y;
            y += labels[i].size;
            ++i;
            --num;
        }
    }
}
