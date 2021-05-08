/**
  *  \file ui/icons/spacer.hpp
  *  \brief Class ui::icons::Spacer
  */
#ifndef C2NG_UI_ICONS_SPACER_HPP
#define C2NG_UI_ICONS_SPACER_HPP

#include "ui/icons/icon.hpp"

namespace ui { namespace icons {

    /** Spacer.
        A spacer just takes up space (=reports a predefined getSize()), but has no appearance. */
    class Spacer : public Icon {
     public:
        /** Constructor.
            \param size Size */
        explicit Spacer(gfx::Point size);
        ~Spacer();

        // Icon:
        virtual gfx::Point getSize() const;
        virtual void draw(gfx::Context<SkinColor::Color>& ctx, gfx::Rectangle area, ButtonFlags_t flags) const;

     private:
        gfx::Point m_size;
    };

} }

#endif
