/**
  *  \file client/widgets/markerkindselector.cpp
  */

#include "client/widgets/markerkindselector.hpp"
#include "afl/base/staticassert.hpp"
#include "client/marker.hpp"
#include "game/map/drawing.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"

namespace {
    const int NUM_USER_MARKERS = game::map::Drawing::NUM_USER_MARKERS;
    static_assert(NUM_USER_MARKERS == client::NUM_USER_MARKERS, "NUM_USER_MARKERS");

    gfx::Point getCellSize(ui::Root& root)
    {
        // ex WColorSelector::getFrameSize (sort-of)
        int em = root.provider().getFont("+")->getLineHeight();
        return gfx::Point(em, em);
    }

    class MarkerIcon : public ui::icons::Icon {
     public:
        MarkerIcon(ui::Root& root, int k)
            : m_root(root),
              m_kind(k)
            { }
                   
        virtual gfx::Point getSize() const
            { return getCellSize(m_root); }

        virtual void draw(gfx::Context<util::SkinColor::Color>& ctx, gfx::Rectangle area, ui::ButtonFlags_t /*flags*/) const
            {
                ctx.setColor(util::SkinColor::Static);
                if (const client::Marker* m = client::getUserMarker(m_kind, true)) {
                    drawMarker(ctx, *m, area.getCenter());
                }
            }

     private:
        ui::Root& m_root;
        int m_kind;
    };

    
}


client::widgets::MarkerKindSelector::MarkerKindSelector(ui::Root& root)
    : IconGrid(root.engine(), getCellSize(root), NUM_USER_MARKERS, 1),
      m_root(root),
      m_deleter()
{
    for (int i = 0; i < NUM_USER_MARKERS; ++i) {
        addIcon(&m_deleter.addNew(new MarkerIcon(root, i)));
    }
}

client::widgets::MarkerKindSelector::~MarkerKindSelector()
{ }

void
client::widgets::MarkerKindSelector::setMarkerKind(int k)
{
    if (k >= 0 && k < NUM_USER_MARKERS) {
        setCurrentItem(static_cast<size_t>(k));
    }
}

int
client::widgets::MarkerKindSelector::getMarkerKind() const
{
    return static_cast<int>(getCurrentItem());
}

bool
client::widgets::MarkerKindSelector::doStandardDialog(String_t title, afl::string::Translator& tx)
{
    // ex WMarkerTypeSelector::doStandardDialog
    return ui::widgets::doStandardDialog(title, tx("Choose marker type:"), *this, false, m_root, tx);
}
