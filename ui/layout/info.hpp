/**
  *  \file ui/layout/info.hpp
  */
#ifndef C2NG_UI_LAYOUT_INFO_HPP
#define C2NG_UI_LAYOUT_INFO_HPP

#include "gfx/point.hpp"

namespace ui { namespace layout {

    class Info {
     public:
        enum Growth {
            NoLayout,
            Fixed,
            GrowHorizontal,
            GrowVertical,
            GrowBoth
        };

        Info(gfx::Point minSize, gfx::Point prefSize, Growth growth);

        Info(gfx::Point fixedSize);

        Info();

        gfx::Point getMinSize() const;

        gfx::Point getPreferredSize() const;

        Growth getGrowthBehaviour() const;

        bool isGrowHorizontal() const;

        bool isGrowVertical() const;

        bool isIgnored() const;

        static Growth makeGrowthBehaviour(bool h, bool v, bool ignore);

     private:
        gfx::Point m_minSize;
        gfx::Point m_preferredSize;
        Growth m_growth;
    };

} }

#endif
