/**
  *  \file ui/layout/axislayout.hpp
  */
#ifndef C2NG_UI_LAYOUT_AXISLAYOUT_HPP
#define C2NG_UI_LAYOUT_AXISLAYOUT_HPP

#include <vector>
#include "afl/base/types.hpp"

namespace ui { namespace layout {

    /** Information for HBox/VBox layouts. */
    // FIXME: this sucks 1.0e+38.
    struct AxisLayout {
        std::vector<int> pref_sizes, min_sizes;
        std::vector<uint8_t> ignore_flags, flex_flags;
        int used_space;
        int used_outer;

        AxisLayout();
        std::vector<int>& doLayout(int space, int outer, int have_size);
    };

} }

#endif
