/**
  *  \file ui/icons/icon.hpp
  *  \brief Base class ui::icons::Icon
  */
#ifndef C2NG_UI_ICONS_ICON_HPP
#define C2NG_UI_ICONS_ICON_HPP

#include "afl/base/deletable.hpp"
#include "gfx/context.hpp"
#include "gfx/point.hpp"
#include "gfx/rectangle.hpp"
#include "ui/draw.hpp"

namespace ui { namespace icons {

    /** A simple shape.
        Icon is the base class for a simple shape with no behaviour on its own.
        Widgets can use variable Icon instances to provide variable appearance.
        There need not be a 1:1 correspondence between widgets and Icons.

        General (non)rules:
        - Icons are passive and cannot trigger screen updates or re-layout on their own.
          Widgets containing icons must handle that by wrapping the Icons accordingly.
        - Icons do not know the widgets they are used in, and therefore do not deregister
          from the containing widgets like child widgets do.
          Lifetime management is up to the user.
        - Icons do not know where they are.
          This makes it possible to re-use one icon instance at multiple places. */
    class Icon : public afl::base::Deletable {
     public:
        /** Get size of this icon.
            \return size */
        virtual gfx::Point getSize() const = 0;

        /** Draw this icon.
            \param ctx   Context (prepared with skin colors, otherwise unspecified setup)
            \param area  Area to draw on.
                         This area should be the same size as returned by getSize(), but is allowed to be different.
            \param flags Flags */
        virtual void draw(gfx::Context<SkinColor::Color>& ctx, gfx::Rectangle area, ButtonFlags_t flags) const = 0;
    };

} }

#endif
