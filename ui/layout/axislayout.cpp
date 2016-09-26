/**
  *  \file ui/layout/axislayout.cpp
  */

#include <cassert>
#include "ui/layout/axislayout.hpp"

ui::layout::AxisLayout::AxisLayout()
    : used_space(),
      used_outer()
{ }

/** Compute generic layout.
    \pre pref_sizes, min_sizes, flags filled in
    \param space      inter-widget space we want to use
    \param outer      outer space we want to leave
    \param have_size  available space
    \param flex_flag  the flag signifying flexibility in our direction
                      (li_FlexibleH or li_FlexibleV)
    \returns vector of widget sizes
    \post used_outer = actual outer space to leave
    \post used_space = actual inter-widget space to leave */
std::vector<int>&
ui::layout::AxisLayout::doLayout(int space, int outer, int have_size)
{
    // ex ui/layout.cc:LayoutInfo::doLayout
    typedef std::vector<int>::size_type index;
    assert(pref_sizes.size() == min_sizes.size());
    assert(pref_sizes.size() == ignore_flags.size());
    assert(pref_sizes.size() == flex_flags.size());

    /* First, figure out widget counts and sizes */
    int num_layout = 0, num_flex = 0;
    int total_pref = 0, total_min = 0;
    for (index i = 0; i < ignore_flags.size(); ++i) {
        if (ignore_flags[i]) {
            ;
        } else {
            ++num_layout;
            total_pref += pref_sizes[i];
            total_min  += min_sizes[i];
            if (flex_flags[i]) {
                ++num_flex;
            }
        }
    }

    /* Pathological case */
    if (!num_layout)
        return pref_sizes;

    /* Space that would be needed to layout this */
    used_space = space;
    if (num_layout > 1) {
        // too little space, reduce inter-widget gap
        while (used_space > 0 && (have_size < (num_layout-1) * used_space + total_min))
            --used_space;
        have_size -= (num_layout-1) * used_space;
    }

    used_outer = outer;
    while (used_outer > 0 && (have_size < 2*used_outer + total_min))
        // too little space, reduce outer space
        --used_outer;
    have_size -= 2*used_outer;

    if (have_size < total_pref) {
        /* Total size too small. Scale down relatively using minimum sizes. */
        /* FIXME: we can probably do better for have_size \in [min_total, pref_total] */
        int total = have_size;
        for (index i = 0; i < min_sizes.size(); ++i) {
            if (ignore_flags[i]) {
                min_sizes[i] = 0;
            } else {
                int new_size = total_min ? min_sizes[i] * total / total_min : 0;
                total     -= new_size;
                total_min -= min_sizes[i];
                min_sizes[i] = new_size;
            }
        }
        assert(total_min == 0);
        assert(total == 0);
        return min_sizes;
    } else if (have_size > total_pref) {
        /* Total size larger than preferred size. Scale up using
           preferred sizes. */
        int delta = have_size - total_pref;  // total amount to enlarge
        if (num_flex) {
            /* we have flexible components; enlarge only those */
            for (index i = 0; i < pref_sizes.size(); ++i) {
                if (!(ignore_flags[i]) && (flex_flags[i])) {
                    int d = delta / num_flex--;
                    pref_sizes[i] += d;
                    delta -= d;
                }
            }
            assert(num_flex == 0);
            assert(delta == 0);
        } else {
            /* no flexible: enlarge all components */
            for (index i = 0; i < pref_sizes.size(); ++i) {
                if (!(ignore_flags[i])) {
                    int d = delta / num_layout--;
                    pref_sizes[i] += d;
                    delta -= d;
                }
            }
            assert(delta == 0);
            assert(num_layout == 0);
        }
        return pref_sizes;
    } else
        return pref_sizes;
}
