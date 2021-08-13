/**
  *  \file client/widgets/itemcostdisplay.hpp
  *  \brief Class client::widgets::ItemCostDisplay
  */
#ifndef C2NG_CLIENT_WIDGETS_ITEMCOSTDISPLAY_HPP
#define C2NG_CLIENT_WIDGETS_ITEMCOSTDISPLAY_HPP

#include "game/spec/cost.hpp"
#include "ui/widgets/simpletable.hpp"

namespace client { namespace widgets {

    /** Display cost of an item and total cost.
        Shows a multi-column table with
        - cost of a selected part
        - total cost of all selected parts (=entire ship)
        - available amounts
        - tech levels

        If part or total cost exceeds available amounts, that value is shown in red. */
    class ItemCostDisplay : public ui::widgets::SimpleTable {
     public:
        enum Mode {
            /// Highlight each column individually.
            IndividualMode,
            /// For item cost, check whether we can buy one more. ex include_item_in_cost.
            TotalMode
        };

        /** Constructor.
            \param root UI root
            \param tx   Translator */
        ItemCostDisplay(ui::Root& root, afl::string::Translator& tx);

        /** Set NumberFormatter.
            \param fmt NumberFormatter */
        void setNumberFormatter(util::NumberFormatter fmt);

        /** Set highlighting mode.
            \param mode Mode */
        void setHighlightingMode(Mode mode);

        /** Set available amount.
            \param cost Amount */
        void setAvailableAmount(game::spec::Cost cost);

        /** Set part cost.
            \param cost Cost */
        void setPartCost(game::spec::Cost cost);

        /** Set part tech level.
            \param have Available tech level
            \param need Required tech level */
        void setPartTechLevel(int have, int need);

        /** Set total cost.
            \param cost Cost */
        void setTotalCost(game::spec::Cost cost);

     private:
        util::NumberFormatter m_formatter;
        game::spec::Cost m_available;
        game::spec::Cost m_partCost;
        game::spec::Cost m_totalCost;
        Mode m_mode;
        int m_haveTech;
        int m_needTech;

        void buildTable(ui::Root& root, afl::string::Translator& tx);

        int32_t getAvailableAmount(bool flag, game::spec::Cost::Type type) const;

        void renderPartCost();
        void renderTotalCost();
        void renderAvailableAmount();
        void renderCost(const game::spec::Cost& cost, int column, bool flag);
        void renderCost(int column, int row, int32_t need, int32_t remain);
        void renderTechLevels();
    };

} }

#endif
