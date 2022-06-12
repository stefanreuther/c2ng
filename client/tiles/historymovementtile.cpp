/**
  *  \file client/tiles/historymovementtile.cpp
  *  \brief Class client::tiles::HistoryMovementTile
  */

#include "client/tiles/historymovementtile.hpp"
#include "afl/base/countof.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/string/format.hpp"
#include "game/tables/headingname.hpp"
#include "ui/draw.hpp"
#include "util/translation.hpp"

using gfx::Point;
using gfx::Rectangle;
using util::SkinColor;

namespace {
    enum {
        LabelColumn,
        ValueColumn
    };
    enum {
        DistanceRow,
        HeadingRow,
        SpeedRow
    };
    const size_t NumColumns = 2;
    const size_t NumLines = 3;
}

client::tiles::HistoryMovementTile::HistoryMovementTile(ui::Root& root, afl::string::Translator& tx)
    : CollapsibleDataView(root),
      m_translator(tx),
      m_table(root, NumColumns, NumLines)
{
    init();
}

void
client::tiles::HistoryMovementTile::attach(HistoryAdaptor& adaptor)
{
    // Add listener
    class TurnChange : public afl::base::Closure<void()> {
     public:
        TurnChange(HistoryMovementTile& parent, HistoryAdaptor& adaptor)
            : m_parent(parent), m_adaptor(adaptor)
            { }
        virtual void call()
            { m_parent.onTurnChange(m_adaptor); }
     private:
        HistoryMovementTile& m_parent;
        HistoryAdaptor& m_adaptor;
    };
    conn_turnChange = adaptor.sig_turnChange.addNewClosure(new TurnChange(*this, adaptor));

    // Initial render
    onTurnChange(adaptor);
}

void
client::tiles::HistoryMovementTile::setChildPositions()
{
    Point anchor = getAnchorPoint(LeftAligned | DataAligned);
    Rectangle area = getExtent();
    m_table.setExtent(Rectangle(anchor.getX(), anchor.getY(),
                                area.getRightX() - anchor.getX(),
                                area.getBottomY() - anchor.getY()));
}

gfx::Point
client::tiles::HistoryMovementTile::getPreferredChildSize() const
{
    return root().provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(30, NumLines);
}

void
client::tiles::HistoryMovementTile::init()
{
    static const char*const LABELS[] = {
        N_("Distance:"),
        N_("Heading:"),
        N_("Warp Factor:"),
    };
    static_assert(countof(LABELS) == NumLines, "countof(LABELS)");
    for (size_t i = 0; i < NumLines; ++i) {
        m_table.cell(LabelColumn, i).setText(m_translator(LABELS[i]));
    }
    m_table.column(LabelColumn).setColor(ui::DARK_COLOR_SET[SkinColor::Static]);
    m_table.setColumnPadding(LabelColumn, 5);
    addChild(m_table, 0);
}

void
client::tiles::HistoryMovementTile::onTurnChange(HistoryAdaptor& adaptor)
{
    // ex WHistoryShipMovementTile::drawData (sort-of), CHistoryMovementTile.DrawData
    if (const game::map::ShipLocationInfo* p = adaptor.getCurrentTurnInformation()) {
        // Colors
        const uint8_t green = ui::DARK_COLOR_SET[SkinColor::Green];
        const uint8_t yellow = ui::DARK_COLOR_SET[SkinColor::Yellow];

        // Distance
        bool notMoved = false;
        double distance;
        if (p->distanceMoved.get(distance)) {
            if (distance <= 0.0001) {
                notMoved = true;
                m_table.cell(ValueColumn, DistanceRow)
                    .setText(m_translator("not moved"))
                    .setColor(green);
            } else {
                m_table.cell(ValueColumn, DistanceRow)
                    .setText(afl::string::Format(m_translator("%.2f ly"), distance))
                    .setColor(green);
            }
        } else {
            m_table.cell(ValueColumn, DistanceRow)
                .setText(m_translator("unknown"))
                .setColor(yellow);
        }

        // Heading
        int heading;
        if (p->heading.get(heading)) {
            m_table.cell(ValueColumn, HeadingRow)
                .setText(afl::string::Format(m_translator("%d\xC2\xB0 (%s)"),
                                             heading,
                                             game::tables::HeadingName().get(heading)))
                .setColor(green);
        } else if (notMoved) {
            m_table.cell(ValueColumn, HeadingRow)
                .setText(m_translator("not moved"))
                .setColor(green);
        } else {
            m_table.cell(ValueColumn, HeadingRow)
                .setText(m_translator("unknown"))
                .setColor(yellow);
        }

        // Speed
        int warpFactor;
        if (p->warpFactor.get(warpFactor)) {
            if (warpFactor == 0 && notMoved) {
                m_table.cell(ValueColumn, SpeedRow)
                    .setText(m_translator("not moved"))
                    .setColor(green);
            } else {
                m_table.cell(ValueColumn, SpeedRow)
                    .setText(afl::string::Format(m_translator("Warp %d"), warpFactor))
                    .setColor(green);
            }
        } else {
            m_table.cell(ValueColumn, SpeedRow)
                .setText(m_translator("unknown"))
                .setColor(yellow);
        }
    } else {
        m_table.column(ValueColumn).setText(String_t());
    }
}
