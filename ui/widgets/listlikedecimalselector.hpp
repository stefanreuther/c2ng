/**
  *  \file ui/widgets/listlikedecimalselector.hpp
  *  \brief Class ui::widgets::ListLikeDecimalSelector
  */
#ifndef C2NG_UI_WIDGETS_LISTLIKEDECIMALSELECTOR_HPP
#define C2NG_UI_WIDGETS_LISTLIKEDECIMALSELECTOR_HPP

#include "ui/widgets/basedecimalselector.hpp"
#include "afl/string/string.hpp"
#include "ui/root.hpp"

namespace ui { namespace widgets {

    /** "Move-or-Type" number selector, list-like type.
        Multiple widgets of this type below each other look similar to a regular list. */
    class ListLikeDecimalSelector : public BaseDecimalSelector {
     public:
        ListLikeDecimalSelector(Root& root, String_t label, afl::base::Observable<int32_t>& value, int32_t min, int32_t max, int32_t step);
        ~ListLikeDecimalSelector();

        // BaseDecimalSelector:
        virtual void draw(gfx::Canvas& can);
        virtual ui::layout::Info getLayoutInfo() const;

     private:
        Root& m_root;
        String_t m_label;

        String_t getValueAsString(int32_t value) const;
    };

} }

#endif
